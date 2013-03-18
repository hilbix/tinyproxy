
#include "track.h"
#include "utils.h"
#include "conns.h"

struct tracks *tracker = 0;

void
track(struct conn_s *connptr, struct request_s *request)
{
  struct tracks *t;
  int fd;
  char name[PATH_MAX];
  char timebuf[30];
  time_t tim;

  t = (struct tracks *)safemalloc(sizeof *t);
  if (!t)
    return;

  tim = time (NULL);
  strftime(timebuf, sizeof (timebuf), "%Y%m%d-%H%M%S", gmtime(&tim));
  snprintf(name, sizeof name, "track/%s.%05ld.dump", timebuf, (long)getpid());

  fd = create_file_safely(name, 0);
  if (fd<0)
    {
      safefree(t);
      return;
    }

  t -> connptr = connptr;
  t -> request = request;
  t -> fd      = fd;
  t -> next = tracker;
  tracker   = t;
}

void
untrack(struct conn_s *connptr)
{
  struct tracks *t, **tt;

  for (tt = &tracker; (t= *tt)!=0; tt = &t->next)
    if (t->connptr == connptr)
      {
        *tt = t->next;
        t->connptr = 0;
        safefree(t);
        return;
      }
}

static struct tracks *
gettrack(int fd)
{
  struct tracks *t;

  for (t=tracker; t; t=t->next)
    if (t->connptr->server_fd == fd)
      return t;
  return 0;
}

static void
writer(int fd, const char *ptr, int len)
{
  while (len>0)
    {
      int put;

      if ((put=write(fd, ptr, len))<0)
	return;
      ptr += put;
      len -= put;
    }
}

static void
writeout(int fd, const void *ptr, int len, char direction)
{
  struct tracks *t;
  char cnt[20];
  
  t = gettrack(fd);
  if (!t)
    return;

  snprintf(cnt, sizeof cnt, "\n%c %d\n", direction, len);
  writer(t->fd, cnt, strlen(cnt));
  writer(t->fd, (const char *)ptr, len);
}


void
track_write(int fd, const void *ptr, int len)
{
  writeout(fd, ptr, len, '>');
}

void
track_read(int fd, const void *ptr, int len)
{
  writeout(fd, ptr, len, '<');
}

