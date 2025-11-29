
#include "include.h"

#include <rte_flow.h>

#define RTE_FLOW_MAX_PATTERNS 16
#define RTE_FLOW_MAX_ITEMS 16
#define RTE_FLOW_MAX_ACTIONS 4

struct rte_flow_attr attr_incoming = { .ingress = 1 };
struct rte_flow_item flow_pattern[RTE_FLOW_MAX_PATTERNS][RTE_FLOW_MAX_ITEMS];
struct rte_flow_action flow_action[RTE_FLOW_MAX_ACTIONS];

struct rte_flow_item_eth eth_spec;
struct rte_flow_item_eth eth_mask;
struct rte_flow_item_vlan vlan_spec;
struct rte_flow_item_vlan vlan_mask;
struct rte_flow_item_ipv4 ipv4_spec;
struct rte_flow_item_ipv4 ipv4_mask;

CLI_COMMAND2 (set_rte_flow_pattern_ether_any,
              "set rte-flow-pattern <0-15> index <0-15> ether (any|zero)",
              SET_HELP)
{
  struct shell *shell = (struct shell *) context;
  int pattern = 0;
  int index = 0;
  pattern = strtol (argv[2], NULL, 0);
  index = strtol (argv[4], NULL, 0);
  memset (&eth_spec, 0, sizeof (eth_spec));
  memset (&eth_mask, 0, sizeof (eth_mask));
  memset (&flow_pattern[pattern][index], 0, sizeof (struct rte_flow_item));
  flow_pattern[pattern][index].type = RTE_FLOW_ITEM_TYPE_ETH;
  flow_pattern[pattern][index].spec = &eth_spec;
  flow_pattern[pattern][index].mask = &eth_mask;
  return 0;
}

CLI_COMMAND2 (set_rte_flow_pattern_vlan_any,
              "set rte-flow-pattern <0-15> index <0-15> vlan (any|zero)",
              SET_HELP)
{
  struct shell *shell = (struct shell *) context;
  int pattern = 0;
  int index = 0;
  pattern = strtol (argv[2], NULL, 0);
  index = strtol (argv[4], NULL, 0);
  memset (&vlan_spec, 0, sizeof (vlan_spec));
  memset (&vlan_mask, 0, sizeof (vlan_mask));
  memset (&flow_pattern[pattern][index], 0, sizeof (struct rte_flow_item));
  flow_pattern[pattern][index].type = RTE_FLOW_ITEM_TYPE_VLAN;
  flow_pattern[pattern][index].spec = &vlan_spec;
  flow_pattern[pattern][index].mask = &vlan_mask;
  return 0;
}

CLI_COMMAND2 (set_rte_flow_pattern_vlan_id,
              "set rte-flow-pattern <0-15> index <0-15> vlan id <1-4094>",
              SET_HELP)
{
  struct shell *shell = (struct shell *) context;
  int pattern = 0;
  int index = 0;
  int vlan_id;
  pattern = strtol (argv[2], NULL, 0);
  index = strtol (argv[4], NULL, 0);
  vlan_id = strtol (argv[7], NULL, 0);
  vlan_spec.tci = rte_cpu_to_be_16 (vlan_id);
  vlan_mask.tci = rte_cpu_to_be_16 (0x0fff);
  flow_pattern[pattern][index].type = RTE_FLOW_ITEM_TYPE_VLAN;
  flow_pattern[pattern][index].spec = &vlan_spec;
  flow_pattern[pattern][index].mask = &vlan_mask;
  return 0;
}

CLI_COMMAND2 (set_rte_flow_pattern_ipv4_any,
              "set rte-flow-pattern <0-15> index <0-15> ipv4 (any|zero)",
              SET_HELP)
{
  struct shell *shell = (struct shell *) context;
  int pattern = 0;
  int index = 0;
  pattern = strtol (argv[2], NULL, 0);
  index = strtol (argv[4], NULL, 0);
  memset (&ipv4_spec, 0, sizeof (ipv4_spec));
  memset (&ipv4_mask, 0, sizeof (ipv4_mask));
  memset (&flow_pattern[pattern][index], 0, sizeof (struct rte_flow_item));
  flow_pattern[pattern][index].type = RTE_FLOW_ITEM_TYPE_IPV4;
  flow_pattern[pattern][index].spec = &ipv4_spec;
  flow_pattern[pattern][index].mask = &ipv4_mask;
  return 0;
}

