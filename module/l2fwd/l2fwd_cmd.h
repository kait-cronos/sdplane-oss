#ifndef __L2FWD_CMD_H__
#define __L2FWD_CMD_H__

EXTERN_COMMAND (l2fwd_init);

void show_l2fwd_lcore_one (struct shell *shell, unsigned int rx_lcore_id);
void show_l2fwd_lcore_by_mask (struct shell *shell,
                          bool brief, bool all, uint64_t mask);

void l2fwd_cmd_init (struct command_set *cmdset);

#endif /*__L2FWD_CMD_H__*/
