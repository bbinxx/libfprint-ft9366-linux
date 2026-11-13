"""Offline reference replay-container audit. Never accesses USB or executes input."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import re


def analyze(data):
    """Validate host-side records; return lengths/counts, never payload contents."""
    result = dict(records=0, writes=0, reads=0, frame_reads=0,
                  delay_ms=0, write_bytes=0, read_bytes=0)
    lengths = Counter()
    position = 0
    while position < len(data):
        if len(data) - position < 3:
            raise ValueError(f'truncated header at offset {position}')
        operation = data[position]
        length = int.from_bytes(data[position + 1:position + 3], 'big')
        position += 3
        if operation not in (0, 1, 2):
            raise ValueError(f'unknown operation {operation}')
        if operation in (0, 1) and length == 0:
            raise ValueError('zero-length transfer')
        result['records'] += 1
        if operation == 0:
            if length > len(data) - position:
                raise ValueError('truncated write payload')
            position += length
            result['writes'] += 1
            result['write_bytes'] += length
        elif operation == 1:
            result['reads'] += 1
            result['read_bytes'] += length
            lengths[length] += 1
            if length == 10240:
                result['frame_reads'] += 1
        else:
            result['delay_ms'] += length
    result['read_lengths'] = dict(sorted(lengths.items()))
    result['sha256'] = hashlib.sha256(data).hexdigest()
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('header', type=Path, help='reference ft9366_script.h')
    args = parser.parse_args()
    source = args.header.read_text()
    arrays = re.findall(r'static const guint8 (ft9366_\w+_script)\[\]\s*=\s*\{([^}]+)\};', source)
    if not arrays:
        parser.error('no supported reference arrays found')
    results = {}
    for name, body in arrays:
        if name in results:
            parser.error('duplicate script name')
        tokens = [t.strip() for t in body.split(',') if t.strip()]
        if any(re.fullmatch(r'0x[0-9a-fA-F]{2}', t) is None for t in tokens):
            parser.error('unsupported array syntax')
        results[name] = analyze(bytes(int(t, 16) for t in tokens))
    print(json.dumps(results, indent=2))


if __name__ == '__main__':
    main()
