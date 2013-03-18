
struct tracks
  {
    struct tracks *next;
    struct conn_s *connptr;
    struct request_s *request;
    int fd;
  };

void	track(struct conn_s *connptr, struct request_s *request);
void	untrack(struct conn_s *connptr);
void	track_write(int fd, const void *ptr, int len);
void	track_read(int fd, const void *ptr, int len);

