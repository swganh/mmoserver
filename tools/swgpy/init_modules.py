''' Kyle Craviotto ANH Studios 2012 '''
''' This script is used to generate __init__.py files under each subdirectory '''
''' Ensure this is run under the expected working directory '''

import os,io
curr_dir = os.path.basename(os.getcwd())
subdirs = list()

''' Walk through all the subdirectories '''
for dirname, dirnames, filenames in os.walk('.'):
    for subdirname in dirnames:
        subdirs.append(os.path.join(dirname, subdirname))

''' create the __init__.py for this current directory '''
init_py = open('__init__.py', 'w')
init_py.write('import sys, os \n')
init_py.write( "sys.path.insert(0, os.path.abspath('../')) \n")

''' Now Create a __init__.py file in each '''
for directory in subdirs:
	if not 'pycache' in directory:
		f = open(directory + "/__init__.py", "w")
		dir_fixed = directory.replace('.\\', '')
		dir_fixed = dir_fixed.replace('./', '')
		dir_fixed = dir_fixed.replace('../', '')
		''' write the import statement '''
		f.write('from ' + 'py_' + dir_fixed + ' import *')
		init_py.write("sys.path.append(os.path.abspath('" + curr_dir + '/' + dir_fixed + "'"  ")) \n")
		init_py.write('from ' + curr_dir + '.' + dir_fixed + ' import * \n')
		f.close()

init_py.close()