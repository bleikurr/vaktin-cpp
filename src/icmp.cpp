#include "vaktin/icmp.hpp"

#include <chrono>
#include <csignal>
#include <cstdint>
#include <cstring>
#include <iostream>

#include <endian.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "netpp/address.hpp"
#include "vaktin/termcolor.hpp"

#define ICMP_PAYLOAD_OFFSET 8

namespace vaktin {
namespace icmp {
using namespace netpp;

namespace {
// Useless rn
[[maybe_unused]] uint16_t icmp_checksum(std::span<const uint8_t> data) {
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

uint64_t timestamp_now() {
  auto ts = std::chrono::steady_clock::now().time_since_epoch().count();
  return static_cast<std::uint64_t>(ts);
}

double
calculate_rtt_millis(const std::chrono::steady_clock::time_point &ts_return,
                     uint64_t sent) {
  auto rtt = std::chrono::duration<double, std::milli>(
      ts_return - std::chrono::steady_clock::time_point(
                      std::chrono::steady_clock::duration(sent)));

  return rtt.count();
}

} // namespace

volatile sig_atomic_t Ping::stop = 0;
bool Ping::colored = true;

void Ping::sigint_handler(int signal) {
  std::cout << "\rStopping" << std::endl << std::flush;
  Ping::stop = 1;
}

void Ping::disable_color() { Ping::colored = false; }

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

  m_packet.header.icmp_type = ICMP_ECHO;
  m_packet.header.icmp_code = 0;
  m_packet.header.icmp_seq = 1;
  m_packet.header.icmp_id = 1024;
  m_packet.header.icmp_cksum = 0;

  m_address = addr;
  m_interval = interval;
  m_timeout = 1 * 1000; // Sec to milliseconds
  m_count = 0;

  m_ready = true;
}

void Ping::print_status(bool success, uint16_t payload_count, double rtt) {
  if (success && (payload_count != m_count))
    return;

  if (Ping::colored)
    std::cout << (success ? termcolor::GREEN : termcolor::RED);

  std::cout << m_address->name() << ":(" << m_address->ip() << ") - " << m_count
            << ": ";
  if (success) {
    std::cout << std::format("{:.3f}ms", rtt);
  } else {
    std::cout << "timeout";
  }
  std::cout << termcolor::RESET << std::endl;
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

  struct icmp_packet rcvd{};

  address::Address source_addr = address::Address::empty_address();
  m_socket.set_address(m_address.value());
  while (true) {
    m_count++;
    m_packet.payload.count = m_count;
    m_packet.payload.timestamp = timestamp_now();
    std::memcpy(m_packetdata.data(), &m_packet, sizeof(m_packet));

    m_socket.sendto(m_packetdata);
    ready = poll(fds, nfds, m_timeout);
    if (Ping::stop) {
      break;
    }
    if (ready > 0) {
      if (fd.revents & POLLIN) {
        m_socket.recvfrom(m_buffer, source_addr);
        auto ts_return = std::chrono::steady_clock().now();
        memcpy(&rcvd, m_buffer.data(), sizeof(rcvd));
        double rtt = calculate_rtt_millis(ts_return, rcvd.payload.timestamp);

        print_status(true, rcvd.payload.count, rtt);
        sleep(m_interval);
      } else {
        std::cout << "Something happened in the socket";
        std::cout << ", but it wasn't a read" << std::endl;
        break;
      }
    } else if (ready == 0) {
      print_status(false, 0, 0);
    } else {
      std::cerr << "Error listening on socket" << std::endl;
      break;
    }
  }
}

} // namespace icmp
} // namespace vaktin
