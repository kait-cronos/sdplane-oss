#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.12"
# dependencies = ["matplotlib"]
# ///
"""Compare several benchmark runs in one chart of loss and one of absorbed routes.

    uv run compare_results.py --out results/compare \\
        results/rib-lthread-netlink-lcore-XXX results/rib-lcore-netlink-lcore-YYY

A run may be labelled with LABEL=DIR; the default label is its directory name.
"""

import argparse
import csv
import statistics
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt

COLORS = ["#2a78d6", "#eb6834", "#eda100", "#1baf7a", "#e87ba4", "#008300", "#6250d6", "#e34948"]
MARKERS = ["o", "s", "D", "^", "v", "P", "X", "*"]
INK, MUTED, GRID, BACKGROUND = "#0b0b0b", "#52514e", "#e4e3df", "#fcfcfb"


def load(spec):
    """Return (label, {route count: [settled rows]}) for LABEL=DIR or DIR."""
    label, separator, directory = spec.rpartition("=")
    path = Path(directory)
    with open(path / "results.csv", newline="") as f:
        rows = [row for row in csv.DictReader(f) if row["status"] == "ok"]
    grouped = {}
    for row in rows:
        grouped.setdefault(int(row["routes"]), []).append(row)
    return (label if separator else path.resolve().name), dict(sorted(grouped.items()))


def compact(count):
    for divisor, suffix in ((1_000_000, "M"), (1000, "k")):
        if count >= divisor:
            return f"{count / divisor:g}{suffix}"
    return str(count)


def draw(runs, field, ylabel, title, path, log_y=False):
    fig, ax = plt.subplots(figsize=(9.5, 5), layout="constrained", facecolor=BACKGROUND)
    ax.set_facecolor(BACKGROUND)
    for i, (label, grouped) in enumerate(runs):
        means = [statistics.mean(float(row[field]) for row in rows) for rows in grouped.values()]
        ax.plot(list(grouped), means, color=COLORS[i], marker=MARKERS[i], linewidth=2,
                markersize=8, markeredgecolor=BACKGROUND, markeredgewidth=2, clip_on=False,
                zorder=3, label=label)
    counts = sorted({count for _, grouped in runs for count in grouped})
    ax.set_xscale("log")
    if log_y:
        ax.set_yscale("log")
    else:
        ax.set_ylim(0, 105)
    ax.set_xticks(counts, [compact(count) for count in counts])
    ax.minorticks_off()
    ax.set_xlabel("Routes installed (log scale)", color=MUTED)
    ax.set_ylabel(ylabel, color=MUTED)
    ax.set_title(title, color=INK, loc="left", fontsize=13)
    ax.legend(frameon=False, labelcolor=MUTED)
    ax.grid(axis="y", color=GRID, linewidth=0.8)
    ax.set_axisbelow(True)
    ax.tick_params(colors=MUTED, length=0)
    ax.tick_params(axis="x", pad=9)
    for side in ("top", "right", "left"):
        ax.spines[side].set_visible(False)
    ax.spines["bottom"].set_color(GRID)
    fig.savefig(path, dpi=160)
    print(f"plot: {path}")


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("runs", nargs="+", metavar="[LABEL=]DIR", help="result directories")
    parser.add_argument("--out", type=Path, default=Path("results/compare"))
    args = parser.parse_args()
    if len(args.runs) > len(COLORS):
        parser.error(f"at most {len(COLORS)} runs can be told apart in one chart")
    runs = [load(spec) for spec in args.runs]
    args.out.mkdir(parents=True, exist_ok=True)
    draw(runs, "loss_pct", "Loss rate (%)", "Route loss by run, mean of trials",
         args.out / "compare-loss.png")
    draw(runs, "seen", "Routes reaching sdplane (log scale)", "Routes absorbed by run",
         args.out / "compare-absorbed.png", log_y=True)


if __name__ == "__main__":
    main()
