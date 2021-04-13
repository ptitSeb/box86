#!/usr/bin/env python

import os
import sys

"""
Generates all files in src/wrapped/generated
===

TL;DR: Automagically creates type definitions (/.F.+/ functions/typedefs...).
       All '//%' in the headers are used by the script.

Reads each lines of each "_private.h" headers (plus wrappedd3dadapter9_gen.h as a special case).
For each of them:
- If if starts with a #ifdef, #else, #ifndef, #endif, it memorizes which definition is required
- If it starts with a "GO", it will do multiple things:
  - It memorizes the type used by the function (second macro argument)
  - It memorizes the type it is mapped to, if needed (eg, iFEv is mapped to iFE: the second argument is dropped)
  - It checks if the type given (both original and )
  - If the signature contains a 'E', it will memorize the original function's signature:
    - If it is the special file case, or if the line contains "//%%", it stops there.
    - Otherwise, if it starts with "GOS":
      - It will search for the special marker "//%".
        That special marker is used to detect the following step:
        - If the marker is followed by a registered structure ID (see later), it transforms the assembly function signature
          into the original signature (eg, transforms pFEp into XFv if //%X was used)
        - If the marker is followed by a '{', it will not attempt to alter the given signature, and instead use the
          signature between '{' and '}'
    - Otherwise, if the line contains '//%{', like before it will use the signature given and not the original one.
    - Finally, if it still didn't stop, it will use the assembly signature (without the emulator).
- If the line starts with a '//%'

`gbl` contains the first list, `redirects` the second, `mytypedefs` the third.
`mystructs` and `mystructs_vals` constains data about the structures.

After sorting the data, it generates:

wrapper.c
---------
(Private) type definitions (/.F.+_t/)
Function definitions (/.F.+/ functions, that actually execute the function given as argument)
isSimpleWrapper definition

wrapper.h
---------
Generic "wrapper_t" type definition
Function declarations (/.F.+/ functions)

*types.h
--------
Local types definition, for the original signatures (eg, XFv for the earlier example)
The SUPER() macro definition, used to generate and initialize the `*_my_t` library structure
(TODO: also automate this declaration/definition? It would require more metadata,
 and may break sometime in the future due to the system changing...)
"""

# Free values:
# AB   F H J      QR T   XYZab    gh jk mno qrst   xyz
values = ['E', 'e', 'v', 'c', 'w', 'i', 'I', 'C', 'W', 'u', 'U', 'f', 'd', 'D', 'K', 'l', 'L', 'p', 'V', 'O', 'S', '2', 'P', 'G', 'N', 'M']
def splitchar(s):
	try:
		ret = [len(s), values.index(s[0])]
		for c in s[2:]:
			ret.append(values.index(c))
		return ret
	except ValueError as e:
		raise ValueError("Value is " + s) from e

def value(define):
	return define[9:-1] if define.startswith("!") else define[8:-1]

def splitdef(dnf, defines):
	cunjs = dnf.split(" || ")
	clauses = [c.split(" && ") for c in cunjs]
	
	ret = [len(cunjs)]
	
	for cunj in clauses:
		for c in cunj:
			ret.append(len(c))
	for cunj in clauses:
		for c in cunj:
			ret.append(defines.index(value(c)) * 2 + (1 if c.startswith("!") else 0))
	ret.append(0)
	return ret

def invert(define):
	return define[1:] if define.startswith("!") else ("!" + define)

