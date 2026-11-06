#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# dependencies = ["pyroute2>=0.7.12,<1", "matplotlib"]
# ///
"""Measure how many kernel routes sdplane loses, per route count, and plot it.

Each trial starts a fresh sdplane, installs N routes with install_routes.py,
waits for `show rib ipv4 route` to settle, and compares it to what was installed.
See README.md for the setup.

    sudo uv run measure_route_loss.py --counts 100 1000 10000
    uv run measure_route_loss.py --plot results/XXX/results.csv
"""

import argparse
import csv
import ipaddress
import os
import re
import shutil
import signal
import socket
import statistics
import subprocess
import sys
import time
from dataclasses import replace
from datetime import datetime
from pathlib import Path

# Running as root must not leave a root-owned __pycache__ in the source tree.
sys.dont_write_bytecode = True
import install_routes

BENCH_DIR = Path(__file__).resolve().parent
REPO_DIR = BENCH_DIR.parents[1]
RESULTS_DIR = BENCH_DIR / "results"
DEFAULT_CONF = BENCH_DIR / "configs" / "rib-lcore-netlink-lcore.conf"
DEFAULT_MRT = BENCH_DIR / "data" / "bview.gz"
NETNS = "a"
VTY_ADDR = ("127.0.0.1", 9882)
PROMPT = re.compile(rb"vty\[\d+\]> ")
TELNET_CMD = re.compile(rb"\xff\xfa.*?\xff\xf0|\xff[\xfb-\xfe].|\xff[\xf0-\xfa]", re.S)
TELNET_PARTIAL = re.compile(rb"\xff(?:\xfa(?:(?!\xff\xf0).)*|[\xfb-\xfe])?\Z", re.S)
# RIB rows start with "[index] prefix", FIB rows with the prefix.
PREFIX_ROW = re.compile(r"^\s*(?:\[\d+\]\s+)?([0-9a-fA-F.:]+/\d+)\s", re.M)
CSV_FIELDS = ["routes", "trial", "installed", "seen", "lost", "loss_pct",
              "socket_drops", "ring_lost", "install_sec", "settle_sec", "status"]


class Vty:
    """Minimal telnet client for the sdplane CLI."""

    def __init__(self, addr=VTY_ADDR, timeout=600):
        self.sock = socket.create_connection(addr, timeout=5)
        self.sock.settimeout(timeout)
        self._reset()
        self._read_until(PROMPT)
        # The pager lives in the vty session; show would otherwise go through less.
        self.command("no pager", b"pager disabled.")

    def __enter__(self):
        return self

    def __exit__(self, *exc):
        self.sock.close()

    def _reset(self):
        self.clean, self.pending = bytearray(), b""

    def _feed(self, data):
        """Append data to clean, holding back a telnet command cut off by recv()."""
        data = self.pending + data
        partial = TELNET_PARTIAL.search(data)
        cut = partial.start() if partial else len(data)
        self.clean += TELNET_CMD.sub(b"", data[:cut])
        self.pending = data[cut:]

    def _read_until(self, pattern, start=0):
        """Read until pattern appears past start; return the end of the match."""
        pos = start
        while True:
            match = pattern.search(self.clean, pos)
            if match:
                return match.end()
            # Rescanning everything is quadratic: a show can be tens of megabytes.
            pos = max(pos, len(self.clean) - 64)
            data = self.sock.recv(1 << 20)
            if not data:
                raise ConnectionError("vty closed")
            self._feed(data)

    def command(self, line, marker):
        """Run line and return its output, from marker up to the next prompt.

        The echo of the command can contain a redrawn prompt, so the prompt only
        counts after a marker that the command itself prints.
        """
        self._reset()
        self.sock.sendall(line.encode() + b"\r\n")
        begin = self._read_until(re.compile(re.escape(marker))) - len(marker)
        end = self._read_until(PROMPT, begin)
        return self.clean[begin:end].decode(errors="replace")

    def prefixes(self, source):
        # One row per nexthop, so a prefix can be listed more than once.
        output = self.command(f"show {source} ipv4 route", b"Destination")
        return set(PREFIX_ROW.findall(output))


