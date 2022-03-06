# Parker Seegmiller Phil Bohlman Winter 2022 Reverse Engineering Final Project

import sys
import subprocess
from os.path import exists

def parse(sig_str):
 """
 Parses sig_str from ctag function into name, line, and signature string
 """
 il = sig_str.split()
 d = {}
 d['name'] = il[0]
 d['line'] = il[2]
 d['signature'] = il[4]+' '+' '.join(il[5:])
 return d

def function_signatures(file_path):
 """
 Given valid file_path, extracts functions as strings from either .c or .cpp file
 """
 cpp = file_path[-2:]=='pp'
 if cpp:
  sigs = subprocess.getoutput('ctags -x --c++-kinds=fp --language-force=c++ '+file_path)
 else:
  sigs = subprocess.getoutput('ctags -x --c-kinds=fp '+file_path)

 sigs = sigs.split('\n')
 ds = []
 for s in sigs:
  ds.append(parse(s))
 return ds


def extract_fun(line_num, file_contents):
 """
 Given line_num and file_contents, extracts entire function definition as string
 """
 i = line_num
 fun_def = ''
 found_end = False
 paren_stack = 0
 while not found_end:
  j = 0
  while j < len(file_contents[i]):
   c = file_contents[i][j]
   if c == '{':
    paren_stack += 1
   if c == '}':
    if paren_stack != 1:
     paren_stack -= 1
    else:
     found_end = True
     fun_def += c
     break
   fun_def += c
   j += 1
  i += 1
 return fun_def

def c_chunkr(c_file_path):
 """
 Chunks c file in c_file_path into functions
 """
 # alright let's do some cool shit
 ds = function_signatures(c_file_path)
 f = open(c_file_path, 'r')
 contents = f.readlines()
 f.close()
 funs = []
 for d in ds:
  fun = extract_fun(int(d['line'])-1, contents)
  d['definition'] = fun
 return ds
