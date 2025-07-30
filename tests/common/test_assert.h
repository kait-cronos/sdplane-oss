void sdplane_assert_fail (const char *expr, const char *file, int line);

#define SDPLANE_ASSERT(cond)                                    \
  do                                                    \
    {                                                   \
      if (! (cond))                                     \
        {                                               \
          sdplane_assert_fail (#cond, __FILE__, __LINE__); \
          return -1;                                    \
        }                                               \
    }                                                   \
  while (0)
