#if !defined MISC_ESCAPE_H
#define MISC_ESCAPE_H

#include "compiler.h"

#include "logger.h"

#define LOGGER_MODULE "escape"
#define LOGGER_TARGET (kLogTarget_Filesystem | kLogTarget_Stdout)
#define misc_alarm_error(...)   log__save(LOGGER_MODULE, kLogLevel_Error, LOGGER_TARGET, __VA_ARGS__)
#define misc_alarm_info(...)    log__save(LOGGER_MODULE, kLogLevel_Info, LOGGER_TARGET, __VA_ARGS__)
#define misc_alarm_warning(...) log__save(LOGGER_MODULE, kLogLevel_Warning, LOGGER_TARGET, __VA_ARGS__)
#define misc_alarm_trace(...)   log__save(LOGGER_MODULE, kLogLevel_Info, kLogTarget_Filesystem, __VA_ARGS__ )

struct misc_endpoint
{
    DDN_IPV4(host); /*char host[16];*/
    unsigned short port;
    unsigned int inet;
};

extern int misc_build_endpoint(const char *str, struct misc_endpoint *endpoint);
extern struct misc_endpoint *misc_duplicate_endpoint(struct misc_endpoint *duplicate, const struct misc_endpoint *source);

#endif
