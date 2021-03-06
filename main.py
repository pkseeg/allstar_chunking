from allstar_c_getter import saveC
from allstar_bin_getter import saveBin
from bin_chunker import getFuncs
from cchunkr import c_chunkr
from demangler import demangle
import random
import editdistance
import os
import sys

DIST_THRESHOLD = 5
NUM_PACKAGES = 20

from allstar import Repo

def write_packages(package, repo_name='', package_name=''):
 c_files = saveC(package)
 bin_files = saveBin(package)
 if len(bin_files) == 0: return set() # early stopping
 if len(c_files) == 0: return set() # early stopping
 bin_funs = []
 bin_fun_names = []
 c_funs = []
 c_fun_names = []
 for bf in bin_files:
  d = getFuncs('data/'+bf)
  bin_fun_names.extend([key for (key, item) in d.items()])
  bin_funs.extend([item for (key, item) in d.items()])
 for cf in c_files:
  d = c_chunkr('data/'+cf)
  c_fun_names.extend([d_i['signature'] for d_i in d])
  c_funs.extend([d_i['definition'] for d_i in d])

 bnames = set(demangle(bin_fun_names))
 cnames = set(c_fun_names)
 intersect = []
 for b in bnames:
  for c in cnames:
   dist = editdistance.eval(b, c)
   if dist < DIST_THRESHOLD:
    intersect.append((b,c))
 # for straight matching, uncomment
 # intersect = bnames.intersection(cnames) # this is straight matching, fuzzy string matching could be implemented as next step
 # Logging
 print('Repo : '+repo_name)
 print('Package : '+package_name)
 print(f'# Extracted Binary Function Chunks : {len(bnames)}')
 print(f'# Extracted C/C++ Source Function Chunks : {len(cnames)}')
 print(f'# Overlapping Signatures : {len(intersect)}\n')
 return intersect


if __name__ == '__main__':
 # all repo names and package names
 repo_names = ['amd64', 'armel', 'i386', 'mipsel', 'ppc64el', 's390x']
 with open('allstar_packages.txt', 'r') as f:
  package_names = [l.strip() for l in f.readlines()]

 # randomly select packages
 collected = []
 while len(collected) < NUM_PACKAGES:
  r_name = random.choice(repo_names)
  p_name = random.choice(package_names)
  try:
   r = Repo(r_name)
   p = r.package(p_name)
   funs = write_packages(p, repo_name=r_name, package_name=p_name)
   if len(funs) != 0:
    collected.append((r_name, p_name))
  except KeyboardInterrupt:
        print('Interrupted')
        try:
            sys.exit(0)
        except SystemExit:
            os._exit(0)
  except:
   print('Failed on '+r_name+' '+p_name)

 print('Finished!')
 print(collected)
