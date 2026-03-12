
#include "include.h"

#include <rte_flow.h>

#include <sdplane/shell.h>
#include <sdplane/command.h>
#include <sdplane/command_shell.h>

#define RTE_FLOW_MAX_PATTERNS 16
#define RTE_FLOW_MAX_ITEMS 16
#define RTE_FLOW_MAX_ACTION_LIST 16
#define RTE_FLOW_MAX_ACTIONS 4

#define SET_HELP "set information\n"

uint8_t configured_action_list[RTE_FLOW_MAX_ACTION_LIST] = { 0 };
uint8_t configured_pattern[RTE_FLOW_MAX_PATTERNS] = { 0 };

struct rte_flow_attr attr_incoming = { .ingress = 1 };
struct rte_flow_item flow_pattern[RTE_FLOW_MAX_PATTERNS][RTE_FLOW_MAX_ITEMS];
struct rte_flow_action flow_action[RTE_FLOW_MAX_ACTION_LIST][RTE_FLOW_MAX_ACTIONS];

/* To support arbitrary order of commands, we need to hold/keep
   all items in all patterns in memory. */
struct rte_flow_item_union
{
  union
    {
      struct rte_flow_item_eth eth;
      struct rte_flow_item_vlan vlan;
      struct rte_flow_item_ipv4 ipv4;
    } u;
};
struct rte_flow_item_union
  spec_union[RTE_FLOW_MAX_PATTERNS][RTE_FLOW_MAX_ITEMS];
struct rte_flow_item_union
  mask_union[RTE_FLOW_MAX_PATTERNS][RTE_FLOW_MAX_ITEMS];

struct rte_flow_action_obj_union
{
  union
    {
      struct rte_flow_action_queue queue;
    } u;
};
struct rte_flow_action_obj_union
  action_union[RTE_FLOW_MAX_ACTION_LIST][RTE_FLOW_MAX_ACTIONS];

CLI_COMMAND2 (set_rte_flow_pattern_ether_any,
              "set rte-flow-pattern <0-15> index <0-15> ether (any|zero)",
              SET_HELP)
{
  struct rte_flow_item_eth *eth_spec;
  struct rte_flow_item_eth *eth_mask;
  int pattern = 0;
  int index = 0;
  pattern = strtol (argv[2], NULL, 0);
  index = strtol (argv[4], NULL, 0);
  configured_pattern[pattern]++;
  eth_spec = &spec_union[pattern][index].u.eth;
  eth_mask = &mask_union[pattern][index].u.eth;
  memset (eth_spec, 0, sizeof (struct rte_flow_item_eth));
  memset (eth_mask, 0, sizeof (struct rte_flow_item_eth));
  memset (&flow_pattern[pattern][index], 0, sizeof (struct rte_flow_item));
  flow_pattern[pattern][index].type = RTE_FLOW_ITEM_TYPE_ETH;
  flow_pattern[pattern][index].spec = eth_spec;
  flow_pattern[pattern][index].mask = eth_mask;
  return 0;
}

CLI_COMMAND2 (set_rte_flow_pattern_ether_src_dst,
              "set rte-flow-pattern <0-15> index <0-15> "
              "ether (src|dst) <WORD>",
              SET_HELP)
{
  struct rte_flow_item_eth *eth_spec;
  struct rte_flow_item_eth *eth_mask;
  int pattern = 0;
  int index = 0;
  pattern = strtol (argv[2], NULL, 0);
  index = strtol (argv[4], NULL, 0);
  configured_pattern[pattern]++;
  eth_spec = &spec_union[pattern][index].u.eth;
  eth_mask = &mask_union[pattern][index].u.eth;
  if (! strcmp (argv[6], "src"))
    {
      rte_ether_unformat_addr (argv[7], &eth_spec->src);
      rte_ether_unformat_addr ("FF:FF:FF:FF:FF:FF", &eth_mask->src);
    }
  else if (! strcmp (argv[6], "dst"))
    {
      rte_ether_unformat_addr (argv[7], &eth_spec->dst);
      rte_ether_unformat_addr ("FF:FF:FF:FF:FF:FF", &eth_mask->dst);
    }
  flow_pattern[pattern][index].type = RTE_FLOW_ITEM_TYPE_ETH;
  flow_pattern[pattern][index].spec = eth_spec;
  flow_pattern[pattern][index].mask = eth_mask;
  return 0;
}