def run(*cmd, check=True):
    return subprocess.run(cmd, check=check, text=True, capture_output=True)


def enter_netns():
    fd = os.open(f"/run/netns/{NETNS}", os.O_RDONLY)
    os.setns(fd, os.CLONE_NEWNET)
    os.close(fd)


def sdplane_pids():
    pids = []
    for pid in run("ip", "netns", "pids", NETNS).stdout.split():
        try:
            if Path(f"/proc/{pid}/comm").read_text().strip() == "sdplane":
                pids.append(int(pid))
        except OSError:
            pass
    return pids


class Sdplane:
    """sdplane for the duration of a trial."""

    def __init__(self, log_path, binary, conf):
        self.log_path = log_path
        self.cmd = ["ip", "netns", "exec", NETNS, str(binary), "-f", str(conf)]

    def __enter__(self):
        with open(self.log_path, "wb") as log:
            # stdin must stay open: EOF would close the console shell.
            self.proc = subprocess.Popen(self.cmd, cwd=BENCH_DIR, stdin=subprocess.PIPE,
                                         stdout=log, stderr=subprocess.STDOUT,
                                         start_new_session=True)
        return self

    def __exit__(self, *exc):
        try:
            os.killpg(self.proc.pid, signal.SIGTERM)
        except ProcessLookupError:
            pass
        deadline = time.monotonic() + 20
        while (pids := sdplane_pids()) and time.monotonic() < deadline:
            time.sleep(0.2)
        for pid in pids:
            print(f"sdplane {pid} ignored SIGTERM; killing", flush=True)
            os.kill(pid, signal.SIGKILL)
        while sdplane_pids():
            time.sleep(0.2)
        self.proc.stdin.close()
        self.proc.wait()

    def wait_for(self, what, predicate, timeout):
        deadline = time.monotonic() + timeout
        while True:
            if not sdplane_pids() and self.proc.poll() is not None:
                raise RuntimeError(f"sdplane exited while waiting for {what}")
            try:
                if predicate():
                    return True
            except (OSError, ConnectionError):
                pass
            if time.monotonic() > deadline:
                return False
            time.sleep(0.5)


def prepare_router(sdplane, args):
    """Bring up the router tap and wait until sdplane has seen its connected route."""
    tap_exists = lambda: run("ip", "link", "show", "dev", args.dev, check=False).returncode == 0
    if not sdplane.wait_for(f"tap {args.dev}", tap_exists, args.startup_timeout):
        raise RuntimeError(f"tap {args.dev} did not appear")
    run("ip", "addr", "replace", args.router_addr, "dev", args.dev)
    run("ip", "link", "set", "dev", args.dev, "up")

    connected = str(ipaddress.ip_interface(args.router_addr).network)

    def learned():
        with Vty() as vty:
            if connected in vty.prefixes("rib"):
                return True
        # With rib-manager on an lcore, the startup route dump can beat the
        # router-if registration and be ignored. Flapping re-announces the route.
        run("ip", "addr", "del", args.router_addr, "dev", args.dev)
        run("ip", "addr", "add", args.router_addr, "dev", args.dev)
        time.sleep(1)
        return False

    if not sdplane.wait_for(f"route {connected}", learned, args.startup_timeout):
        raise RuntimeError(f"{connected} never reached the RIB; netlink not working?")


def socket_drops():
    """Drops on sdplane's multicast netlink socket (needs to be in its netns)."""
    pids = {str(pid) for pid in sdplane_pids()}
    for line in Path("/proc/self/net/netlink").read_text().splitlines()[1:]:
        fields = line.split()
        protocol, pid, groups, drops = fields[1], fields[2], fields[3], fields[-2]
        if protocol == "0" and pid in pids and int(groups, 16):
            return int(drops)
    return ""


