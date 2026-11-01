"""Resolve reviewed symbol/code anchors against current annotated source.

No compiler analysis is implied. Braces are scanned after comments/literals
are masked. Ambiguous or missing manual anchors fail the build.
"""
from pathlib import Path
import bisect
import hashlib
import json
import re
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[1]
DOC = ROOT/'docs/zh_CN'
READER = '11_源码注释与图示对照_离线版.html'
MARKER = re.compile(r'/\* 中文逐项-([^\s]+) ([A-Z]\d+)：')
BIND_NOTE = re.compile(r'/\* 中文精读：图示绑定 V3\.3 [\s\S]*?\*/\n?')
TOKENS = re.compile(r'/\*[\s\S]*?\*/|//[^\n]*|"(?:\\[\s\S]|[^"\\])*"|\'(?:\\[\s\S]|[^\'\\])*\'')


def mask(text):
    return TOKENS.sub(lambda m: re.sub(r'[^\n]', ' ', m[0]), text)


def line_starts(text):
    return [0]+[m.end() for m in re.finditer('\n', text)]


def line_number(starts, offset):
    return bisect.bisect_right(starts, offset)


def load_inputs():
    coverage = json.loads((DOC/'annotation-coverage.json').read_text(encoding='utf-8'))
    seed = json.loads((DOC/'source-diagram-bindings.json').read_text(encoding='utf-8'))
    symbols = {item['id']: dict(item, file=f['path']) for f in coverage['files'] for item in f['items']}
    return coverage, seed, symbols


def write_binding_comments(seed, symbols):
    groups = {}
    for binding in seed['bindings']:
        groups.setdefault(binding['symbol'], []).append(binding)
    by_file = {}
    for ident, bindings in groups.items():
        by_file.setdefault(symbols[ident]['file'], []).append((ident, bindings))
    for file, entries in by_file.items():
        path = ROOT/file
        text = BIND_NOTE.sub('', path.read_text(encoding='utf-8'))
        inserts = []
        for ident, bindings in entries:
            kind = symbols[ident]['kind']
            pattern = (r'/\* 中文内部逻辑 '+ident+r'：' if kind == '函数'
                       else r'/\* 中文逐项-类型 '+ident+r'：')
            matches = list(re.finditer(pattern, text))
            assert len(matches) == 1, (file, ident, 'annotation anchor')
            pos = text.rfind('\n', 0, matches[0].start())+1
            prefix = text[pos:matches[0].start()].strip()
            # Some original annotations follow CUDA qualifiers on the same
            # line. Insert before that complete line; preserve its bytes.
            assert not prefix or re.fullmatch(r'(?:__device__|__forceinline__|\s)+', prefix), (file, ident, 'unsafe inline insertion')
            previous = text[:pos].rstrip('\n').rsplit('\n',1)[-1]
            assert not previous.rstrip().endswith('\\'), (file, ident, 'macro continuation')
            lines = [f'/* 中文精读：图示绑定 V3.3 {ident} {symbols[ident]["name"]}',
                     f' * 源码旁看图：docs/zh_CN/{READER}#symbol={ident}',
                     ' * 普通源码编辑器不直接渲染图片；离线对照页左侧保留本文件原始源码与全部注释。']
            for binding in bindings:
                lines += [f' * [{binding["id"]}] {binding["title"]}（{binding["relation"]}）',
                          ' *   图：'+', '.join(binding['diagrams']),
                          f' *   定位：#{"symbol="+ident+"&binding="+binding["id"]}']
            lines += [' */', '']
            inserts.append((pos, '\n'.join(lines)))
        for pos, note in sorted(inserts, reverse=True):
            text = text[:pos]+note+text[pos:]
        path.write_text(text, encoding='utf-8')
    checks_path = DOC/'source-checksums.json'
    checks = json.loads(checks_path.read_text(encoding='utf-8'))
    for item in checks['code_files']:
        item['annotated_sha256'] = hashlib.sha256((ROOT/item['path']).read_bytes()).hexdigest()
    checks_path.write_text(json.dumps(checks, ensure_ascii=False, indent=2)+'\n', encoding='utf-8')


def refresh_symbol_links(symbols):
    path = DOC/'07_函数与类型定义索引.md'
    text = path.read_text(encoding='utf-8')
    text = re.sub(r'\[([FT]\d+)\]\(([^)]+)#L\d+\)',
                  lambda m: f'[{m[1]}]({m[2]}#L{symbols[m[1]]["line"]})', text)
    path.write_text(text, encoding='utf-8')
    coverage_path = DOC/'annotation-coverage.json'
    coverage = json.loads(coverage_path.read_text(encoding='utf-8'))
    for file in coverage['files']:
        for item in file['items']:
            item['annotated_line'] = symbols[item['id']]['line']
    coverage_path.write_text(json.dumps(coverage,ensure_ascii=False,indent=2)+'\n', encoding='utf-8')


