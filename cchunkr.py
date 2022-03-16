# Parker Seegmiller Phil Bohlman Winter 2022 Reverse Engineering Final Project

import sys
import subprocess
from os.path import exists

def clean_param(p):
 """
 Reformats the parameter string into the same format as angr+demangle will get us
 """
 s = ''
 syms = p.strip().split(' ')
 if syms[0] == 'const':
  if len(syms) < 3: return p # early stopping
  s += 'const'
  p_type = syms[1]
  p_name = syms[2]
 else:
  if len(syms) < 2: return p # early stopping
  p_type = syms[0]
  p_name = syms[1]
 if '&' in p:
  s += '&'
 if '**' in p:
  s += '**'
 elif '*' in p:
  s += '*'
 if s != '': s = ' '+s
 return p_type+s

def clean_signature(sig_str):
 """
 Reformats the function signature into a signature that will match the demangled binary signature
 """
 # remove type of signature
 # this naive way works for simple examples
 if not '::' in sig_str.split(' ')[0]: # if it has a type
  if len(sig_str.split(' ')) == 0: return sig_str # early stopping
  sig_str = ' '.join(sig_str.split(' ')[1:])

 # remove names of parameters
 if '(' not in sig_str or ')' not in sig_str: return sig_str # early stopping
 ps = sig_str.split('(')[1].split(')')[0]
 terms = ps.split(',')
 if terms[0] == '':
  return sig_str.split('(')[0]+'('+''+')'
 terms = [clean_param(t) for t in terms]
 sig_str = sig_str.split('(')[0]+'('+', '.join(terms)+')'
 if sig_str[0] == '*': sig_str = sig_str[1:]
 return sig_str


def parse(sig_str):
 """
 Parses sig_str from ctag function into name, line, and signature string
 """
 il = sig_str.split()
 d = {}
 d['name'] = il[0]
 d['line'] = il[2]
 d['signature'] = il[4]+' '+' '.join(il[5:])
 d['signature'] = clean_signature(d['signature'])
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
  try:
   parsed = parse(s)
   ds.append(parsed)
  except:
   # do nothing
   pass
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
  try:
   fun = extract_fun(int(d['line'])-1, contents)
   d['definition'] = fun
   funs.append(d)
  except:
   pass
 return funs
