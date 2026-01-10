
for i in `seq 200 220`; do
    for j in `seq 0 255`; do
      echo ip route add 192.168.$i.$j/32 via 192.168.1.19
    done
done

