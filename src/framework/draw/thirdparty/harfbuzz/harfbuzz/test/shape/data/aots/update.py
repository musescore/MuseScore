#!/usr/bin/env python3

import sys, os, subprocess, shutil

os.chdir (os.getenv ('srcdir', os.path.dirname (__file__)))

git = shutil.which ('git'); assert git
make = shutil.which ('make'); assert make
java = shutil.which ('java'); assert java
cxx = shutil.which ('c++'); assert cxx

pull = False
if not os.path.exists ('aots'):
	subprocess.run ([git, 'clone', 'https://github.com/adobe-type-tools/aots'], check=True)
	pull = True

if pull or 'pull' in sys.argv:
	subprocess.run ([git, 'pull'], cwd='aots', check=True)
	subprocess.run ([make, '-C', 'aots'], check=True)
	subprocess.run ([make, '-C', 'aots/harfbuzz'], check=True)

shutil.copy ('hb-aots-tester.cpp', 'aots/harfbuzz')
# TODO: remove *nix assumptions
subprocess.run ([cxx, '-std=c++11', '-Wno-narrowing', 'aots/harfbuzz/hb-aots-tester.cpp',
	'../../../../src/harfbuzz.cc', '-DHB_NO_MT', '-fno-exceptions', '-lm',
	'-I../../../../src', '-o', 'aots/harfbuzz/aots'], check=True)
shutil.rmtree ('tests')
os.mkdir ('tests')
subprocess.run (['./aots'], cwd='aots/harfbuzz', check=True)
