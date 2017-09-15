#ifndef ELF_FUNC_H_
#define ELF_FUNC_H_

#include <pico.h>
#include "elf.h"

namespace ELF {
template<class ELF>
FUNCTION uintptr_t get_function_offset(const char* modulepath, const char* fname) {
  using Elf_Ehdr = typename ELF::Elf_Ehdr;
  using Elf_Shdr = typename ELF::Elf_Shdr;
  using Elf_Sym  = typename ELF::Elf_Sym;
  using Elf_Phdr = typename ELF::Elf_Phdr;

  using Elf_Off  = typename ELF::Elf_Off;

  char section_name[128] = {0};
  char symbol_name[1024] = {0};
  ssize_t nread = 0;

  uintptr_t function_offset = 0;

  Elf_Shdr shstr_section, shdr_tmp;
  Elf_Shdr dynsym, dynstr;

  Elf_Phdr phdr_tmp;

  Elf_Sym sym_tmp;

  Pico::Filesystem::File module_file(modulepath, Pico::Filesystem::File::READ);

  // Read ELF Header
  Elf_Ehdr elf_hdr;
  module_file.seek(0);
  nread = module_file.read(&elf_hdr, sizeof(Elf_Ehdr));
  if (nread != sizeof(Elf_Ehdr) || module_file.error()) {
    Pico::printf("Error while reading ELF header\n");
    return 0;
  }

  Pico::printf("Entry point: %p\n", elf_hdr.e_entry);


  Elf_Off shdr_off   = elf_hdr.e_shoff;                                  // Sections table offset
  Elf_Off shstr_base = shdr_off + elf_hdr.e_shstrndx * sizeof(Elf_Shdr); // Section string table offset

  // Read shstr section header
  module_file.seek(shstr_base);
  nread = module_file.read(&shstr_section, sizeof(Elf_Shdr));
  if (nread != sizeof(Elf_Shdr) || module_file.error()) {
    Pico::printf("Error while reading .shstr section\n");
    return 0;
  }

  for (size_t i = 0; i < elf_hdr.e_shnum; ++i) {

    module_file.seek(shdr_off + i * sizeof(Elf_Shdr));
    nread = module_file.read(&shdr_tmp, sizeof(Elf_Shdr));

    if (nread != sizeof(Elf_Shdr) || module_file.error()) {
      Pico::printf("Error while reading section #%d\n", i);
      return 0;
    }

    // Get name
    module_file.seek(shstr_section.sh_offset + shdr_tmp.sh_name);
    nread = module_file.read(section_name, sizeof(section_name));
    //printf("Section[%d]: %s\n", i, section_name);


    // Look for dynamic section which export symbols
    if (shdr_tmp.sh_type == SHT_DYNSYM) {
      memcpy(&dynsym, &shdr_tmp, sizeof(Elf_Shdr));
    }

    if (Pico::String::compare(section_name, ".dynstr") == 0) {
      memcpy(&dynstr, &shdr_tmp, sizeof(Elf_Shdr));
    }
  }


  Elf_Off base_addr = 0;
  for (size_t i = 0; i < elf_hdr.e_phnum; ++i) {
    module_file.seek(elf_hdr.e_phoff + i * sizeof(Elf_Phdr));
    nread = module_file.read(&phdr_tmp, sizeof(Elf_Phdr));
    if (phdr_tmp.p_type == SEGMENT_TYPES::PT_LOAD && (phdr_tmp.p_flags & PF_X)) {
      base_addr = phdr_tmp.p_vaddr;
      break;
    }
  }


  // Loop through all symbols in dynsym and get
  // matching one based on its name (stored in dynstr)
  const size_t nbof_symbols = dynsym.sh_size / sizeof(Elf_Sym);
  //pt_load_offset = 0;
  for (size_t i = 0; i < nbof_symbols; ++i) {

    // Goto symbols[i]
    module_file.seek(dynsym.sh_offset + i * sizeof(Elf_Sym));
    nread = module_file.read(&sym_tmp, sizeof(Elf_Sym));

    // Get name
    module_file.seek(dynstr.sh_offset + sym_tmp.st_name);
    nread = module_file.read(&symbol_name, sizeof(symbol_name));
    //printf("%s\n", symbol_name);
    if (Pico::String::compare(symbol_name, fname) == 0) {
      function_offset = sym_tmp.st_value - base_addr;
    }
  }

  module_file.close();
  return function_offset;
}
}
#endif
