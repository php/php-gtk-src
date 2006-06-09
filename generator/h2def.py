#!/usr/bin/env python
# -*- Mode: Python; py-indent-offset: 4 -*-
# Search through a header file looking for function prototypes.
# For each prototype, generate a scheme style definition.
# GPL'ed
# Toby D. Reeves <toby@max.rl.plh.af.mil>

# Modified by James Henstridge <james@daa.com.au> to output stuff in
# Havoc's new defs format.  Info on this format can be seen at:
#   http://www.gnome.org/mailing-lists/archives/gtk-devel-list/2000-January/0085.shtml


import string, sys, re, types

# ------------------ Find object definitions -----------------

obj_name_pat = "[A-Z][a-z]+[A-Z][A-Za-z0-9]*"

def find_obj_defs(buf, objdefs=[]):
    """
    Try to find object definitions in header files.
    """

    # filter out comments from buffer.
    pat = re.compile(r"""/[*](.|\n)*?[*]/""", re.MULTILINE)
    buf=pat.sub('',buf)

    # first find all structures that look like they may represent a GtkObject
    pat = re.compile("struct _(" + obj_name_pat + ")\s*{\s*" +
                     "(" + obj_name_pat + ")\s+", re.MULTILINE)
    maybeobjdefs = []  # contains all possible objects from file
    pos = 0
    while pos < len(buf):
        m = pat.search(buf, pos)
        if not m: break
        maybeobjdefs.append((m.group(1), m.group(2)))
        pos = m.end()

    # now find all structures that look like they might represent a class:
    pat = re.compile("struct _(" + obj_name_pat + ")Class\s*{\s*" +
                     "(" + obj_name_pat + ")Class\s+", re.MULTILINE)
    pos = 0
    while pos < len(buf):
        m = pat.search(buf, pos)
        if not m: break
        t = (m.group(1), m.group(2))
        # if we find an object structure together with a corresponding
        # class structure, then we have probably found a GtkObject subclass.
        if t in maybeobjdefs:
            objdefs.append(t)
        pos = m.end()

def sort_obj_defs(objdefs):
    objdefs.sort()  # not strictly needed, but looks nice
    pos = 0
    while pos < len(objdefs):
        klass,parent = objdefs[pos]
        for i in range(pos+1, len(objdefs)):
            # parent below subclass ... reorder
            if objdefs[i][0] == parent:
                objdefs.insert(i+1, objdefs[pos])
                del objdefs[pos]
                break
        else:
            pos = pos + 1
    return objdefs

def write_obj_defs(objdefs, output):
    if type(output)==types.StringType:
        fp=open(output,'w')
    elif type(output)==types.FileType:
        fp=output
    else:
        fp=sys.stdout

    fp.write(';; -*- scheme -*-\n')
    fp.write('; object definitions ...\n')

    pat = re.compile('([A-Z][a-z]+)([A-Za-z0-9]+)')
    for klass, parent in objdefs:
        m = pat.match(klass)
        cmodule = None
        cname = klass
        if m:
            cmodule = m.group(1)
            cname = m.group(2)
        if parent:
            m = pat.match(parent)
            pmodule = None
            pname = parent
            if m:
                pmodule = m.group(1)
                pname = m.group(2)

        fp.write('(object ' + cname + '\n')
        if cmodule:
            fp.write('  (in-module ' + cmodule + ')\n')
        if parent:
            fp.write('  (parent ' + pname)
            if pmodule: fp.write(' (' + pmodule + ')')
            fp.write(')\n')
        fp.write('  (c-name ' + klass + ')\n')
        # should do something about accessible fields
        fp.write(')\n\n')

# ------------------ Find enum definitions -----------------

