"""Synthetic archive checks; no network, USB or biometric fixtures."""
import importlib.util
import io
from pathlib import Path
import tarfile
import tempfile
import unittest

spec = importlib.util.spec_from_file_location('prepare', Path(__file__).resolve().parents[1] / 'scripts/prepare_release_sources.py')
prepare = importlib.util.module_from_spec(spec)
spec.loader.exec_module(prepare)


class ArchiveTests(unittest.TestCase):
    def archive(self, name, kind=tarfile.REGTYPE):
        data = io.BytesIO()
        with tarfile.open(fileobj=data, mode='w') as archive:
            entry = tarfile.TarInfo(name)
            entry.type = kind
            entry.mode = 0o755
            if kind == tarfile.REGTYPE:
                entry.size = 5
            archive.addfile(entry, io.BytesIO(b'hello') if entry.size else None)
        data.seek(0)
        return tarfile.open(fileobj=data)

    def test_legacy_api_extracts_file_without_extractall(self):
        with tempfile.TemporaryDirectory() as directory, self.archive('src/tool') as archive:
            class LegacyArchive:
                getmembers = archive.getmembers
                extractfile = archive.extractfile
            prepare.extract_sources(LegacyArchive(), Path(directory))
            output = Path(directory) / 'src/tool'
            self.assertEqual(output.read_bytes(), b'hello')
            self.assertEqual(output.stat().st_mode & 0o777, 0o755)

    def test_rejects_traversal_absolute_links_and_special_files(self):
        for name, kind in [('src/../../outside', tarfile.REGTYPE),
                           ('/outside', tarfile.REGTYPE),
                           ('src/link', tarfile.SYMTYPE),
                           ('src/hardlink', tarfile.LNKTYPE),
                           ('src/fifo', tarfile.FIFOTYPE)]:
            with self.subTest(name=name), tempfile.TemporaryDirectory() as directory, self.archive(name, kind) as archive:
                with self.assertRaises(ValueError):
                    prepare.extract_sources(archive, Path(directory))
                self.assertEqual(list(Path(directory).iterdir()), [])

    def test_preserves_existing_file(self):
        with tempfile.TemporaryDirectory() as directory, self.archive('tool') as archive:
            output = Path(directory) / 'tool'
            output.write_text('preserve')
            with self.assertRaises(FileExistsError):
                prepare.extract_sources(archive, Path(directory))
            self.assertEqual(output.read_text(), 'preserve')

    def test_rejects_existing_symlink_parent(self):
        with tempfile.TemporaryDirectory() as directory, self.archive('src/tool') as archive:
            root = Path(directory)
            (root / 'real').mkdir()
            (root / 'src').symlink_to(root / 'real', target_is_directory=True)
            with self.assertRaises(ValueError):
                prepare.extract_sources(archive, root)


if __name__ == '__main__':
    unittest.main()
