## sdplane tests

This test framework allows you to write unit tests that send some packets to sdplane and expect some packets from specific ports. It uses DPDK's [Ring Based PMD](https://doc.dpdk.org/guides-25.07/nics/pcap_ring.html#rings-based-pmd), so it doesn't require any special hardware including NICs. You can run the tests on a regular Linux machine even in a VM or container.

### How to run

```
cd tests
make test
# You can also run tests inside Docker container
make docker
```

You can see the test logs in `tests/logs` directory.

### How to add test case

1. Add a new file `tests/test_<name>.c`.
2. Add a test case function `test_<name>` in the file.
3. Add a configuration file `tests/config/<name>.conf` for the test case.
4. Call `run_tests` in `main()` function in the file.

```c
char msg[256];

#define RX_PORT  0
#define TX_PORT  1
#define LCORE_ID 1

int
test_l2_repeater (void *arg)
{
  const char *payload = "Hello, R0!";
  size_t payload_len = strlen (payload);

  SEND_TO_PORT (payload, payload_len, RX_PORT, 0);

  EXPECT_FROM_PORT (payload, payload_len, TX_PORT, LCORE_ID);

  return 0;
}

int
main (void)
{
  struct test_config configs[] = { {
      .name = "l2_repeater repeats any data to other ports",
      .test_func = test_l2_repeater,
      .config_path = "config/l2_repeater.conf",
  } };
  return run_tests (configs, sizeof (configs) / sizeof (configs[0]));
}
```

In the test case function, you can use the following macros:

- `SEND_TO_PORT(data, len, port_id, queue_id)`: Send a packet with `data` of length `len` to the specified `port_id` and `queue_id`.
- `EXPECT_FROM_PORT(data, len, port_id, lcore_id)`: Expect to receive a packet with `data` of length `len` from the specified `port_id` and `lcore_id`.