def find_enum_defs(buf, enums=[]):
    # strip comments
    # bulk comments
    pat = re.compile(r"""/[*](.|\n)*?[*]/""", re.MULTILINE)
    buf=pat.sub('',buf)

    buf = re.sub('\n', ' ', buf)
    
    enum_pat = re.compile(r'enum\s*{([^}]*)}\s*([A-Z][A-Za-z]*)(\s|;)')
    splitter = re.compile(r'\s*,\s', re.MULTILINE)
    pos = 0
    while pos < len(buf):
        m = enum_pat.search(buf, pos)
        if not m: break

        name = m.group(2)
        vals = m.group(1)
        isflags = string.find(vals, '<<') >= 0
        entries = []
        for val in splitter.split(vals):
            entries.append(string.split(val)[0])
        enums.append((name, isflags, entries))
        
        pos = m.end()

def write_enum_defs(enums, output=None):
    if type(output)==types.StringType:
        fp=open(output,'w')
    elif type(output)==types.FileType:
        fp=output
    else:
        fp=sys.stdout

    fp.write(';; Enumerations and flags ...\n\n')
    pat = re.compile('([A-Z][a-z]+)([A-Za-z0-9]+)')
    trans = string.maketrans(string.uppercase + '_', string.lowercase + '-')
    for cname, isflags, entries in enums:
        name = cname
        module = None
        m = pat.match(cname)
        if m:
            module = m.group(1)
            name = m.group(2)
        if isflags:
            fp.write('(flags ' + name + '\n')
        else:
            fp.write('(enum ' + name + '\n')
        if module:
            fp.write('  (in-module ' + module + ')\n')
        fp.write('  (c-name ' + cname + ')\n')
        prefix = entries[0]
        for ent in entries:
            # shorten prefix til we get a match ...
            while ent[:len(prefix)] != prefix:
                prefix = prefix[:-1]
        for ent in entries:
            fp.write('  (value (name ' +
                         string.translate(ent[len(prefix):], trans) +
                         ') (c-name ' + ent + '))\n')
        fp.write(')\n\n')

# ------------------ Find function definitions -----------------

#comment_pat = re.compile(r"""(/[*](.|\n)*?[*]/)|(^;.*$)""", re.MULTILINE)

def clean_func(buf):
    """
    Ideally would make buf have a single prototype on each line.
    Actually just cuts out a good deal of junk, but leaves lines
    where a regex can figure prototypes out.
    """
    # bulk comments
    pat = re.compile(r"""/[*](.|\n)*?[*]/""", re.MULTILINE)
    buf=pat.sub('',buf)

    # Preprocess directives
    pat = re.compile(r"""^[#].*?$""", re.MULTILINE) 
    buf=pat.sub('',buf)

    #typedefs, stucts, and enums
    pat = re.compile(r"""^(typedef|struct|enum)(\s|.|\n)*?;\s*""", re.MULTILINE) 
    buf=pat.sub('',buf)

    #multiple whitespace
    pat = re.compile(r"""\s+""", re.MULTILINE) 
    buf=pat.sub(' ',buf)

    #clean up line ends
    pat = re.compile(r""";\s*""", re.MULTILINE) 
    buf=pat.sub('\n',buf)

    #associate *, &, and [] with type instead of variable
    #pat=re.compile(r'\s+([*|&]+)\s*(\w+)')
    pat=re.compile(r' \s+ ([*|&]+) \s* (\w+)',re.VERBOSE)
    buf=pat.sub(r'\1 \2', buf)
    pat=re.compile(r'\s+ (\w+) \[ \s* \]',re.VERBOSE)
    buf=pat.sub(r'[] \1', buf)

    return buf

proto_pat=re.compile(r"""
(?P<ret>(\w|\&|\*)+\s*)  # return type
\s+                   # skip whitespace
(?P<func>\w+)\s*[(]   # match the function name until the opening (
(?P<args>.*?)[)]     # group the function arguments
""", re.IGNORECASE|re.VERBOSE)
#"""
arg_split_pat = re.compile("\s*,\s*")