def wait_settled(expected, args):
    """Poll only after install: show blocks the lthread core netlink runs on."""
    started = time.monotonic()
    previous, same = None, 0
    with Vty() as vty:
        while True:
            seen = vty.prefixes(args.source) & expected
            same = same + 1 if seen == previous else 0
            previous = seen
            elapsed = time.monotonic() - started
            if len(seen) == len(expected) or same + 1 >= args.stable_reads:
                return seen, elapsed, "ok"
            if elapsed > args.settle_timeout:
                return seen, elapsed, "not-settled"
            time.sleep(args.settle_interval)


def run_trial(routes, trial, outdir, args):
    from pyroute2 import IPRoute

    count = len(routes)
    row = dict(routes=count, trial=trial, installed=count)
    log_path = outdir / "logs" / f"sdplane-{count}-{trial}.log"
    installed = []
    with IPRoute() as ipr:
        try:
            with Sdplane(log_path, args.sdplane, args.conf) as sdplane:
                prepare_router(sdplane, args)
                started = time.monotonic()
                installed = install_routes.kernel_install(routes, ipr=ipr)
                row["install_sec"] = round(time.monotonic() - started, 3)
                expected = {route.dst for route in installed}
                seen, settle, status = wait_settled(expected, args)
                row["socket_drops"] = socket_drops()
        finally:
            # Delete after sdplane is gone: the tap outlives it, leftovers would
            # break the next add, and sdplane need not process the delete flood.
            try:
                install_routes.kernel_cleanup(installed, ipr=ipr)
            except Exception as exc:
                print(f"cleanup failed: {exc}", flush=True)
    lost = len(expected) - len(seen)
    # internal_msg_send_to() logs "... is lost." for each message the ring refused.
    row.update(seen=len(seen), lost=lost, loss_pct=round(100 * lost / len(expected), 3),
               ring_lost=log_path.read_bytes().count(b"is lost."),
               settle_sec=round(settle, 3), status=status)
    return row


def load_rows(path):
    with open(path, newline="") as f:
        return list(csv.DictReader(f))


def summarize(rows):
    """{route count: [loss % of each settled trial]}; failed trials are excluded."""
    result = {}
    for row in rows:
        if row["status"] == "ok":
            result.setdefault(int(row["routes"]), []).append(float(row["loss_pct"]))
    return dict(sorted(result.items()))


def print_summary(rows):
    print(f"\n{'routes':>10} {'trials':>6} {'mean %':>9} {'min %':>9} {'max %':>9}")
    for count, losses in summarize(rows).items():
        print(f"{count:>10,} {len(losses):>6} {statistics.mean(losses):>9.3f} "
              f"{min(losses):>9.3f} {max(losses):>9.3f}")
    failed = [row for row in rows if row["status"] != "ok"]
    for row in failed:
        print(f"excluded: routes={row['routes']} trial={row['trial']}: {row['status']}")


def compact(count):
    for divisor, suffix in ((1_000_000, "M"), (1000, "k")):
        if count >= divisor:
            return f"{count / divisor:g}{suffix}"
    return str(count)


def plot(rows, path, source):
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    summary = summarize(rows)
    if not summary:
        print("no successful trials; nothing to plot")
        return
    counts = list(summary)
    means = [statistics.mean(losses) for losses in summary.values()]
    lows = [mean - min(losses) for mean, losses in zip(means, summary.values())]
    highs = [max(losses) - mean for mean, losses in zip(means, summary.values())]

    blue, ink, muted, grid = "#2a78d6", "#0b0b0b", "#52514e", "#e4e3df"
    fig, ax = plt.subplots(figsize=(9, 4.5), layout="constrained", facecolor="#fcfcfb")
    ax.set_facecolor("#fcfcfb")
    ax.errorbar(counts, means, yerr=[lows, highs], color=blue, linewidth=2, marker="o",
                markersize=8, markeredgecolor="#fcfcfb", markeredgewidth=2,
                ecolor=blue, elinewidth=1.5, capsize=4, clip_on=False, zorder=3)
    for count, losses in summary.items():
        ax.scatter([count] * len(losses), losses, s=14, color=blue, alpha=0.35,
                   linewidths=0, zorder=2)
    for count, mean in zip(counts, means):
        if mean > 0:
            ax.annotate(f"{mean:.1f}%", (count, mean), xytext=(-6, 9), ha="right",
                        textcoords="offset points", color=ink, fontsize=9)

    ax.set_xscale("log")
    ax.set_xticks(counts, [compact(count) for count in counts])
    ax.minorticks_off()
    ax.margins(x=0.1)
    top = max(mean + high for mean, high in zip(means, highs))
    ax.set_ylim(0, max(1, top * 1.25))
    ax.set_xlabel("Routes installed (log scale)", color=muted)
    ax.set_ylabel("Loss rate (%)", color=muted)
    ax.set_title("sdplane route loss by route count", color=ink, loc="left", fontsize=13, pad=20)
    trials = max(len(losses) for losses in summary.values())
    ax.text(0, 1.01, f"{source.upper()} vs. kernel, mean of {trials} trials; bars show min-max",
            transform=ax.transAxes, color=muted, fontsize=9, va="bottom")
    ax.grid(axis="y", color=grid, linewidth=0.8)
    ax.set_axisbelow(True)
    ax.tick_params(colors=muted, length=0)
    for side in ("top", "right", "left"):
        ax.spines[side].set_visible(False)
    ax.spines["bottom"].set_color(grid)
    fig.savefig(path, dpi=160)
    print(f"plot: {path}")


