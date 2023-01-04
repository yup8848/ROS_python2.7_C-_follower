#pragma once

#define LOG_ERROR(log_fmt, ...) \
    do{ \
        INFO_W.log(LL_ERROR,   "[%s] " log_fmt "\n", \
                     __FUNCTION__, ##__VA_ARGS__); \
    } while (0) 

#define LOG_WARN(log_fmt, ...) \
    do{ \
        INFO_W.log(LL_WARNING,   "[%s] " log_fmt "\n", \
                     __FUNCTION__, ##__VA_ARGS__); \
    } while (0) 

#define LOG_TRACE(log_fmt, ...) \
    do{ \
        INFO_W.log(LL_TRACE,   "[%s] " log_fmt "\n", \
                      __FUNCTION__, ##__VA_ARGS__); \
    } while (0) 

#define LOG_DEBUG(log_fmt, ...) \
    do{ \
        INFO_W.log(LL_DEBUG,   "[%s] " log_fmt "\n", \
                     __FUNCTION__, ##__VA_ARGS__); \
    } while (0) 
//============extend===================
#define MACRO_RET(condition, return_val) {\
    if (condition) {\
        return return_val;\
    }\
}

#define MACRO_WARN(condition, log_fmt, ...) {\
    if (condition) {\
        LOG_WARN( log_fmt, ##__VA_ARGS__);\
    }\
}

#define MACRO_WARN_RET(condition, return_val, log_fmt, ...) {\
    if ((condition)) {\
        LOG_WARN( log_fmt, ##__VA_ARGS__);\
		return return_val;\
    }\
}
