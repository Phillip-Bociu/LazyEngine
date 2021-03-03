#pragma once

#ifdef DEBUG
#define LOG(...) printf("[LOG]: " __VA_ARGS__); printf("\n");
#define WARN(...)printf("[WARNING]: "  __VA_ARGS__); printf("\n");
#define ERROR(...) printf("[ERROR]: "  __VA_ARGS__); printf("\n");
#else
#define LOG(...)
#define WARN(...)
#define ERROR(...)
#endif