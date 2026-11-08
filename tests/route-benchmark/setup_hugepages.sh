#!/bin/bash
# Reserve the hugepages sdplane needs. Only required once after boot.
set -exu

sudo dpdk-hugepages.py --show
sudo dpdk-hugepages.py --clear
sudo dpdk-hugepages.py --setup 128m
sudo dpdk-hugepages.py --show
