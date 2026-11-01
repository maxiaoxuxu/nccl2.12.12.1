#!/usr/bin/env python3
"""Build the two self-contained Chinese reading pages (stdlib only).

Input: named Markdown chapters, editable SVGs, and function-logic.json.
Output: named HTML pages and byte-identical legacy HTML aliases.
The small renderer supports the Markdown constructs used by these chapters;
it is deliberately not a general-purpose Markdown implementation.
"""
import base64
import html
import json
import re
from pathlib import Path
from urllib.parse import quote, urlsplit
import xml.etree.ElementTree as ET
from source_diagram_model import READER

ROOT = Path(__file__).resolve().parents[1]
DOC = ROOT / 'docs/zh_CN'
BRANCH = 'codex/nccl-2.12.12-full-source-walkthrough'
REPO = 'https://github.com/maxiaoxuxu/nccl2.12.12.1'
GUIDE = '01_通信原语与算法图解_离线版.html'
SEARCH = '05_全部函数内部步骤_离线搜索.html'
CSS = '''
*{box-sizing:border-box}body{margin:0;background:#f4f7fb;color:#172d43;font:17px/1.8 "Microsoft YaHei","Noto Sans SC",sans-serif}
main{max-width:1160px;margin:auto;padding:28px}h1,h2,h3{line-height:1.45;color:#123b60;overflow-wrap:anywhere}
h2{margin-top:44px;border-bottom:2px solid #c8d8e7;padding-bottom:10px}a{color:#1256a1;overflow-wrap:anywhere}
img{display:block;max-width:100%;height:auto;background:white;margin:22px auto;border:1px solid #d6e1eb;border-radius:8px}
.table-wrap{overflow-x:auto;max-width:100%;margin:20px 0}table{border-collapse:collapse;width:100%;background:white}
td,th{padding:10px 14px;border:1px solid #d7e1ea;text-align:left;vertical-align:top;min-width:95px}
code{font-family:Consolas,monospace;font-size:.88em;overflow-wrap:anywhere}pre{background:#e8eef5;padding:16px;overflow:auto;white-space:pre-wrap}
nav,.panel{background:white;border:1px solid #c8d8e7;border-radius:12px;padding:20px;margin:20px 0}
button,select,input{font:inherit;padding:8px 12px;max-width:100%;margin:5px 0;border:1px solid #9db4c9;border-radius:6px;background:white;color:#172d43}
button{cursor:pointer;background:#e7f0fc}button:disabled{opacity:.4;cursor:default}.controls{display:flex;flex-wrap:wrap;gap:8px;align-items:center}
label{display:block}.tag,figcaption{color:#4b647c;font-size:14px}.step{border-left:3px solid #a0bddc;padding:8px 14px;margin:9px 0;background:#f4f7fb}
.step code{display:block;white-space:pre-wrap}summary{cursor:pointer;padding:12px;font-weight:bold;overflow-wrap:anywhere}
.fn{border:1px solid #cad9e7;background:white;border-radius:9px;margin:12px 0}.fn-body{padding:0 16px 16px;min-width:0}
#results{min-height:200px}figure{margin:20px 0}blockquote{border-left:4px solid #a0bddc;padding-left:16px;margin-left:0}
@media(max-width:650px){main{padding:14px}body{font-size:16px}.panel,nav{padding:12px}h1{font-size:26px}td,th{padding:8px}}
@media print{body{background:white}.controls,nav{display:none}main{max-width:none;padding:0}img{break-inside:avoid}}
'''


def json_script(ident, value):
    data = json.dumps(value, ensure_ascii=False, separators=(',', ':')).replace('<', '\\u003c')
    return f'<script type="application/json" id="{ident}">{data}</script>'


def page(title, body, scripts=''):
    return ('<!doctype html>\n<html lang="zh-CN"><head><meta charset="utf-8">'
            '<meta name="viewport" content="width=device-width,initial-scale=1">'
            f'<title>{html.escape(title)}</title><style>{CSS}</style></head>'
            f'<body><main id="top">{body}</main>{scripts}</body></html>\n')


def online_link(href):
    if urlsplit(href).scheme or href.startswith('#'):
        return href
    raw, _, fragment = href.partition('#')
    target = (DOC / raw).resolve()
    if target.name in (GUIDE, 'visual-guide.html'):
        return '#top'
    relative = target.relative_to(ROOT).as_posix()
    kind = 'tree' if target.is_dir() else 'blob'
    result = f'{REPO}/{kind}/{BRANCH}/' + quote(relative, safe='/')
    return result + ('#' + fragment if fragment else '')


