#!/usr/bin/env python3

# Pre-generates the expected output subset files (via fonttools) for
# specified subset test suite(s).

import os
import sys
import shutil
import io
import re
import tempfile

from difflib import unified_diff
from fontTools.ttLib import TTFont

from subprocess import check_call
from subset_test_suite import SubsetTestSuite


def usage():
	print("Usage: generate-expected-outputs.py hb-subset <test suite file> ...")


def strip_check_sum (ttx_string):
	return re.sub ('checkSumAdjustment value=["]0x([0-9a-fA-F])+["]',
		       'checkSumAdjustment value="0x00000000"',
		       ttx_string, count=1)


def generate_expected_output(input_file, unicodes, profile_flags, instance_flags, output_directory, font_name, no_fonttools):
	input_path = input_file
	if not no_fonttools and instance_flags:
		instance_path = os.path.join(tempfile.mkdtemp (), font_name)
		args = ["fonttools", "varLib.instancer",
			"--no-overlap-flag",
			"--no-recalc-timestamp",
			"--output=%s" % instance_path,
			input_file]
		args.extend(instance_flags)
		check_call(args)
		input_path = instance_path

	fonttools_path = os.path.join(tempfile.mkdtemp (), font_name)
	args = ["fonttools", "subset", input_path]
	if instance_flags:
		args.extend(["--recalc-bounds"])
	args.extend(["--drop-tables+=DSIG",
		     "--drop-tables-=sbix",
		     "--no-harfbuzz-repacker", # disable harfbuzz repacker so we aren't comparing to ourself.
		     "--unicodes=%s" % unicodes,
		     "--output-file=%s" % fonttools_path])
	args.extend(profile_flags)
	if not no_fonttools:
		check_call(args)

		with io.StringIO () as fp:
			with TTFont (fonttools_path) as font:
				font.saveXML (fp)
				fonttools_ttx = strip_check_sum (fp.getvalue ())

	harfbuzz_path = os.path.join(tempfile.mkdtemp (), font_name)
	args = [
		hb_subset,
		"--font-file=" + input_file,
		"--output-file=" + harfbuzz_path,
		"--unicodes=%s" % unicodes,
		"--drop-tables+=DSIG",
		"--drop-tables-=sbix"]
	args.extend(profile_flags)
	if instance_flags:
		args.extend(["--instance=%s" % ','.join(instance_flags)])
	check_call(args)

	with io.StringIO () as fp:
		with TTFont (harfbuzz_path) as font:
			font.saveXML (fp)
		harfbuzz_ttx = strip_check_sum (fp.getvalue ())

	if not no_fonttools and harfbuzz_ttx != fonttools_ttx:
		for line in unified_diff (fonttools_ttx.splitlines (1), harfbuzz_ttx.splitlines (1), fonttools_path, harfbuzz_path):
			sys.stdout.write (line)
		sys.stdout.flush ()
		raise Exception ('ttx for fonttools and harfbuzz does not match.')

	output_path = os.path.join(output_directory, font_name)
	shutil.copy(harfbuzz_path, output_path)


args = sys.argv[1:]
if not args:
	usage()
hb_subset, args = args[0], args[1:]
if not args:
	usage()

for path in args:
	with open(path, mode="r", encoding="utf-8") as f:
		test_suite = SubsetTestSuite(path, f.read())
		output_directory = test_suite.get_output_directory()

		print("Generating output files for %s" % output_directory)
		for test in test_suite.tests():
			unicodes = test.unicodes()
			font_name = test.get_font_name()
			no_fonttools = ("no_fonttools" in test.options)
			print("Creating subset %s/%s" % (output_directory, font_name))
			generate_expected_output(test.font_path, unicodes, test.get_profile_flags(),
						 test.get_instance_flags(), output_directory, font_name, no_fonttools=no_fonttools)
