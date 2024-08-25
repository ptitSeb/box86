// Handling of TLS calls, include x86 specifi set_thread_area
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "debug.h"
#include "box86context.h"
#include "x86emu.h"
#include "x86tls.h"
#include "elfloader.h"

typedef struct thread_area_s
{
             int  entry_number;
        uintptr_t base_addr;
    unsigned int  limit;
    unsigned int  seg_32bit:1;
    unsigned int  contents:2;
    unsigned int  read_exec_only:1;
    unsigned int  limit_in_pages:1;
    unsigned int  seg_not_present:1;
    unsigned int  useable:1;
} thread_area_t;

#define SUPER() \
GO(0)   \
GO(1)   \
GO(2)   \
GO(3)   \
GO(4)   \
GO(5)   \
GO(6)   \
GO(7)   \
GO(8)   \
GO(9)   \
GO(10)  \
GO(11)  \
GO(12)  \
GO(13)  \
GO(14)  \
GO(15)  \
GO(16)  \
GO(17)  \
GO(18)  \

#define GO(A) static pthread_once_t thread_key_once##A = PTHREAD_ONCE_INIT;
SUPER()

#undef GO
#define GO(A)\
static void thread_key_alloc##A() {                         \
	pthread_key_create(&my_context->segtls[A].key, NULL);   \
}
SUPER()
#undef GO

uint32_t my_set_thread_area(thread_area_t* td)
{
    printf_log(LOG_DEBUG, "set_thread_area(%p[%d/base=%p/limit=%u/32bits:%u/%u/%u...])\n", td, td->entry_number, (void*)td->base_addr, td->limit_in_pages, td->seg_32bit, td->contents, td->read_exec_only);

    int isempty = 0;
    // first, check if the "user_desc", here td, is "empty"
    if(td->read_exec_only==1 && td->seg_not_present==1)
        if( !td->base_addr 
         && !td->limit
         && !td->seg_32bit 
         && !td->contents 
         && !td->limit_in_pages 
         && !td->useable)
            isempty = 1;
    int idx = td->entry_number;
    if(idx==-1) {
        // find a free one
        for (int i=0; i<3 && idx==-1; ++i)
            if(!my_context->segtls[i].present)
                idx=i;
        if(idx==-1) {
            errno = ESRCH;
            return (uint32_t)-1;
        }
        idx+=7;
        td->entry_number = idx;
    }
    if(isempty && (td->entry_number<7 || td->entry_number>7+2)) {
        errno = EINVAL;
        return (uint32_t)-1;
    }
    if(isempty) {
        memset(&my_context->segtls[td->entry_number-7], 0, sizeof(base_segment_t));
        return 0;
    }
    if((idx<7 || idx>7+2)) {
        errno = EINVAL;
        return (uint32_t)-1;
    }

    my_context->segtls[idx-7].base = td->base_addr;
    my_context->segtls[idx-7].limit = td->limit;
    my_context->segtls[idx-7].present = 1;
    #define GO(A)   case A:	pthread_once(&thread_key_once##A, thread_key_alloc##A); break;
    switch (idx-7) {
        SUPER()
    }
    #undef GO

    pthread_setspecific(my_context->segtls[idx-7].key, (void*)my_context->segtls[idx-7].base);

    ResetSegmentsCache(thread_get_emu());

    return 0;
}

EXPORT uint32_t my_modify_ldt(x86emu_t* emu, int op, thread_area_t* td, int size)
{
    (void)emu;
    printf_log(/*LOG_DEBUG*/LOG_INFO, "modify_ldt(0x%x, %p[0x%x/base=%p/limit=%u/32bits:%u/%u/%u...], %d)\n", op, td, td->entry_number, (void*)td->base_addr, td->limit_in_pages, td->seg_32bit, td->contents, td->read_exec_only, size);
    if(!td) {
        errno = EFAULT;
        return (uint32_t)-1;
    }
    if(op==0 && size==8) {
        td->entry_number = 6;
        td->base_addr = (uintptr_t)GetSegmentBase(6<<3);
        return 8;
    }
    if(op!=0x11) {
        errno = ENOSYS;
        return (uint32_t)-1;
    }
    if(!td->seg_32bit && td->base_addr) {
        // not handling 16bits segments for now
        errno = EINVAL;
        return (uint32_t)-1;
    }

    int idx = td->entry_number - 7;
    if(td->entry_number>=0x1ff0) idx=td->entry_number-0x1ff0+3;
    if(idx<0 || idx>16+3-1) {
        printf_log(LOG_DEBUG, "modify_ldt: idx out of range: %d\n", idx);
        errno = EINVAL;
        return (uint32_t)-1;
    }

    
    my_context->segtls[idx].base = td->base_addr;
    my_context->segtls[idx].limit = td->limit;
    my_context->segtls[idx].present = td->base_addr?1:0;
    #define GO(A) case A:	pthread_once(&thread_key_once##A, thread_key_alloc##A); break;
    switch (idx-7) {
        SUPER()
    }
    #undef GO
    pthread_setspecific(my_context->segtls[idx].key, (void*)my_context->segtls[idx].base);
    
    ResetSegmentsCache(thread_get_emu());

    return 0;
}

#define POS_TLS     0x50

static int sizeDTS(box86context_t* context)
{
    return ((context->elfsize+0xff)&~0xff)*8;
}
static int sizeTLSData(int s)
{
    uint32_t mask = 0xffff/*box64_nogtk?0xffff:0x1fff*/;    // x86 does the mapping per 64K blocks, so it makes sense to have it this large
    return (s+mask)&~mask;
}