CLI_COMMAND2 (set_rte_flow_pattern_ipv4_src_dst,
              "set rte-flow-pattern <0-15> index <0-15> ipv4 (src|dst) A.B.C.D",
              SET_HELP)
{
  struct shell *shell = (struct shell *) context;
  int pattern = 0;
  int index = 0;
  struct in_addr ipv4_addr;
  pattern = strtol (argv[2], NULL, 0);
  index = strtol (argv[4], NULL, 0);
  inet_pton (AF_INET, argv[7], &ipv4_addr);

  if (! strcmp (argv[6], "src"))
    {
      ipv4_spec.hdr.src_addr = ipv4_addr.s_addr;
      ipv4_mask.hdr.src_addr = rte_cpu_to_be_32 (0xffffffff);
    }
  else if (! strcmp (argv[6], "dst"))
    {
      ipv4_spec.hdr.dst_addr = ipv4_addr.s_addr;
      ipv4_mask.hdr.dst_addr = rte_cpu_to_be_32 (0xffffffff);
    }

  flow_pattern[pattern][index].type = RTE_FLOW_ITEM_TYPE_IPV4;
  flow_pattern[pattern][index].spec = &ipv4_spec;
  flow_pattern[pattern][index].mask = &ipv4_mask;
  return 0;
}

CLI_COMMAND2 (set_rte_flow_pattern_ipv4_proto,
              "set rte-flow-pattern <0-15> index <0-15> ipv4 proto <0-255>",
              SET_HELP)
{
  struct shell *shell = (struct shell *) context;
  int pattern = 0;
  int index = 0;
  uint8_t proto;
  pattern = strtol (argv[2], NULL, 0);
  index = strtol (argv[4], NULL, 0);
  proto = strtol (argv[7], NULL, 0);
  ipv4_spec.hdr.next_proto_id = proto;
  ipv4_mask.hdr.next_proto_id = 0xff;
  flow_pattern[pattern][index].type = RTE_FLOW_ITEM_TYPE_IPV4;
  flow_pattern[pattern][index].spec = &ipv4_spec;
  flow_pattern[pattern][index].mask = &ipv4_mask;
  return 0;
}


CLI_COMMAND2 (set_rte_flow_pattern_end,
              "set rte-flow-pattern <0-15> index <0-15> end",
              SET_HELP)
{
  struct shell *shell = (struct shell *) context;
  int pattern = 0;
  int index = 0;
  struct in_addr ipv4_addr;
  pattern = strtol (argv[2], NULL, 0);
  index = strtol (argv[4], NULL, 0);
  memset (&flow_pattern[pattern][index], 0, sizeof (struct rte_flow_item));
  flow_pattern[pattern][index].type = RTE_FLOW_ITEM_TYPE_END;
  return 0;
}

CLI_COMMAND2 (show_rte_flow_pattern,
              "show rte-flow-pattern <0-15>",
              SHOW_HELP)
{
  struct shell *shell = (struct shell *) context;
  int pattern = 0;
  pattern = strtol (argv[2], NULL, 0);
  int i;

  fprintf (shell->terminal, "attr: %p%s", &attr_incoming, shell->NL);
  fprintf (shell->terminal, "action: %p%s", &flow_action[0], shell->NL);

  fprintf (shell->terminal, "eth: spec: %p mask: %p%s",
           &eth_spec, &eth_mask, shell->NL);
  fprintf (shell->terminal, "vlan: spec: %p mask: %p%s",
           &vlan_spec, &vlan_mask, shell->NL);
  fprintf (shell->terminal, "ipv4: spec: %p mask: %p%s",
           &ipv4_spec, &ipv4_mask, shell->NL);

  for (i = 0; i < RTE_FLOW_MAX_ITEMS; i++)
    {
      struct rte_flow_item *item;
      item = &flow_pattern[pattern][i];
      char *typename = NULL;
      switch (item->type)
        {
        case RTE_FLOW_ITEM_TYPE_END: typename = "end"; break;
        case RTE_FLOW_ITEM_TYPE_ETH: typename = "ether"; break;
        case RTE_FLOW_ITEM_TYPE_VLAN: typename = "vlan"; break;
        case RTE_FLOW_ITEM_TYPE_IPV4: typename = "ipv4"; break;
        default: typename = "unknown"; break;
        }
      fprintf (shell->terminal,
               "flow_pattern[%d][%d](%p): type: %d %s spec: %p mask: %p%s",
               pattern, i, item, item->type, typename, item->spec, item->mask,
               shell->NL);
      if (item->type == RTE_FLOW_ITEM_TYPE_END)
        break;
    }
  return 0;
}