CLI_COMMAND2 (set_rte_flow_pattern_ether_type,
              "set rte-flow-pattern <0-15> index <0-15> "
              "ether type <0-0xffff>",
              SET_HELP)
{
  struct rte_flow_item_eth *eth_spec;
  struct rte_flow_item_eth *eth_mask;
  int pattern = 0;
  int index = 0;
  uint16_t type;
  pattern = strtol (argv[2], NULL, 0);
  index = strtol (argv[4], NULL, 0);
  configured_pattern[pattern]++;
  type = (uint16_t) strtol (argv[7], NULL, 0);
  eth_spec = &spec_union[pattern][index].u.eth;
  eth_mask = &mask_union[pattern][index].u.eth;
  eth_spec->type = rte_cpu_to_be_16 (type);
  eth_mask->type = rte_cpu_to_be_16 (0xffff);
  flow_pattern[pattern][index].type = RTE_FLOW_ITEM_TYPE_ETH;
  flow_pattern[pattern][index].spec = eth_spec;
  flow_pattern[pattern][index].mask = eth_mask;
  return 0;
}

CLI_COMMAND2 (set_rte_flow_pattern_vlan_any,
              "set rte-flow-pattern <0-15> index <0-15> vlan (any|zero)",
              SET_HELP)
{
  struct rte_flow_item_vlan *vlan_spec;
  struct rte_flow_item_vlan *vlan_mask;
  int pattern = 0;
  int index = 0;
  pattern = strtol (argv[2], NULL, 0);
  index = strtol (argv[4], NULL, 0);
  configured_pattern[pattern]++;
  vlan_spec = &spec_union[pattern][index].u.vlan;
  vlan_mask = &mask_union[pattern][index].u.vlan;
  memset (vlan_spec, 0, sizeof (struct rte_flow_item_vlan));
  memset (vlan_mask, 0, sizeof (struct rte_flow_item_vlan));
  memset (&flow_pattern[pattern][index], 0, sizeof (struct rte_flow_item));
  flow_pattern[pattern][index].type = RTE_FLOW_ITEM_TYPE_VLAN;
  flow_pattern[pattern][index].spec = vlan_spec;
  flow_pattern[pattern][index].mask = vlan_mask;
  return 0;
}

CLI_COMMAND2 (set_rte_flow_pattern_vlan_id,
              "set rte-flow-pattern <0-15> index <0-15> vlan id <1-4094>",
              SET_HELP)
{
  struct rte_flow_item_vlan *vlan_spec;
  struct rte_flow_item_vlan *vlan_mask;
  int pattern = 0;
  int index = 0;
  int vlan_id;
  pattern = strtol (argv[2], NULL, 0);
  index = strtol (argv[4], NULL, 0);
  configured_pattern[pattern]++;
  vlan_spec = &spec_union[pattern][index].u.vlan;
  vlan_mask = &mask_union[pattern][index].u.vlan;
  vlan_id = strtol (argv[7], NULL, 0);
  vlan_spec->tci = rte_cpu_to_be_16 (vlan_id);
  vlan_mask->tci = rte_cpu_to_be_16 (0x0fff);
  flow_pattern[pattern][index].type = RTE_FLOW_ITEM_TYPE_VLAN;
  flow_pattern[pattern][index].spec = vlan_spec;
  flow_pattern[pattern][index].mask = vlan_mask;
  return 0;
}

CLI_COMMAND2 (set_rte_flow_pattern_ipv4_any,
              "set rte-flow-pattern <0-15> index <0-15> ipv4 (any|zero)",
              SET_HELP)
{
  struct rte_flow_item_ipv4 *ipv4_spec;
  struct rte_flow_item_ipv4 *ipv4_mask;
  int pattern = 0;
  int index = 0;
  pattern = strtol (argv[2], NULL, 0);
  index = strtol (argv[4], NULL, 0);
  configured_pattern[pattern]++;
  ipv4_spec = &spec_union[pattern][index].u.ipv4;
  ipv4_mask = &mask_union[pattern][index].u.ipv4;
  memset (ipv4_spec, 0, sizeof (struct rte_flow_item_ipv4));
  memset (ipv4_mask, 0, sizeof (struct rte_flow_item_ipv4));
  memset (&flow_pattern[pattern][index], 0, sizeof (struct rte_flow_item));
  flow_pattern[pattern][index].type = RTE_FLOW_ITEM_TYPE_IPV4;
  flow_pattern[pattern][index].spec = ipv4_spec;
  flow_pattern[pattern][index].mask = ipv4_mask;
  return 0;
}

