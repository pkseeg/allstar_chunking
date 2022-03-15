from allstar_c_getter import saveC
from allstar_bin_getter import saveBin
from bin_chunker import getFuncs
from cchunkr import c_chunkr
from demangler import demangle

from allstar import Repo

r = Repo('amd64')
p = r.package('2048-qt')

def write_packages(package):
 c_files = saveC(package)
 bin_files = saveBin(package)
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

 bnames = demangle(bin_fun_names)
 cnames = c_fun_names

 print('(Demangled Binary Names\n'+'\n'.join(bnames))
 print('C Source Names\n'+'\n'.join(cnames))

if __name__ == '__main__':
 write_packages(p)
