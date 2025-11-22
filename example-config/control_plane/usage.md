# 事前準備
- frrのインストール

# 何をするためのものか
- router.cを動作させる際にrouter_if(tapデバイス)経由でコントロールプレーンの動作をLinux kernelに委譲するためのもの．
- frrを使用してOSPFを動作させ，経路情報をsdplane側で取得してL3フォワーディング

# 使用方法
1. sdplane起動
2. manage_control_plane.sh実行 (sh ./manage_control_plane.sh up)
3. 対向のホスト(sdplaneと対向ホストそれぞれ物理NICが2つ必要)でnamespace/simple_router_ns.sh実行

# 動作確認
[動作確認用トポロジ](https://github.com/kait-cronos/sdplane-dev/blob/feature/router/example-config/control_plane/topology.png)

## sdplane上でrouter_ifの情報を取得

```
vty[0]> show rib router-if
router interface configurations:
vswitch[1]: router interface configured
  tap_name: rif1
  MAC: 5E:7D:AF:B8:53:1A, IPv4: 198.18.1.1, IPv6: fd00:1::1
  link local: fe80::7812:e3ff:fea0:168c
  sockfd: 58, tap_ring_id: 1
  ring_up: 0x101876900, ring_dn: 0x101876440
vswitch[2]: router interface configured
  tap_name: rif2
  MAC: E2:A1:D3:2E:E0:56, IPv4: 198.18.2.1, IPv6: fd00:2::1
  link local: fe80::6829:7dff:fe46:5f0b
  sockfd: 5, tap_ring_id: 2
  ring_up: 0x101a74900, ring_dn: 0x101a74440
```

## frr

```
sdplane# show ip route
IPv4 unicast VRF default:
K>* 0.0.0.0/0 [0/0] via 192.168.26.1, enx00249b651913, weight 1, 00:01:05
O>* 10.1.0.10/32 [110/10] via 198.18.1.10, rif1, weight 1, 00:00:15
O>* 10.2.0.10/32 [110/10] via 198.18.2.10, rif2, weight 1, 00:00:20
C>* 192.168.26.0/24 is directly connected, enx00249b651913, weight 1, 00:01:05
L>* 192.168.26.51/32 is directly connected, enx00249b651913, weight 1, 00:01:05
O   198.18.1.0/24 [110/10] is directly connected, rif1, weight 1, 00:01:05
C>* 198.18.1.0/24 is directly connected, rif1, weight 1, 00:01:05
L>* 198.18.1.1/32 is directly connected, rif1, weight 1, 00:01:05
O   198.18.2.0/24 [110/10] is directly connected, rif2, weight 1, 00:01:05
C>* 198.18.2.0/24 is directly connected, rif2, weight 1, 00:01:05
L>* 198.18.2.1/32 is directly connected, rif2, weight 1, 00:01:05
```

```
sdplane# show ipv6 route
IPv6 unicast VRF default:
K>* ::/0 [0/512] via fe80::9af1:99ff:fe2b:52bc, enx00249b651913, weight 1, 00:01:26
O>* 2001:1::10/128 [110/20] via fe80::3a05:25ff:fe32:2417, rif1, weight 1, 00:00:36
O>* 2001:2::10/128 [110/20] via fe80::3a05:25ff:fe32:2416, rif2, weight 1, 00:00:36
C>* 2001:ce8:116:99c9::/64 [0/512] is directly connected, enx00249b651913, weight 1, 00:01:26
L>* 2001:ce8:116:99c9:224:9bff:fe65:1913/128 is directly connected, enx00249b651913, weight 1, 00:01:26
O   fd00:1::/64 [110/10] is directly connected, rif1, weight 1, 00:00:41
C>* fd00:1::/64 is directly connected, rif1, weight 1, 00:01:25
K * fd00:1::/64 [0/256] is directly connected, rif1, weight 1, 00:01:26
L>* fd00:1::1/128 is directly connected, rif1, weight 1, 00:01:25
O   fd00:2::/64 [110/10] is directly connected, rif2, weight 1, 00:00:36
C>* fd00:2::/64 is directly connected, rif2, weight 1, 00:01:25
K * fd00:2::/64 [0/256] is directly connected, rif2, weight 1, 00:01:26
L>* fd00:2::1/128 is directly connected, rif2, weight 1, 00:01:25
C * fe80::/64 is directly connected, rif2, weight 1, 00:01:26
C * fe80::/64 is directly connected, rif1, weight 1, 00:01:26
C * fe80::/64 is directly connected, cif2, weight 1, 00:01:26
C * fe80::/64 is directly connected, cif1, weight 1, 00:01:26
C * fe80::/64 is directly connected, tailscale0, weight 1, 00:01:26
C>* fe80::/64 is directly connected, enx00249b651913, weight 1, 00:01:26
```

## 疎通確認

r1(lo) -> r2(lo)
```
# ping 10.2.0.10 -c 5
PING 10.2.0.10 (10.2.0.10) 56(84) bytes of data.
64 bytes from 10.2.0.10: icmp_seq=1 ttl=63 time=0.272 ms
64 bytes from 10.2.0.10: icmp_seq=2 ttl=63 time=0.426 ms
64 bytes from 10.2.0.10: icmp_seq=3 ttl=63 time=0.486 ms
64 bytes from 10.2.0.10: icmp_seq=4 ttl=63 time=0.452 ms
64 bytes from 10.2.0.10: icmp_seq=5 ttl=63 time=0.363 ms

--- 10.2.0.10 ping statistics ---
5 packets transmitted, 5 received, 0% packet loss, time 4134ms
rtt min/avg/max/mdev = 0.272/0.399/0.486/0.075 ms
```

r1(lo) -> r2(lo)
```
# ping 2001:2::10 -c 5
PING 2001:2::10 (2001:2::10) 56 data bytes
64 bytes from 2001:2::10: icmp_seq=1 ttl=63 time=0.212 ms
64 bytes from 2001:2::10: icmp_seq=2 ttl=63 time=0.395 ms
64 bytes from 2001:2::10: icmp_seq=3 ttl=63 time=0.432 ms
64 bytes from 2001:2::10: icmp_seq=4 ttl=63 time=0.401 ms
64 bytes from 2001:2::10: icmp_seq=5 ttl=63 time=0.614 ms

--- 2001:2::10 ping statistics ---
5 packets transmitted, 5 received, 0% packet loss, time 4093ms
rtt min/avg/max/mdev = 0.212/0.410/0.614/0.127 ms
```