CLI_COMMAND2 (set_rte_flow_pattern_ipv4_src_dst,
              "set rte-flow-pattern <0-15> index <0-15> ipv4 (src|dst) A.B.C.D",
              SET_HELP)
{
  struct rte_flow_item_ipv4 *ipv4_spec;
  struct rte_flow_item_ipv4 *ipv4_mask;
  int pattern = 0;
  int index = 0;
  struct in_addr ipv4_addr;
  pattern = strtol (argv[2], NULL, 0);
  index = strtol (argv[4], NULL, 0);
  configured_pattern[pattern]++;
  ipv4_spec = &spec_union[pattern][index].u.ipv4;
  ipv4_mask = &mask_union[pattern][index].u.ipv4;

  inet_pton (AF_INET, argv[7], &ipv4_addr);

  if (! strcmp (argv[6], "src"))
    {
      ipv4_spec->hdr.src_addr = ipv4_addr.s_addr;
      ipv4_mask->hdr.src_addr = rte_cpu_to_be_32 (0xffffffff);
    }
  else if (! strcmp (argv[6], "dst"))
    {
      ipv4_spec->hdr.dst_addr = ipv4_addr.s_addr;
      ipv4_mask->hdr.dst_addr = rte_cpu_to_be_32 (0xffffffff);
    }

  flow_pattern[pattern][index].type = RTE_FLOW_ITEM_TYPE_IPV4;
  flow_pattern[pattern][index].spec = ipv4_spec;
  flow_pattern[pattern][index].mask = ipv4_mask;
  return 0;
}

CLI_COMMAND2 (set_rte_flow_pattern_ipv4_proto,
              "set rte-flow-pattern <0-15> index <0-15> ipv4 proto <0-255>",
              SET_HELP)
{
  struct rte_flow_item_ipv4 *ipv4_spec;
  struct rte_flow_item_ipv4 *ipv4_mask;
  int pattern = 0;
  int index = 0;
  uint8_t proto;
  pattern = strtol (argv[2], NULL, 0);
  index = strtol (argv[4], NULL, 0);
  configured_pattern[pattern]++;
  ipv4_spec = &spec_union[pattern][index].u.ipv4;
  ipv4_mask = &mask_union[pattern][index].u.ipv4;
  proto = strtol (argv[7], NULL, 0);
  ipv4_spec->hdr.next_proto_id = proto;
  ipv4_mask->hdr.next_proto_id = 0xff;
  flow_pattern[pattern][index].type = RTE_FLOW_ITEM_TYPE_IPV4;
  flow_pattern[pattern][index].spec = ipv4_spec;
  flow_pattern[pattern][index].mask = ipv4_mask;
  return 0;
}

CLI_COMMAND2 (set_rte_flow_pattern_end,
              "set rte-flow-pattern <0-15> index <0-15> end",
              SET_HELP)
{
  int pattern = 0;
  int index = 0;
  pattern = strtol (argv[2], NULL, 0);
  index = strtol (argv[4], NULL, 0);
  configured_pattern[pattern]++;
  memset (&flow_pattern[pattern][index], 0, sizeof (struct rte_flow_item));
  flow_pattern[pattern][index].type = RTE_FLOW_ITEM_TYPE_END;
  return 0;
}

void
show_flow_pattern_item_end (struct shell *shell,
                            struct rte_flow_item *item)
{
}

void
show_flow_pattern_item_eth (struct shell *shell,
                            struct rte_flow_item *item)
{
  struct rte_flow_item_eth *eth_spec = item->spec;
  struct rte_flow_item_eth *eth_mask = item->mask;
  char eth_dst[18], eth_src[18];
  rte_ether_format_addr (eth_dst, sizeof (eth_dst), &eth_spec->dst);
  rte_ether_format_addr (eth_src, sizeof (eth_src), &eth_spec->src);
  fprintf (shell->terminal, "    eth spec: dst: %s src: %s type: %#x%s",
           eth_dst, eth_src, rte_be_to_cpu_16 (eth_spec->type),
           shell->NL);
  rte_ether_format_addr (eth_dst, sizeof (eth_dst), &eth_mask->dst);
  rte_ether_format_addr (eth_src, sizeof (eth_src), &eth_mask->src);
  fprintf (shell->terminal, "    eth mask: dst: %s src: %s type: %#x%s",
           eth_dst, eth_src, rte_be_to_cpu_16 (eth_mask->type),
           shell->NL);
}

void
show_flow_pattern_item_vlan (struct shell *shell,
                             struct rte_flow_item *item)
{
  struct rte_flow_item_vlan *vlan_spec = item->spec;
  struct rte_flow_item_vlan *vlan_mask = item->mask;
  fprintf (shell->terminal, "    vlan spec: vlan_id: %d%s",
           RTE_VLAN_TCI_ID (rte_be_to_cpu_16 (vlan_spec->tci)),
           shell->NL);
  fprintf (shell->terminal, "    vlan mask: tci_mask: %#x%s",
           rte_be_to_cpu_16 (vlan_mask->tci),
           shell->NL);
}

