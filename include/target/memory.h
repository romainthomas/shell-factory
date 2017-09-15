#ifndef GENERIC_MEMORY_H_
#define GENERIC_MEMORY_H_

#if defined(__ANDROID__)
#include <target/android/memory.h>
#elif defined(__linux__)
#include <target/linux/memory.h>
#elif defined(__FreeBSD__)
#include <target/freebsd/memory.h>
#elif defined(__APPLE__)
#include <target/darwin/memory.h>
#endif

#endif
