#ifndef SGL_LOG_INCLUDED
#define SGL_LOG_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>

typedef enum {
    MW_P_ERROR = 0,
    MW_P_WARNING = 1,
    MW_P_INFO = 2,
    MW_P_DEBUG = 3,
    MW_P_FATAL = 4
} MW_LOG_LEVEL;

void mw_print(MW_LOG_LEVEL level, const char *fmtStr, ...);

#define mw_debug(format, ...) mw_print(MW_P_DEBUG, format, ## __VA_ARGS__)
#define mw_info(format, ...) mw_print(MW_P_INFO, format, ## __VA_ARGS__)
#define mw_warn(format, ...) mw_print(MW_P_WARNING, format, ## __VA_ARGS__)
#define mw_error(format, ...) mw_print(MW_P_ERROR, format, ## __VA_ARGS__)
#define mw_fatal(format, ...) (mw_print(MW_P_FATAL, format, ## __VA_ARGS__), assert(false))

#ifdef __cplusplus
}
#endif

#endif /* SGL_LOG_INCLUDED */