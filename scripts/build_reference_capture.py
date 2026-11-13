"""Build a reference-sequence experiment; default output is an offline manifest."""
import argparse
import hashlib
import os
from pathlib import Path
import shutil
import shlex
import subprocess

REVISION = '5420cd9b7124e5a70deaae4bffedd969ae4ef91c'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--reference-source', required=True, type=Path)
    parser.add_argument('--output', default='/tmp/ft9366-capture-reference', type=Path)
    parser.add_argument('--cc', default='cc')
    parser.add_argument('--simulate', action='store_true', help='build a distinctly named simulation executable')
    parser.add_argument('--cflags', default='-std=c11 -Wall -Wextra -Wpedantic -Werror -O2')
    args = parser.parse_args()
    source = args.reference_source.resolve()
    revision = subprocess.check_output(['git', '-C', str(source), 'rev-parse', 'HEAD'], text=True).strip()
    if revision != REVISION:
        parser.error('reference must match pinned revision ' + REVISION)
    subprocess.run(['git', '-C', str(source), 'diff', '--exit-code', 'HEAD', '--', 'driver/ft9366_script.h', 'LICENSE'], check=True)
    root = Path(__file__).resolve().parents[1]
    output = args.output.resolve()
    if args.simulate:
        output = Path(str(output) + ".sim")
    output.parent.mkdir(parents=True, exist_ok=True)
    flags = shlex.split(subprocess.check_output(['pkg-config', '--cflags', 'libusb-1.0', 'glib-2.0'], text=True))
    libs = shlex.split(subprocess.check_output(['pkg-config', '--libs', 'libusb-1.0', 'glib-2.0'], text=True))
    command = shlex.split(args.cc) + shlex.split(args.cflags) + flags + [
        '-I' + str(root / 'tools'), '-I' + str(source / 'driver'),
        str(root / 'tools/ft9366-capture-reference.c'), str(root / 'tools/ft9366-replay.c')
    ]
    if args.simulate:
        command.append(str(root / 'tests/fake-reference-usb.c'))
    command += libs + ['-o', str(output)]
    subprocess.run(command, check=True)
    shutil.copyfile(source / 'LICENSE', str(output) + '.LICENSE')
    Path(str(output) + '.NOTICE').write_text(
        'Reference sequence: ilyamiro/ft9366-fingerprint, ' + REVISION + '\n'
        'https://github.com/ilyamiro/ft9366-fingerprint\n'
        'Reference data license: LGPL-2.1-or-later. No vendor binary included.\n'
        'Runner is original project work; project license selection is pending.\n')
    with Path(str(output) + '-plan.txt').open('w') as plan:
        subprocess.run([str(output), '--plan'], stdout=plan, check=True)
    print('Built:', output)
    print('SHA256:', hashlib.sha256(output.read_bytes()).hexdigest())
    print('Offline command manifest:', str(output) + '-plan.txt')
    print('No hardware operations executed. --capture requires explicit approval.')
    if args.simulate:
        for scenario, expected in [('success', 0), ('short', 1), ('disconnect', 1), ('no-finger', 1)]:
            result = subprocess.run([str(output), '--capture'], input='\n', capture_output=True, text=True,
                                    env=os.environ | {'FT9366_SIMULATION_CASE': scenario})
            if result.returncode != expected or 'SIMULATED USB BACKEND' not in result.stdout or 'USB resources closed' not in result.stdout:
                raise RuntimeError('Simulation failed: ' + scenario + '\n' + result.stdout + result.stderr)
            print('SIMULATION PASSED:', scenario)


if __name__ == '__main__':
    main()
