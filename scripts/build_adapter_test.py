"""Compile and run simulated adapter tests against pinned libfprint headers; no USB."""
import argparse
from pathlib import Path
import shlex
import subprocess

PINNED_LIBFPRINT = '80a4b5ec612892c5c056c48dddaa561452cf37ec'


def output(*args):
    return subprocess.check_output(args, text=True).strip()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--libfprint-source', required=True, type=Path)
    parser.add_argument('--build-dir', required=True, type=Path)
    parser.add_argument('--cc', default='cc')
    parser.add_argument('--cflags', default='-std=c11 -Wall -Wextra -Wpedantic -Werror -O2')
    args = parser.parse_args()
    source = args.libfprint_source.resolve()
    if output('git', '-C', str(source), 'rev-parse', 'HEAD') != PINNED_LIBFPRINT:
        parser.error('source must match the documented libfprint v1.94.100 commit')
    # Ignore untracked generated files but refuse modified tracked source headers.
    subprocess.run(['git', '-C', str(source), 'diff', '--exit-code', 'HEAD', '--', 'libfprint'], check=True)
    root = Path(__file__).resolve().parents[1]
    build = args.build_dir.resolve()
    build.mkdir(parents=True, exist_ok=True)
    # fpi-print.h includes generated enum declarations. This compile-only subset
    # is generated from the actual header with GLib's tool, not handwritten stubs.
    with (build / 'fpi-enums.h').open('w') as header:
        subprocess.run([
            'glib-mkenums', '--fhead', '#pragma once\n#include <glib-object.h>\nG_BEGIN_DECLS',
            '--eprod', 'GType @enum_name@_get_type (void);', '--ftail', 'G_END_DECLS',
            str(source / 'libfprint/fpi-print.h')
        ], stdout=header, check=True)
    cflags = shlex.split(output('pkg-config', '--cflags', 'gusb', 'gio-2.0', 'libfprint-2'))
    libs = shlex.split(output('pkg-config', '--libs', 'gusb', 'gio-2.0'))
    executable = build / 'test-fprint-usb'
    command = shlex.split(args.cc) + shlex.split(args.cflags) + [
        '-I' + str(root / 'driver'), '-I' + str(build), '-I' + str(source / 'libfprint')
    ] + cflags + [
        str(root / 'tests/test-fprint-usb.c'), str(root / 'driver/ft9366-fprint-usb.c'),
        str(root / 'driver/ft9366-transport.c')
    ] + libs + ['-o', str(executable)]
    subprocess.run(command, check=True)
    subprocess.run([str(executable)], check=True)


if __name__ == '__main__':
    main()
