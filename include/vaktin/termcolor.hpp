#ifndef __VAKTIN_COLORS_HPP
#define __VAKTIN_COLORS_HPP
#include <string>
namespace termcolor {

constexpr std::string GREEN = "\033[32m";
constexpr std::string RED = "\033[31m";
constexpr std::string YELLOW = "\033[33m";
constexpr std::string BLUE = "\033[34m";

constexpr std::string RESET = "\033[0m";
constexpr std::string BOLD = "\033[1m";
} // namespace termcolor
#endif
