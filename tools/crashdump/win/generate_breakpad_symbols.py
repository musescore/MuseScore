#!/usr/bin/env python
# Copyright (c) 2013 GitHub, Inc. All rights reserved.
# Copyright (c) 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

#
# Modified 2020 MuseScore Limited  
# Added option --dumpsyms-bin

"""Convert pdb to sym for given directories"""

import errno
import glob
import multiprocessing
import optparse
import os
import re
import subprocess
import sys
import threading

is_py2 = sys.version[0] == '2'
if is_py2:
    import Queue as queue
else:
    import queue as queue


CONCURRENT_TASKS=multiprocessing.cpu_count()

def GetCommandOutput(command):
  """Runs the command list, returning its output.

  Prints the given command (which should be a list of one or more strings),
  then runs it and returns its output (stdout) as a string.

  From chromium_utils.
  """
  devnull = open(os.devnull, 'w')
  proc = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=devnull)
  output = proc.communicate()[0]
  return output.decode()


def mkdir_p(path):
  """Simulates mkdir -p."""
  try:
    os.makedirs(path)
  except OSError as e:
    if e.errno == errno.EEXIST and os.path.isdir(path):
      pass
    else: raise


def GenerateSymbols(options, binaries):
  """Dumps the symbols of binary and places them in the given directory."""

  q = queue.Queue()
  print_lock = threading.Lock()

  def _Worker():
    dump_syms = options.dumpsyms_bin
    while True:
      binary = q.get()

      if options.verbose:
        with print_lock:
          print("Generating symbols for %s" % binary)

      syms = GetCommandOutput([dump_syms, binary])
      module_line = re.match("MODULE [^ ]+ [^ ]+ ([0-9A-Fa-f]+) (.*)\r\n", syms)
      if module_line == None:
        with print_lock:
          print("Failed to get symbols for %s" % binary)
        q.task_done()
        continue

      output_path = os.path.join(options.symbols_dir, module_line.group(2),
                                 module_line.group(1))
      mkdir_p(output_path)
      symbol_file = "%s.sym" % module_line.group(2)[:-4]  # strip .pdb
      f = open(os.path.join(output_path, symbol_file), 'w')
      f.write(syms)
      f.close()

      q.task_done()

  for binary in binaries:
    q.put(binary)

  for _ in range(options.jobs):
    t = threading.Thread(target=_Worker)
    t.daemon = True
    t.start()

  q.join()


def main():
  parser = optparse.OptionParser()
  parser.add_option('', '--dumpsyms-bin', default='',
                    help='The dump_syms binary.')
  parser.add_option('', '--build-dir', default='',
                    help='The build output directory.')                  
  parser.add_option('', '--symbols-dir', default='',
                    help='The directory where to write the symbols file.')
  parser.add_option('', '--binary', default='',
                    help='The path of the binary to generate symbols for.')                  
  parser.add_option('', '--clear', default=False, action='store_true',
                    help='Clear the symbols directory before writing new '
                         'symbols.')
  parser.add_option('-j', '--jobs', default=CONCURRENT_TASKS, action='store',
                    type='int', help='Number of parallel tasks to run.')
  parser.add_option('-v', '--verbose', action='store_true',
                    help='Print verbose status output.')

  (options, directories) = parser.parse_args()

  if not options.dumpsyms_bin:
    print("Required option --dumpsyms-bin missing.")
    return 1

  if not options.symbols_dir:
    print("Required option --symbols-dir missing.")
    return 1

  if not options.build_dir:
    print("Required option --build-dir missing.")
    return 1

  if not options.binary:
    print("Required option --binary missing.")
    return 1

  if not os.access(options.binary, os.X_OK):
    print("Cannot find %s." % options.binary)
    return 1  

  if options.clear:
    try:
      shutil.rmtree(options.symbols_dir)
    except:
      pass

  # pdbs = []
  # for directory in directories:
  #   pdbs += glob.glob(os.path.join(directory, '*.exe.pdb'))
  #   pdbs += glob.glob(os.path.join(directory, '*.dll.pdb'))

  # GenerateSymbols(options, pdbs) 

  print("Required binary: %s" % options.binary)

  binary = os.path.abspath(options.binary)
  binaries = []
  binaries += glob.glob(binary)
  GenerateSymbols(options, binaries)

  return 0


if '__main__' == __name__:
  sys.exit(main())