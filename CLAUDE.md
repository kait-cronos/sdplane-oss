# CLAUDE.md

必ず日本語で回答してください。

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

sdplane-dev (soft-dplane) is a high-performance DPDK-based software router written in C. It integrates multiple DPDK modules including L2/L3 forwarding, packet generation, and network virtualization capabilities.

## Dependencies

The project requires these libraries to be installed:
- DPDK (Data Plane Development Kit)
- libsdplane-dev (kait-cronos/libsdplane-dev)
- lthread (yasuhironet/lthread from DPDK)
- liburcu-qsbr (userspace RCU library)
- libpcap

## Build Commands

### Standard Build Process
```bash
# Generate build files
aclocal
autoheader
automake -a -c
autoconf

# Build
mkdir build
cd build
../configure
make
```

### Quick Build (using autogen.sh)
```bash
./autogen.sh
mkdir build
cd build
# For debug builds (recommended for development)
CFLAGS="-g -O0" ../configure
# For production builds
# ../configure
make
```

### Build with Packet Generator Support
```bash
./autogen.sh
mkdir build
cd build
CFLAGS="-g -O0" ../configure --enable-pktgen
make
```

### Alternative DPDK Build
```bash
# For DPDK-specific builds
cd sdplane
make -f Makefile.dpdk
```

## Code Style and Formatting

Use the GNU coding style enforced by clang-format:

```bash
# Check code formatting
./style/check_gnu_style.sh check [files...]

# Show formatting differences
./style/check_gnu_style.sh diff [files...]

# Auto-fix formatting
./style/check_gnu_style.sh update [files...]

# Docker-based formatting (if clang-format version issues)
./style/check_gnu_style_docker.sh
```

Requires clang-format version 18.1.3. Files in `module/` directory are ignored by the style checker.

## Architecture Overview

### Core Components

1. **Main Application** (`sdplane/`)
   - Entry point: `main.c` with signal handling and lthread initialization
   - Core router logic: `sdplane.c`
   - CLI and VTY server: `vty_server.c`, `vty_shell.c`

2. **DPDK Modules** (`module/`)
   - **L2FWD**: Layer 2 forwarding functionality
   - **L3FWD**: Layer 3 forwarding with ACL, LPM, FIB, and event handling
   - **PKTGEN**: Packet generation and testing capabilities with comprehensive CLI

3. **Network Virtualization**
   - TAP interface handling: `tap.c`, `tap_handler.c`
   - VLAN switching: `vlan_switch.c`
   - Virtual switch abstraction with ring-based packet passing

4. **Threading and Concurrency**
   - lthread-based cooperative threading
   - Per-lcore worker management
   - Lock-free ring buffers for packet processing

5. **Configuration and Management**
   - Routing Information Base (RIB): `rib.c`, `rib_manager.c`
   - Startup configuration: `startup_config.c`
   - Statistics collection: `stat_collector.c`

### Key Data Structures

- `struct vswitch`: Virtual switch with port abstraction
- `struct vswitch_port`: Port configuration (DPDK, TAP, etc.)
- `struct lcore_worker`: Per-core worker thread management
- RIB structures for routing table management

## Running the Application

```bash
# Run in foreground
./sdplane/sdplane

# Run as systemd service (after installation)
sudo systemctl start sdplane
sudo systemctl enable sdplane

# Connect to CLI
telnet localhost 9882
```

## Configuration Files

### OS Setup Configuration (`etc/`)
- `etc/sdplane.conf.sample`: Main configuration template
- `etc/sdplane.service`: systemd service file
- `etc/60-netplan-sdplane.yaml`: Network configuration
- `etc/iptables-rules.v4`: IPv4 firewall rules
- `etc/iptables-rules.v6`: IPv6 firewall rules
- `etc/sshd_config`: SSH daemon configuration
- `etc/modules-load.d/`: Kernel module loading configuration

### Application Configuration (`example-config/`)
- `example-config/sdplane-nettlp.conf`: NetTLP configuration
- `example-config/sdplane-pktgen.conf`: Packet generator configuration
- `example-config/sdplane-topton.conf`: Topton hardware configuration
- `example-config/sdplane_l2_repeater.conf`: L2 repeater configuration
- `example-config/sdplane_l2fwd.conf`: L2 forwarding configuration
- `example-config/sdplane_l3fwd-lpm.conf`: L3 forwarding with LPM configuration

## Development Notes

- The project uses autotools for build configuration
- DPDK port and lcore management is centralized in `sdplane.c`
- Packet processing uses zero-copy ring buffers between components
- CLI commands are defined using the CLI_COMMAND2 macro system
- Version information is auto-generated from git via `sdplane-version-c.sh`
- Use `--enable-pktgen` configure option to include packet generation capabilities
- Debug builds with `CFLAGS="-g -O0"` are recommended for development

## AI Contribution Tracking

This project uses AI assistance (primarily Claude Code) for development. To maintain transparency and traceability:

### Branch Naming Convention
- **Human-only changes**: `feature/`, `fix/`, `refactor/`, etc.
- **AI-assisted changes**: `claude/`, `ai/`, `ai-assist/`
- **Examples**: 
  - `claude/improve-documentation`
  - `ai/refactor-networking`
  - `ai-assist/add-tests`

### Commit Message Attribution
All AI-assisted commits should include attribution:
```
feat: improve packet processing performance

🤖 Generated with [Claude Code](https://claude.ai/code)

Co-Authored-By: Claude <noreply@anthropic.com>
```

### Code Comments for AI Contributions
Mark significant AI-generated functions:
```c
/* 
 * AI-generated function - Claude Code YYYY-MM-DD
 * Purpose: [brief description]
 * Human review: [reviewer name/status]
 */
```

### Git Configuration for AI Sessions
Set temporary git identity during AI sessions:
```bash
# Before AI session
git config user.name "Your Name + Claude"
git config user.email "yourname+claude@example.com"

# Restore after session
git config user.name "Your Name"  
git config user.email "your@example.com"
```

### Tracking Commands
```bash
# View AI-assisted commits
git log --grep="Claude\|AI\|🤖" --oneline

# Show AI contribution statistics
./tools/ai-stats.sh

# Check current branch AI status
./tools/check-ai-branch.sh
```

### Pull Request Guidelines
- Use the provided PR template
- Clearly indicate AI involvement level
- Document areas requiring human review
- Include testing verification for AI-generated code

## Documentation

Refer to:
- `doc/install-memo-topton.txt`: Installation guide for 10G NIC systems
- `doc/install-memo.txt`: General installation guide for 1G NIC systems
- `doc/nettlp-memo.txt`: NetTLP configuration guide
- `README.md`: Basic project information and requirements
- `.github/pull_request_template.md`: PR template with AI tracking
