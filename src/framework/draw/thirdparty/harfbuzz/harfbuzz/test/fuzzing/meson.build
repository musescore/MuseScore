tests = [
  'hb-shape-fuzzer.cc',
  'hb-subset-fuzzer.cc',
  'hb-set-fuzzer.cc',
  'hb-draw-fuzzer.cc',
]

if get_option('experimental_api')
  tests += 'hb-repacker-fuzzer.cc'
endif

foreach file_name : tests
  test_name = file_name.split('.')[0]

  sources = [file_name]
  fuzzer_ldflags = []
  extra_cpp_args = []

  if get_option('fuzzer_ldflags') == ''
    sources += 'main.cc'
  else
    fuzzer_ldflags += get_option('fuzzer_ldflags').split()
    extra_cpp_args += '-DHB_IS_IN_FUZZER'
  endif

  if get_option('experimental_api')
    extra_cpp_args += '-DHB_EXPERIMENTAL_API'
  endif

  exe = executable(test_name, sources,
    cpp_args: cpp_args + extra_cpp_args,
    include_directories: [incconfig, incsrc],
    link_args: fuzzer_ldflags,
    link_with: [libharfbuzz, libharfbuzz_subset],
    install: false,
  )
  set_variable('@0@_exe'.format(test_name.underscorify()), exe)
endforeach

env = environment()
env.set('srcdir', meson.current_source_dir())

test('shape_fuzzer', find_program('run-shape-fuzzer-tests.py'),
  args: [
    hb_shape_fuzzer_exe,
  ],
  timeout: 300,
  depends: [hb_shape_fuzzer_exe, libharfbuzz, libharfbuzz_subset],
  workdir: meson.current_build_dir() / '..' / '..',
  env: env,
  suite: ['fuzzing', 'slow'],
)

test('subset_fuzzer', find_program('run-subset-fuzzer-tests.py'),
  args: [
    hb_subset_fuzzer_exe,
  ],
  # as the tests are ran concurrently let's raise acceptable time here
  # ideally better to break and let meson handles them in parallel
  timeout: 300,
  workdir: meson.current_build_dir() / '..' / '..',
  env: env,
  suite: ['fuzzing', 'slow'],
)

if get_option('experimental_api')
  test('repacker_fuzzer', find_program('run-repacker-fuzzer-tests.py'),
    args: [
      hb_repacker_fuzzer_exe,
    ],
    # as the tests are ran concurrently let's raise acceptable time here
    # ideally better to break and let meson handles them in parallel
    timeout: 300,
    workdir: meson.current_build_dir() / '..' / '..',
    env: env,
    suite: ['fuzzing', 'slow'],
  )
endif

test('draw_fuzzer', find_program('run-draw-fuzzer-tests.py'),
  args: [
    hb_draw_fuzzer_exe,
  ],
  workdir: meson.current_build_dir() / '..' / '..',
  env: env,
  suite: ['fuzzing'],
)
