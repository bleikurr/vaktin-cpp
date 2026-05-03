#ifndef __VAKTIN_ICMP_HPP

#define __VAKTIN_ICMP_HPP
#include <csignal>
#include <cstdint>
#include <span>
#include <vector>

#include "netpp/address.hpp"
#include "netpp/socket.hpp"

namespace vaktin::icmp {
using namespace netpp;
constexpr std::size_t ICMP_LEN = 64;

// Calculate ICMP checksum of header
uint16_t icmp_checksum(std::span<const uint8_t> header);
int ping(char *address);

class Ping {
public:
  Ping(char *address, int interval);

  bool is_ready();
  void ping();

  static void handle_sigint();

private:
  bool m_ready;
  std::optional<netpp::address::Address> m_address;
  std::vector<std::byte> m_packetdata = std::vector<std::byte>(ICMP_LEN);
  std::vector<std::byte> m_buffer = std::vector<std::byte>(ICMP_LEN);
  int m_interval;
  int m_timeout;
  int m_count;
  sockets::Socket m_socket;

  void print_status(bool success);

  static volatile sig_atomic_t stop;
  static void sigint_handler(int signal);
};

} // namespace vaktin::icmp

#endif