def inline(text, images):
    saved = []

    def token(value):
        saved.append(value)
        return f'\x00{len(saved)-1}\x00'

    text = re.sub(r'`([^`]+)`', lambda m: token('<code>'+html.escape(m[1])+'</code>'), text)

    def image(m):
        alt, path = m[1], m[2]
        key = Path(path).stem
        assert key in images, path
        return token(f'<img alt="{html.escape(alt, quote=True)}" src="{images[key]}">')

    text = re.sub(r'!\[([^\]]*)\]\(([^)]+)\)', image, text)
    text = re.sub(r'\[([^\]]+)\]\(([^)]+)\)',
                  lambda m: token('<a href="'+html.escape(online_link(m[2]), quote=True)+'">'+html.escape(m[1])+'</a>'), text)
    text = html.escape(text)
    text = re.sub(r'\*\*(.+?)\*\*', r'<strong>\1</strong>', text)
    return re.sub(r'\x00(\d+)\x00', lambda m: saved[int(m[1])], text)


def markdown(text, images, prefix):
    lines = text.splitlines()
    out, headings = [], []
    i = 0
    while i < len(lines):
        line = lines[i]
        if not line.strip():
            i += 1
            continue
        if line.startswith('```'):
            lang = line[3:].strip()
            assert lang != 'mermaid', 'Render precise diagrams to SVG before including them in a standalone page'
            code = []
            i += 1
            while i < len(lines) and not lines[i].startswith('```'):
                code.append(lines[i])
                i += 1
            out.append('<pre><code>'+html.escape('\n'.join(code))+'</code></pre>')
            i += 1
            continue
        heading = re.match(r'^(#{1,6}) (.+)$', line)
        if heading:
            level = len(heading[1])
            ident = f'{prefix}-{len(headings)}'
            headings.append((level, ident, heading[2]))
            out.append(f'<h{level} id="{ident}">'+inline(heading[2], images)+f'</h{level}>')
            i += 1
            continue
        if line.startswith('|') and i+1 < len(lines) and re.fullmatch(r'[| :\-]+', lines[i+1]):
            def cells(row, tag):
                return '<tr>'+''.join(f'<{tag}>'+inline(c.strip(), images)+f'</{tag}>'
                                     for c in row.strip().strip('|').split('|'))+'</tr>'
            table = ['<div class="table-wrap"><table><thead>', cells(line, 'th'), '</thead><tbody>']
            i += 2
            while i < len(lines) and lines[i].startswith('|'):
                table.append(cells(lines[i], 'td'))
                i += 1
            out.append(''.join(table)+'</tbody></table></div>')
            continue
        item = re.match(r'^(?:\d+\. |[-*] )(.+)$', line)
        if item:
            ordered = bool(re.match(r'^\d+\.', line))
            tag = 'ol' if ordered else 'ul'
            items = []
            while i < len(lines):
                match = re.match(r'^(?:\d+\. |[-*] )(.+)$', lines[i])
                if not match:
                    break
                items.append('<li>'+inline(match[1], images)+'</li>')
                i += 1
            out.append(f'<{tag}>'+''.join(items)+f'</{tag}>')
            continue
        if line.startswith('> '):
            out.append('<blockquote>'+inline(line[2:], images)+'</blockquote>')
            i += 1
            continue
        paragraph = [line]
        i += 1
        while i < len(lines) and lines[i].strip() and not re.match(r'^(#|```|\||> |\d+\. |[-*] )', lines[i]):
            paragraph.append(lines[i])
            i += 1
        out.append('<p>'+inline(' '.join(paragraph), images)+'</p>')
    return '\n'.join(out), headings


