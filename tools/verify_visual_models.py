#!/usr/bin/env python3
"""Check the invariants of the teaching diagrams, not CUDA execution."""
import json
import xml.etree.ElementTree as ET
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
VIS = ROOT / 'docs/zh_CN/visuals'


def check_frames():
    models = json.loads((VIS / 'collective-steps.json').read_text(encoding='utf-8'))
    for kind, frames in models.items():
        assert len(frames) == (7 if kind == 'allreduce' else 4)
        for index, frame in enumerate(frames):
            assert (VIS / f'{kind}-step-{index}.svg').is_file()
            if index == 0:
                continue
            previous = frames[index - 1]
            for arrow in frame['arrows']:
                sender, receiver = arrow['from'], arrow['to']
                assert receiver == (sender + 1) % 4
                before = previous['current'][str(sender)]
                after = frame['current'][str(receiver)]
                assert before['chunk'] == arrow['chunk'] == after['chunk']
                assert before['sources'] == arrow['sources']
                expected = set(before['sources'])
                if kind != 'allgather' and index <= 3:
                    assert receiver not in expected, 'A rank contribution would be reduced twice'
                    expected.add(receiver)
                assert set(after['sources']) == expected
                for chunk, sources in previous['outputs'][receiver].items():
                    assert frame['outputs'][receiver][chunk] == sources
        for rank, row in enumerate(frames[-1]['outputs']):
            if kind == 'reducescatter':
                assert row == {str(rank): [0, 1, 2, 3]}
            elif kind == 'allreduce':
                assert row == {str(chunk): [0, 1, 2, 3] for chunk in range(4)}
            else:
                assert row == {str(chunk): [chunk] for chunk in range(4)}


def check_addresses():
    # Fixed teaching configuration from visual-guide.md; not a prediction of tuning.
    channels, ranks, capacity, grain = 2, 4, 2048, 128
    sizes = [0, 1, 7, 8, 127, 128, 129, 1000, 1024, 1025,
             16383, 16384, 16385, 32767, 32768, 65539, 1048579]
    for size in sizes:
        intervals = []
        for grid in range(0, size, channels * ranks * capacity):
            real = min(capacity, (size - grid + channels * ranks - 1) // (channels * ranks))
            real = (real + grain - 1) // grain * grain
            for bid in range(channels):
                for chunk in range(ranks):
                    offset = grid + (bid * ranks + chunk) * real
                    valid = max(0, min(real, size - offset))
                    if valid:
                        intervals.append((offset, offset + valid))
        cursor = 0
        for start, end in sorted(intervals):
            assert start == cursor, (size, start, cursor)
            cursor = end
        assert cursor == size
    # Layout-only figure: both permutations cover exactly the same eight blocks.
    simple = {bid * ranks + chunk for bid in range(channels) for chunk in range(ranks)}
    interleaved = {chunk * channels + bid for bid in range(channels) for chunk in range(ranks)}
    assert simple == interleaved == set(range(8))
    # The worked example has two 64-float slices, yet each advances two FIFO steps.
    assert max(((128 + 16 * 2 - 1) // (16 * 2)) * 16, 512 * 2 // 32) == 64
    assert 2 * (ranks - 1) * 4 == 24
    return len(sizes)


def main():
    diagrams = list(VIS.glob('*.svg'))
    assert len(diagrams) == 42
    for path in diagrams:
        root = ET.parse(path).getroot()
        assert root.tag.endswith('svg') and root.attrib.get('viewBox')
    check_frames()
    sizes = check_addresses()
    print(f'PASS: {len(diagrams)} SVGs; collective ownership/contribution transitions; {sizes} address-size cases')
    print('Teaching models only; no CUDA, GPU or network runtime validation.')


if __name__ == '__main__':
    main()
