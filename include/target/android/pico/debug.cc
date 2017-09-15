#ifndef PICOLIB_DEBUG_IMPL_H_
#define PICOLIB_DEBUG_IMPL_H_

namespace Pico {

    METHOD 
    int Debuggee::attach()
    {
        return Syscall::ptrace(PTRACE_ATTACH, pid, nullptr, nullptr);
    }

    METHOD 
    int Debuggee::detach()
    {
        return Syscall::ptrace(PTRACE_DETACH, pid, nullptr, nullptr);
    }
}

#endif
