#ifndef __VTY_SHELL_TELNET_H__
#define __VTY_SHELL_TELNET_H__

extern shell_keyfunc_t key_func_escape_1[256];
extern shell_keyfunc_t key_func_escape_2[256];
extern shell_keyfunc_t key_func_iac_1[256];
extern shell_keyfunc_t key_func_iac_2[256];
extern shell_keyfunc_t key_func_subnego[256];

extern shell_keyfunc_t *key_func_orig;

// char *telcmds2str (u_char telnet_cmd);
// char *telopts2str (u_char telnet_opt);

void vty_will_echo (struct shell *shell);
void vty_will_suppress_go_ahead (struct shell *shell);
void vty_dont_linemode (struct shell *shell);
void vty_do_window_size (struct shell *shell);

void vty_shell_keyfunc_escape_1 (struct shell *shell);
void vty_shell_keyfunc_escape_2 (struct shell *shell);
void vty_shell_keyfunc_iac_start (struct shell *shell);
void vty_shell_keyfunc_telnet_opt (struct shell *shell);
void vty_shell_keyfunc_telnet_cmd (struct shell *shell);
void vty_shell_keyfunc_subnego (struct shell *shell);
void vty_shell_keyfunc_sb_start (struct shell *shell);
void vty_shell_keyfunc_sb_end (struct shell *shell);

void shell_telnet_keyfunc_init (struct shell *shell);

#endif /*__VTY_SHELL_TELNET_H__*/
