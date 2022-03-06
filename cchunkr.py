# Parker Seegmiller Phil Bohlman Winter 2022 Reverse Engineering Final Project

import sys
import subprocess
from os.path import exists

def parse(sig_str):
 il = sig_str.split()
 d = {}
 d['name'] = il[0]
 d['line'] = il[2]
 d['signature'] = il[4]+' '+' '.join(il[5:])
 return d

def function_signatures(file_path):
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

def main():
 if len(sys.argv) < 2: # usage handling
  print('Usage: cchunkr.py c_file_to_chunk')
  return None
 c_file_path = sys.argv[1] # filepath
 if not exists(c_file_path): # check if filepath exists
  print('File does not exist!')
  return None

 # alright let's do some cool shit
 ds = function_signatures(c_file_path)
 f = open(c_file_path, 'r')
 contents = f.readlines()
 f.close()
 funs = []
 for d in ds:
  fun = extract_fun(int(d['line'])-1, contents)
  print(fun)
#  d['definition'] = fun
#  print(d)

if __name__ == '__main__':
 main()