void
show_flow_pattern_item_ipv4 (struct shell *shell,
                             struct rte_flow_item *item)
{
  struct rte_flow_item_ipv4 *ipv4_spec = item->spec;
  struct rte_flow_item_ipv4 *ipv4_mask = item->mask;
  char ipv4_src[16], ipv4_dst[16];
  inet_ntop (AF_INET, &ipv4_spec->hdr.src_addr,
             ipv4_src, sizeof (ipv4_src));
  inet_ntop (AF_INET, &ipv4_spec->hdr.dst_addr,
             ipv4_dst, sizeof (ipv4_dst));
  fprintf (shell->terminal, "    ipv4 spec: proto: %d src: %s dst: %s%s",
           ipv4_spec->hdr.next_proto_id, ipv4_src, ipv4_dst, shell->NL);
  fprintf (shell->terminal, "    ipv4 mask: proto: %#x src: %#x dst: %#x%s",
           ipv4_mask->hdr.next_proto_id,
           rte_be_to_cpu_32 (ipv4_mask->hdr.src_addr),
           rte_be_to_cpu_32 (ipv4_mask->hdr.dst_addr),
           shell->NL);
}

CLI_COMMAND2 (show_rte_flow_pattern,
              "show rte-flow pattern (|<0-15>)",
              SHOW_HELP)
{
  struct shell *shell = (struct shell *) context;
  int i, j;
  int pattern = -1;

  if (argc > 3)
    pattern = strtol (argv[3], NULL, 0);

  for (i = 0; i < RTE_FLOW_MAX_PATTERNS; i++)
    {
      if (! configured_pattern[i])
        continue;
      if (pattern >= 0 && pattern != i)
        continue;
      fprintf (shell->terminal, "flow_pattern[%d]:%s", i, shell->NL);
      for (j = 0; j < RTE_FLOW_MAX_ITEMS; j++)
        {
          struct rte_flow_item *item;
          item = &flow_pattern[i][j];
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
                   "  item[%d]: type: %d %s%s",
                   j, item->type, typename, shell->NL);
          switch (item->type)
            {
            case RTE_FLOW_ITEM_TYPE_END:
              show_flow_pattern_item_end (shell, item);
              break;

            case RTE_FLOW_ITEM_TYPE_ETH:
              show_flow_pattern_item_eth (shell, item);
              break;

            case RTE_FLOW_ITEM_TYPE_VLAN:
              show_flow_pattern_item_vlan (shell, item);
              break;

            case RTE_FLOW_ITEM_TYPE_IPV4:
              show_flow_pattern_item_ipv4 (shell, item);
              break;

            default:
              fprintf (shell->terminal, "unknown.%s", shell->NL);
              break;
            }
          if (item->type == RTE_FLOW_ITEM_TYPE_END)
            break;
        }
    }
  return 0;
}

CLI_COMMAND2 (set_rte_flow_action_queue,
              "set rte-flow action <0-15> index <0-15> queue <0-127>",
              SET_HELP)
{
  int action_list_id = 0;
  int action_index = 0;
  uint16_t queue_id = 0;
  struct rte_flow_action_queue *queue;
  struct rte_flow_action *action;
  action_list_id = strtol (argv[3], NULL, 0);
  action_index = strtol (argv[5], NULL, 0);
  configured_action_list[action_list_id]++;
  queue_id = (uint16_t) strtol (argv[7], NULL, 0);
  queue = &action_union[action_list_id][action_index].u.queue;
  memset (queue, 0, sizeof (struct rte_flow_action_queue));
  queue->index = queue_id;

  action = &flow_action[action_list_id][action_index];
  memset (action, 0, sizeof (struct rte_flow_action));
  action->type = RTE_FLOW_ACTION_TYPE_QUEUE;
  action->conf = queue;
  return 0;
}

CLI_COMMAND2 (set_rte_flow_action_drop_end,
              "set rte-flow action <0-15> index <0-15> (drop|end)",
              SET_HELP)
{
  int action_list_id = 0;
  int action_index = 0;
  struct rte_flow_action *action;
  action_list_id = strtol (argv[3], NULL, 0);
  action_index = strtol (argv[5], NULL, 0);
  configured_action_list[action_list_id]++;
  action = &flow_action[action_list_id][action_index];
  memset (action, 0, sizeof (struct rte_flow_action));
  if (! strcmp (argv[6], "drop"))
    action->type = RTE_FLOW_ACTION_TYPE_DROP;
  else if (! strcmp (argv[6], "end"))
    action->type = RTE_FLOW_ACTION_TYPE_END;
  return 0;
}

