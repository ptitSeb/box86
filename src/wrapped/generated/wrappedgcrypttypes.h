/*******************************************************************
 * File automatically generated by rebuild_wrappers.py (v2.0.0.11) *
 *******************************************************************/
#ifndef __wrappedgcryptTYPES_H_
#define __wrappedgcryptTYPES_H_

#ifndef LIBNAME
#error You should only #include this file inside a wrapped*.c file
#endif
#ifndef ADDED_FUNCTIONS
#define ADDED_FUNCTIONS() 
#endif

typedef uint32_t (*uFpppV_t)(void*, void*, void*, ...);

#define SUPER() ADDED_FUNCTIONS() \
	GO(gcry_sexp_build, uFpppV_t)

#endif // __wrappedgcryptTYPES_H_
