#!/usr/bin/env python3

import os
import re
import sys
from math import ceil

"""
Generates src/dynarec/arm_printer.c
===

See src/dynarec/arm_instructions.txt (the input file) for the syntax documentation.
"""

# Helper class to avoid displaying '\x1b[' on errors
class string(str):
	def __repr__(self):
		return str(self)

def nextAvailable(num):
	return 8 if num <= 8 else (16 if num <= 16 else (32 if num <= 32 else 64))

def int2hex(i, finsz=-1):
	if (finsz == -1) and (i == 0): return "0x0"
	elif i == 0: return "0x" + ("0" * finsz)
	ret = ""
	while i != 0:
		ret += str(i % 16) if i % 16 < 10 else chr(ord('A') + (i % 16) - 10)
		i = i // 16
	rl = len(ret)
	if rl < finsz:
		ret = ret + ("0" * (finsz - rl))
	return "0x" + ''.join(reversed(ret))
def arr2hex(a, forceBin=False):
	if forceBin:
		return "0b" + ''.join(map(str, a))
	else:
		al = len(a)
		return int2hex(sum(v * 2**(al - i - 1) for i, v in enumerate(a)), ceil(al / 4))

def sz2str(sz, forceBin=False):
	return int2hex(2 ** sz - 1) if not forceBin else ("0b" + "1" * sz)

