
for i in `seq 41 100`; do
    for j in `seq 0 255`; do
      echo ip route add 200.0.$i.$j/32 via 200.0.0.1 dev lo
      sudo ip route add 200.0.$i.$j/32 via 200.0.0.1 dev lo
    done
done

