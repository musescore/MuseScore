if conf.get('HAVE_GLIB', 0) == 0
  message('You need to have glib support enabled to run test/api tests')
  subdir_done()
endif

tests = [
  'test-aat-layout.c',
  'test-baseline.c',
  'test-be-glyph-advance.c',
  'test-be-num-glyphs.c',
  'test-blob.c',
  'test-buffer.c',
  'test-c.c',
  'test-collect-unicodes.c',
  'test-cplusplus.cc',
  'test-common.c',
  'test-draw.c',
  'test-extents.c',
  'test-font.c',
  'test-font-scale.c',
  'test-glyph-names.c',
  'test-instance-cff2.c',
  'test-map.c',
  'test-object.c',
  'test-ot-alternates.c',
  'test-ot-collect-glyphs.c',
  'test-ot-color.c',
  'test-ot-face.c',
  'test-ot-glyphname.c',
  'test-ot-layout.c',
  'test-ot-ligature-carets.c',
  'test-ot-name.c',
  'test-ot-meta.c',
  'test-ot-metrics.c',
  'test-ot-tag.c',
  'test-ot-extents-cff.c',
  'test-ot-metrics-tt-var.c',
  'test-paint.c',
  'test-subset-repacker.c',
  'test-set.c',
  'test-shape.c',
  'test-style.c',
  'test-subset.c',
  'test-subset-cmap.c',
  'test-subset-drop-tables.c',
  'test-subset-glyf.c',
  'test-subset-hdmx.c',
  'test-subset-hmtx.c',
  'test-subset-nameids.c',
  'test-subset-os2.c',
  'test-subset-post.c',
  'test-subset-vmtx.c',
  'test-subset-cff1.c',
  'test-subset-cff2.c',
  'test-subset-gvar.c',
  'test-subset-hvar.c',
  'test-subset-vvar.c',
  'test-subset-sbix.c',
  'test-subset-gpos.c',
  'test-subset-colr.c',
  'test-subset-cbdt.c',
  'test-unicode.c',
  'test-var-coords.c',
  'test-version.c',
]

if conf.get('HAVE_FREETYPE', 0) == 1
  tests += [
    'test-ot-math.c',
    'test-ft.c',
  ]
endif

if conf.get('HAVE_CORETEXT', 0) == 1
  tests += [
    'test-coretext.c',
  ]
endif

if conf.get('HAVE_FREETYPE', 0) == 1 and conf.get('HAVE_PTHREAD', 0) == 1
  tests += 'test-multithread.c'
endif

# Default test running environment
env = environment()
env.set('MALLOC_CHECK_', '2')
env.set('G_DEBUG', 'gc-friendly')
env.set('G_SLICE', 'always-malloc')
env.set('G_TEST_SRCDIR', meson.current_source_dir())
env.set('G_TEST_BUILDDIR', meson.current_build_dir())

foreach source : tests
  cpp_args = []
  test_name = source.split('.')[0]

  deps = [glib_dep, freetype_dep, thread_dep, libharfbuzz_dep, libharfbuzz_icu_dep]
  suite = ['api']
  if test_name.contains('-subset') or test_name.contains('-instance')
    deps += libharfbuzz_subset_dep
    suite += 'subset'
  endif

  if test_name.contains('-instance')
    cpp_args += '-DHB_EXPERIMENTAL_API'
  endif

  test(test_name, executable(test_name, source,
    cpp_args:  cpp_args,
    include_directories: [incconfig],
    dependencies: deps,
    install: false,
  ), env: env, suite: suite)
endforeach
