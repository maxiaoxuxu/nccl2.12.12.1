#!/usr/bin/env python3
"""Validate standalone resources, function payloads, aliases, and local links.

Browser behavior is covered separately by check_offline_docs_browser.cjs.
This file uses only Python's standard library.
"""
import base64
import json
import re
from html.parser import HTMLParser
from pathlib import Path
from urllib.parse import unquote, urlsplit
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[1]
DOC = ROOT/'docs/zh_CN'


class Page(HTMLParser):
    def __init__(self, text):
        super().__init__()
        self.images, self.scripts, self.ids = [], {}, set()
        self.active = None
        self.feed(text)

    def handle_starttag(self, tag, attrs):
        attrs = dict(attrs)
        if 'id' in attrs:
            assert attrs['id'] not in self.ids, ('Duplicate ID', attrs['id'])
            self.ids.add(attrs['id'])
        if tag == 'img':
            self.images.append(attrs.get('src', ''))
            assert attrs.get('alt'), 'Missing image description'
        if tag in ('script', 'link', 'iframe', 'object'):
            assert not (attrs.get('src') or attrs.get('href') or attrs.get('data')), ('External resource', tag, attrs)
        if tag == 'script' and attrs.get('type') == 'application/json':
            self.active = attrs['id']
            self.scripts[self.active] = ''

    def handle_data(self, data):
        if self.active:
            self.scripts[self.active] += data

    def handle_endtag(self, tag):
        if tag == 'script':
            self.active = None


def main():
    aliases = [('01_通信原语与算法图解_离线版.html', 'visual-guide.html'),
               ('05_全部函数内部步骤_离线搜索.html', 'function-logic.html')]
    pages = []
    for name, alias in aliases:
        data = (DOC/name).read_bytes()
        assert data == (DOC/alias).read_bytes(), alias
        text = data.decode('utf-8')
        assert not re.search(r'url\s*\(|\bfetch\s*\(|XMLHttpRequest|\bimport\s*\(', text), name
        pages.append(Page(text))
    guide, search = pages
    images = json.loads(guide.scripts['embedded-images'])
    svgs = list((DOC/'visuals').glob('*.svg'))
    assert set(images) == {p.stem for p in svgs}
    for path in svgs:
        uri = images[path.stem]
        assert uri.startswith('data:image/svg+xml;base64,')
        data = base64.b64decode(uri.partition(',')[2], validate=True)
        assert data == path.read_bytes(), path
        ET.fromstring(data)
    assert guide.images and all(src in images.values() for src in guide.images)
    assert json.loads(guide.scripts['frames']) == json.loads((DOC/'visuals/collective-steps.json').read_text())
    embedded = json.loads(search.scripts['logicData'])
    canonical = json.loads((DOC/'function-logic.json').read_text())
    ids = set()
    for function in embedded['functions']:
        assert function['body'].strip(), function['id']
        ids.add(function['id'])
        del function['body']
    assert embedded == canonical
    assert len(ids) == len(canonical['functions']) == 1178
    # Check both canonical chapters and legacy link stubs. Ignore fenced code,
    # where examples can intentionally contain Markdown-like strings.
    links = 0
    for path in [ROOT/'README.md', ROOT/'START_HERE_ZH.md', ROOT/'00_从这里开始_NCCL源码阅读指南.md', *DOC.rglob('*.md')]:
        text = re.sub(r'```[\s\S]*?```', '', path.read_text())
        for match in re.finditer(r'!?\[([^\]]*)\]\(([^)\s]+)\)', text):
            href = match[2]
            if urlsplit(href).scheme:
                continue
            raw, _, fragment = unquote(href).partition('#')
            target = (path.parent/raw).resolve() if raw else path
            assert target.exists(), (path.relative_to(ROOT), href)
            if fragment.startswith('L') and fragment[1:].isdigit():
                line = int(fragment[1:])
                rows = target.read_text().splitlines()
                assert 1 <= line <= len(rows), (path, href)
                if re.fullmatch(r'[FT]\d+', match[1]):
                    assert match[1] in rows[line-1], ('Stale symbol anchor', path, href, match[1])
            if re.fullmatch(r'f\d+', fragment):
                assert f'id="{fragment}"' in target.read_text(), (path, href)
            links += 1
    print(f'PASS: {len(svgs)} embedded SVG assets; {len(guide.images)} static images; 1178 embedded complete function bodies; legacy HTML aliases identical; {links} local links')
    print('No external runtime resources. Run browser regression for actual file:// interactions.')


if __name__ == '__main__':
    main()
