#include <stdio.h>
#include <stdlib.h>

void
sdplane_assert_fail (const char *expr, const char *file, int line)
{
  fprintf (stderr, "Assertion failed: %s (%s:%d)\n", expr, file, line);
  fflush (stderr);
}
