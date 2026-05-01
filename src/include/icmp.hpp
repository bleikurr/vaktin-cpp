#ifndef __VAKTIN_ICMP_HPP

#define __VAKTIN_ICMP_HPP
#include <csignal>
#include <cstdint>
#include <span>

#include "network.hpp"

#define ICMP_LEN 64

namespace vaktin::icmp {
// Calculate ICMP checksum of header
uint16_t icmp_checksum(std::span<const uint8_t> header);
int ping(char *address);

class Ping {
public:
  Ping(char *address, int interval);
  ~Ping();

  bool is_ready();
  void ping();

  static void handle_sigint();

private:
  bool m_ready;
  network::DNSResult *m_dns;
  uint8_t m_packetdata[ICMP_LEN];
  socklen_t m_addrlen;
  uint8_t m_buffer[ICMP_LEN];
  int m_interval;
  int m_timeout;
  int m_sockfd;
  int m_count;

  void print_status(bool success);

  static volatile sig_atomic_t stop;
  static void sigint_handler(int signal);
};

} // namespace vaktin::icmp

#endif
