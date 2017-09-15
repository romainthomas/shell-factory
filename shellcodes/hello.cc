#include <factory.h>
#include <channel.h>
#include <pico.h>

using namespace Pico;

SHELLCODE_ENTRY
{
  printf("=[ Hello World ]=\n");
  //Process::exit(0);

#if defined(__amd64__)
  __asm__ volatile(
      "pop %rax\n"
      "xor %rax, %rax\n"
      "ret\n");

#elif defined(__aarch64__)
  //Process::exit(0);


  __asm__ volatile(
      "ldr   x30, [sp], #16\n"
      "ret\n");
#endif
  //Process::exit(0);

}
