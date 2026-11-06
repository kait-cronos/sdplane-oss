#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.10"
# dependencies = ["pyroute2>=0.7.12,<1"]
# ///
"""Load MRT prefixes and temporarily install them using one rtnetlink socket.

measure_route_loss.py imports this module. It can also be run by hand, inside
the netns sdplane is watching; press Enter to delete the routes again:

    sudo ip netns exec a "$(command -v uv)" run install_routes.py --gateway 192.168.2.2 \\
        --dev router --protocol 4 --limit 1000
"""

import argparse
import errno
import gzip
import ipaddress
import socket
import struct
import time
from dataclasses import dataclass, replace


@dataclass(frozen=True, slots=True)
class Route:
    dst: str
    gateway: str | None = None
    dev: str | None = None
    table: int = 254
    metric: int = 42760
    protocol: int = 186


def read_mrt(path, *, family=4, limit=None):
    """Read unique unicast prefixes; deliberately discard BGP nexthops/attributes.

    Supports TABLE_DUMP and TABLE_DUMP_V2 AFI-specific RIBs (including ADDPATH).
    family is 4, 6, or None. limit applies after family filtering and deduplication.
    """
    if family not in (4, 6, None) or (limit is not None and limit < 1):
        raise ValueError("family must be 4/6/None and limit must be positive")
    routes, seen = [], set()
    with open(path, "rb") as raw:
        compressed = raw.read(2) == b"\x1f\x8b"
    opener = gzip.open if compressed else open
    with opener(path, "rb") as f:
        record = 0
        while header := f.read(12):
            record += 1
            if len(header) != 12:
                raise ValueError(f"MRT record {record}: truncated header")
            _, kind, subtype, size = struct.unpack("!IHHI", header)
            if size > 64 * 1024 * 1024:
                raise ValueError(f"MRT record {record}: excessive record size {size}")
            data = f.read(size)
            if len(data) != size:
                raise ValueError(f"MRT record {record}: truncated body")
            try:
                if kind == 13 and subtype in (2, 4, 8, 10):
                    version = 4 if subtype in (2, 8) else 6
                    bits = data[4]
                    if bits > (32 if version == 4 else 128):
                        raise ValueError("invalid prefix length")
                    n = (bits + 7) // 8
                    address = data[5:5 + n]
                    pos = 5 + n
                    count = struct.unpack_from("!H", data, pos)[0]
                    pos += 2
                    # Validate every entry without decoding its BGP attributes.
                    for _ in range(count):
                        attr_pos = pos + (10 if subtype in (8, 10) else 6)
                        attr_size = struct.unpack_from("!H", data, attr_pos)[0]
                        pos = attr_pos + 2 + attr_size
                        if pos > size:
                            raise ValueError("truncated RIB entry")
                    if pos != size:
                        raise ValueError("unexpected RIB trailing bytes")
                    if not count:
                        continue
                elif kind == 12 and subtype in (1, 2):
                    version = 4 if subtype == 1 else 6
                    n = 4 if version == 4 else 16
                    address = data[4:4 + n]
                    bits = data[4 + n]
                    minimum = 4 + n + 1 + 1 + 4 + n + 2 + 2
                    if len(data) < minimum:
                        raise ValueError("truncated TABLE_DUMP")
                    attr_pos = 4 + n + 1 + 1 + 4 + n + 2
                    attr_size = struct.unpack_from("!H", data, attr_pos)[0]
                    if attr_pos + 2 + attr_size != size:
                        raise ValueError("invalid TABLE_DUMP attribute length")
                    if data[5 + n] != 1:
                        continue
                elif kind == 13 and subtype == 1:
                    continue  # PEER_INDEX_TABLE
                else:
                    raise ValueError(f"unsupported MRT type/subtype {kind}/{subtype}")
                if family is not None and family != version:
                    continue
                packed = address.ljust(4 if version == 4 else 16, b"\x00")
                network = ipaddress.ip_network((ipaddress.ip_address(packed), bits), strict=False)
            except (IndexError, struct.error, ValueError) as exc:
                raise ValueError(f"MRT record {record}: {exc}") from exc
            dst = str(network)
            if dst not in seen:
                seen.add(dst)
                routes.append(Route(dst))
                if limit is not None and len(routes) >= limit:
                    break
    if not routes:
        raise ValueError("No matching unicast routes in MRT")
    return routes


