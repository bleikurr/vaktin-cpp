#ifndef __VAKTIN_NETWORK_HPP
#define __VAKTIN_NETWORK_HPP

#include <netdb.h>

namespace vaktin::network {
typedef struct {
  char *dns;
  char *ip;
  struct sockaddr *addr;
  socklen_t addrlen;
  bool reverse;
} DNSResult;

DNSResult *dns_lookup(char *address);
void dns_free(DNSResult *res);
} // namespace vaktin::network

#endif
