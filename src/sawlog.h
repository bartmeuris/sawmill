#ifndef __SAWLOG_H
# define __SAWLOG_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_THREADED

void _logout(int lvl, const char *file, int line, const char *func, const char *format, ...);
void _logout_threaded(int lvl, const char *file, int line, const char *func, const char *format, ...);

void log_setlevel(int lvl);

void log_setoutput(FILE *out);

#ifdef __cplusplus
}
#endif

#define LOG_ERROR           1
#define LOG_WARNING         2
#define LOG_NOTICE          3
#define LOG_INFO            4
#define LOG_DEBUG           5
#define LOG_LVL_MAX         LOG_DEBUG



// Note: Due to ##__VA_ARGS__ giving warnings when compiling with -pedantic, the format
// paramaeter is removed. It is however mandatory ;)
#ifdef LOG_THREADED
//# define LOG(lvl, format, ...)	_logout_threaded(lvl, __FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__)
# define LOG(lvl, ...)	_logout_threaded(lvl, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#else
//# define LOG(lvl, format, ...)	_logout(lvl, __FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__)
# define LOG(lvl, ...)	_logout(lvl, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#endif

//#define ERR(format, ...)    LOG(LOG_ERROR, format, ##__VA_ARGS__)
//#define WARN(format, ...)   LOG(LOG_WARNING, format, ##__VA_ARGS__)
//#define NOTICE(format, ...) LOG(LOG_NOTICE, format, ##__VA_ARGS__)
//#define INFO(format, ...)   LOG(LOG_INFO, format, ##__VA_ARGS__)
//#define DBG(format, ...)    LOG(LOG_DEBUG, format, ##__VA_ARGS__)

#define ERR(...)    LOG(LOG_ERROR, __VA_ARGS__)
#define WARN(...)   LOG(LOG_WARNING, __VA_ARGS__)
#define NOTICE(...) LOG(LOG_NOTICE, __VA_ARGS__)
#define INFO(...)   LOG(LOG_INFO, __VA_ARGS__)
#define DBG(...)    LOG(LOG_DEBUG, __VA_ARGS__)

#endif
