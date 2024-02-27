project('ragel', 'c', 'cpp',
  version : '6.10',
  default_options: [
    'cpp_eh=default',
  ],
)

cpp = meson.get_compiler('cpp')
add_project_arguments(cpp.get_supported_arguments([
  '-fexceptions',
]), language: 'cpp')

conf = configuration_data()
conf.set_quoted('PACKAGE', meson.project_name())
conf.set_quoted('VERSION', meson.project_version())
configure_file(
  output : 'config.h',
  configuration : conf
)

ragel_sources = files(
  'ragel/buffer.h', 'ragel/cdcodegen.cpp', 'ragel/cdcodegen.h',
  'ragel/cdfflat.cpp', 'ragel/cdfflat.h', 'ragel/cdfgoto.cpp',
  'ragel/cdfgoto.h', 'ragel/cdflat.cpp', 'ragel/cdflat.h',
  'ragel/cdftable.cpp', 'ragel/cdftable.h', 'ragel/cdgoto.cpp',
  'ragel/cdgoto.h', 'ragel/cdipgoto.cpp', 'ragel/cdipgoto.h',
  'ragel/cdsplit.cpp', 'ragel/cdsplit.h', 'ragel/cdtable.cpp',
  'ragel/cdtable.h', 'ragel/common.cpp', 'ragel/common.h',
  'ragel/cscodegen.cpp', 'ragel/cscodegen.h', 'ragel/csfflat.cpp',
  'ragel/csfflat.h', 'ragel/csfgoto.cpp', 'ragel/csfgoto.h',
  'ragel/csflat.cpp', 'ragel/csflat.h', 'ragel/csftable.cpp',
  'ragel/csftable.h', 'ragel/csgoto.cpp', 'ragel/csgoto.h',
  'ragel/csipgoto.cpp', 'ragel/csipgoto.h', 'ragel/cssplit.cpp',
  'ragel/cssplit.h', 'ragel/cstable.cpp', 'ragel/cstable.h',
  'ragel/dotcodegen.cpp', 'ragel/dotcodegen.h', 'ragel/fsmap.cpp',
  'ragel/fsmattach.cpp', 'ragel/fsmbase.cpp', 'ragel/fsmgraph.cpp',
  'ragel/fsmgraph.h', 'ragel/fsmmin.cpp', 'ragel/fsmstate.cpp',
  'ragel/gendata.cpp', 'ragel/gendata.h', 'ragel/gocodegen.cpp',
  'ragel/gocodegen.h', 'ragel/gofflat.cpp', 'ragel/gofflat.h',
  'ragel/gofgoto.cpp', 'ragel/gofgoto.h', 'ragel/goflat.cpp', 'ragel/goflat.h',
  'ragel/goftable.cpp', 'ragel/goftable.h', 'ragel/gogoto.cpp',
  'ragel/gogoto.h', 'ragel/goipgoto.cpp', 'ragel/goipgoto.h',
  'ragel/gotable.cpp', 'ragel/gotable.h', 'ragel/gotablish.cpp',
  'ragel/gotablish.h', 'ragel/inputdata.cpp', 'ragel/inputdata.h',
  'ragel/javacodegen.cpp', 'ragel/javacodegen.h', 'ragel/main.cpp',
  'ragel/mlcodegen.cpp', 'ragel/mlcodegen.h', 'ragel/mlfflat.cpp',
  'ragel/mlfflat.h', 'ragel/mlfgoto.cpp', 'ragel/mlfgoto.h',
  'ragel/mlflat.cpp', 'ragel/mlflat.h', 'ragel/mlftable.cpp',
  'ragel/mlftable.h', 'ragel/mlgoto.cpp', 'ragel/mlgoto.h',
  'ragel/mltable.cpp', 'ragel/mltable.h', 'ragel/parsedata.cpp',
  'ragel/parsedata.h', 'ragel/parsetree.cpp', 'ragel/parsetree.h',
  'ragel/pcheck.h', 'ragel/ragel.h', 'ragel/rbxgoto.cpp', 'ragel/rbxgoto.h',
  'ragel/redfsm.cpp', 'ragel/redfsm.h', 'ragel/rlparse.cpp', 'ragel/rlparse.h',
  'ragel/rlscan.cpp', 'ragel/rlscan.h', 'ragel/rubycodegen.cpp',
  'ragel/rubycodegen.h', 'ragel/rubyfflat.cpp', 'ragel/rubyfflat.h',
  'ragel/rubyflat.cpp', 'ragel/rubyflat.h', 'ragel/rubyftable.cpp',
  'ragel/rubyftable.h', 'ragel/rubytable.cpp', 'ragel/rubytable.h',
  'ragel/version.h', 'ragel/xmlcodegen.cpp', 'ragel/xmlcodegen.h',
)

ragel = executable(
  meson.project_name(),
  ragel_sources,
  include_directories : ['aapl', 'ragel'],
  install : true,
)
