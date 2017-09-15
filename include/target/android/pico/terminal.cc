#ifndef PICOLIB_TERMINAL_IMPL_H_
#define PICOLIB_TERMINAL_IMPL_H_

#include <asm-generic/termios.h>

namespace Pico {

    METHOD
    TTY::Size TTY::size()
    {
        struct winsize ws;

        Syscall::ioctl(this->file_desc(), TIOCGWINSZ, &ws);
        return TTY::Size{ .rows = ws.ws_row, .cols = ws.ws_col };
    }

    METHOD
    int TTY::set_size(TTY::Size ts)
    {
        struct winsize ws = {
            .ws_row = ts.rows,
            .ws_col = ts.cols,
            .ws_xpixel = 0,
            .ws_ypixel = 0,
        };

        int ret = Syscall::ioctl(this->file_desc(), TIOCSWINSZ, &ws);

        return Target::is_error(ret) ? -1 : 0;
    }

    METHOD
    int TTY::flush()
    {
        int ret = Syscall::ioctl(this->file_desc(), TCFLSH, TCIOFLUSH);

        return Target::is_error(ret) ? -1 : 0;
    }

    METHOD
    int TTY::get_settings(struct termios& tios)
    {
        int ret = Syscall::ioctl(this->file_desc(), TCGETS, &tios);

        return Target::is_error(ret) ? -1 : 0;
    }

    METHOD
    int TTY::set_settings(struct termios tios)
    {
        int ret = Syscall::ioctl(this->file_desc(), TCSETS, &tios);

        return Target::is_error(ret) ? -1 : 0;
    }

    METHOD int TTY::set_raw(struct termios& old)
    {
        if ( get_settings(old) != 0 )
            return -1;

        struct termios raw_tios = old;
        raw_tios.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
        raw_tios.c_oflag &= ~OPOST;
        raw_tios.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
        raw_tios.c_cflag &= ~(CSIZE | PARENB);
        raw_tios.c_cflag |= CS8;

        return set_settings(raw_tios);
    }
}

#endif
