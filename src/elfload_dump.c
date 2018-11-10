#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>

#include "box86version.h"
#include "elfloader.h"
#include "debug.h"
#include "elfload_dump.h"

const char* DumpSection(Elf32_Shdr *s, char* SST) {
    static char buff[200];
    switch (s->sh_type) {
        case SHT_NULL:
            return "SHT_NULL";
        #define GO(A) \
        case A:     \
            sprintf(buff, #A " Name=\"%s\"(%d) off=0x%X, size=%d, attr=0x%04X, addr=%p(%02X), link/info=%d/%d", \
                SST+s->sh_name, s->sh_name, s->sh_offset, s->sh_size, s->sh_flags, s->sh_addr, s->sh_addralign, s->sh_link, s->sh_info); \
            break
        GO(SHT_PROGBITS);
        GO(SHT_SYMTAB);
        GO(SHT_STRTAB);
        GO(SHT_RELA);
        GO(SHT_HASH);
        GO(SHT_DYNAMIC);
        GO(SHT_NOTE);
        GO(SHT_NOBITS);
        GO(SHT_REL);
        GO(SHT_SHLIB);
        GO(SHT_DYNSYM);
        GO(SHT_INIT_ARRAY);
        GO(SHT_FINI_ARRAY);
        GO(SHT_PREINIT_ARRAY);
        GO(SHT_GROUP);
        GO(SHT_SYMTAB_SHNDX);
        GO(SHT_NUM);
        GO(SHT_LOPROC);
        GO(SHT_HIPROC);
        GO(SHT_LOUSER);
        GO(SHT_HIUSER);
        GO(SHT_GNU_versym);
        GO(SHT_GNU_ATTRIBUTES);
        GO(SHT_GNU_HASH);
        GO(SHT_GNU_LIBLIST);
        GO(SHT_CHECKSUM);
        GO(SHT_LOSUNW);
        //GO(SHT_SUNW_move);
        GO(SHT_SUNW_COMDAT);
        GO(SHT_SUNW_syminfo);
        GO(SHT_GNU_verdef);
        GO(SHT_GNU_verneed);
        #undef GO
        default:
            sprintf(buff, "0x%X unknown type", s->sh_type);
    }
    return buff;
}

const char* DumpDynamic(Elf32_Dyn *s) {
    static char buff[200];
    switch (s->d_tag) {
        case DT_NULL:
            return "DT_NULL: End Dynamic Section";
        #define GO(A, Add) \
        case A:     \
            sprintf(buff, #A " %s=0x%X", (Add)?"Addr":"Val", (Add)?s->d_un.d_ptr:s->d_un.d_val); \
            break
            GO(DT_NEEDED, 0);
            GO(DT_PLTRELSZ, 0);
            GO(DT_PLTGOT, 1);
            GO(DT_HASH, 1);
            GO(DT_STRTAB, 1);
            GO(DT_SYMTAB, 1);
            GO(DT_RELA, 1);
            GO(DT_RELASZ, 0);
            GO(DT_RELAENT, 0);
            GO(DT_STRSZ, 0);
            GO(DT_SYMENT, 0);
            GO(DT_INIT, 1);
            GO(DT_FINI, 1);
            GO(DT_SONAME, 0);
            GO(DT_RPATH, 0);
            GO(DT_SYMBOLIC, 0);
            GO(DT_REL, 1);
            GO(DT_RELSZ, 0);
            GO(DT_RELENT, 0);
            GO(DT_PLTREL, 0);
            GO(DT_DEBUG, 0);
            GO(DT_TEXTREL, 0);
            GO(DT_JMPREL, 1);
            GO(DT_BIND_NOW, 1);
            GO(DT_INIT_ARRAY, 1);
            GO(DT_FINI_ARRAY, 1);
            GO(DT_INIT_ARRAYSZ, 0);
            GO(DT_FINI_ARRAYSZ, 0);
            GO(DT_RUNPATH, 0);
            GO(DT_FLAGS, 0);
            GO(DT_ENCODING, 0);
            GO(DT_NUM, 0);
            GO(DT_VALRNGLO, 0);
            GO(DT_GNU_PRELINKED, 0);
            GO(DT_GNU_CONFLICTSZ, 0);
            GO(DT_GNU_LIBLISTSZ, 0);
            GO(DT_CHECKSUM, 0);
            GO(DT_PLTPADSZ, 0);
            GO(DT_MOVEENT, 0);
            GO(DT_MOVESZ, 0);
            GO(DT_FEATURE_1, 0);
            GO(DT_POSFLAG_1, 0);
            GO(DT_SYMINSZ, 0);
            GO(DT_SYMINENT, 0);
            GO(DT_ADDRRNGLO, 0);
            GO(DT_GNU_HASH, 0);
            GO(DT_TLSDESC_PLT, 0);
            GO(DT_TLSDESC_GOT, 0);
            GO(DT_GNU_CONFLICT, 0);
            GO(DT_GNU_LIBLIST, 0);
            GO(DT_CONFIG, 0);
            GO(DT_DEPAUDIT, 0);
            GO(DT_AUDIT, 0);
            GO(DT_PLTPAD, 0);
            GO(DT_MOVETAB, 0);
            GO(DT_SYMINFO, 0);
            GO(DT_VERSYM, 0);
            GO(DT_RELACOUNT, 0);
            GO(DT_RELCOUNT, 0);
            GO(DT_FLAGS_1, 0);
            GO(DT_VERDEF, 0);
            GO(DT_VERDEFNUM, 0);
            GO(DT_VERNEED, 0);
            GO(DT_VERNEEDNUM, 0);
            GO(DT_AUXILIARY, 0);
            GO(DT_FILTER, 0);
        #undef GO
        default:
            sprintf(buff, "0x%X unknown type", s->d_tag);
    }
    return buff;
}


