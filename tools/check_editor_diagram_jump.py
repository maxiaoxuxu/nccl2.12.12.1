#!/usr/bin/env python3
"""Exercise real source locations, editor task expansion and safe failures."""
import contextlib
import io
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch
from urllib.parse import parse_qs, unquote, urlsplit

import open_source_diagram as jump


class EditorJumpTests(unittest.TestCase):
    def test_known_collective_and_primitive_phases(self):
        examples = [('src/collectives/device/all_reduce.h', 306, 'B0007'),
                    ('src/collectives/device/all_reduce.h', 328, 'B0009'),
                    ('src/collectives/device/all_reduce.h', 12, 'B0001'),
                    ('src/collectives/device/prims_simple.h', 657, 'B0030')]
        for file, line, binding in examples:
            with self.subTest(file=file, line=line):
                target = jump.resolve_target(jump.ROOT, file, line)
                params = parse_qs(urlsplit(target.url).fragment)
                self.assertEqual(target.binding, binding)
                self.assertEqual(params['binding'], [binding])
                self.assertEqual(params['line'], [str(line)])
                self.assertEqual(params['file'], [file])

    def test_unbound_sdk_is_not_assigned_a_diagram(self):
        model = json.loads((jump.ROOT / jump.INDEX).read_text(encoding='utf-8'))
        symbol = next(s for s in model['symbols'] if s['kind'] == '函数' and '/nvtx3/' in s['file'])
        target = jump.resolve_target(jump.ROOT, symbol['file'], symbol['line'])
        self.assertIsNone(target.binding)
        self.assertNotIn('binding', parse_qs(urlsplit(target.url).fragment))

    def test_invalid_and_uncollected_locations(self):
        for line in [0, -1, 1000000]:
            with self.assertRaisesRegex(jump.JumpError, '行号超出范围'):
                jump.resolve_target(jump.ROOT, 'src/collectives/device/all_reduce.h', line)
        with self.assertRaisesRegex(jump.JumpError, '尚未收录'):
            jump.resolve_target(jump.ROOT, 'README.md', 1)
        with self.assertRaisesRegex(jump.JumpError, '不在这份NCCL目录'):
            jump.resolve_target(jump.ROOT, jump.ROOT.parent / 'outside.cc', 1)

    def test_unicode_paths_source_edits_and_windows_newlines(self):
        with tempfile.TemporaryDirectory(prefix='NCCL 中文 空格 #50% & ') as folder:
            root = Path(folder)
            file = Path('src/collectives/device/all_reduce.h')
            for relative in [file, jump.INDEX, jump.READER]:
                (root / relative).parent.mkdir(parents=True, exist_ok=True)
                shutil.copyfile(jump.ROOT / relative, root / relative)
            target = jump.resolve_target(root, str(root / file), 306)
            self.assertEqual(unquote(urlsplit(target.url).path), str(root / jump.READER))
            self.assertEqual(target.binding, 'B0007')
            self.assertNotIn(' ', target.url)
            original = (root / file).read_bytes()
            (root / file).write_bytes(original.replace(b'\n', b'\r\n'))
            self.assertEqual(jump.resolve_target(root, file, 306).binding, 'B0007')
            (root / file).write_bytes(b'// an inserted editor line\n' + original)
            with self.assertRaisesRegex(jump.JumpError, '本地源码与图解版本不同'):
                jump.resolve_target(root, file, 306)
            (root / file).write_bytes(original)
            (root / jump.READER).unlink()
            with self.assertRaisesRegex(jump.JumpError, '完整解压'):
                jump.resolve_target(root, file, 306)

    def test_vscode_process_task_and_user_shortcut(self):
        task = json.loads((jump.ROOT / '.vscode/tasks.json').read_text(encoding='utf-8'))['tasks'][0]
        self.assertEqual(task['type'], 'process')
        self.assertEqual(task['windows']['command'], 'python')
        values = {'${workspaceFolder}': str(jump.ROOT),
                  '${file}': str(jump.ROOT / 'src/collectives/device/all_reduce.h'),
                  '${lineNumber}': '306'}
        args = task['args'] + ['--print-url']
        for variable, value in values.items():
            args = [arg.replace(variable, value) for arg in args]
        result = subprocess.run([sys.executable, *args], cwd=jump.ROOT.parent,
                                capture_output=True, text=True, encoding='utf-8', check=True)
        self.assertEqual(parse_qs(urlsplit(result.stdout.strip()).fragment)['binding'], ['B0007'])
        shortcut = json.loads((jump.ROOT / 'tools/editor-integration/VSCode_图示跳转快捷键示例.json').read_text(encoding='utf-8'))[0]
        self.assertEqual(shortcut['args'], task['label'])

    def test_browser_arguments_and_failure_output(self):
        args = ['--file', 'src/collectives/device/all_reduce.h', '--line', '306']
        with patch.object(jump.subprocess, 'Popen') as spawn, contextlib.redirect_stdout(io.StringIO()):
            self.assertEqual(jump.main(args + ['--browser', 'C:/Program Files/浏览器/browser.exe']), 0)
            self.assertEqual(spawn.call_args.args[0][0], 'C:/Program Files/浏览器/browser.exe')
            self.assertEqual(len(spawn.call_args.args[0]), 2)
            self.assertEqual(parse_qs(urlsplit(spawn.call_args.args[0][1]).fragment)['binding'], ['B0007'])
            self.assertNotIn('shell', spawn.call_args.kwargs)
        with tempfile.TemporaryDirectory() as folder:
            relay = jump.write_jump_page(jump.resolve_target(jump.ROOT, args[1], 306).url, folder)
            stderr = io.StringIO()
            with patch.object(jump, 'write_jump_page', return_value=relay), \
                    patch.object(jump.webbrowser, 'open', return_value=False) as opener, \
                    contextlib.redirect_stderr(stderr):
                self.assertEqual(jump.main(args), 1)
                self.assertFalse(urlsplit(opener.call_args.args[0]).fragment)
                self.assertIn('可复制到浏览器', stderr.getvalue())


if __name__ == '__main__':
    unittest.main(verbosity=2)
