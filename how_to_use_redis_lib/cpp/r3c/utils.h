// Writed by yijian (eyjian@qq.com)
#ifndef REDIS_CLUSTER_CLIENT_UTILS_H
#define REDIS_CLUSTER_CLIENT_UTILS_H

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4200)
#include <hiredis.h>
#pragma warning(pop)
#else
#include <hiredis/hiredis.h>
#include <sys/time.h>
#include <unistd.h>
#endif

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <string>
#include <vector>

#ifdef __GNUC__
#  define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#else
#  define UNUSED(x) UNUSED_ ## x
#endif


#define PRINT_COLOR_NONE         "\033[m"
#define PRINT_COLOR_RED          "\033[0;32;31m"
#define PRINT_COLOR_YELLOW       "\033[1;33m"
#define PRINT_COLOR_BLUE         "\033[0;32;34m"
#define PRINT_COLOR_GREEN        "\033[0;32;32m"
#define PRINT_COLOR_WHITE        "\033[1;37m"
#define PRINT_COLOR_CYAN         "\033[0;36m"
#define PRINT_COLOR_PURPLE       "\033[0;35m"
#define PRINT_COLOR_BROWN        "\033[0;33m"
#define PRINT_COLOR_DARY_GRAY    "\033[1;30m"
#define PRINT_COLOR_LIGHT_RED    "\033[1;31m"
#define PRINT_COLOR_LIGHT_GREEN  "\033[1;32m"
#define PRINT_COLOR_LIGHT_BLUE   "\033[1;34m"
#define PRINT_COLOR_LIGHT_CYAN   "\033[1;36m"
#define PRINT_COLOR_LIGHT_PURPLE "\033[1;35m"
#define PRINT_COLOR_LIGHT_GRAY   "\033[0;37m"

std::ostream& operator <<(std::ostream& os, const struct redisReply& redis_reply);

namespace r3c {
    extern void null_log_write(const char* UNUSED(format), ...) __attribute__((format(printf, 1, 2))); // Discard log
    extern void r3c_log_write(const char* format, ...) __attribute__((format(printf, 1, 2))); // Ouput log to stdout

    extern int keyHashSlot(const char *key, size_t keylen);
    extern int parse_nodes(std::vector<std::pair<std::string, uint16_t> >* nodes, const std::string& nodes_string);
    extern bool parse_node_string(const std::string& node_string, std::string* ip, uint16_t* port);
    extern void parse_slot_string(const std::string& slot_string, int* start_slot, int* end_slot);
    extern bool parse_moved_string(const std::string& moved_string, std::pair<std::string, uint16_t>* node);
    extern uint64_t get_random_number(uint64_t base);

} // namespace r3c {
#endif // REDIS_CLUSTER_CLIENT_UTILS_H
