To build the fuzzers with libFuzzer to perform actual fuzzing, build with:

```shell
CXX=clang++ CXXFLAGS="-fsanitize=address,fuzzer-no-link" meson fuzzbuild --default-library=static -Dfuzzer_ldflags="-fsanitize=address,fuzzer"

ninja -Cfuzzbuild
```

Then, run the fuzzer like this:

fuzzbuild/test/fuzzing/hb-{shape,draw,subset,set}-fuzzer [-max_len=2048] [CORPUS_DIR]

Where max_len specifies the maximal length of font files to handle.
The smaller the faster.

For more details consult the following locations:
  - http://llvm.org/docs/LibFuzzer.html
