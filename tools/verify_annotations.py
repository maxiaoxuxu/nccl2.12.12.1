#!/usr/bin/env python3
"""Verify the distributed NCCL Chinese annotations using only Python's standard library."""
import hashlib
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MARKER_PATTERN = re.compile(r'/\* 中文逐项-([^\s]+) ([A-Z]\d+)：')
COMMENT_PATTERN = re.compile(
    rb'/\* ' + '中文'.encode('utf-8') +
    rb'(?:' + '逐项-'.encode('utf-8') + rb'[\s\S]*?|' +
    '(?:导读|精读|内部逻辑)'.encode('utf-8') + rb'[\s\S]*?)\*/(?:\n)?'
)
BUILD_PATTERN = re.compile(rb'^# ' + '中文构建：'.encode('utf-8') + rb'[^\n]*\n', re.MULTILINE)


def main():
    coverage = json.loads((ROOT / 'docs/zh_CN/annotation-coverage.json').read_text(encoding='utf-8'))
    checks = json.loads((ROOT / 'docs/zh_CN/source-checksums.json').read_text(encoding='utf-8'))
    errors = []
    symbol_count = 0
    for source in coverage['files']:
        path = ROOT / source['path']
        if not path.is_file():
            errors.append('Missing source: ' + source['path'])
            continue
        text = path.read_text(encoding='utf-8')
        found = [(kind, ident) for kind, ident in MARKER_PATTERN.findall(text)]
        wanted = [(item['kind'], item['id']) for item in source['items']]
        if sorted(found) != sorted(wanted):
            errors.append('Annotation inventory mismatch: ' + source['path'])
        symbol_count += len(wanted)
    for source in checks['code_files']:
        path = ROOT / source['path']
        if not path.is_file():
            errors.append('Missing code: ' + source['path'])
            continue
        data = path.read_bytes()
        if hashlib.sha256(data).hexdigest() != source['annotated_sha256']:
            errors.append('Annotated content checksum mismatch: ' + source['path'])
        original = BUILD_PATTERN.sub(b'', COMMENT_PATTERN.sub(b'', data))
        # Git object IDs are SHA1('blob ' + byte length + NUL + original bytes).
        blob = b'blob ' + str(len(original)).encode('ascii') + b'\0' + original
        if hashlib.sha1(blob).hexdigest() != source['original_git_blob']:
            errors.append('Non-comment source change: ' + source['path'])
    logic = json.loads((ROOT / 'docs/zh_CN/function-logic.json').read_text(encoding='utf-8'))
    wanted_functions = sorted(item['id'] for f in coverage['files'] for item in f['items'] if item['kind'] == '函数')
    internal_functions = sorted(ident for f in coverage['files'] for ident in re.findall(r'/\* 中文内部逻辑 (F\d+)：', (ROOT / f['path']).read_text(encoding='utf-8')))
    if wanted_functions != internal_functions or wanted_functions != sorted(f['id'] for f in logic['functions']):
        errors.append('Function internal-logic inventory mismatch')
    entry = (ROOT / 'START_HERE_ZH.md').read_text(encoding='utf-8')
    if 'NCCL-BEGINNER-ANNOTATED-V3' not in entry:
        errors.append('Missing beginner edition identifier')
    if errors:
        print('FAILED:')
        print('\n'.join(errors))
        return 1
    print('PASS: NCCL-BEGINNER-ANNOTATED-V3')
    print('Annotated source files:', len(coverage['files']))
    print('Function definitions:', coverage['counts']['函数'])
    print('Type definitions:', coverage['counts']['类型'])
    print('Variable declaration items:', coverage['counts']['variable_items'])
    print('Verified annotation IDs:', symbol_count)
    print('Code files byte-identical to baseline after removing added comments:', len(checks['code_files']))
    print('Function internal steps:', logic['stats']['steps'])
    print('CUDA compilation and GPU execution were not tested by this check.')
    return 0


if __name__ == '__main__':
    sys.exit(main())
