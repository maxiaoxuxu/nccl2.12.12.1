#!/usr/bin/env python3
"""Check the invariants of the teaching diagrams, not CUDA execution."""
import json
import itertools
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


def check_segment_addresses():
    """AG/RS partition each user segment, not an AR-sized C*P domain."""
    channels, ranks, capacity, grain = 2, 4, 2048, 128
    sizes = [0, 1, 7, 8, 127, 128, 129, 1000, 1024, 1025,
             4095, 4096, 4097, 16385, 32768, 65539, 1048579]
    for size in sizes:
        intervals = []
        for grid in range(0, size, channels * capacity):
            real = min(capacity, (size-grid+channels-1)//channels)
            real = (real+grain-1)//grain*grain
            for bid in range(channels):
                offset = grid+bid*real
                count = max(0, min(real, size-offset))
                if count:
                    intervals.append((offset, offset+count))
        cursor = 0
        for start, end in sorted(intervals):
            assert start == cursor, ('AG/RS segment', size, start, cursor)
            cursor = end
        assert cursor == size
        all_segments = sorted((rank*size+a, rank*size+b)
                              for rank in range(ranks) for a, b in intervals)
        cursor = 0
        for start, end in all_segments:
            assert start == cursor
            cursor = end
        assert cursor == ranks*size
    # ArgsCheck: the public count is 8 float for every row in chapter 01.
    count, element_bytes = 8, 4
    assert (count, count*element_bytes) == (8, 32)  # AR device count, nBytes
    assert (count*element_bytes, count*element_bytes*ranks) == (32, 128)  # AG
    assert (count, count*element_bytes*ranks) == (8, 128)  # RS
    return len(sizes)


def check_ring_permutations():
    """Trace the indices used by the actual five Ring algorithms.

    The oracle is public API layout: every AG rank owns every original user
    segment; RS rank r owns reduced segment r; AR owns all reduced chunks.
    It is independent of arbitrary ring order, where rank and index differ.
    This models data dependencies, not CUDA concurrency or floating rounding.
    """
    cases = 0
    for ranks in range(2, 6):
        for ring in itertools.permutations(range(ranks)):
            views = {r: ring[ring.index(r):]+ring[:ring.index(r)] for r in ring}
            ix_zero = ring.index(0)
            indices = {r: (ring.index(r)-ix_zero+ranks)%ranks for r in ring}
            previous = {r: views[r][-1] for r in ring}
            # AG: initial own segment; j=1..P-2, then final view[1].
            messages = {r: (r, {r}) for r in ring}
            outputs = {r: {r: {r}} for r in ring}
            for step in range(1, ranks):
                incoming = {r: messages[previous[r]] for r in ring}
                for r, (chunk, sources) in incoming.items():
                    assert chunk == views[r][ranks-step]
                    outputs[r][chunk] = sources
                messages = incoming
            assert all(out == {c: {c} for c in range(ranks)} for out in outputs.values())
            # RS and AR: same ring reduction dependence, different block labels.
            for kind in ('reducescatter', 'allreduce'):
                messages = {r: ((views[r][-1] if kind == 'reducescatter'
                                 else (indices[r]-1+ranks)%ranks), {r}) for r in ring}
                for step in range(1, ranks):
                    incoming = {}
                    for r in ring:
                        chunk, sources = messages[previous[r]]
                        expected = (views[r][ranks-1-step] if kind == 'reducescatter'
                                    else (indices[r]-1-step+ranks)%ranks)
                        assert chunk == expected and r not in sources
                        incoming[r] = (chunk, sources | {r})
                    messages = incoming
                for r, (chunk, sources) in messages.items():
                    assert sources == set(range(ranks))
                    assert chunk == (r if kind == 'reducescatter' else indices[r])
                if kind == 'allreduce':
                    outputs = {r: {messages[r][0]: messages[r][1]} for r in ring}
                    for step in range(1, ranks):
                        messages = {r: messages[previous[r]] for r in ring}
                        for r, (chunk, sources) in messages.items():
                            assert chunk == (indices[r]-step+ranks)%ranks
                            outputs[r][chunk] = sources
                    assert all(out == {c: set(range(ranks)) for c in range(ranks)}
                               for out in outputs.values())
            # Bcast and Reduce: every root, not just root=0.
            for root in ring:
                chain = views[root]
                assert views[chain[-1]][1] == root  # Bcast tail: nextRank==root
                reduce_chain = chain[1:]+chain[:1]
                assert previous[reduce_chain[0]] == root  # Reduce start
                assert reduce_chain[-1] == root
                assert set(chain) == set(reduce_chain) == set(range(ranks))
                total = 0
                for r in reduce_chain:
                    total += (r+1)*11
                assert total == sum((r+1)*11 for r in range(ranks))
            cases += 1
    # Explicit counterexample to treating ring.index as user rank/array index.
    ring = (2, 0, 3, 1)
    assert (ring.index(3)-ring.index(0)) % 4 == 1
    assert ring[ring.index(3):]+ring[:ring.index(3)] == (3, 1, 2, 0)
    return cases


def main():
    diagrams = list(VIS.glob('*.svg'))
    assert len(diagrams) == 45  # V3.3 adds Channel generation, QP creation, MR cache.
    for path in diagrams:
        root = ET.parse(path).getroot()
        assert root.tag.endswith('svg') and root.attrib.get('viewBox')
    check_frames()
    sizes = check_addresses()
    segments = check_segment_addresses()
    permutations = check_ring_permutations()
    print(f'PASS: {len(diagrams)} SVGs; collective ownership/contribution transitions; {sizes} address-size cases')
    print(f'PASS: {segments} AG/RS segment-size cases; five Ring API semantics over {permutations} ring permutations and every root')
    print('Teaching models only; no CUDA, GPU or network runtime validation.')


if __name__ == '__main__':
    main()
