#!/usr/bin/env python3

import os

# Parses a single repacking test file. The first line of the file is
# the name of the font to use and the remaining lines define the set of
# codepoints in the subset.
class RepackTest:

	def __init__(self, test_path, definition):
		self.test_path = test_path
		self.font_name = None
		self.codepoints = set ()
		self._parse(definition)

	def font_path(self):
		return os.path.join (self._base_path (), "fonts", self.font_name)

	def codepoints_string (self):
		return ",".join (self.codepoints)

	def _base_path(self):
	        return os.path.join(
		    os.path.dirname(self.test_path),
		    "../")


	def _parse(self, definition):
		lines = definition.splitlines ()
		self.font_name = lines.pop (0)
		for line in lines:
			line = line.strip()
			if not line:
				continue

			self.codepoints.add (line)
