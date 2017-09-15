#ifndef ARM_AEABI_H_
#define ARM_AEABI_H_

#include <cstdint>

extern "C" {

    #if defined(__ARM_EABI__)
    EXPORT_ABI_FUNCTION
    void __aeabi_memcpy(void *dest, const void *src, size_t n)
    {
        memcpy(dest, src, n);
    }

    EXPORT_ABI_FUNCTION
    int32_t __aeabi_idiv(int32_t dividend, int32_t divisor)
    {
        return __divsi3(dividend, divisor);
    }

    EXPORT_ABI_FUNCTION
    uint32_t __aeabi_uidiv(uint32_t dividend, uint32_t divisor)
    {
        return __udivsi3(dividend, divisor);
    }

    EXPORT_ABI_FUNCTION
    uint64_t __aeabi_uidivmod(uint32_t dividend, uint32_t divisor)
    {
        uint64_t q = __aeabi_uidiv(dividend, divisor);
        uint64_t r = dividend - q * divisor;

        uint64_t result = q | (r << 32);
        return result;
    }

    EXPORT_ABI_FUNCTION
    int __aeabi_atexit(void *objptr, void (* f)(void *), void *dso)
    {
        return __cxa_atexit(f, objptr, dso);
    }


    EXPORT_ABI_FUNCTION
    void __aeabi_memclr(void *dest, size_t n)
    {
        memset(dest, 0, n);
    }

    // TODO: Use weak_alias
    EXPORT_ABI_FUNCTION
    void __aeabi_memclr4(void *dest, size_t n)
    {
        __aeabi_memclr(dest, n);
    }


    EXPORT_ABI_FUNCTION
    void __aeabi_memclr8(void *dest, size_t n)
    {
        __aeabi_memclr(dest, n);
    }
    #endif
}

#endif