GUIDE_JS = '''
const frames=JSON.parse(document.getElementById('frames').textContent);
const images=JSON.parse(document.getElementById('embedded-images').textContent);
const bindings=JSON.parse(document.getElementById('diagram-bindings').textContent);
function sourceLink(id,element,preferredSymbol){
 const candidates=bindings.filter(b=>b.diagrams.includes(id));
 candidates.sort((a,b)=>(a.symbol===preferredSymbol?0:1)-(b.symbol===preferredSymbol?0:1)||(a.scope==='code'?0:1)-(b.scope==='code'?0:1));
 const b=candidates[0];if(b){element.href='11_源码注释与图示对照_离线版.html#'+new URLSearchParams({symbol:b.symbol,binding:b.id});element.textContent=b.id+' · 对应源码与注释（完整ZIP）'}
}
const byId=id=>document.getElementById(id);let round=0;
function render(){
 const k=byId('collective').value,fs=frames[k],f=fs[round];
 byId('frame').src=images[k+'-step-'+round];byId('frame').alt=k+' '+f.title;
 byId('counter').textContent=(round+1)+' / '+fs.length;
 byId('desc').textContent=f.title+'。'+f.description;
 sourceLink(k+'-step-'+round,byId('frameSource'),{allreduce:'F0058',allgather:'F0054',reducescatter:'F0370'}[k]);
 byId('prev').disabled=round===0;byId('next').disabled=round===fs.length-1;
 const table=document.createElement('table');
 for(let r=0;r<4;r++){const tr=document.createElement('tr');
  const chunks=Object.entries(f.outputs[r]).sort(([a],[b])=>Number(a)-Number(b));
  for(const val of ['rank '+r,chunks.map(([c,s])=>'C'+c+' ← '+s.map(x=>'X'+x).join('+')).join('；')||'尚无最终块']){
   const td=document.createElement('td');td.textContent=val;tr.append(td)}table.append(tr)}
 byId('ownership').replaceChildren(table);
}
byId('collective').onchange=()=>{round=0;render()};
byId('prev').onclick=()=>{if(round>0)round--;render()};
byId('next').onclick=()=>{if(round<frames[byId('collective').value].length-1)round++;render()};
const ps=['send','recv','copySend','recvCopySend','recvReduceSend','recvReduceCopy','recvReduceCopySend'];
ps.forEach((p,i)=>{const o=document.createElement('option');o.value=i;o.textContent=p;byId('primitive').append(o)});
function primitive(){const i=Number(byId('primitive').value),id=String(i+5).padStart(2,'0')+'-'+ps[i];byId('primitiveImage').src=images[id];byId('primitiveImage').alt=ps[i]+'原语数据流';sourceLink(id,byId('primitiveSource'),['F0240','F0244','F0246','F0248','F0252','F0251','F0254'][i])}
byId('primitive').onchange=primitive;primitive();render();
function revealDiagram(){const id=decodeURIComponent(location.hash.slice(1));if(id.startsWith('diagram-')){byId('all-diagrams').open=true;requestAnimationFrame(()=>byId(id)?.scrollIntoView({block:'start'}))}}
addEventListener('hashchange',revealDiagram);revealDiagram();
'''


SEARCH_JS = '''
const D=JSON.parse(document.getElementById('logicData').textContent).functions;
const bindings=JSON.parse(document.getElementById('search-bindings').textContent);
const images=JSON.parse(document.getElementById('search-images').textContent);
let filtered=D,limit=40;const el=id=>document.getElementById(id);
function node(tag,txt,cls){const n=document.createElement(tag);if(txt!==undefined)n.textContent=txt;if(cls)n.className=cls;return n}
function render(){el('results').replaceChildren();el('count').textContent=' 匹配 '+filtered.length+' 个';
 for(const f of filtered.slice(0,limit)){const d=node('details',undefined,'fn');d.id=f.id.toLowerCase();
  d.append(node('summary',f.id+' · '+(f.owner?f.owner+'::':'')+f.name+' — '+f.file));
  d.addEventListener('toggle',()=>{if(!d.open||d.dataset.loaded)return;d.dataset.loaded='1';
   const box=node('div',undefined,'fn-body');box.append(node('p',f.role));
   for(const n of f.human_notes)box.append(node('p','精读：'+n));
   const related=bindings.filter(b=>b.symbol===f.id);
   if(related.length){const panel=node('section',undefined,'panel');panel.append(node('b','这个函数对应的图与分析'));
    const choose=node('select');choose.setAttribute('aria-label','选择函数的图示代码段');
    for(const b of related){const o=node('option',b.id+' · '+b.title);o.value=b.id;choose.append(o)}
    const note=node('p'),imageChoose=node('select'),img=node('img'),link=node('a','打开带注释的源码对照（完整ZIP）');
    imageChoose.setAttribute('aria-label','选择代码段中的图示');
    function showImage(){img.src=images[imageChoose.value];img.alt=imageChoose.value+' · 函数对应图示'}
    function showBinding(){const b=related.find(b=>b.id===choose.value);note.textContent=b.relation+'：'+b.explanation;imageChoose.replaceChildren();for(const id of b.diagrams){const o=node('option',id);o.value=id;imageChoose.append(o)}link.href='11_源码注释与图示对照_离线版.html#'+new URLSearchParams({symbol:b.symbol,binding:b.id});showImage()}
    choose.onchange=showBinding;imageChoose.onchange=showImage;panel.append(choose,note,imageChoose,img,link);showBinding();box.append(panel);
   }
   const source=node('details',undefined,'source');source.append(node('summary','完整函数体（已内嵌，离线可读）'));
   const pre=node('pre');pre.append(node('code',f.body));source.append(pre);box.append(source);
   f.steps.forEach((s,i)=>{const row=node('div',undefined,'step');row.style.marginLeft=Math.min(s.depth,4)*8+'px';
    row.append(node('b',(i+1)+'. '+s.kind));if(s.code)row.append(node('code',s.code));row.append(node('div',s.note));box.append(row)});d.append(box)});
  el('results').append(d)}el('more').hidden=limit>=filtered.length;
}
el('q').oninput=()=>{const q=el('q').value.trim().toLowerCase();filtered=D.filter(f=>(f.id+' '+f.name+' '+f.owner+' '+f.file+' '+f.role).toLowerCase().includes(q));limit=40;render()};
el('more').onclick=()=>{limit+=40;render()};
function hash(){const id=location.hash.slice(1).toLowerCase(),f=D.find(f=>f.id.toLowerCase()===id);if(f){el('q').value=f.id;el('q').oninput();el(id).open=true}}
render();hash();addEventListener('hashchange',hash);
'''