def plot_absorbed(rows, path):
    """Routes absorbed against install time, for trials where loss was substantial."""
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    points = [(float(row["install_sec"]), int(row["seen"])) for row in rows
              if row["status"] == "ok" and float(row["loss_pct"]) >= 20]
    if not points:
        return
    points.sort()
    # If each route costs time proportional to the RIB, then seen^2 = k * t.
    k = statistics.median(seen ** 2 / sec for sec, seen in points)
    ends = [points[0][0], points[-1][0]]

    blue, ink, muted, grid = "#2a78d6", "#0b0b0b", "#52514e", "#e4e3df"
    fig, ax = plt.subplots(figsize=(9, 4.5), layout="constrained", facecolor="#fcfcfb")
    ax.set_facecolor("#fcfcfb")
    ax.plot(ends, [(k * sec) ** 0.5 for sec in ends], color=muted, linewidth=2,
            linestyle=(0, (4, 3)), label=f"sqrt({k / 1e6:.1f}M * t)", zorder=2)
    ax.scatter(*zip(*points), s=64, color=blue, edgecolors="#fcfcfb", linewidths=2,
               label="Each trial", zorder=3)
    ax.set_xscale("log")
    ax.set_yscale("log")
    ax.set_xlabel("Install duration (s, log scale)", color=muted)
    ax.set_ylabel("Routes in RIB (log scale)", color=muted)
    ax.set_title("Routes absorbed grow with the square root of time", color=ink,
                 loc="left", fontsize=13)
    ax.legend(frameon=False, labelcolor=muted, loc="upper left")
    ax.grid(color=grid, linewidth=0.8)
    ax.set_axisbelow(True)
    ax.tick_params(colors=muted, length=0, which="both")
    for side in ("top", "right", "left"):
        ax.spines[side].set_visible(False)
    ax.spines["bottom"].set_color(grid)
    fig.savefig(path, dpi=160)
    print(f"plot: {path}")


def give_back(path):
    """Hand the results to the user who ran sudo."""
    uid, gid = os.environ.get("SUDO_UID"), os.environ.get("SUDO_GID")
    if uid and gid:
        items = [path, *path.rglob("*")]
        if path.parent == RESULTS_DIR:
            items.append(RESULTS_DIR)
        for item in items:
            os.chown(item, int(uid), int(gid))


