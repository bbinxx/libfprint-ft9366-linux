"""Prepare checked Track A sources; never install or access USB hardware."""
import argparse
import hashlib
import re
import shutil
import subprocess
import tarfile
import urllib.request
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--work-dir', type=Path, required=True)
    parser.add_argument('--cache-dir', type=Path)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    recipe = root / 'packaging/arch'
    text = (recipe / 'PKGBUILD').read_text()
    commits = [re.search(r'^_' + name + r"_commit=([0-9a-f]{40})$", text, re.M).group(1)
               for name in ('libfprint', 'ft9366')]
    checks = re.findall(r"'([0-9a-f]{128})'", text)
    lib, ref = commits
    sources = [
        (f'libfprint-{lib}.tar.gz', f'https://gitlab.freedesktop.org/libfprint/libfprint/-/archive/{lib}/libfprint-{lib}.tar.gz'),
        (f'ft9366-fingerprint-{ref}.tar.gz', f'https://codeload.github.com/ilyamiro/ft9366-fingerprint/tar.gz/{ref}'),
    ]
    work = args.work_dir.resolve()
    work.mkdir(parents=True, exist_ok=True)
    for (name, url), expected in zip(sources, checks[:2], strict=True):
        target = work / name
        cached = args.cache_dir / name if args.cache_dir else None
        if cached and cached.is_file():
            shutil.copyfile(cached, target)
        else:
            with urllib.request.urlopen(url, timeout=120) as response, target.open('wb') as output:
                shutil.copyfileobj(response, output)
        if hashlib.blake2b(target.read_bytes()).hexdigest() != expected:
            raise RuntimeError('Source checksum mismatch: ' + name)
        with tarfile.open(target) as archive:
            archive.extractall(work, filter='data')
    local = ['ft9366-integration.patch', 'egis-replay-cancellation.patch',
             '70-libfprint-ft9366.rules', 'UPSTREAM-NOTICE']
    for name, expected in zip(local, checks[2:], strict=True):
        if hashlib.blake2b((recipe / name).read_bytes()).hexdigest() != expected:
            raise RuntimeError('Local source checksum mismatch: ' + name)
    source = work / f'libfprint-{lib}'
    for patch in local[:2]:
        subprocess.run(['patch', '-Np1', '-i', str(recipe / patch)], cwd=source, check=True)
    for name in ('ft9366.c', 'ft9366_match.c', 'ft9366_match.h', 'ft9366_pairs.h', 'ft9366_script.h'):
        shutil.copyfile(work / f'ft9366-fingerprint-{ref}/driver' / name,
                        source / 'libfprint/drivers' / name)
    shutil.copytree(recipe, work / 'arch-packaging')
    print(source)


if __name__ == '__main__':
    main()
