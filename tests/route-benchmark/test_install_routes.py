import gzip
import struct
import tempfile
import unittest
from pathlib import Path

from install_routes import Route, kernel_cleanup, kernel_install, read_mrt, rewrite_routes


def rib(prefix=b"\x0a", bits=8, subtype=2):
    entry = struct.pack("!HI", 0, 0)
    if subtype in (8, 10):
        entry += struct.pack("!I", 123)
    body = struct.pack("!IB", 0, bits) + prefix + struct.pack("!H", 1) + entry + b"\x00\x00"
    return struct.pack("!IHHI", 0, 13, subtype, len(body)) + body


class NetlinkError(Exception):
    def __init__(self, code):
        self.code = code


class FakeIPRoute:
    def __init__(self, fail_add=None, fail_delete=None):
        self.calls = []
        self.fail_add = fail_add
        self.fail_delete = fail_delete

    def link_lookup(self, **kwargs):
        return [2]

    def route(self, command, **request):
        self.calls.append((command, request))
        if command == "add" and request["dst"] == self.fail_add:
            raise NetlinkError(17)
        if command == "del" and request["dst"] == self.fail_delete:
            raise NetlinkError(1)


class Tests(unittest.TestCase):
    def read(self, data, **kwargs):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "rib.gz"
            with gzip.open(path, "wb") as f:
                f.write(data)
            return read_mrt(path, **kwargs)

    def test_dedup_addpath_and_family(self):
        data = rib() + rib(subtype=8) + rib(b"\x20\x01\x0d\xb8", 32, 4)
        self.assertEqual([r.dst for r in self.read(data)], ["10.0.0.0/8"])
        self.assertEqual([r.dst for r in self.read(data, family=6)], ["2001:db8::/32"])
        self.assertEqual(len(self.read(data, family=None, limit=1)), 1)

    def test_truncated_input(self):
        with self.assertRaisesRegex(ValueError, "truncated"):
            self.read(rib()[:-1])

    def test_invalid_entry_length(self):
        with self.assertRaisesRegex(ValueError, "truncated RIB entry"):
            self.read(rib()[:-2] + b"\xff\xff")

    def test_gateway_family(self):
        with self.assertRaises(ValueError):
            rewrite_routes([Route("::/0")], gateway="192.0.2.1", dev="eth0")

    def test_collision_rolls_back_only_added_routes(self):
        routes = rewrite_routes([Route("10.0.0.0/8"), Route("11.0.0.0/8")],
                                gateway="192.0.2.1", dev="eth0")
        ipr = FakeIPRoute(fail_add="11.0.0.0/8")
        with self.assertRaises(NetlinkError):
            kernel_install(routes, ipr=ipr)
        self.assertEqual([(c, r["dst"]) for c, r in ipr.calls],
                         [("add", "10.0.0.0/8"), ("add", "11.0.0.0/8"), ("del", "10.0.0.0/8")])

    def test_cleanup_continues_and_retains_failed_routes(self):
        routes = rewrite_routes([Route("10.0.0.0/8"), Route("11.0.0.0/8")],
                                gateway="192.0.2.1", dev="eth0")
        ipr = FakeIPRoute(fail_delete="11.0.0.0/8")
        with self.assertRaises(RuntimeError):
            kernel_cleanup(routes, ipr=ipr)
        self.assertEqual([r.dst for r in routes], ["11.0.0.0/8"])
        self.assertEqual(len(ipr.calls), 2)


if __name__ == "__main__":
    unittest.main()
