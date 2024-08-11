#ifndef __VTY_SHELL_TELNET_H__
#define __VTY_SHELL_TELNET_H__

void vty_will_echo (struct shell *shell);
void vty_will_suppress_go_ahead (struct shell *shell);
void vty_dont_linemode (struct shell *shell);
void vty_do_window_size (struct shell *shell);

#endif /*__VTY_SHELL_TELNET_H__*/