CLI_COMMAND2 (show_rte_flow_action,
              "show rte-flow action",
              SHOW_HELP)
{
  struct shell *shell = (struct shell *) context;
  struct rte_flow_action *action;
  char *action_name = "none";
  int i, j;
  for (i = 0; i < RTE_FLOW_MAX_ACTION_LIST; i++)
    {
      if (! configured_action_list[i])
        continue;
      for (j = 0; j < RTE_FLOW_MAX_ACTIONS; j++)
        {
          action = &flow_action[i][j];
          switch (action->type)
            {
            case RTE_FLOW_ACTION_TYPE_QUEUE:
              action_name = "queue";
              break;
            case RTE_FLOW_ACTION_TYPE_DROP:
              action_name = "drop";
              break;
            case RTE_FLOW_ACTION_TYPE_END:
              action_name = "end";
              break;
            default:
              action_name = "unknown";
              break;
            }
          fprintf (shell->terminal, "flow_action[%d][%d].type: %d (%s)%s",
                   i, j, action->type, action_name, shell->NL);
          if (action->type == RTE_FLOW_ACTION_TYPE_END)
            break;
        }
    }
  return 0;
}

//set rte-flow port 0 pattern 0 action 0
CLI_COMMAND2 (set_rte_flow_port_pattern_action,
              "set rte-flow port <0-127> pattern <0-15> action <0-15>",
              SET_HELP)
{
  struct shell *shell = (struct shell *) context;
  int ret;
  int port_id = 0;
  int pattern = 0;
  int action_list_id = 0;
  port_id = strtol (argv[3], NULL, 0);
  pattern = strtol (argv[5], NULL, 0);
  action_list_id = strtol (argv[7], NULL, 0);

  struct rte_flow *flow = NULL;
  struct rte_flow_error error;
  ret = rte_flow_validate (port_id, &attr_incoming, flow_pattern[pattern],
                           flow_action[action_list_id], &error);
  if (ret)
    {
      fprintf (shell->terminal, "rte_flow_validate() failed: %d.%s",
               ret, shell->NL);
      fprintf (shell->terminal, "rte_flow_error: "
               "type: %d obj: %p message: %s%s",
               error.type, error.cause, error.message, shell->NL);
      return 0;
    }
  else
    fprintf (shell->terminal, "rte_flow_validate() succeeded: %d.%s",
             ret, shell->NL);

  flow = rte_flow_create (port_id, &attr_incoming, flow_pattern[pattern],
                           flow_action[action_list_id], &error);
  if (! flow)
    {
      fprintf (shell->terminal, "rte_flow_create() returned: NULL%s",
               shell->NL);
      fprintf (shell->terminal, "rte_flow_error: "
               "type: %d obj: %p message: %s%s",
               error.type, error.cause, error.message, shell->NL);
      return 0;
    }
  else
    fprintf (shell->terminal, "rte_flow_create() succeeded: flow: %p.%s",
             flow, shell->NL);

  return 0;
}

void
rte_flow_cmd_init (struct command_set *cmdset)
{
  INSTALL_COMMAND2 (cmdset, set_rte_flow_pattern_ether_any);
  INSTALL_COMMAND2 (cmdset, set_rte_flow_pattern_ether_src_dst);
  INSTALL_COMMAND2 (cmdset, set_rte_flow_pattern_ether_type);
  INSTALL_COMMAND2 (cmdset, set_rte_flow_pattern_vlan_any);
  INSTALL_COMMAND2 (cmdset, set_rte_flow_pattern_vlan_id);
  INSTALL_COMMAND2 (cmdset, set_rte_flow_pattern_ipv4_any);
  INSTALL_COMMAND2 (cmdset, set_rte_flow_pattern_ipv4_src_dst);
  INSTALL_COMMAND2 (cmdset, set_rte_flow_pattern_ipv4_proto);
  INSTALL_COMMAND2 (cmdset, set_rte_flow_pattern_end);
  INSTALL_COMMAND2 (cmdset, show_rte_flow_pattern);

  INSTALL_COMMAND2 (cmdset, set_rte_flow_action_queue);
  INSTALL_COMMAND2 (cmdset, set_rte_flow_action_drop_end);
  INSTALL_COMMAND2 (cmdset, show_rte_flow_action);

  INSTALL_COMMAND2 (cmdset, set_rte_flow_port_pattern_action);
}
