from allstar import Repo

class Allstar_Extractor(object):
  def __init__(self, arch):
    self.repo = Repo(arch)
    self.all_pkg_names = self.repo.packages()

  def get_src_and_obj(self, num_samples):
    fields = {'name', 'source', 'gimple', 'obj'}
    added = 0
    pkg_name = []
    source = []
    gimple = []
    obj = []
    for name in self.all_pkg_names:
      pkg = self.repo.package(name)
      for bin in pkg.binaries:
        for u in bin['units']:
          if fields.issubset(set(u.keys())):
            added += 1
            pkg_name.append(u['name'])
            source.append(pkg.get_file(u['source']))
            gimple.append(pkg.get_file(u['gimple']))
            obj.append(pkg.get_file(u['obj']))
          
            if added <= num_samples:
              return {"file_names": pkg_name,
                      "src_code": source,
                      "gimples" : gimple,
                      "obj_files": obj}


if __name__ == "__main__":
  extractor = Allstar_Extractor('amd64')
  result = extractor.get_src_and_obj(num_samples=1)
  print(result['file_names'])
  print(result['gimples'])

