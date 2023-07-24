 
This directory contains small executables to verify fluidsynths correct behaviour, i.e. unit tests.

#### Do *not* blindly use the tests as template for your application!

Although some tests might serve as educational demonstration of how to use certain parts of fluidsynth,
they are **not** intended to do so! It is most likely that those tests will consist of many hacky parts
that are necessary to test fluidsynth (e.g. including fluidsynth's private headers to access internal
data types and functions). For user applications this programming style is strongly discouraged!
Keep referring to the documentation and code examples listed in the [API documentation](http://www.fluidsynth.org/api/).

#### Developers

To add a unit test just duplicate an existing one, give it a unique name and update the CMakeLists.txt by

* adding a call to `ADD_FLUID_TEST()` and
* a dependency to the custom `check` target.

Execute the tests via `make check`. Unit tests should use the `VintageDreamsWaves-v2.sf2` as test soundfont.
Use the `TEST_SOUNDFONT` macro to access it.
