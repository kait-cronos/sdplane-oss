#!/usr/bin/env python3
"""Manage a/b namespaces joined by two veth pairs; no IP/routes.
Usage: sudo ./setup_netns.py up | sudo ./setup_netns.py down
Both links are bidirectional; names indicate intended use.
"""
import argparse
import json
import os
import shutil
import subprocess

NAMES = ('a', 'b')
PAIRS = (('a-out', 'b-in'), ('a-in', 'b-out'))
MARKER = 'lab:a-b:v1'


def ip(*args):
    return subprocess.run(['ip', *args], check=True, text=True,
                          stdout=subprocess.PIPE).stdout


def namespaces():
    return {line.split()[0] for line in ip('netns', 'list').splitlines() if line.strip()}


def owned(ns):
    links = json.loads(ip('-n', ns, '-j', 'link', 'show', 'dev', 'lo'))
    return links[0].get('ifalias') == MARKER


def up():
    present = namespaces() & set(NAMES)
    if present:
        if present != set(NAMES) or not all(owned(ns) for ns in NAMES):
            raise RuntimeError('a/b already exist or lab is incomplete; refusing to overwrite')
        for left, right in PAIRS:
            for ns, dev in (('a', left), ('b', right)):
                ip('-n', ns, 'link', 'show', 'dev', dev)
        for ns in NAMES:
            ip('-n', ns, 'link', 'set', 'dev', 'lo', 'up')
        for left, right in PAIRS:
            ip('-n', 'a', 'link', 'set', 'dev', left, 'up')
            ip('-n', 'b', 'link', 'set', 'dev', right, 'up')
        print('lab already up')
        return
    created = []
    try:
        for ns in NAMES:
            ip('netns', 'add', ns)
            created.append(ns)
            ip('-n', ns, 'link', 'set', 'dev', 'lo', 'alias', MARKER)
            ip('-n', ns, 'link', 'set', 'dev', 'lo', 'up')
        for left, right in PAIRS:
            ip('-n', 'a', 'link', 'add', left, 'type', 'veth',
               'peer', 'name', right, 'netns', 'b')
            ip('-n', 'a', 'link', 'set', 'dev', left, 'up')
            ip('-n', 'b', 'link', 'set', 'dev', right, 'up')
    except BaseException:
        for ns in reversed(created):
            subprocess.run(['ip', 'netns', 'del', ns], check=False)
        raise
    print('lab up: a:a-out <-> b:b-in, a:a-in <-> b:b-out (no IP/routes)')


def down():
    present = namespaces() & set(NAMES)
    for ns in sorted(present):
        if not owned(ns):
            raise RuntimeError(f'namespace {ns!r} is not owned by lab; refusing to delete')
        pids = ip('netns', 'pids', ns).split()
        if pids:
            raise RuntimeError(f'namespace {ns!r} has running processes; stop them first')
    for ns in sorted(present):
        ip('netns', 'del', ns)
    print('lab down')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('action', choices=('up', 'down'))
    args = parser.parse_args()
    if os.geteuid() != 0:
        parser.exit(1, f'root required: sudo ./setup_netns.py {args.action}\n')
    if not shutil.which('ip'):
        parser.exit(1, 'ip command required (install iproute2)\n')
    try:
        {'up': up, 'down': down}[args.action]()
    except (RuntimeError, OSError, subprocess.CalledProcessError) as exc:
        parser.exit(1, f'lab: {exc}\n')


if __name__ == '__main__':
    main()
