"""Build the pinned Track A Arch package in a fresh native /tmp tree; never install it."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output-dir', type=Path)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    destination = (args.output_dir or root / 'artifacts/arch').resolve()
    build = Path(tempfile.mkdtemp(prefix='ft9366-arch-build-', dir='/tmp'))
    for source in (root / 'packaging/arch').iterdir():
        if source.is_file():
            shutil.copyfile(source, build / source.name)
    env = os.environ.copy()
    env.setdefault('SOURCE_DATE_EPOCH', '1792590607')
    print('Build tree:', build, flush=True)
    # Dependencies may prompt through makepkg -s; replacement package is never installed.
    subprocess.run(['makepkg', '-s', '--log'], cwd=build, env=env, check=True)
    subprocess.run(['namcap', str(build / 'PKGBUILD')], check=True)
    destination.mkdir(parents=True, exist_ok=True)
    packages = list(build.glob('*.pkg.tar.zst'))
    if not packages:
        raise RuntimeError('makepkg returned without a package artifact')
    for package in packages:
        subprocess.run(['namcap', str(package)], check=True)
        target = destination / package.name
        if target.exists() and digest(target) != digest(package):
            # Package contents can embed the randomized native build path.
            # Keep prior output and place this build in its own subdirectory.
            destination = destination / build.name
            destination.mkdir(parents=True, exist_ok=True)
            target = destination / package.name
            if target.exists() and digest(target) != digest(package):
                raise RuntimeError('Preserving existing artifact; choose another --output-dir: ' + str(target))
        if not target.exists():
            shutil.copyfile(package, target)
        print('Package:', target, 'SHA256:', digest(target), flush=True)
    testlog = build / 'src/build/meson-logs/testlog.json'
    results = [json.loads(line) for line in testlog.read_text().splitlines()]
    report = {
        'track': 'A: pinned upstream FT9366 integration, not original driver code',
        'source_date_epoch': env['SOURCE_DATE_EPOCH'],
        'build_tree': str(build),
        'tests': {value: sum(t.get('result') == value for t in results)
                  for value in sorted({t.get('result') for t in results})},
        'packages': {p.name: digest(p) for p in packages},
        'installed': False,
    }
    (destination / ('build-' + build.name + '.json')).write_text(json.dumps(report, indent=2) + '\n')
    print('Build finished. Installation needs a separately reviewed Pacman transaction and rollback.')


if __name__ == '__main__':
    main()
