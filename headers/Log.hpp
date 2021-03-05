#pragma once
#define LOG_LVL 2

#ifdef DEBUG
    #if LOG_LVL == 3
    #define LOG(...)                        \
        printf("\033[0;33m");               \
        printf("[LOG]: " __VA_ARGS__);      \
        printf("\n");                       \
        printf("\033[0m")
    #else
        #define LOG(...)
#endif
#if LOG_LVL >= 2
    #define WARN(...)                       \
        printf("\033[0;36m");               \
        printf("[WARNING]: " __VA_ARGS__);  \
        printf("\n");                       \
        printf("\033[0m")
    #else
        #define WARN(...)
#endif
#if LOG_LVL >= 1
    #define ERROR(...)                      \
        printf("\033[0;31m");               \
        printf("[ERROR]: " __VA_ARGS__);    \
        printf("\n");                       \
        printf("\033[0m")
    #else
        #define ERROR(...)
#endif
#else
#define LOG(...)
#define WARN(...)
#define ERROR(...)
#endif