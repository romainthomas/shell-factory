#ifndef POSIX_PICO_PROCESS_H_
#define POSIX_PICO_PROCESS_H_

namespace Pico {

    constexpr int POSIX_COMM_MAX = 16;

    METHOD
    Process Process::current()
    {
        return Process( Syscall::getpid() );
    }

    METHOD
    Process Process::parent()
    {
        return Process( Syscall::getppid() );
    }

    NO_RETURN METHOD
    void Process::execute(const char *filename, char *const argv[], char *const envp[])
    {
        Syscall::execve(filename, argv, envp);
    }

    METHOD
    Process Process::spawn(const char *filename, char *const argv[], char *const envp[])
    {
        pid_t pid = Syscall::fork();
        if ( pid == 0 )
            execute(filename, argv, envp);
        else
            return Process(pid);
    }

    template <typename T>
    METHOD
    Process Process::spawn(Stream<T>& stm, const char *filename, char *const argv[], char *const envp[])
    {
        pid_t pid = Syscall::fork();
        if ( pid == 0 )
        {
            execute(stm, filename, argv, envp);
        }
        else
            return Process(pid);
    }

    template <typename... T>
    METHOD
    Process Process::spawn(const char *filename, T... args)
    {
        pid_t pid = Syscall::fork();
        if ( pid == 0 )
        {
            execute(filename, args...);
        }
        else
            return Process(pid);
    }

    METHOD
    Process::signal_handler Process::set_signal_handler(int signal, Process::signal_handler handler)
    {
        struct sigaction act, old_act;

        act.sa_handler = handler;
        Memory::zero(&act.sa_mask, sizeof(sigset_t));
        act.sa_flags = SA_RESETHAND;

        Syscall::sigaction(signal, &act, &old_act);

        return old_act.sa_handler;
    }

    METHOD
    int Process::wait(int *status)
    {
        return Syscall::wait4(pid, status, 0, nullptr);
    }

    METHOD
    int Process::signal(int signal)
    {
        return Syscall::kill(pid, signal);
    }

    METHOD
    int Process::kill()
    {
        return signal(SIGKILL);
    }


    METHOD
    String Process::name()
    {
        char comm_path[PATH_MAX];
        char comm[POSIX_COMM_MAX + 1];

        sprintf(comm_path, "/proc/%d/comm", this->id());

        Filesystem::File comm_file(comm_path, Filesystem::File::READ);
        ssize_t n = comm_file.read(comm, sizeof(comm));
        comm[n - 1] = '\0';
        comm_file.close();
        return comm;
    }

    METHOD
    String Process::path()
    {
        char link_path[PATH_MAX];
        char exe[PATH_MAX + 1];

        sprintf(link_path, "/proc/%d/exe", this->id());

        ssize_t n = Syscall::readlink(link_path, exe, sizeof(exe));
        if ( n < 0 )
            return "";

        exe[n] = '\0';
        return exe;
    }

    METHOD
    String Process::maps()
    {
        char maps_path[PATH_MAX];
        char maps_buff[PATH_MAX + 1];

        String maps = "";

        sprintf(maps_path, "/proc/%d/maps", this->id());
        ssize_t offset = 0;
        ssize_t size_read = 1;

        Filesystem::File maps_file(maps_path, Filesystem::File::READ);
        maps_file.each_line(
            [&maps] (const String& s) {
              maps += s;
              maps += "\n";
              return 0;
            });

        maps_file.close();
        return maps;
    }


    METHOD
    String Process::cmdline()
    {
        char cmdline_path[PATH_MAX];
        char cmdline[PATH_MAX + 1];

        sprintf(cmdline_path, "/proc/%d/cmdline", this->id());

        Filesystem::File cmdline_file(cmdline_path, Filesystem::File::READ);
        ssize_t n = cmdline_file.read(cmdline, sizeof(cmdline));
        cmdline[n - 1] = '\0';
        cmdline_file.close();
        return cmdline;
    }


    METHOD Array<uintptr_t, 2> Process::module_baseaddr(const char* modulename) {
        String maps = this->maps();
        Array<uintptr_t, 2> result{0u, 0u};

        maps.each_line([&modulename, &result] (String line) {
            size_t off = line.find(modulename);

            if (off != String::npos) {
                Array<String, 2> addr_ranges = line.split<2>('-');
                const char* start_str = addr_ranges[0].pointer();
                const char* end_str   = addr_ranges[1].pointer();

                const uintptr_t start = static_cast<uintptr_t>(strtoll(start_str, nullptr, 16));
                const uintptr_t end   = static_cast<uintptr_t>(strtoll(end_str, nullptr, 16));

                result[0] = start;
                result[1] = end;

                return 1;
            }
            return 0;
        });

        return result;
    }


    METHOD uintptr_t Process::current_baseaddress() {
        String maps = this->maps();
        //CPU::reg_t pc = CpuContext::instruction_pointer();
        return 0;

    }


}

#endif