def resolve(update_annotations=False, write_index=True):
    coverage, seed, inventory = load_inputs()
    if update_annotations:
        write_binding_comments(seed, inventory)
    checks = json.loads((DOC/'source-checksums.json').read_text(encoding='utf-8'))
    files = {}
    for item in checks['code_files']:
        path = item['path']
        text = (ROOT/path).read_text(encoding='utf-8')
        files[path] = {'text':text, 'sha256':hashlib.sha256(text.encode()).hexdigest(), 'lines':len(text.splitlines())}
    symbols, positions, masked, starts = {}, {}, {}, {}
    for file, data in files.items():
        text = data['text']
        starts[file] = line_starts(text)
        masked[file] = mask(text)
        for match in MARKER.finditer(text):
            ident = match[2]
            assert ident in inventory, (file, ident)
            assert ident not in symbols, ('Duplicate symbol ID', ident)
            assert inventory[ident]['file'] == file
            symbols[ident] = {k:inventory[ident][k] for k in ['id', 'kind', 'name', 'file']}
            symbols[ident]['line'] = line_number(starts[file], match.start())
            positions[ident] = match
    assert len(symbols) == len(inventory)
    scopes = {}
    for ident in sorted({b['symbol'] for b in seed['bindings']}):
        symbol = symbols[ident]
        file = symbol['file']
        text, code = files[file]['text'], masked[file]
        marker = positions[ident]
        comment_end = text.index('*/', marker.end())+2
        declaration = re.search(r'\S', code[comment_end:])
        assert declaration, ident
        begin = comment_end+declaration.start()
        opening = code.find('{', begin)
        assert opening >= 0, (ident, 'missing body')
        depth, end = 1, opening+1
        while end < len(code) and depth:
            depth += (code[end]=='{')-(code[end]=='}')
            end += 1
        assert depth == 0, (ident, 'unbalanced body')
        note = re.search(r'/\* 中文精读：图示绑定 V3\.3 '+ident+r'\b', text)
        assert note, (ident, 'binding comment missing: run with --update-annotations')
        scopes[ident] = {'file':file, 'start':line_number(starts[file],note.start()),
                         'code_start':line_number(starts[file],begin),
                         'end':line_number(starts[file],end-1), 'offset_start':begin, 'offset_end':end}
    diagrams = {}
    for path in sorted((DOC/'visuals').glob('*.svg')):
        root = ET.parse(path).getroot()
        title = root.find('{http://www.w3.org/2000/svg}title')
        diagrams[path.stem] = {'id':path.stem, 'file':path.relative_to(ROOT).as_posix(),
                               'title':title.text if title is not None else path.stem}
    bindings = []
    for raw in seed['bindings']:
        binding = dict(raw)
        ident = binding['symbol']
        scope = scopes[ident]
        file = scope['file']
        binding.update(file=file, name=symbols[ident]['name'], start=scope['start'], end=scope['end'], code_start=scope['code_start'])
        assert binding['diagrams'] and all(d in diagrams for d in binding['diagrams']), binding['id']
        if 'anchor' in binding:
            body = masked[file][scope['offset_start']:scope['offset_end']]
            # Variable annotations can appear between punctuation and a type,
            # e.g. for (/*...*/ssize_t gridOffset...). Match token content after
            # masking comments, while retaining a map back to source positions.
            offsets = [i for i,c in enumerate(body) if not c.isspace()]
            compact = ''.join(body[i] for i in offsets)
            pattern = re.escape(re.sub(r'\s+', '', binding['anchor']))
            hits = list(re.finditer(pattern, compact))
            assert hits and binding.get('occurrence',0) < len(hits), (ident,binding['id'],binding['anchor'],len(hits))
            if 'occurrence' not in raw:
                assert len(hits)==1, ('Ambiguous anchor',binding['id'])
            offset = scope['offset_start']+offsets[hits[binding.get('occurrence',0)].start()]
            anchor_line = line_number(starts[file],offset)
            binding['code_start'] = anchor_line
            # Attach comments immediately preceding this statement to the same
            # diagram. Stop at the previous executable/source line so a phase's
            # introductory explanation does not retain the previous phase image.
            masked_lines = masked[file].splitlines()
            lead = anchor_line
            while lead > scope['code_start'] and not masked_lines[lead-2].strip():
                lead -= 1
            binding['start'] = lead
            binding['scope'] = 'code'
        else:
            binding['scope'] = 'symbol'
        bindings.append(binding)
    for ident in scopes:
        segments = sorted((b for b in bindings if b['symbol']==ident and b['scope']=='code'), key=lambda b:b['start'])
        for i, b in enumerate(segments):
            b['end'] = segments[i+1]['start']-1 if i+1<len(segments) else scopes[ident]['end']
            assert b['start'] <= b['end'], b['id']
    used = {d for b in bindings for d in b['diagrams']}
    assert used == set(diagrams), ('Unbound diagrams', set(diagrams)-used)
    for scope in scopes.values():
        scope.pop('offset_start')
        scope.pop('offset_end')
    model = {'version':'3.3', 'files':files, 'symbols':list(symbols.values()), 'scopes':scopes,
             'bindings':bindings, 'diagrams':diagrams}
    if write_index:
        refresh_symbol_links(symbols)
    index = {k:v for k,v in model.items() if k!='files'}
    index['files'] = {p:{k:v for k,v in f.items() if k!='text'} for p,f in files.items()}
    if write_index:
        (DOC/'source-diagram-index.json').write_text(json.dumps(index,ensure_ascii=False,indent=2)+'\n', encoding='utf-8')
    return model
