#!/usr/bin/env python3

import sys, os, subprocess, shutil, glob
import xml.etree.ElementTree as ET

# Can we extract this from HTML element itself? I couldn't.
namespaces = {
	'ft': 'https://github.com/OpenType/fonttest',
	'xlink': 'http://www.w3.org/1999/xlink',
}
def ns (s):
	ns,s = s.split(':')
	return '{%s}%s' % (namespaces[ns], s)

def unistr (s):
	return ','.join('U+%04X' % ord(c) for c in s)

def glyphstr (glyphs):
	out = []
	for glyphname, x, y in glyphs:
		if x or y:
			out.append ('%s@%d,%d' % (glyphname, x, y))
		else:
			out.append (glyphname)
	return '[' + '|'.join (out) + ']'

def extract_tests (input):
	html = ET.fromstring (input)
	found = False

	result = []

	for elt in html.findall (".//*[@class='expected'][@ft:id]", namespaces):
		found = True
		name = elt.get (ns ('ft:id'))
		text = elt.get (ns ('ft:render'))
		font = elt.get (ns ('ft:font'))
		variations = elt.get (ns ('ft:var'), '').replace (':', '=').replace (';', ',')
		glyphs = []
		for use in elt.findall (".//use"):
			x = int (use.get ('x'))
			y = int (use.get ('y'))
			href = use.get (ns ('xlink:href'))
			assert href[0] == '#'
			glyphname = '.'.join (href[1:].split ('/')[1].split ('.')[1:])
			glyphs.append ((glyphname, x, y))
		opts = '--font-size=1000 --ned --remove-default-ignorables --font-funcs=ft'
		if variations:
			opts = opts + ' --variations=%s' % variations
		result.append ("../fonts/%s:%s:%s:%s" % (font, opts, unistr(text), glyphstr(glyphs)))

	for elt in html.findall (".//*[@class='expected-no-crash'][@ft:id]", namespaces):
		found = True
		name = elt.get (ns ('ft:id'))
		text = elt.get (ns ('ft:render'))
		font = elt.get (ns ('ft:font'))
		variations = elt.get (ns ('ft:var'), '').replace (':', '=').replace (';', ',')
		opts = ''
		if variations:
			opts = '--variations=%s' % variations
		result.append ("../fonts/%s:%s:%s:*" % (font, opts, unistr (text)))

	assert found
	return '\n'.join (result) + '\n'

os.chdir (os.getenv ('srcdir', os.path.dirname (__file__)))

git = shutil.which ('git')
assert git

if os.path.isdir ('./text-rendering-tests'):
	subprocess.run ([git, 'pull'], cwd='text-rendering-tests', check=True)
else:
	subprocess.run ([git, 'clone', 'https://github.com/unicode-org/text-rendering-tests'], check=True)

shutil.rmtree ('fonts', ignore_errors=True)
assert not os.path.exists ('fonts')
shutil.copytree ('text-rendering-tests/fonts', 'fonts')
subprocess.run([git, 'add', 'fonts'], check=True)

shutil.rmtree ('tests', ignore_errors=True)
assert not os.path.isdir('tests')
os.mkdir ('tests')

with open ('DISABLED', 'r') as f: disabled = f.read ()

tests = []
disabled_tests = []

for x in sorted (os.listdir ('text-rendering-tests/testcases')):
	if not x.endswith ('.html') or x == 'index.html': continue
	out = 'tests/%s.tests' % x.split('.html')[0]
	with open ('text-rendering-tests/testcases/' + x, 'r') as f: content = f.read ()
	with open (out, 'w') as f: f.write (extract_tests (content))
	if out in disabled:
		disabled_tests.append (out)
	else:
		tests.append (out)

subprocess.run([git, 'add', 'tests'], check=True)

with open ('meson.build', 'w') as f: f.write ('\n'.join (
	['text_rendering_tests = ['] +
	['  \'%s\',' % x.split('tests/')[1] for x in tests] +
	[']', '', 'disabled_text_rendering_tests = ['] +
	['  \'%s\',' % x.split('tests/')[1] for x in disabled_tests] +
	[']', '']
))

with open ('Makefile.sources', 'w') as f: f.write ('\n'.join (
	['TESTS = \\'] +
	['	%s \\' % x for x in tests] +
	['	$(NULL)', '', 'DISBALED_TESTS = \\'] +
	['	%s \\' % x for x in disabled_tests] +
	['	$(NULL)', '']
))

subprocess.run([git, 'add', 'Makefile.sources'], check=True)

print ('Updated the testsuit, now run `git commit -e -m "[test/text-rendering-tests] Update from upstream"`')
