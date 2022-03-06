# Parker Seegmiller Phil Bohlman Winter 2022 Reverse Engineering Final Project
import subprocess

def demangle(names):
 args = ['c++filt']
 args.extend(names)
 pipe = subprocess.Popen(args, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
 stdout, _ = pipe.communicate()
 demangled = stdout.decode("utf-8").split("\n")
 return demangled[:-1] # last one is a '', always


if __name__ == '__main__':
 test_names = ['_ZNSt15basic_stringbufIcSt11char_traitsIcESaIcEE17_M_stringbuf_initESt13_Ios_Openmode', '_ZNSt15basic_stringbufIcSt11char_traitsIcESaIcEE6setbufEPci']
 names = demangle(test_names)
 a = '\n'.join(test_names)
 b = '\n'.join(names)
 print(f'test_names: {a}')
 print(f'names: {b}')

