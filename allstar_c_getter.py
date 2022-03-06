from allstar import Repo
import requests
from urllib.parse import urljoin

r = Repo('amd64')
p = r.package('2048-qt')

def saveC(package):
    """
    Given an Allstar Package object, saves any associate .c or .cpp files in the current directory
    """
    files = []
    for b in package.binaries:
        for u in b['units']:
            if 'source' in u:
                name = u['source']
                url = urljoin(package.base, name)
                r = package.rsession.get(url)
                file = open('data/'+name, 'wb')
                files.append(name)
                file.write(r.content)
                file.close()
    return [f[2:] for f in files]

if __name__ == '__main__':
 print(saveC(p))
