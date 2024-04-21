#ifndef __L2FWD_CMD_H__
#define __L2FWD_CMD_H__

void show_l2fwd_lcore_one (struct shell *shell, unsigned int rx_lcore_id);
void show_l2fwd_lcore_by_mask (struct shell *shell,
                          bool brief, bool all, uint64_t mask);

#endif /*__L2FWD_CMD_H__*/
