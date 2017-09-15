#ifndef ANDROID_MEMORY_H_
#define ANDROID_MEMORY_H_

#if defined(__i386__)
#include <target/android/i386/memory.h>
#elif defined(__amd64__)
#include <target/android/amd64/memory.h>
#elif defined(__arm__)
#include <target/android/arm/memory.h>
#elif defined(__aarch64__)
#include <target/android/aarch64/memory.h>
#elif defined(__mips__)
#include <target/android/mips/memory.h>
#elif defined(__powerpc__)
#include <target/android/ppc/memory.h>
#elif defined(__SH4__)
#include <target/android/sh4/memory.h>
#else
#error "No memory header defined for this architecture."
#endif

#endif
