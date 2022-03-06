# Parker Seegmiller Phil Bohlman Winter 2022 Reverse Engineering Final Project
import subprocess

def demangle(names):
 """
 Demangles list of c++ function names
 """
 args = ['c++filt']
 args.extend(names)
 pipe = subprocess.Popen(args, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
 stdout, _ = pipe.communicate()
 demangled = stdout.decode("utf-8").split("\n")
 return demangled[:-1] # last one is a '', always