def main(root, ver, __debug_forceAllDebugging=False):
	# Initialize variables
	output = ""
	
	# Debugging variable
	invalidationCount = 0
	
	tabCount = 1
	def append(strg):
		assert("\t" not in strg)
		nonlocal output, tabCount
		strg = strg.split("\n")
		for s in strg[:-1]:
			if s.endswith("{"):
				tabCount = tabCount + 1
			if s.startswith("}") and output.endswith("\t"):
				tabCount = tabCount - 1
				output = output[:-1]
			output = output + s + "\n" + "\t" * tabCount
		if strg[-1].startswith("}") and output.endswith("\t"):
			tabCount = tabCount - 1
			output = output[:-1]
		output = output + strg[-1]
	
	insts = None
	# Read the instruction and exit if nothing changed since last run
	with open(os.path.join(root, "src", "dynarec", "arm_instructions.txt"), 'r') as file:
		insts = file.read()
		
		# Get all actual instructions
		# Ignore white lines and lines beginning with either !, ; or #
		insts = list(filter(lambda l: not re.match("^\\s*$", l) and not re.match("^([!;#])", l), insts.split('\n')))
		
		try:
			# Do not open with `with` to be able to open it in writing mode
			last = open(os.path.join(root, "src", "dynarec", "last_run.txt"), 'r')
			if '\n'.join(insts) == last.read():
				last.close()
				with open(os.path.join(root, "src", "dynarec", "last_run.txt"), 'w') as f:
					f.write('\n'.join(insts))
				return 0
			last.close()
		except OSError:
			# No last run file
			pass
	
	for lnno, line in enumerate(insts):
		ln = line.strip()
		
		def fail(errType, reas, allow_use_curSplt=True):
			"""
			Throw an error of type `errType`, with `reas` as the reason.
			Appends the line number and the erroring line.
			
			`allow_use_curSplt` is a boolean, set to False if you want to ignore `curSplt` (no colored line)
			"""
			try:
				nonlocal curSplt
				if allow_use_curSplt and (curSplt >= 0):
					# Get a colorized line
					# (Blah blah [CSI-color change][Here is the error][CSI-color change] blah)
					line = ""
					alreadyChanged = 0
					csp = 0
					while csp < curSplt:
						if spltln[csp] == '\x01':
							alreadyChanged = 1
							curSplt = curSplt + 1
						else:
							if alreadyChanged == 0:
								line = line + spltln[csp] + " "
							elif alreadyChanged == 1:
								line = line + spltln[csp] + "<"
								alreadyChanged = 2
							else:
								line = line + spltln[csp] + ">"
								alreadyChanged = 1
						csp = csp + 1
					if spltln[csp] == '\x01':
						alreadyChanged = 1
						curSplt = curSplt + 1
						csp = csp + 1
					line = line + "\033[31m[\033[91m" + spltln[csp] + "\033[31m]\033[m"
					if alreadyChanged == 0:
						line = line + " "
					elif alreadyChanged == 1:
						line = line + "<"
						alreadyChanged = 2
					else:
						line = line + ">"
						alreadyChanged = 1
					csp = csp + 1
					while csp < len(spltln):
						if spltln[csp] == '\x01':
							alreadyChanged = 1
							curSplt = curSplt + 1
						else:
							if alreadyChanged == 0:
								line = line + spltln[csp] + " "
							elif alreadyChanged == 1:
								line = line + spltln[csp] + "<"
								alreadyChanged = 2
							else:
								line = line + spltln[csp] + ">"
								alreadyChanged = 1
						csp = csp + 1
					raise errType(string(str(reas) + " (" + str(lnno + 1) + ": " + line[:-1] + ")"))
				else:
					# No colored line
					raise errType(str(reas) + " (" + str(lnno + 1) + ": " + ln + ")")
			except errType as e:
				# Raise a BaseException as an error wrapper
				# (otherwise it will be caught and the line will be re-appended)
				raise BaseException("[Error wrapper]") from e
		
		spltln = ln.split(' ')
		curSplt = -1
		
		mask = [0] * 32
		correctBits = [0] * 32
		curBit = 32
		ocurBit = -1
		req = ''
		
		imms = [] # Immediates
		parm = [] # Custom variable length parameters
		
		variables = {}
		
		def generate_bin_test(positions=[], specifics=[]):
			"""
			Generates the if statement at the beginning.
			
			You may use positions and specifics to implement a "multiple choice if":
			- positions is an array that contains a bit position (MSB = 0)
			- specifics is an array of arrays the same length as positions that contains a tuple (mask, correctBit)
			  that is at position positions[current_pos]
			"""
			
			if specifics == []:
				append("if ((opcode & " + arr2hex(mask) + ") == " + arr2hex(correctBits) + ") {\n")
			else:
				l = len(positions)
				if any(map(lambda v: (v < 0) or (v > 31), positions)):
					fail(
						AssertionError,
						"generate_bin_tests requires a valid positions ({}) and specifics ({})!".format(
							len(positions), len(specifics)
						)
					)
				if any(map(lambda s: len(s) != l, specifics)):
					fail(
						AssertionError,
						"generate_bin_tests requires the same length for positions ({}) and each element "
						"of the specifics array ({})!".format(
							len(positions), [len(s) for s in specifics]
						)
					)
				
				inner = []
				for specific in specifics:
					for i, (m, c) in zip(positions, specific):
						mask[i] = m
						correctBits[i] = c
					inner.append("((opcode & " + arr2hex(mask) + ") == " + arr2hex(correctBits) + ")")
				append("if (" + " || ".join(inner) + ") {\n")
		
		def parse_var_requirements():
			"""
			Parse the `/` restrictions on the bit fields
			"""
			nonlocal req
			
			while req != '':
				key = req[0]
				req = req[1:]
				if key == '=':
					if len(req) < ocurBit - curBit:
						fail(KeyError, "Not enough data in constraint value (type '=' for val " + val + ")")
					for i in range(ocurBit - curBit):
						if req[0] == 'x':
							req = req[1:]
							continue
						elif (req[0] != '0') and (req[0] != '1'):
							fail(KeyError, "Invalid constraint '" + key + "..." + req[0] + "'")
						mask[32 - ocurBit + i] = 1
						correctBits[32 - ocurBit + i] = ord(req[0]) - ord('0')
						req = req[1:]
				elif (ord(key) >= ord('0')) and (ord(key) <= ord('9')):
					if req == '':
						fail(KeyError, "Not enough data in constraint value (type '" + key + "' for val " + \
							val + ")")
					elif (req[0] != '0') and (req[0] != '1'):
						fail(KeyError, "Invalid constraint '" + key + req[0] + "'")
					mask[31 - curBit + ord('0') - ord(key)] = 1
					correctBits[31 - curBit + ord('0') - ord(key)] = ord(req[0]) - ord('0')
					req = req[1:]
				elif (ord(key) >= ord('A')) and (ord(key) <= ord('F')):
					if req == '':
						fail(KeyError, "Not enough data in constraint value (type '" + key + "' for val " + \
							val + ")")
					elif (req[0] != '0') and (req[0] != '1'):
						fail(KeyError, "Invalid constraint '" + key + req[0] + "'")
					mask[31 - curBit + ord('A') + 10 - ord(key)] = 1
					correctBits[31 - curBit + ord('A') + 10 - ord(key)] = ord(req[0]) - ord('0')
					req = req[1:]
		
		def add_custom_variables():
			nonlocal curSplt
			
			# Check for any custom variables
			if len(parm) == 1:
				# One parameter, name it param
				append("int param = (opcode >> " + str(parm[0][0]) + ") & " + sz2str(parm[0][1]) + ";\n")
			else:
				# Multiple parameters, name them "param" paramNr "_" paramBitsSize
				for i, p in enumerate(parm):
					append(
						"int param" + str(i + 1) + "_" + str(p[1]) + " = (opcode >> " + \
						str(p[0]) + ") & " + sz2str(p[1]) + ";\n"
					)
			if spltln[curSplt] == '@':
				# Additional custom modifications
				append("\n")
				
				curSplt = curSplt + 1
				while spltln[curSplt] != '@':
					if '=' in spltln[curSplt]:
						# Add an `if` statement
						
						eq = spltln[curSplt]
						
						# Read initialization statement...
						curSplt = curSplt + 1
						oldSplt = curSplt
						while spltln[curSplt][-1] != '@':
							curSplt = curSplt + 1
							if len(spltln) == curSplt:
								fail(KeyError, "End of '=' switch not found!")
						
						# Always initialize if necessary
						if oldSplt != curSplt:
							append(' '.join(spltln[oldSplt:curSplt]) + ";\n")
						
						ifs = [""]
						
						if spltln[curSplt] == '@@':
							# Custom ifs
							# Also, requires only a single '='
							if eq != "=":
								fail(ValueError, "Too many '=' switches (@@ modifier)")
							
							# Extract the statements
							statements = []
							while spltln[curSplt] == '@@':
								curSplt = curSplt + 1
								
								statement = spltln[curSplt]
								while spltln[curSplt][-1] != '@':
									curSplt = curSplt + 1
									statement = statement + " " + spltln[curSplt]
									if len(spltln) == curSplt:
										fail(KeyError, "End of '=' switch (@@ modifier) not found!")
								statements.append(statement[:-1])
								curSplt = curSplt + 1
							
							# Unified eq length
							eq = statements + [0]
							
							# Make the ifs array
							for stmt in statements:
								ifs[-1] = ifs[-1] + "if (" + stmt + ") {\n"
								ifs.append("} else ")
							
							ifs[-1] = ifs[-1] + "{\n"
						else:
							# Standard if
							eq = eq.split('=')
							
							for e in eq[1:]:
								ifs[-1] = ifs[-1] + "if (" + eq[0] + " == " + e + ") {\n"
								ifs.append("} else ")
							
							ifs[-1] = ifs[-1] + "{\n"
						
						if spltln[curSplt] == '!@':
							# Custom statements
							curSplt = curSplt + 1
							
							# Extract the common value
							commonPart = spltln[curSplt]
							while spltln[curSplt][-1] != '@':
								curSplt = curSplt + 1
								commonPart = commonPart + " " + spltln[curSplt]
								if len(spltln) == curSplt:
									fail(KeyError, "End of '=' switch (!@ modifier) not found!")
							commonPart = commonPart[:-1].replace("\\n", "\n")
							curSplt = curSplt + 1
							
							# Extract the parts
							commonPart = commonPart.split('%')
							if len(commonPart) < 2:
								fail(ValueError, "No replacement place!")
							
							for if_ in ifs[:-1]:
								append(if_)
								
								# For each '%', append the preceding part and the variable part
								for common in commonPart[:-1]:
									insert = spltln[curSplt]
									while spltln[curSplt][-1] != '@':
										curSplt = curSplt + 1
										insert = insert + " " + spltln[curSplt]
										if len(spltln) == curSplt:
											fail(
												KeyError,
												"End of '=' switch (!@ modifier, repl1 i=" + str(i) + \
													" part) not found!"
											)
									
									insert = insert[:-1].replace("\\n", "\n")
									append(common + insert)
									
									curSplt = curSplt + 1
								append(commonPart[-1] + ";\n")
							
							# Finish with the `else` part
							append(ifs[-1])
							
							# For each '%', append the preceding part and the variable part
							for common in commonPart[:-1]:
								insert = spltln[curSplt]
								while spltln[curSplt][-1] != '@':
									curSplt = curSplt + 1
									insert = insert + " " + spltln[curSplt]
									if len(spltln) == curSplt:
										fail(KeyError, "End of '=' switch (!@ modifier, repl2 part) not found!")
								
								insert = insert[:-1].replace("\\n", "\n")
								append(common + insert)
								
								curSplt = curSplt + 1
							
							append(commonPart[-1] + ";\n}\n")
							
						else:
							# Standard statements
							dynvars = [dynvar.split(',') for dynvar in spltln[curSplt].split(';')]
							dynvars[-1][-1] = dynvars[-1][-1][:-1]
							curSplt = curSplt + 1
							if any(len(dynvar) != len(eq) + 1 for dynvar in dynvars):
								fail(ValueError, "Not enough/too many possibilities in switch")
							
							# Reorganize dynvars so it matches [[variables], [set-if-A-true], ..., [set-else]]
							dynvars = list(map(lambda i: [dv[i] for dv in dynvars], range(len(dynvars[0]))))
							
							for if_, vals in zip(ifs[:-1], dynvars[1:]):
								append(if_)
								for var, val in zip(dynvars[0], vals):
									append(var + " = " + val + ";\n")
							
							# Else
							append(ifs[-1])
							for var, val in zip(dynvars[0], dynvars[-1]):
								append(var + " = " + val + ";\n")
							append("}\n")
						
					elif spltln[curSplt] == "set":
						# Set a (new?) variable
						curSplt = curSplt + 1
						oldSplt = curSplt
						while spltln[curSplt][-1] != '@':
							curSplt = curSplt + 1
							if len(spltln) == curSplt:
								fail(KeyError, "End of '=' switch not found!")
						
						# If the statement is empty just add a blank line
						if (oldSplt == curSplt) and (spltln[curSplt] == "@"):
							append("\n")
						else:
							append(' '.join(spltln[oldSplt:curSplt + 1])[:-1] + ";\n")
						
						curSplt = curSplt + 1
					else:
						fail(KeyError, "Unknown custom statement type '" + spltln[curSplt] + "'")
				curSplt = curSplt + 1
		
		try:
			if spltln[0] == "ARM_":
				# ARM instruction
				
				variables["cond"] = -1
				variables["registers"] = {
					"d": [-1, -1, -1, -1, -1], # Register: #, ##, Rd, RdLo, RdHi
					"t": [-1, -1, -1], # Register: #, ##, Rt
					"n": [-1, -1, -1], # Register: #, ##, Rn
					"m": [-1, -1, -1], # Register: #, ##, Rm
					"a": [-1, -1, -1]  # Register: #, ##, Ra
				}
				variables["reglist16"] = -1 # Register list (16-bits)
				
				variables["s"] = -1 # Set flags
				
				variables["u"] = -1 # Unsigned
				
				variables["r"] = -1 # Rotate
				
				variables["sb"] = [-1, -1, -1] # lsb, msb
				
				variables["w"] = -1 # wback
				
				for i, val in enumerate(spltln[1:]):
					if '/' in val:
						ocurBit = curBit
						req = val.split('/')
						val, req = req[0], '/'.join(req[1:])
					
					if val == '0':
						curBit = curBit - 1
						mask[31 - curBit] = 1
					elif val == '1':
						curBit = curBit - 1
						mask[31 - curBit] = 1
						correctBits[31 - curBit] = 1
					elif (val == '(0)') or (val == '(1)'):
						# Ignore, even though the result should be undefined...
						curBit = curBit - 1
					elif val == 'cond':
						curBit = curBit - 4
						variables["cond"] = curBit
					elif val == 'Rd':
						curBit = curBit - 4
						variables["registers"]["d"][2] = curBit
					elif val == 'RdLo':
						curBit = curBit - 4
						variables["registers"]["d"][3] = curBit
					elif val == 'RdHi':
						curBit = curBit - 4
						variables["registers"]["d"][4] = curBit
					elif val == 'Rt':
						curBit = curBit - 4
						variables["registers"]["t"][2] = curBit
					elif val == 'Rn':
						curBit = curBit - 4
						variables["registers"]["n"][2] = curBit
					elif val == 'Rm':
						curBit = curBit - 4
						variables["registers"]["m"][2] = curBit
					elif val == 'Ra':
						curBit = curBit - 4
						variables["registers"]["a"][2] = curBit
					elif val == 'S':
						curBit = curBit - 1
						variables["s"] = curBit
					elif val == 'U':
						curBit = curBit - 1
						variables["u"] = curBit
					elif val == 'W':
						curBit = curBit - 1
						variables["w"] = curBit
					elif val == 'rotate':
						curBit = curBit - 2
						variables["r"] = curBit
					elif val == 'lsb':
						curBit = curBit - 5
						variables["sb"][0] = curBit
					elif val == 'msb':
						curBit = curBit - 5
						variables["sb"][1] = curBit
					elif val == 'widthm1':
						curBit = curBit - 5
						variables["sb"][2] = curBit
					elif val == 'register_list':
						curBit = curBit - 16
						variables["reglist16"] = curBit
					elif val == 'sat_imm':
						curBit = curBit - 4
						imms.append([curBit, 4])
						
					elif val.startswith('@<') and val.endswith('>'):
						parmlen = int(val[2:-1])
						curBit = curBit - parmlen
						parm.append([curBit, parmlen])
					elif val.startswith('imm'):
						if val.endswith('H') or val.endswith('L'): val = val[:-1]
						immsz = int(val[3:])
						curBit = curBit - immsz
						imms.append([curBit, immsz])
						
					else:
						fail(KeyError, "Unknown value '" + val + "'")
					
					curSplt = i + 2
					parse_var_requirements()
					
					if curBit == 0:
						break
					elif curBit < 0:
						fail(KeyError, "Current bit too low (" + str(curBit) + ")")
				
				if curSplt == -1:
					fail(KeyError, "Not enough arguments")
				
				generate_bin_test()
				
				# Add C variables
				if variables["s"] != -1:
					append("int s = (opcode >> " + str(variables["s"]) + ") & 1;\n")
				if variables["cond"] != -1:
					if mask[31 - variables["cond"]] == 0:
						append("const char* cond = conds[(opcode >> " + str(variables["cond"]) + ") & 0xF];\n")
					else:
						variables["cond"] = -2
				if variables["registers"]["d"][2] != -1:
					append("int rd = (opcode >> " + str(variables["registers"]["d"][2]) + ") & 0xF;\n")
				if variables["registers"]["d"][3] != -1:
					assert(variables["registers"]["d"][4] != -1)
					append("int rdlo = (opcode >> " + str(variables["registers"]["d"][3]) + ") & 0xF;\n")
					append("int rdhi = (opcode >> " + str(variables["registers"]["d"][4]) + ") & 0xF;\n")
				if variables["registers"]["t"][2] != -1:
					append("int rt = (opcode >> " + str(variables["registers"]["t"][2]) + ") & 0xF;\n")
				if variables["registers"]["n"][2] != -1:
					append("int rn = (opcode >> " + str(variables["registers"]["n"][2]) + ") & 0xF;\n")
				if variables["registers"]["m"][2] != -1:
					append("int rm = (opcode >> " + str(variables["registers"]["m"][2]) + ") & 0xF;\n")
				if variables["registers"]["a"][2] != -1:
					append("int ra = (opcode >> " + str(variables["registers"]["a"][2]) + ") & 0xF;\n")
				if variables["u"] != -1:
					append("int u = (opcode >> " + str(variables["u"]) + ") & 1;\n")
				if variables["w"] != -1:
					append("int w = (opcode >> " + str(variables["w"]) + ") & 1;\n")
				if variables["r"] != -1:
					append("int rot = (opcode >> " + str(variables["r"]) + ") & 3;\nchar tmprot[8] = {0};\n")
					append("if (rot) {\nsprintf(tmprot, \" ror %d\", rot * 8);\n}\n")
				if variables["sb"][0] != -1:
					append("int lsb = (opcode >> " + str(variables["sb"][0]) + ") & 0x1F;\n")
				if variables["sb"][1] != -1:
					append("int msb = (opcode >> " + str(variables["sb"][1]) + ") & 0x1F;\n")
				if variables["sb"][2] != -1:
					append("int widthm1 = (opcode >> " + str(variables["sb"][2]) + ") & 0x1F;\n")
				if variables["reglist16"] != -1:
					append("int reglist = (opcode >> " + str(variables["reglist16"]) + ") & 0xFFFF;\n")
				if imms != []:
					immssz = sum(map(lambda v: v[1], imms))
					tmp = "(" * len(imms)
					for immpos, immsz in imms:
						if tmp[-1] != '(': tmp = tmp + " << " + str(immsz) + ") | "
						tmp = tmp + "((opcode >> " + str(immpos) + ") & " + sz2str(immsz) + ")"
					tmp = tmp[1:]
					append("uint" + str(nextAvailable(immssz)) + "_t imm" + str(immssz) + " = " + tmp + ";\n")
					
					# Destroy imms since we don't need it anymore, but we do need immssz
					imms = immssz
				
				add_custom_variables()
				
				append("\nsprintf(ret, \"")
				
				# Assemble the variables into the printf
				instText = ' '.join(spltln[curSplt:]).split('<')
				instText = [instTextPart.split('>') for instTextPart in instText]
				instText = [itp for itp2 in instText for itp in itp2]
				
				# Make the failures nicer
				spltln = spltln[:curSplt] + ["\x01"] + instText
				
				printf_args = ""
				for idx, text in enumerate(instText):
					text = text.replace("&l", "<").replace("&g", ">")
					
					if "+/-" in text:
						text = text.replace("+/-", "%s")
						printf_args = printf_args + ", (u ? \"\" : \"-\")"
					if "{!}" in text:
						text = text.replace("{!}", "%s")
						printf_args = printf_args + ", (w ? \"!\" : \"\")"
					
					skiprbr = False
					if idx % 2:
						if text == "c":
							if variables["cond"] >= 0:
								append("%s")
								printf_args = printf_args + ", cond"
						elif text == "Rd":
							append("%s")
							printf_args = printf_args + ", regname[rd]"
						elif text == "RdLo":
							append("%s")
							printf_args = printf_args + ", regname[rdlo]"
						elif text == "RdHi":
							append("%s")
							printf_args = printf_args + ", regname[rdhi]"
						elif text == "Rt":
							append("%s")
							printf_args = printf_args + ", regname[rt]"
						elif text == "Rt2":
							append("%s")
							printf_args = printf_args + ", regname[rt + 1]"
						elif text == "Rn":
							append("%s")
							printf_args = printf_args + ", regname[rn]"
						elif text == "Rm":
							append("%s")
							printf_args = printf_args + ", regname[rm]"
						elif text == "Ra":
							append("%s")
							printf_args = printf_args + ", regname[ra]"
						elif text == "const":
							assert(imms == 12)
							append("0x%x")
							printf_args = printf_args + ", print_modified_imm_ARM(imm12)"
						elif text == "rotation":
							if output.endswith("{, "):
								output = output[:-3]
								skiprbr = True
							append("%s")
							printf_args = printf_args + ", tmprot"
						elif text == "label":
							append("%+d")
							printf_args = printf_args + ", (imm24 & 0x800000 ? imm24 | 0xFF000000 : imm24) + 2"
						elif text == "lsb":
							append("%d")
							printf_args = printf_args + ", lsb"
						elif text == "width":
							append("%d")
							printf_args = printf_args + (", msb - lsb + 1" if variables["sb"][1] != -1 else ", widthm1 + 1")
						elif text == "registers":
							append("%s")
							printf_args = printf_args + ", print_register_list(reglist, 16)"
						elif text == "imm":
							#append("0x%0" + str(ceil(imms / 4)) + "x")
							append("0x%x")
							printf_args = printf_args + ", imm" + str(imms)
							
						elif text.startswith("imm"):
							if variables["u"] != -1:
								append("%d")
							else:
								#append("0x%0" + str(ceil(int(text[3:]) / 4)) + "x")
								append("0x%x")
							printf_args = printf_args + ", " + text
						else:
							fail(KeyError, "Unknown variable " + text)
					elif text.endswith("{S}"):
						append(text[:-3] + "%s")
						printf_args = printf_args + ", (s ? \"S\" : \"\")"
					else:
						if text == "":
							continue
						
						text = text.split('\\')
						while len(text) > 1:
							if text[1][0] == '%':
								modifier, text[1] = '%', text[1][1:]
								while (len(text[1]) > 1) \
								and (text[1][0] in ['0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+']):
									modifier, text[1] = modifier + text[1][0], text[1][1:]
								modifier, text[1] = modifier + text[1][0], text[1][1:]
								append(text[0] + modifier)
							else:
								append(text[0] + "%s")
							printf_args = printf_args + ", " + text[1]
							text = text[2:]
						if len(text) == 0:
							fail(AssertionError, "Substitution not finished")
						if skiprbr and (text[0][0] == "}"):
							text[0] = text[0][1:]
						append(text[0])
					
					curSplt = curSplt + 1
				append("\"" + printf_args + ");\n} else ")
				
			elif (spltln[0].upper() == "ARMS") or (spltln[0] == "ARM$"):
				# ARM Shift instruction
				# ARMs is only with immediate, ARM$ is only with register
				
				variables["cond"] = -1
				variables["registers"] = {
					"d": [-1, -1, -1], # Register: #, ##, Rd
					"t": [-1, -1, -1], # Register: #, ##, Rt
					"n": [-1, -1, -1], # Register: #, ##, Rn
					"m": [-1, -1, -1]  # Register: #, ##, Rm
				}
				
				variables["s"] = -1 # Set flags
				
				variables["u"] = -1 # Unsigned shift
				
				variables["w"] = -1 # wback (used in adressing)
				
				variables["shift"] = False # Shift already detected?
				
				for i, val in enumerate(spltln[1:]):
					if '/' in val:
						ocurBit = curBit
						req = val.split('/')
						val, req = req[0], '/'.join(req[1:])
					
					if val == '0':
						curBit = curBit - 1
						mask[31 - curBit] = 1
					elif val == '1':
						curBit = curBit - 1
						mask[31 - curBit] = 1
						correctBits[31 - curBit] = 1
					elif (val == '(0)') or (val == '(1)'):
						# Ignore, even though the result should be undefined...
						curBit = curBit - 1
					elif val == 'cond':
						curBit = curBit - 4
						variables["cond"] = curBit
					elif val == 'Rd':
						curBit = curBit - 4
						variables["registers"]["d"][2] = curBit
					elif val == 'Rt':
						curBit = curBit - 4
						variables["registers"]["t"][2] = curBit
					elif val == 'Rn':
						curBit = curBit - 4
						variables["registers"]["n"][2] = curBit
					elif val == 'Rm':
						curBit = curBit - 4
						variables["registers"]["m"][2] = curBit
					elif val == 'S':
						curBit = curBit - 1
						variables["s"] = curBit
					elif val == 'U':
						curBit = curBit - 1
						variables["u"] = curBit
					elif val == 'W':
						curBit = curBit - 1
						variables["w"] = curBit
					elif val == 'type':
						curBit = curBit - 2
						imms.append([curBit, 2])
						variables["shift"] = True
					elif val == 'sat_imm':
						curBit = curBit - 5
						imms.append([curBit, 5, True])
					elif val == 'sh':
						curBit = curBit - 1
						
					elif val.startswith('@<') and val.endswith('>'):
						parmlen = int(val[2:-1])
						curBit = curBit - parmlen
						parm.append([curBit, parmlen])
					elif val.startswith('imm'):
						if variables["shift"]: fail(ValueError, "Immediate after a 'type' value detected")
						immsz = int(val[3:])
						curBit = curBit - immsz
						imms.append([curBit, immsz])
						variables["shift"] = immsz == 12
						
					else:
						fail(KeyError, "Unknown value '" + val + "'")
					
					curSplt = i + 2
					parse_var_requirements()
					
					if curBit == 0:
						break
					elif curBit < 0:
						fail(KeyError, "Current bit too low (" + str(curBit) + ")")
				
				if curSplt == -1:
					fail(KeyError, "Not enough arguments")
				# There is not necessarily an explicit shift when using an ARMs
				elif not variables["shift"] and (spltln[0] == "ARMS"):
					fail(KeyError, "No shift detected")
				
				# Assumption:
				# bytes 11-4 are [imm5 type 0] (then auto-complete for [Rs 0 type 1 Rm])
				if (spltln[0] == "ARMS") and ((mask[20:24] + mask[25:28] != [0, 0, 0, 0, 0, 0, 1]) \
				 or ((correctBits[27] != 0) or (len(imms) != 2) or (imms[0] != [7, 5]))):
					fail(NotImplementedError, "Unknown case with shift")
				
				if (spltln[0] == "ARMS"):
					generate_bin_test([24, 27], [[(0, 0), (1, 0)], [(1, 0), (1, 1)]])
				else:
					generate_bin_test()
				
				# Add C variables
				if variables["s"] != -1:
					append("int s = (opcode >> " + str(variables["s"]) + ") & 1;\n")
				if variables["cond"] != -1:
					if mask[31 - variables["cond"]] == 0:
						append("const char* cond = conds[(opcode >> " + str(variables["cond"]) + ") & 0xF];\n")
					else:
						variables["cond"] = -2
				if variables["registers"]["d"][2] != -1:
					append("int rd = (opcode >> " + str(variables["registers"]["d"][2]) + ") & 0xF;\n")
				if variables["registers"]["t"][2] != -1:
					append("int rt = (opcode >> " + str(variables["registers"]["t"][2]) + ") & 0xF;\n")
				if variables["registers"]["n"][2] != -1:
					append("int rn = (opcode >> " + str(variables["registers"]["n"][2]) + ") & 0xF;\n")
				if variables["registers"]["m"][2] != -1:
					append("int rm = (opcode >> " + str(variables["registers"]["m"][2]) + ") & 0xF;\n")
				if variables["u"] != -1:
					append("int u = (opcode >> " + str(variables["u"]) + ") & 1;\n")
				if variables["w"] != -1:
					append("int w = (opcode >> " + str(variables["w"]) + ") & 1;\n")
				if (len(imms) == 2) and (len(imms[0]) == 3):
					append("uint8_t imm = ((opcode >> " + str(imms[0][0]) + ") & 0x1F) + 1;\n")
				append("uint8_t shift = ((opcode >> 4) & " + \
					("0xFF)" if (spltln[0] == "ARMS") else ("0xFE)" if (spltln[0] == "ARMs") else "0xFF) | 0x01")) + \
					";\n")
				
				add_custom_variables()
				
				append("\nsprintf(ret, \"")
				
				# Assemble the variables into the printf
				instText = ' '.join(spltln[curSplt:]).split('<')
				instText = [instTextPart.split('>') for instTextPart in instText]
				instText = [itp for itp2 in instText for itp in itp2]
				
				# Make the failures nicer
				spltln = spltln[:curSplt] + ["\x01"] + instText
				
				printf_args = ""
				for idx, text in enumerate(instText):
					text = text.replace("&l", "<").replace("&g", ">")
					
					if "+/-" in text:
						text = text.replace("+/-", "%s")
						printf_args = printf_args + ", (u ? \"\" : \"-\")"
					if "{!}" in text:
						text = text.replace("{!}", "%s")
						printf_args = printf_args + ", (w ? \"!\" : \"\")"
					
					if idx % 2:
						if text == "c":
							if variables["cond"] != -1:
								append("%s")
								printf_args = printf_args + ", cond"
						elif text == "Rd":
							append("%s")
							printf_args = printf_args + ", regname[rd]"
						elif text == "Rt":
							append("%s")
							printf_args = printf_args + ", regname[rt]"
						elif text == "Rt2":
							append("%s")
							printf_args = printf_args + ", regname[rt + 1]"
						elif text == "Rn":
							append("%s")
							printf_args = printf_args + ", regname[rn]"
						elif text == "Rm":
							append("%s")
							printf_args = printf_args + ", regname[rm]"
						elif text == "imm":
							append("%d")
							printf_args = printf_args + ", imm"
						elif text == "shift":
							# Tricky, since it is optional, but I want to have a pure copy-paste of the official doc
							# so we remove the end of the output if necessary
							# Otherwise it is as usual, however we use the print_shift method
							comma = "0"
							if output.endswith("{, "):
								output = output[:-3]
								comma = "1"
							append("%s")
							printf_args = printf_args + ", print_shift(shift, " + comma + ")"
						else:
							fail(KeyError, "Unknown variable " + text)
					elif text.endswith("{S}"):
						append(text[:-3] + "%s")
						printf_args = printf_args + ", (s ? \"S\" : \"\")"
					elif text.startswith("}"):
						append(text[1:])
					else:
						if text == "":
							continue
						
						text = text.split('\\')
						while len(text) > 1:
							if text[1][0] == '%':
								modifier, text[1] = '%', text[1][1:]
								while (len(text[1]) > 1) \
								and (text[1][0] in ['0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+']):
									modifier, text[1] = modifier + text[1][0], text[1][1:]
								modifier, text[1] = modifier + text[1][0], text[1][1:]
								append(text[0] + modifier)
							else:
								append(text[0] + "%s")
							printf_args = printf_args + ", " + text[1]
							text = text[2:]
						if len(text) == 0:
							fail(AssertionError, "Substitution not finished")
						append(text[0])
					
					curSplt = curSplt + 1
				append("\"" + printf_args + ");\n} else ")
				
			elif spltln[0] == "NEON":
				# NEON (advanced SIMD) instruction
				
				variables["registers"] = {
					"d": [-1, -1, -1], # Register: D, Vd, ##
					"t": [-1, -1, -1], # Register: #, ##, Rt
					"n": [-1, -1, -1], # Register: N, Vn, ##
					"m": [-1, -1, -1]  # Register: M, Vm, ##
				}
				
				variables["op"] = -1 # Operation
				variables["u"] = -1 # Unsigned
				variables["f"] = -1 # Floating-point
				variables["sz"] = [-1, 0] # Operation size: pos, len
				
				variables["cmode"] = -1 # Used with immediates
				
				variables["q"] = -1 # Quadword
				
				for i, val in enumerate(spltln[1:]):
					if '/' in val:
						ocurBit = curBit
						req = val.split('/')
						val, req = req[0], '/'.join(req[1:])
					
					if val == '0':
						curBit = curBit - 1
						mask[31 - curBit] = 1
					elif val == '1':
						curBit = curBit - 1
						mask[31 - curBit] = 1
						correctBits[31 - curBit] = 1
					elif (val == '(0)') or (val == '(1)'):
						# Ignore, even though the result should be undefined...
						curBit = curBit - 1
					elif val == 'D':
						curBit = curBit - 1
						variables["registers"]["d"][0] = curBit
					elif val == 'Vd':
						curBit = curBit - 4
						variables["registers"]["d"][1] = curBit
					elif val == 'Rt':
						curBit = curBit - 4
						variables["registers"]["t"][2] = curBit
					elif val == 'N':
						curBit = curBit - 1
						variables["registers"]["n"][0] = curBit
					elif val == 'Vn':
						curBit = curBit - 4
						variables["registers"]["n"][1] = curBit
					elif val == 'Rn':
						curBit = curBit - 4
						variables["registers"]["n"][2] = curBit
					elif val == 'M':
						curBit = curBit - 1
						variables["registers"]["m"][0] = curBit
					elif val == 'Vm':
						curBit = curBit - 4
						variables["registers"]["m"][1] = curBit
					elif val == 'Rm':
						curBit = curBit - 4
						variables["registers"]["m"][2] = curBit
					elif val == 'U':
						curBit = curBit - 1
						variables["u"] = curBit
					elif val == 'F':
						curBit = curBit - 1
						variables["f"] = curBit
					elif val == 'op':
						curBit = curBit - 1
						variables["op"] = curBit
					elif val == 'cmode':
						curBit = curBit - 4
						variables["cmode"] = curBit
					elif val == 'size':
						curBit = curBit - 2
						variables["sz"] = [curBit, 2]
					elif val == 'sz':
						curBit = curBit - 1
						variables["sz"] = [curBit, 1]
					elif val == 'Q':
						curBit = curBit - 1
						variables["q"] = curBit
					elif val == 'i':
						curBit = curBit - 1
						imms.append([curBit, 1, val])
					elif val == 'L':
						curBit = curBit - 1
						imms.append([curBit, 1, val])
						
					elif val.startswith('@<') and val.endswith('>'):
						parmlen = int(val[2:-1])
						curBit = curBit - parmlen
						parm.append([curBit, parmlen])
					elif val.lower().startswith('imm'):
						immsz = int(val[3:])
						curBit = curBit - immsz
						imms.append([curBit, immsz, val])
						
					else:
						fail(KeyError, "Unknown value '" + val + "'")
					
					curSplt = i + 2
					parse_var_requirements()
					
					if curBit == 0:
						break
					elif curBit < 0:
						fail(KeyError, "Current bit too low (" + str(curBit) + ")")
				
				if curSplt == -1:
					fail(KeyError, "Not enough arguments")
				
				generate_bin_test()
				
				# Add C variables
				if variables["op"] != -1:
					append("int op = (opcode >> " + str(variables["op"]) + ") & 1;\n")
				if (variables["u"] != -1) and (variables["f"] != -1):
					fail(ValueError, "Cannot have both unsigned and floating-point", False)
				if variables["u"] != -1:
					append("int u = (opcode >> " + str(variables["u"]) + ") & 1;\n")
				if variables["f"] != -1:
					append("int f = (opcode >> " + str(variables["f"]) + ") & 1;\n")
				if variables["sz"][0] != -1:
					if (mask[31 - variables["sz"][0]] == 0) and ((variables["sz"][1] == 1) or (mask[30 - variables["sz"][0]] == 0)):
						append("int size = (opcode >> " + str(variables["sz"][0]) + ") & " + sz2str(variables["sz"][1]) + ";\n")
					else:
						variables["sz"][0] = -2
				if variables["q"] != -1:
					if mask[31 - variables["q"]] == 0:
						append("int q = (opcode >> " + str(variables["q"]) + ") & 1;\n")
					else:
						variables["q"] = -2
				if variables["registers"]["d"][0] != -1:
					assert(variables["registers"]["d"][1] != -1)
					append("int d = ((opcode >> " + str(variables["registers"]["d"][0]) + ") & 1) << 4 " + \
						"| ((opcode >> " + str(variables["registers"]["d"][1]) + ") & 0xF);\n")
				if variables["registers"]["t"][2] != -1:
					append("int rt = (opcode >> " + str(variables["registers"]["t"][2]) + ") & 0xF;\n")
				if variables["registers"]["n"][0] != -1:
					assert(variables["registers"]["n"][1] != -1)
					append("int n = ((opcode >> " + str(variables["registers"]["n"][0]) + ") & 1) << 4 " + \
						"| ((opcode >> " + str(variables["registers"]["n"][1]) + ") & 0xF);\n")
				if variables["registers"]["n"][2] != -1:
					append("int rn = (opcode >> " + str(variables["registers"]["n"][2]) + ") & 0xF;\n")
				if variables["registers"]["m"][0] != -1:
					assert(variables["registers"]["m"][1] != -1)
					append("int m = ((opcode >> " + str(variables["registers"]["m"][0]) + ") & 1) << 4 " + \
						"| ((opcode >> " + str(variables["registers"]["m"][1]) + ") & 0xF);\n")
				if variables["registers"]["m"][2] != -1:
					append("int rm = (opcode >> " + str(variables["registers"]["m"][2]) + ") & 0xF;\n")
				if variables["cmode"] != -1:
					append("int cmode = (opcode >> " + str(variables["cmode"]) + ") & 0xF;\n")
				if imms != []:
					if (variables["cmode"] != -1) and (variables["op"] != -1):
						# Modified immediate value
						
						# Assert unique imms pattern since this is a NEON instruction, reduce the result's length
						assert(imms == [[24, 1, 'i'], [16, 3, 'imm3'], [0, 4, 'imm4']])
						append("uint8_t imm8 = " + \
							"(((opcode >> 24) & 0x1) << 7) | (((opcode >> 16) & 0x7) << 4) | (opcode & 0xF);" + \
							"\nconst char* decodedImm = print_modified_imm_NEON((op << 4) + cmode, imm8);\n")
						
						imms = [0, 8] + imms
					elif imms[0][1] in [6]:
						# Shift value
						assert((len(imms) == 1) or ((len(imms) == 2) and (imms[1][2] == 'L')))
						
						l = len(imms) == 2
						if l:
							append("uint8_t l = (opcode >> " + str(imms[1][0]) + ") & 0x1;\n")
						
						immssz = 6
						append("uint8_t imm6 = (opcode >> " + str(imms[0][0]) + ") & 0x3F;\n")
						
						if imms[0][2][:3] != "IMM":
							append("uint8_t decodedImm = 0;\nuint8_t size = 0;\n")
							if imms[0][2][:3] == "imm":
								append(
									("if (l == 1) {\ndecodedImm = 64 - imm6;\nsize = 3;\n} else " if l else "") + \
									"if (imm6 & 0x20) {\ndecodedImm = 64 - imm6;\nsize = 2;\n} else " + \
									"if (imm6 & 0x10) {\ndecodedImm = 32 - imm6;\nsize = 1;\n} else " + \
									"{\ndecodedImm = 16 - imm6;\n}\n"
								)
							else:
								append(
									("if (l == 1) {\ndecodedImm = imm6;\nsize = 3;\n} else " if l else "") + \
									"if (imm6 & 0x20) {\ndecodedImm = imm6 - 32;\nsize = 2;\n" + \
									"} else if (imm6 & 0x10) {\ndecodedImm = imm6 - 16;\nsize = 1;\n" + \
									"} else {\ndecodedImm = imm6 - 8;\n}\n"
								)
						
						imms = [1, immssz] + imms
					elif imms[0][1] == 4:
						# Weird shift...
						assert(len(imms) == 1)
						
						immssz = 4
						append("uint8_t imm4 = (opcode >> " + str(imms[0][0]) + ") & 0xF;\n")
						
						if imms[0][2][:3] != "IMM":
							append(
								"uint8_t decodedImm = 0;\nuint8_t size = 0;\n" + \
								"if (imm4 & 0b0001) {\ndecodedImm = imm4 >> 1;\nsize = 8;\n} else " + \
								"if (imm4 & 0b0011) {\ndecodedImm = imm4 >> 2;\nsize = 16;\n} else " + \
								"if (imm4 & 0b0111) {\ndecodedImm = imm4 >> 3;\nsize = 32;\n}\n"
							)
						
						imms = [2, immssz] + imms
				
				add_custom_variables()
				
				append("\nsprintf(ret, \"")
				
				# Assemble the variables into the printf
				instText = ' '.join(spltln[curSplt:]).split('<')
				instText = [instTextPart.split('>') for instTextPart in instText]
				instText = [itp for itp2 in instText for itp in itp2]
				
				# Make the failures nicer
				spltln = spltln[:curSplt] + ["\x01"] + instText
				
				printf_args = ""
				for idx, text in enumerate(instText):
					text = text.replace("&l", "<").replace("&g", ">")
					
					if "+/-" in text:
						text = text.replace("+/-", "%s")
						printf_args = printf_args + ", (u ? \"\" : \"-\")"
					
					if idx % 2:
						if text == "c":
							pass
						elif text == "dt":
							append("%s")
							if imms == []:
								printf_args = printf_args + ", dts[" + ("0x8 + (f << 2) + " if variables["f"] != -1 else "")
								printf_args = printf_args + ("(u << 2) + " if variables["u"] != -1 else "") + "size]"
							elif imms[0] == 0:
								printf_args = printf_args + ", decodedImm"
							elif imms[0] == 1:
								printf_args = printf_args + ", dts[" + ("0x8 + (f << 2) + " if variables["f"] != -1 else "")
								printf_args = printf_args + ("(u << 2) + " if variables["u"] != -1 else "") + "size]"
							else:
								fail(ValueError, "Unsupported immediate type {} (on dt)".format(imms[0]))
						elif text == "size":
							if imms[0] == 1:
								append("%d")
								printf_args = printf_args + ", 8 << size"
							else:
								fail(ValueError, "Unsupported immediate type {} (on size)".format(imms[0]))
						elif text == "Qd":
							append("%s")
							printf_args = printf_args + ", vecname[" + ("(q << 5) + 0x20" if variables["q"] != -1 else "0x40") + " + d]"
						elif text == "qd":
							append("%s")
							printf_args = printf_args + ", vecname[0x40 + d]"
						elif text == "Dd":
							append("%s")
							printf_args = printf_args + ", vecname[0x20 + d]"
						elif text == "Rt":
							append("%s")
							printf_args = printf_args + ", regname[rt]"
						elif text == "Qn":
							append("%s")
							printf_args = printf_args + ", vecname[" + ("(q << 5) + 0x20" if variables["q"] != -1 else "0x40") + " + n]"
						elif text == "Dn":
							append("%s")
							printf_args = printf_args + ", vecname[0x20 + n]"
						elif text == "Rn":
							append("%s")
							printf_args = printf_args + ", regname[rn]"
						elif text == "Qm":
							append("%s")
							printf_args = printf_args + ", vecname[" + ("(q << 5) + 0x20" if variables["q"] != -1 else "0x40") + " + m]"
						elif text == "Dm":
							append("%s")
							printf_args = printf_args + ", vecname[0x20 + m]"
						elif text == "Dm[x]":
							append("%s[%d]")
							printf_args = printf_args + ", vecname[0x20 + m], decodedImm"
						elif text == "Rm":
							append("%s")
							printf_args = printf_args + ", regname[rm]"
						elif text == "imm":
							if imms[0] == 0:
								append("%s")
								printf_args = printf_args + ", decodedImm + 4"
							elif imms[0] == 1:
								append("%d")
								printf_args = printf_args + ", decodedImm"
							elif imms[0] == 2:
								append("%d")
								printf_args = printf_args + ", imm4"
							else:
								fail(ValueError, "Unsupported immediate type {}".format(imms[0]))
						elif text == "fbits":
							if (imms[0] == 1) and (imms[2][2][:3] == "IMM"):
								append("%d")
								printf_args = printf_args + ", 64 - imm6"
							else:
								fail(ValueError, "Unsupported immediate type {} (first imm = {})".format(imms[0], imms[2][2]))
						else:
							fail(KeyError, "Unknown variable " + text)
					else:
						if text == "":
							continue
						
						text = text.split('\\')
						while len(text) > 1:
							if text[1][0] == '%':
								modifier, text[1] = '%', text[1][1:]
								while (len(text[1]) > 1) \
								and (text[1][0] in ['0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+']):
									modifier, text[1] = modifier + text[1][0], text[1][1:]
								modifier, text[1] = modifier + text[1][0], text[1][1:]
								append(text[0] + modifier)
							else:
								append(text[0] + "%s")
							printf_args = printf_args + ", " + text[1]
							text = text[2:]
						if len(text) == 0:
							fail(AssertionError, "Substitution not finished")
						append(text[0])
					
					curSplt = curSplt + 1
				append("\"" + printf_args + ");\n} else ")
				
			elif spltln[0] == "VFPU":
				# vFPU instruction
				
				variables["cond"] = -1
				variables["registers"] = {
					"d": [-1, -1, -1], # Register: D, Vd, ##
					"t": [-1, -1, -1], # Register: #, ##, Rt
					"n": [-1, -1, -1], # Register: N, Vn, Rn
					"m": [-1, -1, -1]  # Register: M, Vm, ##
				}
				
				variables["op"] = -1 # Operation
				variables["u"] = -1 # Unsigned
				variables["f"] = -1 # Floating-point
				variables["sz"] = [-1, 0] # Operation size: pos, len
				
				variables["cmode"] = -1 # Used with immediates
				
				variables["q"] = -1 # Quadword
				
				variables["w"] = -1 # wback
				
				for i, val in enumerate(spltln[1:]):
					if '/' in val:
						ocurBit = curBit
						req = val.split('/')
						val, req = req[0], '/'.join(req[1:])
					
					if val == '0':
						curBit = curBit - 1
						mask[31 - curBit] = 1
					elif val == '1':
						curBit = curBit - 1
						mask[31 - curBit] = 1
						correctBits[31 - curBit] = 1
					elif (val == '(0)') or (val == '(1)'):
						# Ignore, even though the result should be undefined...
						curBit = curBit - 1
					elif val == 'cond':
						curBit = curBit - 4
						variables["cond"] = curBit
					elif val == 'D':
						curBit = curBit - 1
						variables["registers"]["d"][0] = curBit
					elif val == 'Vd':
						curBit = curBit - 4
						variables["registers"]["d"][1] = curBit
					elif val == 'Rt':
						curBit = curBit - 4
						variables["registers"]["t"][2] = curBit
					elif val == 'N':
						curBit = curBit - 1
						variables["registers"]["n"][0] = curBit
					elif val == 'Vn':
						curBit = curBit - 4
						variables["registers"]["n"][1] = curBit
					elif val == 'Rn':
						curBit = curBit - 4
						variables["registers"]["n"][2] = curBit
					elif val == 'M':
						curBit = curBit - 1
						variables["registers"]["m"][0] = curBit
					elif val == 'Vm':
						curBit = curBit - 4
						variables["registers"]["m"][1] = curBit
					elif val == 'Rm':
						curBit = curBit - 4
						variables["registers"]["m"][2] = curBit
					elif val == 'U':
						curBit = curBit - 1
						variables["u"] = curBit
					elif val == 'F':
						curBit = curBit - 1
						variables["f"] = curBit
					elif val == 'W':
						curBit = curBit - 1
						variables["w"] = curBit
					elif val == 'op':
						curBit = curBit - 1
						variables["op"] = curBit
					elif val == 'cmode':
						curBit = curBit - 4
						variables["cmode"] = curBit
					elif val == 'size':
						curBit = curBit - 2
						variables["sz"] = [curBit, 2]
					elif val == 'sz':
						curBit = curBit - 1
						variables["sz"] = [curBit, 1]
					elif val == 'Q':
						curBit = curBit - 1
						variables["q"] = curBit
					elif val == 'i':
						curBit = curBit - 1
						imms.append([curBit, 1, val])
					elif val == 'L':
						curBit = curBit - 1
						imms.append([curBit, 1, val])
						
					elif val.startswith('@<') and val.endswith('>'):
						parmlen = int(val[2:-1])
						curBit = curBit - parmlen
						parm.append([curBit, parmlen])
					elif val.lower().startswith('imm'):
						immsz = int(val[3:])
						curBit = curBit - immsz
						imms.append([curBit, immsz, val])
						
					else:
						fail(KeyError, "Unknown value '" + val + "'")
					
					curSplt = i + 2
					parse_var_requirements()
					
					if curBit == 0:
						break
					elif curBit < 0:
						fail(KeyError, "Current bit too low (" + str(curBit) + ")")
				
				if curSplt == -1:
					fail(KeyError, "Not enough arguments")
				
				generate_bin_test()
				
				# Add C variables
				if variables["cond"] != -1:
					if mask[31 - variables["cond"]] == 0:
						append("const char* cond = conds[(opcode >> " + str(variables["cond"]) + ") & 0xF];\n")
					else:
						variables["cond"] = -2
				if variables["op"] != -1:
					append("int op = (opcode >> " + str(variables["op"]) + ") & 1;\n")
				if (variables["u"] != -1) and (variables["f"] != -1):
					fail(ValueError, "Cannot have both unsigned and floating-point", False)
				if variables["u"] != -1:
					append("int u = (opcode >> " + str(variables["u"]) + ") & 1;\n")
				if variables["f"] != -1:
					append("int f = (opcode >> " + str(variables["f"]) + ") & 1;\n")
				if variables["sz"][0] != -1:
					if (mask[31 - variables["sz"][0]] == 0) and ((variables["sz"][1] == 1) or (mask[30 - variables["sz"][0]] == 0)):
						append("int size = (opcode >> " + str(variables["sz"][0]) + ") & " + sz2str(variables["sz"][1]) + ";\n")
					else:
						variables["sz"][0] = -2
				if variables["q"] != -1:
					if mask[31 - variables["q"]] == 0:
						append("int q = (opcode >> " + str(variables["q"]) + ") & 1;\n")
					else:
						variables["q"] = -2
				if variables["registers"]["d"][0] != -1:
					assert(variables["registers"]["d"][1] != -1)
					append("int d = ((opcode >> " + str(variables["registers"]["d"][0]) + ") & 1) << 4 " + \
						"| ((opcode >> " + str(variables["registers"]["d"][1]) + ") & 0xF);\n")
				if variables["registers"]["t"][2] != -1:
					append("int rt = (opcode >> " + str(variables["registers"]["t"][2]) + ") & 0xF;\n")
				if variables["registers"]["n"][0] != -1:
					assert(variables["registers"]["n"][1] != -1)
					append("int n = ((opcode >> " + str(variables["registers"]["n"][0]) + ") & 1) << 4 " + \
						"| ((opcode >> " + str(variables["registers"]["n"][1]) + ") & 0xF);\n")
				if variables["registers"]["n"][2] != -1:
					append("int rn = (opcode >> " + str(variables["registers"]["n"][2]) + ") & 0xF;\n")
				if variables["registers"]["m"][0] != -1:
					assert(variables["registers"]["m"][1] != -1)
					append("int m = ((opcode >> " + str(variables["registers"]["m"][0]) + ") & 1) << 4 " + \
						"| ((opcode >> " + str(variables["registers"]["m"][1]) + ") & 0xF);\n")
				if variables["registers"]["m"][2] != -1:
					append("int rm = (opcode >> " + str(variables["registers"]["m"][2]) + ") & 0xF;\n")
				if variables["w"] != -1:
					append("int w = (opcode >> " + str(variables["w"]) + ") & 1;\n")
				if variables["cmode"] != -1:
					append("int cmode = (opcode >> " + str(variables["cmode"]) + ") & 0xF;\n")
				if imms != []:
					immssz = sum(map(lambda v: v[1], imms))
					tmp = "(" * len(imms)
					for immpos, immsz, _ in imms:
						if tmp[-1] != '(': tmp = tmp + " << " + str(immsz) + ") | "
						tmp = tmp + "((opcode >> " + str(immpos) + ") & " + sz2str(immsz) + ")"
					tmp = tmp[1:]
					append("uint" + str(nextAvailable(immssz)) + "_t imm" + str(immssz) + " = " + tmp + ";\n")
					
					# Destroy imms since we don't need it anymore, but we do need immssz
					imms = [-1, immssz] + imms
				
				add_custom_variables()
				
				append("\nsprintf(ret, \"")
				
				# Assemble the variables into the printf
				instText = ' '.join(spltln[curSplt:]).split('<')
				instText = [instTextPart.split('>') for instTextPart in instText]
				instText = [itp for itp2 in instText for itp in itp2]
				
				# Make the failures nicer
				spltln = spltln[:curSplt] + ["\x01"] + instText
				
				# inside '{, '...'}'?
				hasOpened = False
				
				printf_args = ""
				for idx, text in enumerate(instText):
					text = text.replace("&l", "<").replace("&g", ">")
					
					if "+/-" in text:
						text = text.replace("+/-", "%s")
						printf_args = printf_args + ", (u ? \"\" : \"-\")"
					if "{!}" in text:
						text = text.replace("{!}", "%s")
						printf_args = printf_args + ", (w ? \"!\" : \"\")"
					if "{, " in text:
						if hasOpened:
							fail(AssertionError, "Cannot have an inner curly brace")
						text = text.replace("{, ", ", ", 1)
						hasOpened = True
					if "}" in text:
						if not hasOpened:
							fail(AssertionError, "Cannot close an inexistant curly brace")
						text = text.replace("}", "", 1)
						hasOpened = False
					
					if idx % 2:
						if text == "c":
							if variables["cond"] != -1:
								append("%s")
								printf_args = printf_args + ", cond"
						elif text == "dt":
							append("%s")
							if imms == []:
								printf_args = printf_args + ", dts[" + ("0x8 + (f << 2) + " if variables["f"] != -1 else "")
								printf_args = printf_args + ("(u << 2) + " if variables["u"] != -1 else "") + "size]"
							else:
								fail(ValueError, "Unsupported immediate type {} (on dt)".format(imms[0]))
						elif text == "size":
							if imms[0] == 1:
								append("%d")
								printf_args = printf_args + ", 8 << size"
							else:
								fail(ValueError, "Unsupported immediate type {} (on size)".format(imms[0]))
						elif text == "Qd":
							# VDUP (ARM core register) only
							assert(variables["q"] != -1)
							assert((variables["registers"]["t"][0:2] == [-1, -1]) and (variables["registers"]["t"][2] != -1))
							append("%s")
							printf_args = printf_args + ", vecname[(q << 5) + 0x20 + d]"
						elif text == "Dd":
							append("%s")
							printf_args = printf_args + ", vecname[" + ("(size << 5)" if variables["sz"][0] != -1 else "0x20")
							printf_args = printf_args + " + d]"
						elif text == "Dd[x]":
							append("%s[%d]")
							printf_args = printf_args + ", vecname[" + ("(size << 5)" if variables["sz"][0] != -1 else "0x20")
							printf_args = printf_args + " + d], decodedImm"
						elif text == "dd":
							append("%s")
							printf_args = printf_args + ", vecname[0x20 + d]"
						elif text == "Sd":
							append("%s")
							printf_args = printf_args + ", vecname[d]"
						elif text == "Rt":
							append("%s")
							printf_args = printf_args + ", regname[rt]"
						elif text == "Dn":
							append("%s")
							printf_args = printf_args + ", vecname[" + ("(size << 5)" if variables["sz"][0] != -1 else "0x20")
							printf_args = printf_args + " + n]"
						elif text == "Dn[x]":
							append("%s[%d]")
							printf_args = printf_args + ", vecname[" + ("(size << 5)" if variables["sz"][0] != -1 else "0x20")
							printf_args = printf_args + " + n], decodedImm"
						elif text == "Sn":
							append("%s")
							printf_args = printf_args + ", vecname[n]"
						elif text == "Rn":
							append("%s")
							printf_args = printf_args + ", regname[rn]"
						elif text == "Dm":
							append("%s")
							printf_args = printf_args + ", vecname[" + ("(size << 5)" if variables["sz"][0] != -1 else "0x20")
							printf_args = printf_args + " + m]"
						elif text == "Sm":
							append("%s")
							printf_args = printf_args + ", vecname[m]"
						elif text == "Sm1":
							append("%s")
							printf_args = printf_args + ", vecname[m + 1]"
						elif text == "Rm":
							append("%s")
							printf_args = printf_args + ", regname[rm]"
						elif text == "imm":
							if imms[0] == -1:
								append("%d")
								printf_args = printf_args + ", imm" + str(imms[1])
							else:
								fail(ValueError, "Unsupported immediate type {}".format(imms[0]))
						elif text == "label":
							if (imms[0] != -1) or ((imms[1] != 8) and (imms[1] != 24)):
								fail(AssertionError, "Unsupported case (label A)")
							if imms[1] == 8:
								if variables["u"] == -1:
									fail(AssertionError, "Unsupported case (label 1.B)")
								append("%s%d")
								printf_args = printf_args + ", (u ? \"\" : \"-\"), imm8 + 2"
							elif imms[1] == 24:
								append("%+d")
								printf_args = printf_args + ", (imm24 & 0x800000 ? imm24 | 0xFF000000 : imm24) + 2"
						elif text.lower() == "list":
							if (text == "list") and (variables["sz"] == -1):
								fail(AssertionError, "Unsupported case (list A)")
							if (imms[0] != -1) or (imms[1] != 8):
								fail(AssertionError, "Unsupported case (list B)")
							if variables["registers"]["d"][0] + variables["registers"]["d"][1] == -2:
								fail(AssertionError, "Unsupported case (list C)")
							
							append("%s")
							printf_args = printf_args + ", print_register_list_fpu(d, imm8, "
							printf_args = printf_args + ("0" if text == "List" else ("1" if text == "LIst" else "size"))
							printf_args = printf_args + ")"
						else:
							fail(KeyError, "Unknown variable " + text)
					else:
						if text == "":
							continue
						
						text = text.split('\\')
						while len(text) > 1:
							if text[1][0] == '%':
								modifier, text[1] = '%', text[1][1:]
								while (len(text[1]) > 1) \
								and (text[1][0] in ['0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+']):
									modifier, text[1] = modifier + text[1][0], text[1][1:]
								modifier, text[1] = modifier + text[1][0], text[1][1:]
								append(text[0] + modifier)
							else:
								append(text[0] + "%s")
							printf_args = printf_args + ", " + text[1]
							text = text[2:]
						if len(text) == 0:
							fail(AssertionError, "Substitution not finished")
						append(text[0])
					
					curSplt = curSplt + 1
				append("\"" + printf_args + ");\n} else ")
				
			elif spltln[0] == "INVALIDATE":
				# All instructions matching left are invalid
				
				# Constant -- set to True when debugging output
				# Appends a `#n` (with n being the number of INVALIDATEs before + 1) after `???`
				numberInvalids = __debug_forceAllDebugging
				
				for i, val in enumerate(spltln[1:]):
					if '/' in val:
						ocurBit = curBit
						req = val.split('/')
						val, req = req[0], '/'.join(req[1:])
					
					if val == '0':
						curBit = curBit - 1
						mask[31 - curBit] = 1
					elif val == '1':
						curBit = curBit - 1
						mask[31 - curBit] = 1
						correctBits[31 - curBit] = 1
					elif (val == '(0)') or (val == '(1)'):
						# Ignore, even though the result should be undefined...
						curBit = curBit - 1
						
					elif val.startswith('<') and val.endswith('>'):
						blanklen = int(val[1:-1])
						curBit = curBit - blanklen
						
					else:
						fail(KeyError, "Unknown value '" + val + "'")
					
					curSplt = i + 2
					parse_var_requirements()
					
					if curBit == 0:
						break
					elif curBit < 0:
						fail(KeyError, "Current bit too low (" + str(curBit) + ")")
				
				if curSplt == -1:
					fail(KeyError, "Not enough arguments")
				elif len(spltln) != curSplt:
					fail(KeyError, "Too many arguments")
				
				if 1 in mask:
					generate_bin_test()
					
					# No C variable since we're invalidating!
					
					# Now print the invalidation, number if debugging
					if numberInvalids:
						append("strcpy(ret, \"??? #" + str(invalidationCount) + "\");\n} else ")
						invalidationCount = invalidationCount + 1
					else:
						append("strcpy(ret, \"???\");\n} else ")
				else:
					# No more input, we invalidated everything!
					break
				
			elif spltln[0] == "TODO":
				print("\033[96mNote:\033[m todo '{}' (line {})".format(' '.join(spltln[1:]), lnno))
				
			else:
				fail(NotImplementedError, spltln[0] + " is not implemented")
		except Exception as e:
			# Add the failing line in case of error (BaseException is not caught!)
			raise Exception(str(lnno + 1) + ": " + ln) from e
	
	# Now the files rebuilding part
	# File header and guard
	header = """
	#include <stdint.h>
	#include <stddef.h>
	#include <string.h>
	#include <stdio.h>
	
	#include "arm_printer.h"
	
	// conditions
	#define cEQ (0b0000<<28)
	#define cNE (0b0001<<28)
	#define cCS (0b0010<<28)
	#define cCC (0b0011<<28)
	#define cMI (0b0100<<28)
	#define cPL (0b0101<<28)
	#define cVS (0b0110<<28)
	#define cVC (0b0111<<28)
	#define cHI (0b1000<<28)
	#define cLS (0b1001<<28)
	#define cGE (0b1010<<28)
	#define cLT (0b1011<<28)
	#define cGT (0b1100<<28)
	#define cLE (0b1101<<28)
	#define c__ (0b1110<<28)	// means all
	
	static const char* conds[16] = {
		"EQ", "NE", "CS", "CC",
		"MI", "PL", "VS", "VC",
		"HI", "LS", "GE", "LT",
		"GT", "LE",   "", "##"
	};
	
	static const char* regname[16] = {
		"r0", "r1", "r2", "r3", "r4",
		"r5", "r6", "r7", "r8", "r9",
		"r10", "r11", "r12", "SP", "LR", "PC"
	};
	
	// Single precision are V_:_
	// Double and quad precision are _:V_
	// So we always use _:V_ and invert the corresponding single-precision (cleaner? code)
	static const char* vecname[96] = {
		"S0",  "S2",  "S4",  "S6",  "S8", "S10", "S12", "S14",
		"S16", "S18", "S20", "S22", "S24", "S26", "S28", "S30",
		"S1",  "S3",  "S5",  "S7",  "S9", "S11", "S13", "S15",
		"S17", "S19", "S21", "S23", "S25", "S27", "S29", "S31",
		"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7",
		"D8", "D9", "D10", "D11", "D12", "D13", "D14", "D15",
		"D16", "D17", "D18", "D19", "D20", "D21", "D22", "D23",
		"D24", "D25", "D26", "D27", "D28", "D29", "D30", "D31",
		"Q0", "!!!", "Q1", "!!!", "Q2", "!!!", "Q3", "!!!",
		"Q4", "!!!", "Q5", "!!!", "Q6", "!!!", "Q7", "!!!",
		"Q8", "!!!", "Q9", "!!!", "Q10", "!!!", "Q11", "!!!",
		"Q12", "!!!", "Q13", "!!!", "Q14", "!!!", "Q15", "!!!"
	};
	
	static const char* dts[16] = {
		"S8", "S16", "S32", "S64",
		"U8", "U16", "U32", "U64",
		"I8", "I16", "I32", "I64",
		"F8", "F16", "F32", "F64"
	};
	
	static const char* shift_type[4] = {
		"lsl ", "lsr ", "asr ", "ror "
	};
	
	const char* print_shift(int shift, int comma) {
		static __thread char ret[20];
		ret[0] = '\\0';
		
		int sh_op = (shift >> 1) & 3;
		if(shift & 1) {
			int rs = shift >> 4;
			sprintf(ret, "%s%s%s", (comma ? ", " : ""), shift_type[sh_op], regname[rs]);
		} else {
			uint8_t amount = shift >> 3;
			if (!amount) {
				switch (sh_op) {
				case 0b00:
					break;
				case 0b01:
				case 0b10:
					sprintf(ret, "%s%s#32", (comma ? ", " : ""), shift_type[sh_op]);
					break;
				case 0b11:
					sprintf(ret, "%srrx", (comma ? ", " : ""));
					break;
				}
			} else {
				sprintf(ret, "%s%s#%u", (comma ? ", " : ""), shift_type[sh_op], amount);
			}
		}
		return ret;
	}
	
	#define print_modified_imm_ARM(imm12) (((imm12 & 0xFF) >> (2*(imm12 >> 8))) | ((imm12 & 0xFF) << (32 - 2*(imm12 >> 8))))
	
	const char* print_register_list(int list, int size) {
		int last = -2;
		int cnt = 0;
		static __thread char ret[68];
		
		ret[0] = '{';
		ret[1] = '\\0';
		for (int i = 0; i < size; ++i) {
			if (list & (1 << i)) {
				if (last >= 0) {
					++cnt;
				} else {
					if (last == -1)
						strcat(ret, ", ");
					strcat(ret, regname[i]);
					last = i;
				}
			} else {
				if (cnt) {
					strcat(ret, "-");
					strcat(ret, regname[i - 1]);
				}
				if (last != -2) last = -1;
				cnt = 0;
			}
		}
		if (cnt) {
			strcat(ret, "-");
			strcat(ret, regname[size - 1]);
		}
		strcat(ret, "}");
		
		return ret;
	}
	
	const char* print_register_list_fpu(uint8_t start, uint8_t size, uint8_t double_prec) {
		static __thread char ret[68];
		
		// Assume VFPSmallRegisterBank(), so no D16 and no S32 (and above)
		if (start + (double_prec + 1) * size > 32) {
			return "!!!";
		}
		if (size == 0) {
			return "!!!";
		}
		if (double_prec && (size % 2 == 1)) {
			strcpy(ret, "!! {");
		} else {
			ret[0] = '{';
			ret[1] = '\\0';
		}
		
		char regChr;
		if (double_prec) {
			regChr = 'D';
			size /= 2;
		} else {
			regChr = 'S';
			start = (start >> 4) + ((start & 0xF) << 1);
		}
		
		char tmp[7];
		for (int cur = start; cur < start + size - 1; ++cur) {
			sprintf(tmp, "%c%d, ", regChr, cur);
			strcat(ret, tmp);
		}
		sprintf(tmp, "%c%d}", regChr, start + size - 1);
		strcat(ret, tmp);
		
		return ret;
	}
	
	static const char* _print_modified_imm_I64_bytes[2] = {"00", "FF"};
	
	const char* print_modified_imm_NEON(int op_cmode, int imm) {
		static __thread char ret[34] = "???\\0-"; // [dt: 4][imm: 30]
		
		strcpy(&ret[4], "UNPREDICTABLE");
		
		switch (op_cmode) {
		case 0b00000: // VMOV.I32
		case 0b00001: // VORR.I32
		case 0b10000: // VMVN.I32
		case 0b10001: // VBIC.I32
			strcpy(ret, "I32");
			sprintf(&ret[4], "0x%02X", imm);
			break;
		case 0b00010: // VMOV.I32
		case 0b00011: // VORR.I32
		case 0b10010: // VMVN.I32
		case 0b10011: // VBIC.I32
			if (imm) {
				strcpy(ret, "I32");
				sprintf(&ret[4], "0x%02X00", imm);
			}
			break;
		case 0b00100: // VMOV.I32
		case 0b00101: // VORR.I32
		case 0b10100: // VMVN.I32
		case 0b10101: // VBIC.I32
			if (imm) {
				strcpy(ret, "I32");
				sprintf(&ret[4], "0x%02X0000", imm);
			}
			break;
		case 0b00110: // VMOV.I32
		case 0b00111: // VORR.I32
		case 0b10110: // VMVN.I32
		case 0b10111: // VBIC.I32
			if (imm) {
				strcpy(ret, "I32");
				sprintf(&ret[4], "0x%02X000000", imm);
			}
			break;
		case 0b01000: // VMOV.I16
		case 0b01001: // VORR.I16
		case 0b11000: // VMVN.I16
		case 0b11001: // VBIC.I16
			strcpy(ret, "I16");
			sprintf(&ret[4], "0x%02X", imm);
			break;
		case 0b01010: // VMOV.I16
		case 0b01011: // VORR.I16
		case 0b11010: // VMVN.I16
		case 0b11011: // VBIC.I16
			if (imm) {
				strcpy(ret, "I16");
				sprintf(&ret[4], "0x%02X00", imm);
			}
			break;
		case 0b01100: // VMOV.I32
		case 0b11100: // VMVN.I32
			if (imm) {
				strcpy(ret, "I32");
				sprintf(&ret[4], "0x%02XFF", imm);
			}
			break;
		case 0b01101: // VMOV.I32
		case 0b11101: // VMVN.I32
			if (imm) {
				strcpy(ret, "I32");
				sprintf(&ret[4], "0x%02XFFFF", imm);
			}
			break;
		case 0b01110: // VMOV.I8
			strcpy(ret, "I8");
			sprintf(&ret[4], "0x%02X", imm);
			break;
		case 0b01111: // VMOV.F32
			strcpy(ret, "F32");
			sprintf(&ret[4], "%e", (imm & 0x80 ? -1 : 1) * (1 << (((imm >> 4) & 0x7) ^ 0b100)) * (float) (16 + (imm & 0xF)) / 128.);
			break;
		case 0b11110: // VMOV.I64
			{
				strcpy(ret, "I64");
				sprintf(&ret[4], "#0x%s%s%s%s%s%s%s%s",
					_print_modified_imm_I64_bytes[(imm >> 7) & 1], _print_modified_imm_I64_bytes[(imm >> 6) & 1],
					_print_modified_imm_I64_bytes[(imm >> 5) & 1], _print_modified_imm_I64_bytes[(imm >> 4) & 1],
					_print_modified_imm_I64_bytes[(imm >> 3) & 1], _print_modified_imm_I64_bytes[(imm >> 2) & 1],
					_print_modified_imm_I64_bytes[(imm >> 1) & 1], _print_modified_imm_I64_bytes[(imm >> 0) & 1]);
			}
			break;
		case 0b11111: // V???.-
		default:
			strcpy(ret, "-");
			strcpy(&ret[4], "UNDEFINED");
			break;
		}
		
		return ret;
	}
	
	const char* arm_print(uint32_t opcode) {
		static __thread char ret[100];
		memset(ret, 0, sizeof(ret));
		
		"""
	footer = """
	{
			strcpy(ret, "???");
		}
		
		return ret;
	}
	"""
	banner = "/*******************************************************" + ('*'*len(ver)) + "***\n" \
	         " * File automatically generated by rebuild_printer.py (v" + ver + ") *\n" \
	         " *******************************************************" + ('*'*len(ver)) + "***/\n"
	trim = lambda string: '\n'.join(line[1:] for line in string.splitlines())[1:]
	header = banner + trim(header)
	footer = trim(footer)
	
	with open(os.path.join(root, "src", "dynarec", "arm_printer.c"), 'w') as file:
		file.write(header)
		file.write(output)
		file.write(footer)
	# Save the string for the next iteration, writing was successful
	with open(os.path.join(root, "src", "dynarec", "last_run.txt"), 'w') as file:
		file.write('\n'.join(insts))
	
	return 0

if __name__ == '__main__':
	limit = []
	for i, v in enumerate(sys.argv):
		if v == "--":
			limit.append(i)
	if main(sys.argv[1], "1.2.0.06") != 0:
		exit(2)
	exit(0)
