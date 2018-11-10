#ifndef ELFLOADER_DUMP_H
#define ELFLOADER_DUMP_H

const char* DumpSection(Elf32_Shdr *s, char* SST);
const char* DumpDynamic(Elf32_Dyn *s);

#endif //ELFLOADER_DUMP_H