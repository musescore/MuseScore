test('shape_threads', executable('hb-shape-threads', 'hb-shape-threads.cc',
  dependencies: [
    freetype_dep, thread_dep
  ],
  cpp_args: [],
  include_directories: [incconfig, incsrc],
  link_with: [libharfbuzz],
  install: false,
  ),
  workdir: meson.current_source_dir() / '..' / '..',
  timeout: 300,
  suite: ['threads', 'slow'],
)


test('subset_threads', executable('hb-subset-threads', 'hb-subset-threads.cc',
  dependencies: [
    thread_dep
  ],
  cpp_args: [],
  include_directories: [incconfig, incsrc],
  link_with: [libharfbuzz, libharfbuzz_subset],
  install: false,
  ),
  workdir: meson.current_source_dir() / '..' / '..',
  timeout: 300,
  suite: ['threads', 'slow'],
)
