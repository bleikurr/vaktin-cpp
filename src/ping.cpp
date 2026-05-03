#include "vaktin/icmp.hpp"

#include <arpa/inet.h>
#include <cstdlib>
#include <getopt.h>
#include <iostream>

#include <netinet/in.h>

using namespace vaktin;

typedef struct {
  char *address;
  int interval;
  bool help;
  bool valid;
} Options;

Options parse_opts(int argc, char *argv[]) {
  Options opts{.address = nullptr, .interval = 1, .help = false, .valid = true};

  struct option long_opts[] = {{"help", no_argument, 0, 'h'},
                               {"interval", required_argument, 0, 'i'},
                               {0, 0, 0, 0}};
  int c = -1;
  while ((c = getopt_long(argc, argv, "hi:", long_opts, NULL)) != -1) {
    switch (c) {
    case 'h':
      opts.help = true;
      break;
    case 'i':
      int ival = std::atoi(optarg);
      if (ival == 0) {
        opts.valid = false;
        std::cerr << "Interval needs to be a valid integer > 0" << std::endl;
        return opts;
      }
    }
  }

  if (optind == argc) {
    std::cerr << "Missing IP/Address argument" << std::endl;
    opts.valid = false;
  }

  opts.address = argv[optind];
  return opts;
}

typedef vaktin::icmp::Ping Ping;
int main(int argc, char *argv[]) {
  Options opts = parse_opts(argc, argv);
  if (!opts.valid)
    return -1;

  Ping ping(opts.address, opts.interval);
  if (ping.is_ready()) {
    vaktin::icmp::Ping::handle_sigint();
    ping.ping();
  }
  return 0;
}
