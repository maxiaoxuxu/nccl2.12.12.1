#!/usr/bin/env python3
"""Open the offline diagram reader at an editor's current source line.

Python 3.9+, standard library only. Editors supply the file and a 1-based line;
the same reviewed, narrowest-code-first bindings are used by the HTML reader.
"""
import argparse
from dataclasses import dataclass
import hashlib
import html
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
from typing import Optional
from urllib.parse import urlencode
import webbrowser


ROOT = Path(__file__).resolve().parents[1]
READER = Path('docs/zh_CN/11_源码注释与图示对照_离线版.html')
INDEX = Path('docs/zh_CN/source-diagram-index.json')


class JumpError(ValueError):
    """A source location cannot safely be mapped to the packaged reader."""


@dataclass(frozen=True)
class Target:
    url: str
    file: str
    line: int
    binding: Optional[str]
    title: str


def resolve_target(root, source_file, line):
    """Validate a saved source file and resolve its exact offline reader URL."""
    root = Path(root).resolve()
    source = Path(source_file)
    source = (source if source.is_absolute() else root / source).resolve()
    try:
        relative = source.relative_to(root).as_posix()
    except ValueError as exc:
        raise JumpError('当前文件不在这份NCCL目录中；请打开完整解压后的分析分支。') from exc
    if not source.is_file():
        raise JumpError('找不到当前源码文件：' + str(source))
    if not (root / READER).is_file() or not (root / INDEX).is_file():
        raise JumpError('缺少图解HTML或绑定索引；请完整解压源码与图解ZIP，保留目录结构。')
    model = json.loads((root / INDEX).read_text(encoding='utf-8'))
    metadata = model['files'].get(relative)
    if not metadata:
        raise JumpError('当前文件尚未收录到源码对照页：' + relative)
    # The builder reads UTF-8 text with universal newlines. A Windows CRLF save
    # alone does not alter locations; any actual text change must be rebuilt.
    text = source.read_text(encoding='utf-8')
    if hashlib.sha256(text.encode('utf-8')).hexdigest() != metadata['sha256']:
        raise JumpError('本地源码与图解版本不同，已停止跳转以免行号错位。'
                        '请使用配套源码；维护者修改后需运行 '
                        'python tools/build_source_diagram_reader.py 并完成绑定核验。')
    if not 1 <= line <= metadata['lines']:
        raise JumpError('行号超出范围：' + relative + '，有效行号为1至' + str(metadata['lines']))
    choices = [b for b in model['bindings']
               if b['file'] == relative and b['start'] <= line <= b['end']]
    choices.sort(key=lambda b: (0 if b['scope'] == 'code' else 1,
                                b['end'] - b['start'], b['id']))
    binding = choices[0] if choices else None
    params = {'file': relative, 'line': str(line)}
    if binding:
        params.update(symbol=binding['symbol'], binding=binding['id'])
    return Target((root / READER).as_uri() + '#' + urlencode(params), relative,
                  line, binding['id'] if binding else None,
                  binding['title'] if binding else '本行暂无专属图示')


def write_jump_page(url, directory=None):
    """Use a tiny local relay so OS file handlers receive no URL fragment.

    The browser itself applies the fragment, preserving the target on systems
    whose file association handlers discard it. No server or network is used.
    The small temporary HTML remains available for the browser to load.
    """
    data = json.dumps(url, ensure_ascii=True).replace('<', '\\u003c')
    body = ('<!doctype html><html lang="zh-CN"><meta charset="utf-8">'
            '<title>正在打开NCCL源码图解</title>'
            '<p>正在定位源码与图示… <a href="' + html.escape(url, quote=True) + '">'
            '点击继续</a></p><script>location.replace(' + data + ');</script></html>')
    fd, name = tempfile.mkstemp(prefix='nccl-diagram-jump-', suffix='.html', dir=directory)
    with os.fdopen(fd, 'w', encoding='utf-8', newline='\n') as stream:
        stream.write(body)
    return Path(name)


def main(argv=None):
    parser = argparse.ArgumentParser(description='按编辑器中的当前文件、行号打开NCCL离线图解。')
    parser.add_argument('--file', required=True, help='当前源码文件的完整路径，或相对仓库路径')
    parser.add_argument('--line', required=True, type=int, help='编辑器显示的行号，从1开始')
    parser.add_argument('--print-url', action='store_true', help='只输出定位URL，不打开浏览器')
    parser.add_argument('--browser', help='可选：Edge/Chrome/Firefox可执行文件路径')
    args = parser.parse_args(argv)
    target = None
    try:
        target = resolve_target(ROOT, args.file, args.line)
        if args.print_url:
            print(target.url)
            return 0
        if args.browser:
            # Pass an argument vector, never interpolate source paths in a shell.
            subprocess.Popen([args.browser, target.url])
        elif not webbrowser.open(write_jump_page(target.url).as_uri(), new=2):
            raise JumpError('系统没有成功启动浏览器；可用--browser指定浏览器可执行文件。')
        print('已请求打开图解：' + target.file + ' 第' + str(target.line) + '行 · ' + target.title)
        return 0
    except (JumpError, OSError, UnicodeError, json.JSONDecodeError, webbrowser.Error) as exc:
        print('图示跳转失败：' + str(exc), file=sys.stderr)
        if target:
            print('可复制到浏览器打开：' + target.url, file=sys.stderr)
        return 1


if __name__ == '__main__':
    raise SystemExit(main())
