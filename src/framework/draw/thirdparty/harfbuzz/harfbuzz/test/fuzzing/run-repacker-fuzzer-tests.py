#!/usr/bin/env python3

import sys, os, subprocess, tempfile, shutil


def cmd (command):
	# https://stackoverflow.com/a/4408409 as we might have huge output sometimes
	with tempfile.TemporaryFile () as tempf:
		p = subprocess.Popen (command, stderr=tempf)

		try:
			p.wait ()
			tempf.seek (0)
			text = tempf.read ()

			#TODO: Detect debug mode with a better way
			is_debug_mode = b"SANITIZE" in text

			return ("" if is_debug_mode else text.decode ("utf-8").strip ()), p.returncode
		except subprocess.TimeoutExpired:
			return 'error: timeout, ' + ' '.join (command), 1


srcdir = os.getenv ("srcdir", ".")
EXEEXT = os.getenv ("EXEEXT", "")
top_builddir = os.getenv ("top_builddir", ".")
hb_repacker_fuzzer = os.path.join (top_builddir, "hb-repacker-fuzzer" + EXEEXT)

if not os.path.exists (hb_repacker_fuzzer):
        if len (sys.argv) < 2 or not os.path.exists (sys.argv[1]):
                sys.exit ("""Failed to find hb-repacker-fuzzer binary automatically,
please provide it as the first argument to the tool""")

        hb_repacker_fuzzer = sys.argv[1]

print ('hb_repacker_fuzzer:', hb_repacker_fuzzer)
fails = 0

valgrind = None
if os.getenv ('RUN_VALGRIND', ''):
	valgrind = shutil.which ('valgrind')
	if valgrind is None:
		sys.exit ("""Valgrind requested but not found.""")

def run_dir (parent_path):
	global fails
	for file in os.listdir (parent_path):
		path = os.path.join(parent_path, file)
		print ("running repacker fuzzer against %s" % path)
		if valgrind:
			text, returncode = cmd ([valgrind, '--leak-check=full', '--error-exitcode=1', hb_repacker_fuzzer, path])
		else:
			text, returncode = cmd ([hb_repacker_fuzzer, path])
			if 'error' in text:
				returncode = 1

		if (not valgrind or returncode) and text.strip ():
			print (text)

		if returncode != 0:
			print ("failed for %s" % path)
			fails = fails + 1


run_dir (os.path.join (srcdir, "graphs"))

if fails:
	sys.exit ("%d repacker fuzzer related tests failed." % fails)
