#ifndef PROTOCOL_H
#define PROTOCOL_H
#ifdef __cplusplus
extern "C" {
#endif
int protocol_send( int fd, void *data,  int len);

int protocol_recv( int fd, void *data,  int *plen);

#ifdef __cplusplus
}
#endif
#endif // PROTOCOL_H
