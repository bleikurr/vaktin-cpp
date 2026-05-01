#include "icmp.hpp"
#include "network.hpp"

#include <csignal>
#include <iostream>
#include <netinet/ip_icmp.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#define ICMP_PAYLOAD_OFFSET 8

namespace vaktin {
namespace icmp {

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
  Ping::stop = 1;
  std::cout << "\rStopping" << std::endl;
}

void Ping::handle_sigint() { signal(SIGINT, Ping::sigint_handler); }

Ping::Ping(char *address, int interval) {
  network::DNSResult *dnsres = network::dns_lookup(address);
  if (dnsres == nullptr) {
    m_dns = nullptr;
    m_ready = false;
    return;
  }
  std::fill(m_packetdata, m_packetdata + ICMP_LEN, 0);
  std::fill(m_buffer, m_buffer + ICMP_LEN, 0);

  struct icmp *packet = (struct icmp *)m_packetdata;
  packet->icmp_type = ICMP_ECHO;
  packet->icmp_code = 0;
  packet->icmp_seq = 1;
  packet->icmp_id = 1024;
  packet->icmp_cksum = 0;

  m_dns = dnsres;
  m_addrlen = sizeof(sockaddr_in);
  m_interval = interval;
  m_timeout = 1 * 1000; // Sec to milliseconds
  m_count = 0;

  m_sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
  if (m_sockfd < 0) {
    std::cerr << "Error creating socket" << std::endl;
    m_ready = false;
  }

  m_ready = true;
}

void Ping::print_status(bool success) {
  std::cout << m_dns->dns << ": (" << m_dns->ip << ") - " << m_count << " ";
  if (success) {
    std::cout << "Ping!" << std::endl;
  } else {
    std::cout << "NoAnswer" << std::endl;
  }
}

Ping::~Ping() {
  if (m_dns != nullptr)
    dns_free(m_dns);
}

bool Ping::is_ready() { return m_ready; }

void Ping::ping() {
  if (!m_ready) {
    std::cerr << "Ping instance can't ping." << std::endl;
    return;
  }
  socklen_t addrlen = sizeof(sockaddr_storage);
  nfds_t nfds = 1;
  struct pollfd fds[1];
  struct pollfd &fd = fds[0];
  fd.fd = m_sockfd;
  fd.events = POLLIN;
  int ready = 0;

  while (true) {
    sendto(m_sockfd, m_packetdata, ICMP_LEN, 0, m_dns->addr, m_addrlen);
    m_count++;

    ready = poll(fds, nfds, m_timeout);
    if (Ping::stop) {
      break;
    }
    if (ready > 0) {
      if (fd.revents & POLLIN) {
        recvfrom(m_sockfd, m_buffer, 128, 0, m_dns->addr, &addrlen);
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