def rewrite_routes(routes, *, gateway, dev, table=254, metric=42760):
    """Replace all nexthops with a real, reachable adjacent router."""
    gw = ipaddress.ip_address(gateway)
    if not dev or not 1 <= table <= 0xFFFFFFFF or not 0 <= metric <= 0xFFFFFFFF:
        raise ValueError("invalid dev/table/metric")
    result = []
    for route in routes:
        if ipaddress.ip_network(route.dst).version != gw.version:
            raise ValueError(f"gateway family does not match {route.dst}")
        result.append(replace(route, gateway=str(gw), dev=dev, table=table, metric=metric))
    return result


def _request(ipr, route):
    if route.gateway is None or route.dev is None:
        raise ValueError("call rewrite_routes() or set Route.gateway and Route.dev first")
    links = ipr.link_lookup(ifname=route.dev)
    if len(links) != 1:
        raise ValueError(f"interface not found: {route.dev}")
    return dict(dst=route.dst, gateway=route.gateway, oif=links[0],
                family=socket.AF_INET if ipaddress.ip_network(route.dst).version == 4 else socket.AF_INET6,
                table=route.table, priority=route.metric, proto=route.protocol,
                type=1, scope=0)


def kernel_cleanup(installed, *, ipr=None):
    """Delete only successful adds, keeping failed deletes in installed for retry."""
    if ipr is None:
        from pyroute2 import IPRoute
        with IPRoute() as connection:
            return kernel_cleanup(installed, ipr=connection)
    failures, remaining, requests = [], [], {}
    for route in reversed(installed):
        try:
            key = (route.gateway, route.dev, route.table, route.metric, route.protocol,
                   ipaddress.ip_network(route.dst).version)
            if key not in requests:
                requests[key] = _request(ipr, route)
            ipr.route("del", **dict(requests[key], dst=route.dst))
        except Exception as exc:
            if getattr(exc, "code", None) not in (errno.ENOENT, errno.ESRCH):
                failures.append((route.dst, exc))
                remaining.append(route)
                continue
    installed[:] = reversed(remaining)
    if failures:
        raise RuntimeError(f"cleanup failed for {len(failures)} routes; first: {failures[0]}")


def kernel_install(routes, *, ipr=None):
    """Return successful adds. On any failure/interrupt roll back earlier adds.

    Uses add (not replace): an existing route causes failure and is left intact.
    """
    if ipr is None:
        from pyroute2 import IPRoute
        with IPRoute() as connection:
            return kernel_install(routes, ipr=connection)
    installed = []
    requests = {}
    started = time.monotonic()
    try:
        for route in routes:
            key = (route.gateway, route.dev, route.table, route.metric, route.protocol,
                   ipaddress.ip_network(route.dst).version)
            if key not in requests:
                requests[key] = _request(ipr, route)
            request = dict(requests[key], dst=route.dst)
            ipr.route("add", **request)
            installed.append(route)
            if len(installed) % 10000 == 0:
                print(f"installed {len(installed):,} ({time.monotonic() - started:.1f}s)", flush=True)
    except BaseException:
        kernel_cleanup(installed, ipr=ipr)
        raise
    print(f"installed {len(installed):,} ({time.monotonic() - started:.1f}s)", flush=True)
    return installed


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("path", nargs="?", default="data/bview.gz")
    parser.add_argument("--gateway", required=True)
    parser.add_argument("--dev", required=True)
    parser.add_argument("--table", type=int, default=254,
                        help="routing table ID (default: 254, main)")
    parser.add_argument("--metric", type=int, default=42760)
    parser.add_argument("--protocol", type=int, choices=range(256), default=186,
                        metavar="0..255", help="kernel route protocol (default: 186, BGP)")
    parser.add_argument("--limit", type=int)
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args()
    family = ipaddress.ip_address(args.gateway).version
    routes = read_mrt(args.path, family=family, limit=args.limit)
    routes = rewrite_routes(routes, gateway=args.gateway, dev=args.dev,
                            table=args.table, metric=args.metric)
    routes = [replace(route, protocol=args.protocol) for route in routes]
    print(f"read {len(routes):,} unique IPv{family} routes; table={args.table}")
    if args.dry_run:
        for route in routes[:5]:
            print(route)
        return
    from pyroute2 import IPRoute
    with IPRoute() as ipr:
        installed = kernel_install(routes, ipr=ipr)
        try:
            input("Enterで追加した経路を削除: ")
        finally:
            kernel_cleanup(installed, ipr=ipr)
            print("cleanup complete")


if __name__ == "__main__":
    main()
