"""Synthetic replay-container fixtures; no hardware or external files required."""
import importlib.util
from pathlib import Path
import unittest

spec = importlib.util.spec_from_file_location('audit', Path(__file__).parents[1] / 'scripts/ft9366_script_audit.py')
audit = importlib.util.module_from_spec(spec)
spec.loader.exec_module(audit)

class AuditTests(unittest.TestCase):
    def test_counts_container_records_without_retaining_payloads(self):
        result = audit.analyze(bytes.fromhex('00 0002 1234 01 2800 02 000a'))
        self.assertEqual(result['writes'], 1)
        self.assertEqual(result['reads'], 1)
        self.assertEqual(result['frame_reads'], 1)
        self.assertEqual(result['delay_ms'], 10)
        self.assertEqual(result['write_bytes'], 2)
        self.assertEqual(result['read_bytes'], 10240)
        self.assertNotIn('payload', result)

    def test_rejects_truncation_and_unknown_operations(self):
        for raw in ['00', '00 00', '00 0002 ff', '03 0000', '01 0000']:
            with self.subTest(raw=raw), self.assertRaises(ValueError):
                audit.analyze(bytes.fromhex(raw))

    def test_delay_has_no_payload_and_write_payload_is_not_a_header(self):
        result = audit.analyze(bytes.fromhex('02 00ff 00 0003 010203'))
        self.assertEqual(result['delay_ms'], 255)
        self.assertEqual(result['writes'], 1)
        self.assertEqual(result['reads'], 0)

    def test_empty_container(self):
        self.assertEqual(audit.analyze(b'')['records'], 0)

if __name__ == '__main__':
    unittest.main()