def parse_args():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--counts", type=int, nargs="+", default=[100, 1000, 10000])
    parser.add_argument("--trials", type=int, default=3)
    parser.add_argument("--conf", type=Path, default=DEFAULT_CONF,
                        help="sdplane config (default: configs/rib-lcore-netlink-lcore.conf)")
    parser.add_argument("--sdplane", type=Path, default=REPO_DIR / "sdplane" / "sdplane",
                        help="sdplane binary (default: sdplane/sdplane in this repository)")
    parser.add_argument("--source", choices=("rib", "fib"), default="rib",
                        help="table compared to the kernel (default: rib)")
    parser.add_argument("--mrt", type=Path, default=DEFAULT_MRT,
                        help="MRT dump to take the routes from (default: data/bview.gz)")
    parser.add_argument("--gateway", default="192.168.2.2")
    parser.add_argument("--dev", default="router")
    parser.add_argument("--router-addr", default="192.168.2.1/24",
                        help="address put on the tap so the gateway is reachable")
    parser.add_argument("--protocol", type=int, default=4)
    parser.add_argument("--startup-timeout", type=float, default=60)
    parser.add_argument("--settle-interval", type=float, default=2)
    parser.add_argument("--stable-reads", type=int, default=3,
                        help="identical show results needed to call it settled")
    parser.add_argument("--settle-timeout", type=float, default=300)
    parser.add_argument("--out", type=Path, help="default: results/<config>-<time>")
    parser.add_argument("--plot", type=Path, metavar="CSV", help="only redraw from a results.csv")
    return parser.parse_args()


def measure(args):
    if os.geteuid() != 0:
        sys.exit("root required: use ./run_sweep.sh or sudo uv run measure_route_loss.py")
    args.sdplane, args.conf, args.mrt = (path.resolve() for path in (args.sdplane, args.conf, args.mrt))
    for path, hint in ((args.sdplane, "build sdplane first"), (args.conf, "see configs/"),
                       (args.mrt, "see \"MRT データを用意する\" in README.md")):
        if not path.is_file():
            sys.exit(f"{path} not found: {hint}")

    signal.signal(signal.SIGTERM, signal.default_int_handler)
    subprocess.run([BENCH_DIR / "setup_netns.py", "up"], check=True)
    enter_netns()
    if sdplane_pids():
        sys.exit(f"sdplane is already running in netns {NETNS}; stop it first")

    outdir = args.out or RESULTS_DIR / f"{args.conf.stem}-{datetime.now():%Y%m%d-%H%M%S}"
    (outdir / "logs").mkdir(parents=True)
    shutil.copyfile(args.conf, outdir / "sdplane.conf")
    family = ipaddress.ip_address(args.gateway).version
    print(f"reading {max(args.counts):,} routes from {args.mrt}", flush=True)
    all_routes = install_routes.read_mrt(args.mrt, family=family, limit=max(args.counts))
    all_routes = install_routes.rewrite_routes(all_routes, gateway=args.gateway, dev=args.dev)
    all_routes = [replace(route, protocol=args.protocol) for route in all_routes]
    if len(all_routes) < max(args.counts):
        sys.exit(f"MRT only has {len(all_routes):,} routes")

    rows = []
    csv_path = outdir / "results.csv"
    try:
        with open(csv_path, "w", newline="") as f:
            writer = csv.DictWriter(f, CSV_FIELDS)
            writer.writeheader()
            for count in sorted(args.counts):
                for trial in range(1, args.trials + 1):
                    print(f"--- routes={count:,} trial={trial}/{args.trials}", flush=True)
                    try:
                        row = run_trial(all_routes[:count], trial, outdir, args)
                    except Exception as exc:
                        row = dict(routes=count, trial=trial, status=f"error: {exc}")
                    print(row, flush=True)
                    rows.append(row)
                    writer.writerow(row)
                    f.flush()
    except KeyboardInterrupt:
        print("interrupted; keeping the trials finished so far")
    print_summary(rows_as_text(rows))
    plot(rows_as_text(rows), outdir / "loss.png", args.source)
    plot_absorbed(rows_as_text(rows), outdir / "absorbed.png")
    give_back(outdir)
    print(f"results: {outdir}")


def rows_as_text(rows):
    return [{key: str(value) for key, value in row.items()} for row in rows]


def main():
    args = parse_args()
    if args.plot:
        rows = load_rows(args.plot)
        print_summary(rows)
        plot(rows, args.plot.with_name("loss.png"), args.source)
        plot_absorbed(rows, args.plot.with_name("absorbed.png"))
    else:
        measure(args)


if __name__ == "__main__":
    main()
