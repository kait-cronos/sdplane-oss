#ifndef __DEBUG_BACKTRACE_H__
#define __DEBUG_BACKTRACE_H__

#ifdef HAVE_EXECINFO_H
#include <execinfo.h>
#endif

#define BACKTRACE_FRAME_SIZE 128
static inline __attribute__ ((always_inline)) void
debug_backtrace ()
{
  int nptrs;
  void *frames[BACKTRACE_FRAME_SIZE];
  char **strings;
  int i;

  nptrs = backtrace (frames, BACKTRACE_FRAME_SIZE);
  DEBUG_LOG (DEFAULT, BACKTRACE, "backtrace frames: %d", nptrs);

  strings = backtrace_symbols (frames, nptrs);
  if (! strings)
    {
      DEBUG_LOG (DEFAULT, BACKTRACE, "backtrace_symbols: null.");
      return;
    }

  for (i = 0; i < nptrs; i++)
    {
      DEBUG_LOG (DEFAULT, LOGGING, "%s", strings[i]);
    }

  free (strings);
}

#endif /*__DEBUG_BACKTRACE_H__*/
