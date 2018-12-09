#ifndef ELFLOADER_DUMP_H
#define ELFLOADER_DUMP_H

typedef struct elfheader_s elfheader_t;

const char* DumpSection(Elf32_Shdr *s, char* SST);
const char* DumpDynamic(Elf32_Dyn *s);
const char* DumpPHEntry(Elf32_Phdr *e);
void DumpMainHeader(Elf32_Ehdr *header, elfheader_t *h);
void DumpSymTab(elfheader_t *h);
void DumpDynamicSections(elfheader_t *h);
void DumpDynSym(elfheader_t *h);

#endif //ELFLOADER_DUMP_H