def build_guide():
    binding_path=DOC/'source-diagram-index.json'
    bindings=json.loads(binding_path.read_text(encoding='utf-8'))['bindings']
    images, gallery = {}, []
    for p in sorted((DOC/'visuals').glob('*.svg')):
        images[p.stem] = 'data:image/svg+xml;base64,'+base64.b64encode(p.read_bytes()).decode('ascii')
        title = ET.parse(p).getroot().find('{http://www.w3.org/2000/svg}title')
        label = title.text if title is not None else p.stem
        related=[b for b in bindings if p.stem in b['diagrams']]
        links='<details><summary>对应源码与注释：'+str(len(related))+'处（完整ZIP）</summary><ul>'
        for b in related:
            href=READER+'#symbol='+b['symbol']+'&binding='+b['id']
            links+='<li><a href="'+html.escape(href,quote=True)+'">'+html.escape(b['id']+' · '+b['symbol']+' · '+b['title'])+'</a></li>'
        links+='</ul></details>'
        gallery.append('<figure id="diagram-'+p.stem+'"><figcaption>'+html.escape(p.name+' · '+label)+'</figcaption>'
                       f'<img alt="{html.escape(label, quote=True)}" src="{images[p.stem]}">'+links+'</figure>')
    body, headings = markdown((DOC/'01_通信原语与算法图解.md').read_text(encoding='utf-8'), images, 'guide')
    audit, _ = markdown((DOC/'09_本轮修正记录与依据.md').read_text(encoding='utf-8'), images, 'audit')
    toc = '<ul>'+''.join(f'<li><a href="#{ident}">{html.escape(title)}</a></li>'
                        for level, ident, title in headings if level == 2)+'</ul>'
    intro = '''<nav><strong>NCCL 2.12.12 · V3.3 · 通信原语与算法图解</strong>
<p>本HTML内含全部45张图、逐轮模型和交互代码。单独下载后可断网打开；没有外置图片、脚本或字体。“对应源码”链接需完整ZIP中的对照页，其他章节在线链接需要网络。</p>
<a href="#contents">章节目录</a> · <a href="#all-diagrams">全部图解</a> · <a href="#corrections">本轮修正</a> · '''
    intro += '<a href="'+online_link(SEARCH)+'">函数搜索页下载（在线）</a></nav>'
    intro += '''<section class="panel"><h2 style="margin-top:0">逐轮看：数据中包含谁的贡献？</h2>
<p>环顺序0→1→2→3→0；归约图以Sum举例。箭头表示数据依赖，不表示所有rank锁步执行。</p>
<div class="controls"><label for="collective">算法</label><select id="collective"><option value="allreduce">Ring AllReduce</option><option value="allgather">Ring AllGather</option><option value="reducescatter">Ring ReduceScatter</option></select>
<button id="prev">上一轮</button><button id="next">下一轮</button><span id="counter" aria-live="polite"></span></div><p id="desc"></p>'''
    intro += f'<img id="frame" alt="AllReduce初始块" src="{images["allreduce-step-0"]}"><a id="frameSource" href="{READER}">对应源码与注释（完整ZIP）</a><div id="ownership" class="table-wrap"></div></section>'
    intro += '<section class="panel"><h2 style="margin-top:0">七种基础原语：读哪里，写哪里？</h2><label for="primitive">Ring单收单发示例</label><select id="primitive"></select>'
    intro += f'<img id="primitiveImage" alt="send原语" src="{images["05-send"]}"><a id="primitiveSource" href="{READER}">对应源码与注释（完整ZIP）</a><p>direct能力和多源归约需结合模板及实际连接判断，详见正文。</p></section>'
    intro += '<noscript><p>浏览器未启用JavaScript，切换控件不可用；正文与“全部42张图”仍可阅读。</p></noscript>'
    intro += '<details class="panel" id="contents"><summary>章节目录</summary>'+toc+'</details>'
    result = intro+body+'<details id="all-diagrams" class="panel"><summary>全部45张图（包括每一轮与七类原语、Channel和QP/MR）</summary>'+''.join(gallery)+'</details>'
    result += '<section id="corrections">'+audit+'</section>'
    frames = json.loads((DOC/'visuals/collective-steps.json').read_text(encoding='utf-8'))
    scripts = json_script('frames', frames)+json_script('embedded-images', images)+json_script('diagram-bindings',bindings)+'<script>'+GUIDE_JS+'</script>'
    return page('NCCL 2.12.12 通信原语与算法图解 V3.3（离线单文件）', result, scripts)