def define_func(buf,fp):
    buf=clean_func(buf)
    buf=string.split(buf,'\n')
    for p in buf:
        if len(p)==0: continue
        m=proto_pat.match(p)
        if m==None:
            if verbose:
                sys.stderr.write('No match:|%s|\n'%p)
            continue
        func = m.group('func')
        ret = m.group('ret')
        ret = string.replace(ret, 'const ', 'const-')
        args=m.group('args')
        args=arg_split_pat.split(args)

        for i in range(len(args)):
            args[i] = string.replace(args[i], 'const ', 'const-')

        write_func(fp, func, ret, args)

get_type_pat = re.compile(r'(const-)?([A-Za-z0-9]+)\*?\s+')
get_mod_pat = re.compile('([A-Z][a-z]+)([A-Za-z0-9]+)')

def write_func(fp, name, ret, args):
    if len(args) >= 1:
        # methods must have at least one argument
        munged_name = string.replace(name, '_', '')
        m = get_type_pat.match(args[0])
        if m:
            obj = m.group(2)
            if munged_name[:len(obj)] == string.lower(obj):
                regex = string.join(map(lambda x: x+'_?',string.lower(obj)),'')
                mname = re.sub(regex, '', name)
                fp.write('(method ' + mname + '\n')
                m = get_mod_pat.match(obj)
                if m:
                    fp.write('  (of-object ' + m.group(2) +
                             ' (' + m.group(1) + '))\n')
                fp.write('  (c-name ' + name + ')\n')
                if ret != 'void':
                    fp.write('  (return-type ' + ret + ')\n')
                else:
                    fp.write('  (return-type none)\n')
                for arg in args[1:]:
                    if arg == '...':
                        fp.write('  (varargs t)\n')
                    elif arg in ('void', 'void '):
                        pass
                    else:
                        fp.write('  (parameter (type-and-name ' + arg + '))\n')
                fp.write(')\n\n')
                return
    # it is either a constructor or normal function
    # FIXME: put in constructor check
    fp.write('(function ' + name + '\n')
    # do in-module thingee
    fp.write('  (c-name ' + name + ')\n')
    if ret != 'void':
        fp.write('  (return-type ' + ret + ')\n')
    else:
        fp.write('  (return-type none)\n')
    for arg in args:
        if arg == '...':
            fp.write('  (varargs t)\n')
        elif arg == 'void' or arg == '':
            pass
        else:
            fp.write('  (parameter (type-and-name ' + arg + '))\n')
    fp.write(')\n\n')

def write_def(input,output=None):
    fp = open(input)
    buf = fp.read()
    fp.close()

    if type(output) == types.StringType:
        fp = open(output,'w')
    elif type(output) == types.FileType:
        fp = output
    else:
        fp = sys.stdout

    fp.write('\n;; From %s\n\n' % input)
    buf = define_func(buf, fp)
    fp.write('\n')

# ------------------ Glue code -----------------

def make_gdk_defs():
    """ This is intended to be run only by the package maintainer!!! """
    p='/usr/local/src/gtk+-1.2.6/gdk/'
    gdk= [
        'gdk.h',
        'gdkrgb.h',
        #'gdktypes.h'
        ]
    fp=open('_gdk_func.defs','w')
    for h in gdk:
        write_def(p+h,fp)
    fp.close()

verbose=0
if __name__ == '__main__':
    import getopt
    
    opts, args = getopt.getopt(sys.argv[1:], 'v')
    for o, v in opts:
        if o=='-v': verbose=1
            
    if not args[0:1]:
        print 'Must specify at least one input file name'
        sys.exit(-1)

    # read all the object definitions in
    objdefs = []
    enums = []
    for filename in args:
        buf = open(filename).read()
        find_obj_defs(buf, objdefs)
        if len(filename) > 11 and filename[-11:] == 'gtkobject.h':
            objdefs.append(('GtkObject', None))
        find_enum_defs(buf, enums)
    objdefs = sort_obj_defs(objdefs)
    write_obj_defs(objdefs,None)
    write_enum_defs(enums,None)

    for filename in args:
        write_def(filename,None)
