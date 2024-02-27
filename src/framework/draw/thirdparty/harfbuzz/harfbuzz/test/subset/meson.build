tests = [
  'basics',
  'full-font',
  'cff-full-font',
  'japanese',
  'cff-japanese',
  'cff.notoserifmyanmar',
  'glyf_bug_3131',
  'layout',
  'layout.gpos',
  'layout.gpos2',
  'layout.gpos3',
  'layout.gpos4',
  'layout.gpos5',
  'layout.gpos6',
  'layout.gpos8',
  'layout.gpos8.amiri',
  'layout.gpos9',
  'layout.gsub3',
  'layout.gsub5',
  'layout.gsub5_format2',
  'layout.gsub6',
  'layout.gsub8',
  'layout.gdef',
  'layout.gdef.glyphset',
  'layout.khmer',
  'layout.context',
  'layout.context_format2',
  'layout.gdef-varstore',
  'layout.gdef-attachlist',
  'layout.notonastaliqurdu',
  'layout.tinos',
  'layout.duplicate_features',
  'layout.unsorted_featurelist',
  'layout.drop_feature',
  'cmap',
  'cmap14',
  'sbix',
  'colr',
  'colr_glyphs',
  'math',
  'math_coverage_offset',
  # TODO: re-enable once colrv1 subsetting is stabilized.
  # 'colrv1.notoemoji',
  'colrv1',
  'colrv1_copy_varstore',
  'colr_with_components',
  'cbdt',
  'variable',
  'glyph_names',
  'post',
  '32bit_var_store',
  'pin_all_at_default',
  'instantiate_glyf',
  'instantiate_cff2',
  'full_instance',
  'instance_feature_variations',
  'instance_no_double_free',
  'mvar_full_instance',
  'instance_comp_glyph_empty_child',
  'post_apply_mvar_delta',
  'apply_cvar_delta',
]

repack_tests = [
  'basic',
  'prioritization',
  'table_duplication',
  'isolation',
  'advanced_prioritization',
  'space_splitting',
]

run_test = find_program('run-tests.py')

foreach t : tests
  fname = '@0@.tests'.format(t)

  test(t, run_test,
    args: [
      hb_subset,
      meson.current_source_dir() / 'data' / 'tests' / fname,
    ],
    # as the tests are ran concurrently let's raise acceptable time here
    # ideally better to break and let meson handles them in parallel
    timeout: 500,
    workdir: meson.current_build_dir() / '..' / '..',
    suite: 'subset',
  )
endforeach

run_repack_test = find_program('run-repack-tests.py')

foreach t : repack_tests
  fname = '@0@.tests'.format(t)

  test(t, run_repack_test,
    args: [
      hb_subset,
      meson.current_source_dir() / 'data' / 'repack_tests' / fname,
    ],
    workdir: meson.current_build_dir() / '..' / '..',
    suite: ['subset', 'repack'],
  )
endforeach