//set rte-flow port 0 pattern 1 action queue 1
CLI_COMMAND2 (set_rte_flow_port_pattern_action_queue,
              "set rte-flow port <0-127> pattern <0-15> action queue <0-127>",
              SET_HELP)
{
  struct shell *shell = (struct shell *) context;
  int ret;
  int port_id = 0;
  int pattern = 0;
  int queue_id = 0;
  port_id = strtol (argv[3], NULL, 0);
  pattern = strtol (argv[5], NULL, 0);
  queue_id = strtol (argv[8], NULL, 0);

  struct rte_flow_action_queue queue;
  memset (&queue, 0, sizeof (queue));
  queue.index = queue_id;
  memset (&flow_action, 0, sizeof (flow_action));
  flow_action[0].type = RTE_FLOW_ACTION_TYPE_QUEUE;
  flow_action[0].conf = &queue;
  flow_action[1].type = RTE_FLOW_ACTION_TYPE_END;

  struct rte_flow *flow = NULL;
  struct rte_flow_error error;
  ret = rte_flow_validate (port_id, &attr_incoming,
                           flow_pattern[pattern], flow_action, &error);
  if (ret)
    {
      fprintf (shell->terminal, "rte_flow_validate() failed: %d.%s",
               ret, shell->NL);
      fprintf (shell->terminal, "rte_flow_error: "
               "type: %d obj: %p message: %s%s",
               error.type, error.cause, error.message, shell->NL);
      return -1;
    }
  else
    fprintf (shell->terminal, "rte_flow_validate() succeeded: %d.%s",
             ret, shell->NL);

  flow = rte_flow_create (port_id, &attr_incoming,
                          flow_pattern[pattern], flow_action, &error);
  if (! flow)
    {
      fprintf (shell->terminal, "rte_flow_create() returned: NULL%s",
               shell->NL);
      fprintf (shell->terminal, "rte_flow_error: "
               "type: %d obj: %p message: %s%s",
               error.type, error.cause, error.message, shell->NL);
      return -1;
    }
  else
    fprintf (shell->terminal, "rte_flow_create() succeeded: flow: %p.%s",
             flow, shell->NL);

  return 0;
}

CLI_COMMAND2 (set_rte_flow_port_pattern_action_none_drop,
              "set rte-flow port <0-127> pattern <0-15> action (none|drop)",
              SET_HELP)
{
  struct shell *shell = (struct shell *) context;
  int ret;
  int port_id = 0;
  int pattern = 0;
  port_id = strtol (argv[3], NULL, 0);
  pattern = strtol (argv[5], NULL, 0);

  memset (&flow_action, 0, sizeof (flow_action));
  if (! strcmp (argv[7], "none"))
    {
      flow_action[0].type = RTE_FLOW_ACTION_TYPE_END;
    }
  else if (! strcmp (argv[7], "drop"))
    {
      flow_action[0].type = RTE_FLOW_ACTION_TYPE_DROP;
      flow_action[1].type = RTE_FLOW_ACTION_TYPE_END;
    }

  struct rte_flow *flow = NULL;
  struct rte_flow_error error;
  ret = rte_flow_validate (port_id, &attr_incoming,
                           flow_pattern[pattern], flow_action, &error);
  if (ret)
    {
      fprintf (shell->terminal, "rte_flow_validate() failed: %d.%s",
               ret, shell->NL);
      fprintf (shell->terminal, "rte_flow_error: "
               "type: %d obj: %p message: %s%s",
               error.type, error.cause, error.message, shell->NL);
      return -1;
    }
  else
    fprintf (shell->terminal, "rte_flow_validate() succeeded: %d.%s",
             ret, shell->NL);

  flow = rte_flow_create (port_id, &attr_incoming,
                          flow_pattern[pattern], flow_action, &error);
  if (! flow)
    {
      fprintf (shell->terminal, "rte_flow_create() returned: NULL%s",
               shell->NL);
      fprintf (shell->terminal, "rte_flow_error: "
               "type: %d obj: %p message: %s%s",
               error.type, error.cause, error.message, shell->NL);
      return -1;
    }
  else
    fprintf (shell->terminal, "rte_flow_create() succeeded: flow: %p.%s",
             flow, shell->NL);

  return 0;
}

void
sdplane_rte_flow_cmd_init (struct command_set *cmdset)
{
  INSTALL_COMMAND2 (cmdset, set_rte_flow_pattern_ether_any);
  INSTALL_COMMAND2 (cmdset, set_rte_flow_pattern_vlan_any);
  INSTALL_COMMAND2 (cmdset, set_rte_flow_pattern_vlan_id);
  INSTALL_COMMAND2 (cmdset, set_rte_flow_pattern_ipv4_any);
  INSTALL_COMMAND2 (cmdset, set_rte_flow_pattern_ipv4_src_dst);
  INSTALL_COMMAND2 (cmdset, set_rte_flow_pattern_ipv4_proto);
  INSTALL_COMMAND2 (cmdset, set_rte_flow_pattern_end);
  INSTALL_COMMAND2 (cmdset, show_rte_flow_pattern);

  INSTALL_COMMAND2 (cmdset, set_rte_flow_port_pattern_action_queue);
  INSTALL_COMMAND2 (cmdset, set_rte_flow_port_pattern_action_none_drop);
}

