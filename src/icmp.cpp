#include "vaktin/icmp.hpp"

#include <csignal>
#include <iostream>
#include <netinet/ip_icmp.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#define ICMP_PAYLOAD_OFFSET 8

namespace vaktin {
namespace icmp {

using namespace netpp;
// Useless rn
uint16_t icmp_checksum(std::span<const uint8_t> data) {
  int i = 0;
  size_t len = data.size();
  uint32_t sum = 0;

  while (len > 1) {
    sum += static_cast<uint16_t>(data[i] << 8) | data[i + 1];
    i += 2;
    len -= 2;
  }
  if (len == 1) {
    sum += static_cast<uint16_t>(data[i] << 8);
  }

  while ((sum >> 16) > 0)
    sum = (sum & 0xFFFF) + (sum >> 16);

  return static_cast<uint16_t>(~sum);
}

volatile sig_atomic_t Ping::stop = 0;

void Ping::sigint_handler(int signal) {
  std::cout << "\rStopping" << std::endl << std::flush;
  Ping::stop = 1;
}

void Ping::handle_sigint() { signal(SIGINT, Ping::sigint_handler); }

Ping::Ping(char *address, int interval)
    : m_socket(sockets::Socket::ClientSocket(sockets::IP4, sockets::DATAGRAM,
                                             "icmp")) {
  using namespace address;
  std::optional<Address> addr = Address::get_address(address);
  if (!addr) {
    m_address = std::nullopt;
    m_ready = false;
    return;
  }

  struct icmp *packet = reinterpret_cast<struct icmp *>(m_packetdata.data());
  packet->icmp_type = ICMP_ECHO;
  packet->icmp_code = 0;
  packet->icmp_seq = 1;
  packet->icmp_id = 1024;
  packet->icmp_cksum = 0;

  m_address = addr;
  m_interval = interval;
  m_timeout = 1 * 1000; // Sec to milliseconds
  m_count = 0;

  m_ready = true;
}

void Ping::print_status(bool success) {
  std::cout << m_address->name() << ": (" << m_address->ip() << ") - "
            << m_count << " ";
  if (success) {
    std::cout << "Ping!" << std::endl;
  } else {
    std::cout << "NoAnswer" << std::endl;
  }
}

bool Ping::is_ready() { return m_ready; }

void Ping::ping() {
  if (!m_ready) {
    std::cerr << "Ping instance can't ping." << std::endl;
    return;
  }
  nfds_t nfds = 1;
  struct pollfd fds[1];
  struct pollfd &fd = fds[0];
  fd.fd = m_socket.sockfd();
  fd.events = POLLIN;
  int ready = 0;

  m_socket.set_address(m_address.value());
  while (true) {
    m_socket.sendto(m_packetdata);
    m_count++;

    ready = poll(fds, nfds, m_timeout);
    if (Ping::stop) {
      break;
    }
    if (ready > 0) {
      if (fd.revents & POLLIN) {
        m_socket.recvfrom(m_buffer);
        print_status(true);
        sleep(m_interval);
      } else {
        std::cout << "Something happened in the socket";
        std::cout << ", but it wasn't a read" << std::endl;
        break;
      }
    } else if (ready == 0) {
      print_status(false);
    } else {
      std::cerr << "Error listening on socket" << std::endl;
      break;
    }
  }
}

} // namespace icmp
} // namespace vaktin
