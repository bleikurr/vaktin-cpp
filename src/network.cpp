#include "network.hpp"

#include <iostream>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>

namespace vaktin::network {

DNSResult *forward_dns_lookup(char *address) {
  DNSResult *dnsdata = nullptr;
  struct addrinfo *ainfo, *rp;
  int ret = getaddrinfo(address, nullptr, nullptr, &ainfo);
  if (ret != 0) {
    std::cerr << "Address not found: " << address << std::endl;
    return nullptr;
  }

  rp = ainfo;
  while (rp != nullptr) {
    if (rp->ai_family == AF_INET) {
      char *ip_str = new char[INET_ADDRSTRLEN];
      struct sockaddr_storage *addr = new struct sockaddr_storage;
      memcpy(addr, rp->ai_addr, rp->ai_addrlen);
      struct sockaddr_in *addr_in = (sockaddr_in *)addr;
      inet_ntop(AF_INET, &(addr_in->sin_addr), ip_str, INET_ADDRSTRLEN);

      dnsdata = new DNSResult{address, ip_str, (sockaddr *)addr, rp->ai_addrlen,
                              false};
      break;
    }
  }

  freeaddrinfo(ainfo);

  return dnsdata;
}

DNSResult *reverse_dns_lookup(char *ip_addr, in_addr *iaddr) {
  char *dns = new char[64];
  struct sockaddr_in *addr = (sockaddr_in *)new sockaddr_storage;
  addr->sin_family = AF_INET;
  addr->sin_addr = *iaddr;
  addr->sin_port = 0;

  getnameinfo((sockaddr *)addr, sizeof(*addr), dns, 64, nullptr, 0, 0);

  DNSResult *dnsdata =
      new DNSResult{dns, ip_addr, (sockaddr *)addr, sizeof(*addr), true};
  return dnsdata;
}

DNSResult *dns_lookup(char *address) {
  struct in_addr iaddr;
  if (inet_pton(AF_INET, address, &iaddr) == 1) {
    return reverse_dns_lookup(address, &iaddr);
  } else {
    return forward_dns_lookup(address);
  }

  return nullptr;
}

void dns_free(DNSResult *res) {
  if (res->reverse) {
    delete[] res->dns;
  } else {
    delete[] res->ip;
  }
  delete (sockaddr_storage *)res->addr;
  delete res;
}
} // namespace vaktin::network
