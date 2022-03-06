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