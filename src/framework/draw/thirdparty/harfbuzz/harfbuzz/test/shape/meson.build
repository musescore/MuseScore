
subdir('data/in-house') # in_house_tests
subdir('data/aots') # aots_tests
subdir('data/text-rendering-tests') # text_rendering_tests

shape_run_tests_py = find_program('run-tests.py')

env = environment()
env.set('HAVE_FREETYPE', '@0@'.format(conf.get('HAVE_FREETYPE', 0)))
env.set('HAVE_CORETEXT', '@0@'.format(conf.get('HAVE_CORETEXT', 0)))
env.set('HAVE_DIRECTWRITE', '@0@'.format(conf.get('HAVE_DIRECTWRITE', 0)))
env.set('HAVE_UNISCRIBE', '@0@'.format(conf.get('HAVE_UNISCRIBE', 0)))

foreach file_name : in_house_tests
  test_name = file_name.split('.')[0]

  test(test_name, shape_run_tests_py,
    args: [
      hb_shape,
      meson.current_source_dir() / 'data' / 'in-house' / 'tests' / file_name,
    ],
    env: env,
    workdir: meson.current_build_dir() / '..' / '..',
    suite: ['shape', 'in-house'],
  )
endforeach

foreach file_name : aots_tests
  test_name = file_name.split('.')[0]

  test(test_name, shape_run_tests_py,
    args: [
      hb_shape,
      meson.current_source_dir() / 'data' / 'aots' / 'tests' / file_name,
    ],
    env: env,
    workdir: meson.current_build_dir() / '..' / '..',
    suite: ['shape', 'aots'],
  )
endforeach

foreach file_name : text_rendering_tests
  test_name = file_name.split('.')[0]

  test(test_name, shape_run_tests_py,
    args: [
      hb_shape,
      meson.current_source_dir() / 'data' / 'text-rendering-tests' / 'tests' / file_name,
    ],
    env: env,
    workdir: meson.current_build_dir() / '..' / '..',
    suite: ['shape', 'text-rendering-tests'],
  )
endforeach
