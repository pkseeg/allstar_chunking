import angr

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
