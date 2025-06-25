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

struct test_config
{
  char *name;
  int (*test_f) ();
  char *config_path;
};

int run_test (struct test_config *config);