static tlsdatasize_t* setupTLSData(box86context_t* context)
{
    // Setup the GS segment:
    int dtssize = sizeDTS(context);
    int datasize = sizeTLSData(context->tlssize);
    void *ptr_oversized = (char*)box_malloc(dtssize+POS_TLS+datasize);
    void *ptr = (void*)((uintptr_t)ptr_oversized + datasize);
    memcpy((void*)((uintptr_t)ptr-context->tlssize), context->tlsdata, context->tlssize);
    tlsdatasize_t *data = (tlsdatasize_t*)box_calloc(1, sizeof(tlsdatasize_t));
    data->data = ptr;
    data->tlssize = context->tlssize;
    data->ptr = ptr_oversized;
    data->n_elfs = context->elfsize;
    pthread_setspecific(context->tlskey, data);
    // copy canary...
    memset((void*)((uintptr_t)ptr), 0, POS_TLS+dtssize);            // set to 0 remining bytes
    memcpy((void*)((uintptr_t)ptr+0x14), context->canary, 4);      // put canary in place
    uintptr_t tlsptr = (uintptr_t)ptr;
    memcpy((void*)((uintptr_t)ptr+0x0), &tlsptr, 4);
    uintptr_t dtp = (uintptr_t)ptr+POS_TLS;
    memcpy((void*)(tlsptr+0x4), &dtp, 4);
    if(dtssize) {
        for (int i=0; i<context->elfsize; ++i) {
            // set pointer
            dtp = (uintptr_t)ptr + GetTLSBase(context->elfs[i]);
            memcpy((void*)((uintptr_t)ptr+POS_TLS+i*8), &dtp, 4);
            memcpy((void*)((uintptr_t)ptr+POS_TLS+i*8+4), &i, 4); // index
        }
    }
    memcpy((void*)((uintptr_t)ptr+0x10), &context->vsyscall, 4);  // address of vsyscall
    return data;
}

void* fillTLSData(box86context_t *context)
{
        mutex_lock(&context->mutex_tls);
        tlsdatasize_t *data = setupTLSData(context);
        mutex_unlock(&context->mutex_tls);
        return data;
}

void* resizeTLSData(box86context_t *context, void* oldptr)
{
        mutex_lock(&context->mutex_tls);
        tlsdatasize_t* oldata = (tlsdatasize_t*)oldptr;
        if(sizeTLSData(oldata->tlssize)!=sizeTLSData(context->tlssize) || (oldata->n_elfs/0xff)!=(context->elfsize/0xff)) {
            if(sizeTLSData(oldata->tlssize)) {
                printf_log(LOG_INFO, "Warning, resizing of TLS occured! size: %d->%d / n_elfs: %d->%d\n", sizeTLSData(oldata->tlssize), sizeTLSData(context->tlssize), 1+(oldata->n_elfs/0xff), 1+(context->elfsize/0xff));
            }
            tlsdatasize_t *data = setupTLSData(context);
            // copy the relevent old part, in case something changed
            memcpy((void*)((uintptr_t)data->data-oldata->tlssize), (void*)((uintptr_t)oldata->data-oldata->tlssize), oldata->tlssize);
            // all done, update new size, free old pointer and exit
            mutex_unlock(&context->mutex_tls);
            free_tlsdatasize(oldptr);
            return data;
        } else {
            // keep the same tlsdata, but fill in the blanks
            // adjust tlsdata
            void *ptr = oldata->data;
            if(context->tlssize!=oldata->tlssize) {
                memcpy((void*)((uintptr_t)ptr-context->tlssize), context->tlsdata, context->tlssize-oldata->tlssize);
                oldata->tlssize = context->tlssize;
            }
            // adjust DTS
            if(oldata->n_elfs!=context->elfsize) {
                uintptr_t dtp = (uintptr_t)ptr+POS_TLS;
                for (int i=oldata->n_elfs; i<context->elfsize; ++i) {
                    // set pointer
                    dtp = (uintptr_t)ptr + GetTLSBase(context->elfs[i]);
                    *(uint32_t*)((uintptr_t)ptr+POS_TLS+i*8) = dtp;
                    *(uint32_t*)((uintptr_t)ptr+POS_TLS+i*8+4) = i; // index
                }
                oldata->n_elfs = context->elfsize;
            }
            mutex_unlock(&context->mutex_tls);
            return oldata;
        }
}

static void* GetSeg33Base()
{
    tlsdatasize_t* ptr;
    if ((ptr = (tlsdatasize_t*)pthread_getspecific(my_context->tlskey)) == NULL) {
        ptr = (tlsdatasize_t*)fillTLSData(my_context);
    }
    if(ptr->tlssize != my_context->tlssize) {
        ptr = (tlsdatasize_t*)resizeTLSData(my_context, ptr);
    }
    return ptr->data;
}

void* GetSegmentBase(uint32_t desc)
{
    if(!desc) {
        printf_log(LOG_NONE, "Warning, accessing segment NULL\n");
        return NULL;
    }
    int base = desc>>3;
    if(base==0xe || base==0xf)
        return NULL;    // regular value...
    if(base==0x6)
        return GetSeg33Base();

    if(base>=0x1ff0)
        base = base - 0x1ff0 + 3 + 7;

    if(base>6 && base<16+3+7 && my_context->segtls[base-7].present) {
        void* ptr = pthread_getspecific(my_context->segtls[base-7].key);
        return ptr;
    }

    printf_log(LOG_NONE, "Warning, accessing segment unknown 0x%x (%d) or unset\n", desc, base);
    return NULL;
}
