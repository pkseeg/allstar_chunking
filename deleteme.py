from allstar.allstar.allstar import Repo

arch = 'amd64' # this will change, but might as well choose something we're relatively familiar with

r = Repo(arch)

print('We have '+str(len(r.packages()))+' packages from the '+arch+' architecture')
print('Here are the first 3: '+str(r.packages()[:3]))
print('Taking the fourth one.')

p = r.package(r.packages()[3])
print('Here\'s our package: '+str(p.name))

print('Has binaries: '+str(p.has_binaries()))

for u in p.binaries[0]['units']:
 if 'obj' in u:
  print('Object: '+str(u['obj']))

b = p.get_binaries()
g = p.get_gimples()

print('Number of binaries: '+str([c['name'] for c in b]))
print('Number of gimples: '+str([c['name'] for c in g]))

print(b[0]['content'][:20])
print(g[0]['content'][:20])
