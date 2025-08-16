#ifndef TEST_UTIL_H
#define TEST_UTIL_H

#include <rte_ring.h>

#define TEST_PORT_NUM 4
#define TEST_NRXQ_NUM 1
#define TEST_NTXQ_NUM 8
#define TEST_RING_NUM (TEST_PORT_NUM * TEST_NRXQ_NUM * TEST_NTXQ_NUM)
#define TXQ_TO_RING_IDX(port, txq)                                            \
  ((port) * (TEST_NRXQ_NUM + TEST_NTXQ_NUM) + (TEST_NRXQ_NUM + txq))
#define RXQ_TO_RING_IDX(port, rxq)                                            \
  ((port) * (TEST_NRXQ_NUM + TEST_NTXQ_NUM) + (rxq))

extern int test_ports[TEST_PORT_NUM];
extern struct rte_ring *test_rings[TEST_RING_NUM];
extern struct rte_mempool *test_mbuf_pool;

struct test_config
{
  char *name;
  int (*test_func) ();
  char *config_path;
};

int run_tests (struct test_config config[], int count);
int apply_config (const char *config_path);

#define TEST_DEFAULT_DEQ_TIMEOUT_MS 1000

struct rte_mbuf *test_mbuf_from_bytes (const void *data, uint16_t len);

int test_enqueue_packet (struct rte_mbuf *m, int port_id, int rx_queue_idx);

int test_dequeue_packet (int port_id, int tx_queue_idx, struct rte_mbuf **out,
                         int timeout_ms);

int test_mbuf_compare (struct rte_mbuf *a, struct rte_mbuf *b);

int test_expect_packet_equal (struct rte_mbuf *expected, int port_id,
                              int tx_queue_idx, int timeout_ms);

int test_enqueue_bytes (const void *data, uint16_t len, int port_id,
                        int rx_queue_idx);
int test_expect_bytes_equal (const void *data, uint16_t len, int port_id,
                             int tx_queue_idx, int timeout_ms);

#define SEND_TO_PORT(data_ptr, data_len, port_id, queue_idx)                  \
  do                                                                          \
    {                                                                         \
      int __ret =                                                             \
          test_enqueue_bytes ((const void *) (data_ptr),                      \
                              (uint16_t) (data_len), (port_id), (queue_idx)); \
      SDPLANE_ASSERT (__ret >= 0);                                            \
    }                                                                         \
  while (0)

#define EXPECT_FROM_PORT(data_ptr, data_len, port_id, queue_idx)              \
  do                                                                          \
    {                                                                         \
      int __ret = test_expect_bytes_equal (                                   \
          (const void *) (data_ptr), (uint16_t) (data_len), (port_id),        \
          (queue_idx), TEST_DEFAULT_DEQ_TIMEOUT_MS);                          \
      SDPLANE_ASSERT (__ret == 0);                                            \
    }                                                                         \
  while (0)

#endif /* TEST_UTIL_H */
