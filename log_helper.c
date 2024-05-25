//
// Created by snownf on 24-4-14.
//


#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "log_helper.h"
#define WINDOWS
#define MAXSTRING 1000

static const char *level_strings[] = {
        /* the order is important */
        "error",
        "warning",
        "info",
        "debug",
        "fatal"
};

const char *levels[5] = {"0;33", "0;31", "0;32", "0;36", "0;33"};

void mw_print(MW_LOG_LEVEL level, const char *fmtStr, ...) {
    va_list args;
    char msg[MAXSTRING];
    int ret;

    va_start(args, fmtStr);
    ret = vsnprintf(msg, MAXSTRING, fmtStr, args);
    if (ret < 0 || ret >= MAXSTRING)
        strcpy(msg, "<message truncated>");
    va_end(args);

#ifdef LINUX
    fprintf(stderr, "\033[%sm%s : %s\033[0m\n", levels[level], level_strings[level], msg);
#elif defined(WINDOWS)
    fprintf(stderr, "%s : %s\n", level_strings[level], msg);
#else
#error ""
#endif
}
