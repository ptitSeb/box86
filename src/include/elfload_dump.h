#ifndef ELFLOADER_DUMP_H
#define ELFLOADER_DUMP_H

typedef struct elfheader_s elfheader_t;

const char* DumpSection(Elf32_Shdr *s, char* SST);
const char* DumpDynamic(Elf32_Dyn *s);
const char* DumpPHEntry(Elf32_Phdr *e);
const char* DumpSym(elfheader_t *h, Elf32_Sym* sym, int version);
const char* DumpRelType(int t);
const char* SymName(elfheader_t *h, Elf32_Sym* sym);
const char* IdxSymName(elfheader_t *h, int sym);
void DumpMainHeader(Elf32_Ehdr *header, elfheader_t *h);
void DumpSymTab(elfheader_t *h);
void DumpDynamicSections(elfheader_t *h);
void DumpDynamicNeeded(elfheader_t *h);
void DumpDynamicRPath(elfheader_t *h);
void DumpDynSym(elfheader_t *h);
void DumpRelTable(elfheader_t *h, int cnt, Elf32_Rel *rel, const char* name);
void DumpRelATable(elfheader_t *h, int cnt, Elf32_Rela *rela, const char* name);

void DumpBinary(char* p, int sz);

#endif //ELFLOADER_DUMP_H