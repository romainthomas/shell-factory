#include <factory.h>
#include <pico.h>

using namespace Pico;

SHELLCODE_ENTRY {
    Process::exit(0);
}
