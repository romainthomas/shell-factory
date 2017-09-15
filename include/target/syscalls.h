#ifndef GENERIC_SYSCALL_ABI_H_
#define GENERIC_SYSCALL_ABI_H_

#define SYSCALL_HANDLE_ERROR(result)

#define SYSCALL_ARG_BIND_REGISTER(i, reg, value)                        \
    register auto __arg##i asm (reg) = value;                           \

#define EMIT_SYSCALL0(class, name, ...)                                 \
({                                                                      \
    int sys_num = SYSCALL_NAME_TO_NUM(class, name);                     \
                                                                        \
    register int __sys_num asm ( SYSCALL_NUMBER_REGISTER ) = sys_num;   \
    register long __sys_result asm ( SYSCALL_RESULT_REGISTER );         \
                                                                        \
    asm volatile (                                                      \
        SYSCALL_INSTRUCTION "\n"                                        \
        : "=r" (__sys_result)                                           \
        : "r" (__sys_num), ##__VA_ARGS__                                \
        : "memory", "cc"                                                \
    );                                                                  \
    asm volatile ("" ::: SYSCALL_CLOBBERED_REGISTERS );                 \
                                                                        \
    SYSCALL_HANDLE_ERROR(__sys_result);                                 \
                                                                        \
    __sys_result;                                                       \
})                                                                      \

#define EMIT_SYSCALL1(class, name, arg1, ...)                           \
({                                                                      \
    SYSCALL_SET_ARG_1(arg1);                                            \
    EMIT_SYSCALL0(class, name,                                          \
                  "X"(__arg1), ##__VA_ARGS__);                          \
})                                                                      \

#define EMIT_SYSCALL2(class, name, arg1, arg2, ...)                     \
({                                                                      \
    SYSCALL_SET_ARG_2(arg2);                                            \
    EMIT_SYSCALL1(class, name, arg1,                                    \
                  "X"(__arg2), ##__VA_ARGS__);                          \
})                                                                      \

#define EMIT_SYSCALL3(class, name, arg1, arg2, arg3, ...)               \
({                                                                      \
    SYSCALL_SET_ARG_3(arg3);                                            \
    EMIT_SYSCALL2(class, name, arg1, arg2,                              \
                  "X"(__arg3), ##__VA_ARGS__);                          \
})                                                                      \

#define EMIT_SYSCALL4(class, name, arg1, arg2, arg3, arg4, ...)         \
({                                                                      \
    SYSCALL_SET_ARG_4(arg4);                                            \
    EMIT_SYSCALL3(class, name, arg1, arg2, arg3,                        \
                  "X"(__arg4), ##__VA_ARGS__);                          \
})                                                                      \

#define EMIT_SYSCALL5(class, name, arg1, arg2, arg3, arg4, arg5, ...)   \
({                                                                      \
    SYSCALL_SET_ARG_5(arg5);                                            \
    EMIT_SYSCALL4(class, name, arg1, arg2, arg3, arg4,                  \
                  "X"(__arg5), ##__VA_ARGS__);                          \
})                                                                      \

#define EMIT_SYSCALL6(class, name, arg1, arg2, arg3, arg4,              \
                      arg5, arg6, ...)                                  \
({                                                                      \
    SYSCALL_SET_ARG_6(arg6);                                            \
    EMIT_SYSCALL5(class, name, arg1, arg2, arg3, arg4, arg5,            \
                  "X"(__arg6), ##__VA_ARGS__);                          \
})                                                                      \

#define EMIT_SYSCALL7(class, name, arg1, arg2, arg3, arg4,              \
                      arg5, arg6, arg7, ...)                            \
({                                                                      \
    SYSCALL_SET_ARG_7(arg7);                                            \
    EMIT_SYSCALL6(class, name, arg1, arg2, arg3, arg4, arg5, arg6,      \
                  "X"(__arg7), ##__VA_ARGS__);                          \
})                                                                      \

#define EMIT_SYSCALL_SWITCH(_1,_2,_3,_4,_5,_6,_7,_8,N,...) N
#define EMIT_SYSCALL(class, name, args...)                              \
        EMIT_SYSCALL_SWITCH(0, ##args,                                  \
                EMIT_SYSCALL7,                                          \
                EMIT_SYSCALL6,                                          \
                EMIT_SYSCALL5,                                          \
                EMIT_SYSCALL4,                                          \
                EMIT_SYSCALL3,                                          \
                EMIT_SYSCALL2,                                          \
                EMIT_SYSCALL1,                                          \
                EMIT_SYSCALL0)(class, name, ##args)                     \


// System types definitions.
#include <target/types.h>

#if defined(__ANDROID__)
#include <target/android/syscalls.h>
#elif defined(__linux__)
#include <target/linux/syscalls.h>
#elif defined(__FreeBSD__)
#include <target/freebsd/syscalls.h>
#elif defined(__APPLE__)
#include <target/darwin/syscalls.h>
#else
#pragma message "No operating system detected, compiling to bare-metal."
#endif

#endif