def build_search():
    logic = json.loads((DOC/'function-logic.json').read_text(encoding='utf-8'))
    bodies = {}
    for p in (DOC/'logic').glob('*.md'):
        for match in re.finditer(r'<a id="(f\d+)"></a>([\s\S]*?)(?=<a id="f\d+"></a>|\Z)', p.read_text(encoding='utf-8')):
            body = re.search(r'```cpp\n([\s\S]*?)\n```', match[2])
            if body:
                bodies[match[1].upper()] = body[1]
    for f in logic['functions']:
        assert f['id'] in bodies, ('Missing complete body', f['id'])
        f['body'] = bodies[f['id']]
    body = '<nav><b>NCCL 2.12.12 · V3.3 · 全部函数内部步骤</b><p>本HTML包含搜索数据、完整函数体及已绑定函数的内嵌图示，单独下载即可断网使用。带完整注释的源码对照链接需完整ZIP。</p><a href="'+online_link('01_通信原语与算法图解.md')+'">通信原语与算法图解（在线）</a></nav>'
    body += f'<h1>{len(logic["functions"])}个函数：内部步骤与完整函数体</h1>'
    body += '<p>步骤是静态结构展开；编号是阅读顺序，缩进表示嵌套。关键路径另有人工精读，不能将清单数量当作全部语义已经验证。</p>'
    body += '<label for="q">搜索函数名、文件、F编号或用途</label><input id="q" type="search" placeholder="例如 genericOp、prims_ll、F0001"><span id="count" aria-live="polite"></span><div id="results"></div><button id="more">显示更多</button><noscript>请启用JavaScript以搜索和展开函数。</noscript>'
    bindings=json.loads((DOC/'source-diagram-index.json').read_text(encoding='utf-8'))['bindings']
    images={p.stem:'data:image/svg+xml;base64,'+base64.b64encode(p.read_bytes()).decode('ascii') for p in sorted((DOC/'visuals').glob('*.svg'))}
    return page('NCCL 2.12.12 全部函数内部步骤 V3.3（离线搜索）', body, json_script('logicData', logic)+json_script('search-bindings',bindings)+json_script('search-images',images)+'<script>'+SEARCH_JS+'</script>')


def main():
    for names, content in [((GUIDE, 'visual-guide.html'), build_guide()),
                           ((SEARCH, 'function-logic.html'), build_search())]:
        for name in names:
            (DOC/name).write_text(content, encoding='utf-8')
        print(f'Built {names[0]} ({len(content.encode("utf-8")):,} bytes); legacy alias identical')


if __name__ == '__main__':
    main()
