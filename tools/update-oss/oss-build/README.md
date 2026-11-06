<div align="center">
<img src="sdplane-oss-logo.png" alt="sdplane-oss Logo" width="160">
</div>

# sdplane-oss

sdplane is a software data plane built on DPDK.
It comes with an interactive shell to configure and control
packet processing threads at runtime.

## Install

Packages for Ubuntu 24.04 (amd64) are available on the
[Releases](https://github.com/kait-cronos/sdplane-oss/releases) page.

    sudo apt install ./sdplane_*_amd64.deb

The DPDK runtime libraries are installed from Ubuntu as dependencies.
The configuration files are installed in `/etc/sdplane/`.

sdplane is not started on install. Create `/etc/sdplane/sdplane.conf`
(see `sdplane.conf.sample`), then start it:

    sudo systemctl enable --now sdplane

Connect to the CLI:

    telnet localhost 9882

## Documentation

See [kait-cronos/sdplane-doc](https://github.com/kait-cronos/sdplane-doc)
for installation, configuration, and the user guide.

## License

GNU General Public License v3.0. See [COPYING](COPYING).
