#include "sys/time.h"

static char *tv_fmt(struct timeval *tvp, char *buf)
{
  sprintf(buf, "%ld.%06ld", tvp->tv_sec, tvp->tv_usec);
  return buf;
}

static char *tv_now(char *buf)
{
  struct timeval tv;
  gettimeofday(&tv, 0);
  return tv_fmt(&tv, buf);
}