def main(root, defines, files, ver):
	global values
	
	# Initialize variables: gbl for all values, redirects for redirections
	# mytypedefs is a list of all functions per "*FE*" types per filename, mystructs of structures per filename
	gbl        = {}
	redirects  = {}
	mytypedefs = {}
	mystructs  = {}
	mystructs_vals = {}
	
	# First read the files inside the headers
	# TODO: remove the special case...
	for filepath in files + [os.path.join(root, "src", "wrapped", "wrappedd3dadapter9_gen.h")]:
		filename = filepath.split("/")[-1]
		dependants = []
		with open(filepath, 'r') as file:
			for line in file:
				def fail(s, causedby=None):
					if causedby:
						raise NotImplementedError(s + " ({0}:{1})".format(filename, line[:-1])) from causedby
					else:
						raise NotImplementedError(s + " ({0}:{1})".format(filename, line[:-1]))
				
				ln = line.strip()
				# If the line is a `#' line (#ifdef LD80BITS/#ifndef LD80BITS/header)
				if ln.startswith("#"):
					preproc_cmd = ln[1:].strip()
					try:
						if preproc_cmd.startswith("if defined(GO)"):
							continue #if defined(GO) && defined(GOM)...
						elif preproc_cmd.startswith("if !(defined(GO)"):
							continue #if !(defined(GO) && defined(GOM)...)
						elif preproc_cmd.startswith("error"):
							continue #error meh!
						elif preproc_cmd.startswith("include"):
							continue #inherit other library
						elif preproc_cmd.startswith("endif"):
							if dependants != []:
								dependants.pop()
						elif preproc_cmd.startswith("ifdef"):
							if preproc_cmd[5:].strip() not in defines:
								raise KeyError(preproc_cmd[5:].strip())
							dependants.append("defined(" + preproc_cmd[5:].strip() + ")")
						elif preproc_cmd.startswith("ifndef"):
							if preproc_cmd[6:].strip() not in defines:
								raise KeyError(preproc_cmd[6:].strip())
							dependants.append("!defined(" + preproc_cmd[6:].strip() + ")")
						elif preproc_cmd.startswith("else"):
							dependants[-1] = invert(dependants[-1])
						else:
							fail("Unknown preprocessor directive: {0}".format(preproc_cmd.split(" ")[0]))
					except KeyError as k:
						fail("Unknown key: {0}".format(k.args[0]), k)
				# If the line is a `GO...' line (GO/GOM/GO2/...)...
				elif ln.startswith("GO"):
					# ... then look at the second parameter of the line
					gotype = ln.split("(")[0].strip()
					funname = ln.split(",")[0].split("(")[1].strip()
					ln = ln.split(",")[1].split(")")[0].strip()
					
					if len(ln) < 3:
						fail("Type {0} too short".format(ln))
					if "E" in ln:
						if ("E" in ln[:2]) or ("E" in ln[3:]):
							fail("emu64_t* not as the first parameter")
						if len(ln) < 4:
							fail("Type {0} too short".format(ln))
					if ln[0] not in values:
						fail("Invalid return type {0}".format(ln[0]))
					
					if ln[1] != "F":
						fail("Bad middle letter {0}".format(ln[1]))
					if any(c not in values for c in ln[2:]) or (('v' in ln[2:]) and (len(ln) > 3)):
						old = ln
						# This needs more work
						acceptables = ['v', '0', '1'] + values
						if any(c not in acceptables for c in ln[2:]):
							fail("Invalid type {0}".format(ln[2:]))
						# Ok, this is acceptable: there is 0, 1 and/or void
						ln = ln[:2] + (ln[2:]
							.replace("v", "")   # void   -> nothing
							.replace("0", "p")  # 0      -> pointer
							.replace("1", "i")) # 1      -> integer
						redirects.setdefault(" && ".join(dependants), {})
						redirects[" && ".join(dependants)][old] = ln
					# Simply append the function name if it's not yet existing
					gbl.setdefault(" && ".join(dependants), [])
					if ln not in gbl[" && ".join(dependants)]:
						gbl[" && ".join(dependants)].append(ln)
					
					if filename == "wrappedd3dadapter9_gen.h":
						pass # Special case...
					elif ln[2] == 'E':
						if "//%%" in line:
							# Do not dlsym functions containing "//%%" as metadata
							pass
						elif gotype == "GOS":
							# Scan the rest of the line to extract the return structure ID
							if filename[:-10] not in mystructs:
								fail("No structure info in the file")
							if "//%" not in line:
								fail("Invalid GOS (missing structure ID info)")
							if ln[0] != 'p':
								fail("Invalid GOS return type ('{0}' and not 'p')".format(ln[0]))
							#if (ln[2] != 'p') and ((ln[2] != 'E') or (ln[3] != 'p')): -> only allow pFEp for now
							#	fail("Invalid GOS first parameter ('{0}' and not 'p' or 'Ep')".format(ln[2:4]))
							if (ln[2] != 'E') or (ln[3] != 'p'):
								fail("Invalid GOS first parameter ('{0}' and not 'Ep')".format(ln[2:4]))
							
							sid = line.split("//%")[1].split(" ")[0].strip()
							if sid[0] == '{':
								# Change type completely, not just the return type...
								if sid[-1] != '}':
									fail("Invalid type (EOL or space met, expected '}')")
								if len(sid) < 5:
									fail("Invalid type (Type {0} too short)".format(sid[1:-1]))
								if sid[2] != "F":
									fail("Invalid type (Bad middle letter {0})".format(sid[2]))
								inval_char = lambda c:\
									(c not in mystructs[filename[:-10]]) and (c not in acceptables) \
									or (c == 'E') or (c == 'e')
								if inval_char(sid[1]):
									fail("Invalid type (Invalid return type {0})".format(sid[1]))
								if any(map(inval_char, sid[3:-1])):
									fail("Invalid type (Invalid type {0})".format(sid[3:-1]))
								mytypedefs.setdefault(filename[:-10], {})
								mytypedefs[filename[:-10]].setdefault(sid[1:-1], [])
								mytypedefs[filename[:-10]][sid[1:-1]].append((0, funname))
							else:
								if len(sid) != 1:
									fail("Invalid structure ID {0} (length is too big)".format(sid))
								if sid not in mystructs[filename[:-10]]:
									fail("Invalid structure ID {0} (unknown ID)".format(sid))
								mytypedefs.setdefault(filename[:-10], {})
								mytypedefs[filename[:-10]].setdefault(sid + "F" + ln[4:], [])
								mytypedefs[filename[:-10]][sid + "F" + ln[4:]].append((1, funname))
						elif "//%{" in line:
							# Change type completely...
							# ...Maybe?
							if filename[:-10] not in mystructs:
								fail("No structure info in the file")
							
							newtype = line.split("//%")[1].split(" ")[0].strip()
							if newtype[-1] != '}':
								fail("Invalid type (EOL or space met, expected '}')")
							if len(newtype) < 5:
								fail("Invalid type (Type {0} too short)".format(newtype[1:-1]))
							if newtype[2] != "F":
								fail("Invalid type (Bad middle letter {0})".format(newtype[2]))
							inval_char = lambda c:\
								(c not in mystructs[filename[:-10]]) and (c not in acceptables) \
								or (c == 'E') or (c == 'e')
							if inval_char(newtype[1]):
								fail("Invalid type (Invalid return type {0})".format(newtype[1]))
							if any(map(inval_char, newtype[3:-1])):
								fail("Invalid type (Invalid type {0})".format(newtype[3:-1]))
							mytypedefs.setdefault(filename[:-10], {})
							mytypedefs[filename[:-10]].setdefault(newtype[1:-1], [])
							mytypedefs[filename[:-10]][newtype[1:-1]].append((2, funname))
						else:
							# filename isn't stored with the '_private.h' part
							if not filename.endswith('_private.h'):
								fail("??? {0}".format(filename))
							if len(ln) > 3:
								ln = ln[:2] + ln[3:]
							else:
								ln = ln[:2] + "v"
							mytypedefs.setdefault(filename[:-10], {})
							mytypedefs[filename[:-10]].setdefault(ln, [])
							mytypedefs[filename[:-10]][ln].append((3, funname))
				elif ln.startswith("//%S"):
					# Extract a structure ID-name pair
					data = [s for s in map(lambda s: s.strip(), ln.split(" ")) if s != ""]
					if len(data) != 3:
						fail("Too much data ({0})".format(len(data)))
					if not filename.endswith('_private.h'):
						fail("??? {0}".format(filename))
					if (data[0] != "//%S") or (len(data[1]) != 1):
						fail("Invalid structure data {0} {1}".format(data[0], data[1]))
					if data[1] in values:
						fail("{0} cannot be used as a structure type".format(data[1]))
					mystructs.setdefault(filename[:-10], {})
					if data[1] in mystructs[filename[:-10]]:
						fail("Duplicate structure ID {0} ({1}/{2})".format(data[1], mystructs[filename[:-10]], data[2]))
					mystructs[filename[:-10]][data[1]] = data[2]
					mystructs_vals.setdefault(filename[:-10], [])
					mystructs_vals[filename[:-10]].append(data[1])
	
	if ("" not in gbl) or ("" not in redirects):
		print("\033[1;31mThere is suspiciously not many types...\033[m")
		print("Check the CMakeLists.txt file. If you are SURE there is nothing wrong"
			  " (as a random example, `set()` resets the variable...), then comment out the following return.")
		print("(Also, the program WILL crash later if you proceed.)")
		return 2 # Check what you did, not proceeding
	
	gbl_vals = {}
	for k in gbl:
		ks = k.split(" && ")
		for v in gbl[k]:
			if k == "":
				gbl_vals[v] = []
				continue
			if v in gbl_vals:
				if gbl_vals[v] == []:
					continue
				for other_key in gbl_vals[v]:
					other_key_vals = other_key.split(" && ")
					for other_key_val in other_key_vals:
						if other_key_val not in ks:
							break
					else:
						break
				else:
					gbl_vals[v].append(k)
			else:
				gbl_vals[v] = [k]
	for v in gbl_vals:
		for k in gbl_vals[v]:
			if " && ".join([invert(v2) for v2 in k.split(" && ")]) in gbl_vals[v]:
				gbl_vals[v] = []
				break
	gbl = {}
	gbl_idxs = []
	for k in gbl_vals:
		if len(gbl_vals[k]) == 1:
			key = gbl_vals[k][0]
		else:
			key = "(" + (") || (".join(gbl_vals[k])) + ")"
		gbl[key] = gbl.get(key, []) + [k]
		if (key not in gbl_idxs) and (key != "()"):
			gbl_idxs.append(key)
	gbl_idxs.sort(key=lambda v: splitdef(v, defines))
	
	redirects_vals = {}
	for k in redirects:
		ks = k.split(" && ")
		for v in redirects[k]:
			if k == "":
				redirects_vals[(v, redirects[k][v])] = []
				continue
			if (v, redirects[k][v]) in redirects_vals:
				if redirects_vals[(v, redirects[k][v])] == []:
					continue
				for other_key in redirects_vals[(v, redirects[k][v])]:
					if other_key == "()":
						break
					other_key_vals = other_key.split(" && ")
					for other_key_val in other_key_vals:
						if other_key_val not in ks:
							break
					else:
						break
				else:
					redirects_vals[(v, redirects[k][v])].append(k)
			else:
				redirects_vals[(v, redirects[k][v])] = [k]
	redirects = {}
	redirects_idxs = []
	for k, v in redirects_vals:
		key = "(" + (") || (".join(redirects_vals[(k, v)])) + ")"
		if key in redirects:
			redirects[key].append([k, v])
		else:
			redirects[key] = [[k, v]]
		if (key not in redirects_idxs) and (key != "()"):
			redirects_idxs.append(key)
	redirects_idxs.sort(key=lambda v: splitdef(v, defines))
	
	# Sort the tables
	for k in gbl:
		gbl[k].sort(key=lambda v: splitchar(v))
	values = values + ['0', '1']
	for k in redirects:
		redirects[k].sort(key=lambda v: splitchar(v[0]) + [-1] + splitchar(v[1]))
	values = values[:-2]
	mytypedefs_vals = {}
	for fn in mytypedefs:
		if fn in mystructs:
			values = values + list(mystructs[fn].keys())
		mytypedefs_vals[fn] = sorted(mytypedefs[fn].keys(), key=lambda v: splitchar(v))
		if fn in mystructs:
			values = values[:-len(mystructs[fn])]
		for v in mytypedefs_vals[fn]:
			mytypedefs[fn][v].sort()
	
	# Check if there was any new functions
	functions_list = ""
	for k in ["()"] + gbl_idxs:
		for v in gbl[k]:
			functions_list = functions_list + "#" + k + " " + v + "\n"
	for k in ["()"] + redirects_idxs:
		for v in redirects[k]:
			functions_list = functions_list + "#" + k + " " + v[0] + " -> " + v[1] + "\n"
	for fn in sorted(mystructs.keys()):
		for t in mystructs_vals[fn]:
			# Structure Definition
			functions_list = functions_list + fn + "/Sd" + t + mystructs[fn][t] + "\n"
	for fn in sorted(mytypedefs.keys()):
		for t in mytypedefs_vals[fn]:
			# Structure Usage
			for tnum, f in mytypedefs[fn][t]:
				functions_list = functions_list + fn + "/Su" + t + str(tnum) + f + "\n"
	
	# functions_list is a unique string, compare it with the last run
	try:
		last_run = ""
		with open(os.path.join(root, "src", "wrapped", "generated", "functions_list.txt"), 'r') as file:
			last_run = file.read()
		if last_run == functions_list:
			# Mark as OK for CMake
			with open(os.path.join(root, "src", "wrapped", "generated", "functions_list.txt"), 'w') as file:
				file.write(functions_list)
			return 0
	except IOError:
		# The file does not exist yet, first run
		pass
	
	# Now the files rebuilding part
	# Files header and guard
	files_header = {
		"wrapper.c": """/*****************************************************************
 * File automatically generated by rebuild_wrappers.py (v{version})
 *****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "wrapper.h"
#include "emu/x86emu_private.h"
#include "emu/x87emu_private.h"
#include "regs.h"
#include "x86emu.h"

typedef union ui64_s {lbr}
    int64_t     i;
    uint64_t    u;
    uint32_t    d[2];
{rbr} ui64_t;

typedef struct _2uint_struct_s {lbr}
	uint32_t	a;
	uint32_t	b;
{rbr} _2uint_struct_t;

extern void* my__IO_2_1_stderr_;
extern void* my__IO_2_1_stdin_ ;
extern void* my__IO_2_1_stdout_;

static void* io_convert(void* v)
{lbr}
	if(!v)
		return v;
	if(v==my__IO_2_1_stderr_)
		return stderr;
	if(v==my__IO_2_1_stdin_)
		return stdin;
	if(v==my__IO_2_1_stdout_)
		return stdout;
	return v;
{rbr}

typedef struct my_GValue_s
{lbr}
  int         g_type;
  union {lbr}
    int        v_int;
    int64_t    v_int64;
    uint64_t   v_uint64;
    float      v_float;
    double     v_double;
    void*      v_pointer;
  {rbr} data[2];
{rbr} my_GValue_t;

static void alignGValue(my_GValue_t* v, void* value)
{lbr}
    v->g_type = *(int*)value;
    memcpy(v->data, value+4, 2*sizeof(double));
{rbr}
static void unalignGValue(void* value, my_GValue_t* v)
{lbr}
    *(int*)value = v->g_type;
    memcpy(value+4, v->data, 2*sizeof(double));
{rbr}

void* VulkanFromx86(void* src, void** save);
void VulkanTox86(void* src, void* save);

#define ST0val ST0.d

int of_convert(int);

""",
		"wrapper.h": """/*****************************************************************
 * File automatically generated by rebuild_wrappers.py (v{version})
 *****************************************************************/
#ifndef __WRAPPER_H_
#define __WRAPPER_H_
#include <stdint.h>
#include <string.h>

typedef struct x86emu_s x86emu_t;

// the generic wrapper pointer functions
typedef void (*wrapper_t)(x86emu_t* emu, uintptr_t fnc);

// list of defined wrapper
// v = void, i = int32, u = uint32, U/I= (u)int64
// l = signed long, L = unsigned long (long is an int with the size of a pointer)
// p = pointer, P = callback
// f = float, d = double, D = long double, K = fake long double
// V = vaargs, E = current x86emu struct, e = ref to current x86emu struct
// 0 = constant 0, 1 = constant 1
// o = stdout
// C = unsigned byte c = char
// W = unsigned short w = short
// O = libc O_ flags bitfield
// S = _IO_2_1_stdXXX_ pointer (or FILE*)
// Q = ...
// 2 = struct of 2 uint
// P = Vulkan struture pointer
// G = a single GValue pointer
// N = ... automatically sending 1 arg
// M = ... automatically sending 2 args

""",
		"fntypes.h": """/*****************************************************************
 * File automatically generated by rebuild_wrappers.py (v{version})
 *****************************************************************/
#ifndef __{filename}TYPES_H_
#define __{filename}TYPES_H_

#ifndef LIBNAME
#error You should only #include this file inside a wrapped*.c file
#endif
#ifndef ADDED_FUNCTIONS
#define ADDED_FUNCTIONS() 
#endif

"""
	}
	files_guard = {"wrapper.c": """""",
		"wrapper.h": """
#endif // __WRAPPER_H_
""",
		"fntypes.h": """
#endif // __{filename}TYPES_H_
"""
	}
	
	#           E            e             v       c         w          i          I          C          W           u           U           f        d         D              K         l           L            p        V        O          S        2                  P        G        N      M
	tdtypes = ["x86emu_t*", "x86emu_t**", "void", "int8_t", "int16_t", "int32_t", "int64_t", "uint8_t", "uint16_t", "uint32_t", "uint64_t", "float", "double", "long double", "double", "intptr_t", "uintptr_t", "void*", "void*", "int32_t", "void*", "_2uint_struct_t", "void*", "void*", "...", "..."]
	if len(values) != len(tdtypes):
		raise NotImplementedError("len(values) = {lenval} != len(tdtypes) = {lentypes}".format(lenval=len(values), lentypes=len(tdtypes)))
	def generate_typedefs(arr, file):
		for v in arr:
			file.write("typedef " + tdtypes[values.index(v[0])] + " (*" + v + "_t)"
							+ "(" + ', '.join(tdtypes[values.index(t)] for t in v[2:]) + ");\n")
	
	# Rewrite the wrapper.c file:
	with open(os.path.join(root, "src", "wrapped", "generated", "wrapper.c"), 'w') as file:
		file.write(files_header["wrapper.c"].format(lbr="{", rbr="}", version=ver))
		
		# First part: typedefs
		generate_typedefs(gbl["()"], file)
		for k in gbl_idxs:
			file.write("\n#if " + k + "\n")
			generate_typedefs(gbl[k], file)
			file.write("#endif\n")
		
		file.write("\n")
		
		# Next part: function definitions
		
		# Helper variables
		arg = [
			"emu, ",                                  # E
			"&emu, ",                                 # e
			"",                                       # v
			"*(int8_t*)(R_ESP + {p}), ",              # c
			"*(int16_t*)(R_ESP + {p}), ",             # w
			"*(int32_t*)(R_ESP + {p}), ",             # i
			"*(int64_t*)(R_ESP + {p}), ",             # I
			"*(uint8_t*)(R_ESP + {p}), ",             # C
			"*(uint16_t*)(R_ESP + {p}), ",            # W
			"*(uint32_t*)(R_ESP + {p}), ",            # u
			"*(uint64_t*)(R_ESP + {p}), ",            # U
			"*(float*)(R_ESP + {p}), ",               # f
			"*(double*)(R_ESP + {p}), ",              # d
			"*(long double*)(R_ESP + {p}), ",         # D
			"FromLD((void*)(R_ESP + {p})), ",         # K
			"*(intptr_t*)(R_ESP + {p}), ",            # l
			"*(uintptr_t*)(R_ESP + {p}), ",           # L
			"*(void**)(R_ESP + {p}), ",               # p
			"(void*)(R_ESP + {p}), ",                 # V
			"of_convert(*(int32_t*)(R_ESP + {p})), ", # O
			"io_convert(*(void**)(R_ESP + {p})), ",   # S
			"(_2uint_struct_t){{*(uintptr_t*)(R_ESP + {p}),*(uintptr_t*)(R_ESP + {p} + 4)}}, ",	# 2
			"arg{p}, ",                               # P
			"&arg{p}, ",                              # G
			"*(void**)(R_ESP + {p}), ",				  # N
			"*(void**)(R_ESP + {p}),*(void**)(R_ESP + {p} + 4), ",	# M
		]
		#         E  e  v  c  w  i  I  C  W  u  U  f  d  D   K   l  L  p  V  O  S  2  P  G  N, M
		deltas = [0, 0, 4, 4, 4, 4, 8, 4, 4, 4, 8, 4, 8, 12, 12, 4, 4, 4, 0, 4, 4, 8, 4, 4, 0, 0]
		vals = [
			"\n#error Invalid return type: emulator\n",                     # E
			"\n#error Invalid return type: &emulator\n",                    # e
			"fn({0});",                                                     # v
			"R_EAX=fn({0});",                                               # c
			"R_EAX=fn({0});",                                               # w
			"R_EAX=fn({0});",                                               # i
			"ui64_t r; r.i=fn({0}); R_EAX=r.d[0]; R_EDX=r.d[1];",           # I
			"R_EAX=(unsigned char)fn({0});",                                # C
			"R_EAX=(unsigned short)fn({0});",                               # W
			"R_EAX=(uint32_t)fn({0});",                                     # u
			"ui64_t r; r.u=(uint64_t)fn({0}); R_EAX=r.d[0]; R_EDX=r.d[1];", # U
			"float fl=fn({0}); fpu_do_push(emu); ST0val = fl;",             # f
			"double db=fn({0}); fpu_do_push(emu); ST0val = db;",            # d
			"long double ld=fn({0}); fpu_do_push(emu); ST0val = ld;",       # D
			"double db=fn({0}); fpu_do_push(emu); ST0val = db;",            # K
			"R_EAX=(intptr_t)fn({0});",                                     # l
			"R_EAX=(uintptr_t)fn({0});",                                    # L
			"R_EAX=(uintptr_t)fn({0});",                                    # p
			"\n#error Invalid return type: va_list\n",                      # V
			"\n#error Invalid return type: at_flags\n",                     # O
			"\n#error Invalid return type: _io_file*\n",                    # S
			"\n#error Invalid return type: _2uint_struct\n",                # 2
			"\n#error Invalid return type: Vulkan Struct\n",                # P
			"\n#error Invalid return type: GValue Pointer\n",               # G
			"\n#error Invalid return type: ... with 1 arg\n",               # N
			"\n#error Invalid return type: ... with 2 args\n",              # M
		]
		# Asserts
		if len(values) != len(arg):
			raise NotImplementedError("len(values) = {lenval} != len(arg) = {lenarg}".format(lenval=len(values), lenarg=len(arg)))
		if len(values) != len(deltas):
			raise NotImplementedError("len(values) = {lenval} != len(deltas) = {lendeltas}".format(lenval=len(values), lendeltas=len(deltas)))
		if len(values) != len(vals):
			raise NotImplementedError("len(values) = {lenval} != len(vals) = {lenvals}".format(lenval=len(values), lenvals=len(vals)))
		
		# Helper functions to write the function definitions
		def function_args(args, d=4):
			if len(args) == 0:
				return ""
			if d % 4 != 0:
				raise ValueError("{d} is not a multiple of 4. Did you try passing a V and something else?".format(d=d))
			
			if args[0] == "0":
				return "(void*)(R_ESP + {p}), ".format(p=d) + function_args(args[1:], d + 4)
			elif args[0] == "1":
				return "1, " + function_args(args[1:], d)
			
			return arg[values.index(args[0])].format(p=d) + function_args(args[1:], d + deltas[values.index(args[0])])
		
		def function_writer(f, N, W, rettype, args):
			f.write("void {0}(x86emu_t *emu, uintptr_t fcn) {2} {1} fn = ({1})fcn; ".format(N, W, "{"))
			if any(cc in 'PG' for cc in args):
				# Vulkan struct or GValue pointer, need to unwrap functions at the end
				delta = 4
				for c in args:
					if c == 'P':
						f.write("void* save{d}=NULL; void *arg{d} = VulkanFromx86(*(void**)(R_ESP + {d}), &save{d}); ".format(d=delta))
					if c == 'G':
    						f.write("my_GValue_t arg{d}; alignGValue(&arg{d}, *(void**)(R_ESP + {d})); ".format(d=delta))
					delta = delta + deltas[values.index(c)]
				f.write(vals[values.index(rettype)].format(function_args(args)[:-2]) + " ")
				delta = 4
				for c in args:
					if c == 'P':
						f.write("VulkanTox86(arg{d}, save{d}); ".format(d=delta))
					if c == 'G':
						f.write("unalignGValue(*(void**)(R_ESP + {d}), &arg{d}); ".format(d=delta))
					delta = delta + deltas[values.index(c)]
				f.write("}\n")
			else:
				# Generic function
				f.write(vals[values.index(rettype)].format(function_args(args)[:-2]) + " }\n")
		
		for v in gbl["()"]:
			function_writer(file, v, v + "_t", v[0], v[2:])
		for k in gbl_idxs:
			file.write("\n#if " + k + "\n")
			for v in gbl[k]:
				function_writer(file, v, v + "_t", v[0], v[2:])
			file.write("#endif\n")
		file.write("\n")
		for v in redirects["()"]:
			function_writer(file, v[0], v[1] + "_t", v[0][0], v[0][2:])
		for k in redirects_idxs:
			file.write("\n#if " + k + "\n")
			for v in redirects[k]:
				function_writer(file, v[0], v[1] + "_t", v[0][0], v[0][2:])
			file.write("#endif\n")
		
		file.write(files_guard["wrapper.c"].format(lbr="{", rbr="}", version=ver))
	
	# Rewrite the wrapper.h file:
	with open(os.path.join(root, "src", "wrapped", "generated", "wrapper.h"), 'w') as file:
		file.write(files_header["wrapper.h"].format(lbr="{", rbr="}", version=ver))
		for v in gbl["()"]:
			file.write("void " + v + "(x86emu_t *emu, uintptr_t fnc);\n")
		for k in gbl_idxs:
			file.write("\n#if " + k + "\n")
			for v in gbl[k]:
				file.write("void " + v + "(x86emu_t *emu, uintptr_t fnc);\n")
			file.write("#endif\n")
		file.write("\n")
		for v in redirects["()"]:
			file.write("void " + v[0] + "(x86emu_t *emu, uintptr_t fnc);\n")
		for k in redirects_idxs:
			file.write("\n#if " + k + "\n")
			for v in redirects[k]:
				file.write("void " + v[0] + "(x86emu_t *emu, uintptr_t fnc);\n")
			file.write("#endif\n")
		file.write(files_guard["wrapper.h"].format(lbr="{", rbr="}", version=ver))
	
	for fn in mytypedefs:
		with open(os.path.join(root, "src", "wrapped", "generated", fn + "types.h"), 'w') as file:
			file.write(files_header["fntypes.h"].format(lbr="{", rbr="}", version=ver, filename=fn))
			if fn in mystructs:
				values = values + mystructs_vals[fn]
				tdtypes = tdtypes + [mystructs[fn][k] for k in mystructs_vals[fn]]
			generate_typedefs(mytypedefs_vals[fn], file)
			if fn in mystructs:
				values = values[:-len(mystructs_vals[fn])]
				tdtypes = tdtypes[:-len(mystructs_vals[fn])]
			file.write("\n#define SUPER() ADDED_FUNCTIONS()")
			for v in mytypedefs_vals[fn]:
				for t, f in mytypedefs[fn][v]:
					assert(t in [0, 1, 2, 3])
					file.write(" \\\n\tGO({0}, {1}_t)".format(f, v))
			file.write("\n")
			file.write(files_guard["fntypes.h"].format(lbr="{", rbr="}", version=ver, filename=fn))
	
	# Save the string for the next iteration, writing was successful
	with open(os.path.join(root, "src", "wrapped", "generated", "functions_list.txt"), 'w') as file:
		file.write(functions_list)
	
	return 0

if __name__ == '__main__':
	limit = []
	for i, v in enumerate(sys.argv):
		if v == "--":
			limit.append(i)
	if main(sys.argv[1], sys.argv[2:limit[0]], sys.argv[limit[0]+1:], "1.2.0.09") != 0:
		exit(2)
	exit(0)
