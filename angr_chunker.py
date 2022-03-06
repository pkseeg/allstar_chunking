import angr
from allstar import Repo

r = Repo('amd64')
p = r.package('2048-qt')

def saveBin(package):
    """
    Given an Allstar Package object, saves any associate binary file in the current directory
    """
    files = []
    bins = package.get_binaries()
    for b in bins:
        file = open(b['name'], "wb")
        files.append(b['name'])
        file.write(b['content'])
        file.close() 

    return files

bins = saveBin(p)


def getFuncs(filename: str):
    """
    Given a valid binary filepath, returns a dictionary of function names and function bytes.
    """
    p = angr.Project(filename, load_options={'auto_load_libs': False})
    cfg = p.analyses.CFGFast()
    funs = dict(cfg.kb.functions)
    all_bytes = {funs[addr].name: b"".join([block.bytes for block in funs[addr].blocks]) for addr in funs}
    filtered = {key: value for (key, value) in all_bytes.items() if value != b''}
    return filtered


print(getFuncs(bins[0]))
