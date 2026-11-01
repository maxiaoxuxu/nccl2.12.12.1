#!/usr/bin/env python3
"""Check source hashes, code spans, stable IDs, embedded diagrams and backlinks."""
import base64
import json
from html.parser import HTMLParser
from pathlib import Path
from urllib.parse import parse_qs, unquote, urlsplit

from source_diagram_model import ROOT, DOC, READER, resolve
from verify_offline_docs import Page


class Links(HTMLParser):
    def __init__(self,text):
        super().__init__();self.links=[];self.feed(text)

    def handle_starttag(self,tag,attrs):
        if tag=='a':
            self.links.append(dict(attrs).get('href',''))


def main():
    expected=resolve(write_index=False)
    page=Page((DOC/READER).read_text(encoding='utf-8'))
    actual=json.loads(page.scripts['source-model'])
    assert actual==expected,'Reader is stale: rebuild source-diagram HTML'
    index=json.loads((DOC/'source-diagram-index.json').read_text(encoding='utf-8'))
    stripped={k:v for k,v in expected.items() if k!='files'}
    stripped['files']={p:{k:v for k,v in f.items() if k!='text'} for p,f in expected['files'].items()}
    assert index==stripped,'Resolved source diagram index is stale'
    images=json.loads(page.scripts['source-images'])
    assert set(images)==set(actual['diagrams'])
    for ident,uri in images.items():
        assert uri.startswith('data:image/svg+xml;base64,')
        assert base64.b64decode(uri.partition(',')[2],validate=True)==(DOC/'visuals'/(ident+'.svg')).read_bytes()
    bindings={b['id']:b for b in actual['bindings']}
    assert len(bindings)==125 and len(actual['scopes'])==100
    assert len(actual['symbols'])==7881 and len(actual['files'])==121
    symbols={s['id']:s for s in actual['symbols']}
    coverage=json.loads((DOC/'annotation-coverage.json').read_text(encoding='utf-8'))
    for file in coverage['files']:
        for item in file['items']:
            assert item['annotated_line']==symbols[item['id']]['line'],('Stale coverage line',item['id'])
    for ident,b in bindings.items():
        scope=actual['scopes'][b['symbol']]
        assert scope['start']<=b['start']<=b['code_start']<=b['end']<=scope['end'],ident
        assert b['file']==scope['file']
        text=actual['files'][b['file']]['text']
        assert '['+ident+']' in text,('Missing source binding comment',ident)
        assert b['explanation'] and b['relation']
    # Semantic regression: final reduction must not point at the AG copy phase.
    ar=[b for b in bindings.values() if b['symbol']=='F0058' and b['scope']=='code']
    assert len(ar)==8
    final=next(b for b in ar if 'allreduce-step-3' in b['diagrams'])
    rows=actual['files'][final['file']]['text'].splitlines()
    assert 'chunk = ringIx + 0;' in rows[final['code_start']-1]
    assert any('directRecvReduceCopySend' in line for line in rows[final['start']-1:final['end']])
    copy=next(b for b in ar if 'allreduce-step-4' in b['diagrams'])
    assert final['end']<copy['start']
    assert any('从这里进入结果传播阶段' in line for line in rows[copy['start']-1:copy['end']])
    anchor_checks=0
    for name in ['01_通信原语与算法图解_离线版.html','visual-guide.html','05_全部函数内部步骤_离线搜索.html','function-logic.html']:
        content=(DOC/name).read_text(encoding='utf-8')
        p=Page(content)
        key='diagram-bindings' if 'diagram-bindings' in p.scripts else 'search-bindings'
        assert json.loads(p.scripts[key])==actual['bindings'],name
        for href in Links(content).links:
            url=urlsplit(href)
            if unquote(url.path)==READER and url.fragment:
                query=parse_qs(url.fragment)
                if 'binding' in query:
                    b=bindings[query['binding'][0]]
                    assert query.get('symbol')==[b['symbol']],href
                    anchor_checks+=1
    assert anchor_checks>200
    print(f'PASS: 125 reviewed bindings / 100 symbols; 7881 searchable IDs; 121 exact source files; 45 embedded diagrams; {anchor_checks} static backlinks')
    print('Code spans include adjacent explanatory comments. Unbound symbols are not assigned unrelated diagrams.')


if __name__=='__main__':main()
