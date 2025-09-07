# 시스템 구성

**Language:** [English](../en/system-configuration.md) | [Japanese](../ja/system-configuration.md) | [Français](../fr/system-configuration.md) | [中文](../zh/system-configuration.md) | [Deutsch](../de/system-configuration.md) | [Italiano](../it/system-configuration.md) | **한국어**

- **Hugepages**: DPDK를 위한 시스템 hugepages 구성
- **네트워크**: 네트워크 인터페이스 구성을 위한 netplan 사용
- **방화벽**: CLI를 위해 telnet 9882/tcp 포트 필요

**⚠️ CLI에는 인증이 없습니다. localhost에서만 연결을 허용하는 것이 좋습니다 ⚠️**

## Hugepages 구성
```bash
# GRUB 구성 편집
sudo vi /etc/default/grub

# 다음 줄 중 하나 추가:
# 2MB hugepages용 (1536 페이지 = ~3GB):
GRUB_CMDLINE_LINUX="hugepages=1536"

# 또는 1GB hugepages용 (8 페이지 = 8GB):
GRUB_CMDLINE_LINUX="default_hugepagesz=1G hugepagesz=1G hugepages=8"

# GRUB 업데이트 및 재부팅
sudo update-grub
sudo reboot
```

## DPDK IGB 커널 모듈 설치 (선택사항)

vfio-pci와 작동하지 않는 NIC의 경우, 선택적으로 igb_uio를 설치합니다:

```bash
git clone http://dpdk.org/git/dpdk-kmods
cd dpdk-kmods/linux/igb_uio
make
sudo mkdir -p /lib/modules/`uname -r`/extra/dpdk/
sudo cp igb_uio.ko /lib/modules/`uname -r`/extra/dpdk/
echo igb_uio | sudo tee /etc/modules-load.d/igb_uio.conf
```

## DPDK UIO 드라이버 구성

DPDK는 사용자 애플리케이션에서 네트워크 인터페이스에 액세스하기 위해 User I/O (UIO) 드라이버가 필요합니다.

### 표준 NIC

대부분의 표준 네트워크 카드의 경우 세 가지 UIO 드라이버 옵션이 있습니다:

1. **vfio-pci** (권장)
   - 가장 안전하고 현대적인 옵션
   - IOMMU 지원 필요 (Intel VT-d 또는 AMD-Vi)
   - 추가 커널 모듈 컴파일 불필요

2. **uio_pci_generic**
   - Linux 커널에 포함된 일반 UIO 드라이버
   - 추가 설치 불필요
   - vfio-pci에 비해 기능 제한

3. **igb_uio**
   - DPDK 전용 UIO 드라이버
   - 수동 컴파일 및 설치 필요 (위 참조)
   - 구형 하드웨어를 위한 추가 기능 제공

### 분기 드라이버가 있는 NIC

**Mellanox ConnectX 시리즈**와 같은 일부 NIC는 동일한 물리적 포트를 커널 네트워크와 DPDK 애플리케이션에서 동시에 사용할 수 있는 분기 드라이버를 제공합니다. 이러한 NIC의 경우:

- UIO 드라이버 구성 불필요
- NIC는 커널 드라이버에 바인딩된 상태로 유지 가능
- DPDK 애플리케이션이 하드웨어에 직접 액세스 가능

### UIO 드라이버 구성

UIO 드라이버를 구성하려면 다음이 필요합니다:

1. **드라이버 모듈 로드**:
```bash
# vfio-pci용 (BIOS/UEFI 및 커널에서 IOMMU 활성화 필요)
sudo modprobe vfio-pci

# uio_pci_generic용
sudo modprobe uio_pci_generic

# igb_uio용 (설치 후)
sudo modprobe igb_uio
```

2. `/etc/modules-load.d/`에 구성 파일을 생성하여 **부팅 시 드라이버 로드**:
```bash
# vfio-pci 예시
echo "vfio-pci" | sudo tee /etc/modules-load.d/vfio-pci.conf

# uio_pci_generic 예시
echo "uio_pci_generic" | sudo tee /etc/modules-load.d/uio_pci_generic.conf

# igb_uio 예시
echo "igb_uio" | sudo tee /etc/modules-load.d/igb_uio.conf
```

3. DPDK의 `dpdk-devbind.py` 스크립트를 사용하여 **NIC를 UIO 드라이버에 바인딩**:
```bash
# 커널 드라이버에서 언바인드하고 UIO 드라이버에 바인드
sudo dpdk-devbind.py --bind=vfio-pci 0000:01:00.0
sudo dpdk-devbind.py --bind=uio_pci_generic 0000:01:00.1
sudo dpdk-devbind.py --bind=igb_uio 0000:01:00.2
```

**참고**: `0000:01:00.0`을 실제 PCI 장치 주소로 교체하세요. NIC를 식별하려면 `lspci` 또는 `dpdk-devbind.py --status`를 사용하세요.

**대안**: 수동으로 `dpdk-devbind.py`를 사용하는 대신, 구성 파일 시작 부분에서 `set device XX:XX.X driver ...` 명령을 사용하여 `sdplane.conf` 파일에서 장치 바인딩을 구성할 수 있습니다. 이를 통해 sdplane이 시작 시 장치 바인딩을 자동으로 관리할 수 있습니다.