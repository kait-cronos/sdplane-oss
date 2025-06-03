#include "sdplane_version_for_test.h"

#include <rte_common.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include <rte_mbuf.h>
#include <rte_mempool.h>
#include <rte_eal.h>

#include <stdio.h>
#include "l2_repeater.h"

int main() {
    printf("Hello, World!\n");

    rte_eal_init(0, NULL);

    l2_repeater(NULL);
}
