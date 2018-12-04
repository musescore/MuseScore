![MuseScore Logo](resources/mscore.svg)

MuseScore's Assets
==================

This is where all assets are stored as SVG (Scalable Vector Graphics) files.
These source files are processed by various command line tools, including
[Inkscape], to produce more SVGs as well as icons (ICO, ICNS) and other raster
images (PNG) at build time. There is a list of all the files generated during
the build in [assets-manifest.txt].

[Inkscape]: https://inkscape.org/ "Inkscape open source vector graphics editor"

- [Justification]
- [Structure]
  - [Files]
  - [Directories]
- [Building the Assets]
  - [Building with MuseScore]
  - [Building Alone]
    - [The First Build]
      - [CMake Generators]
      - [Dependencies]
    - [Subsequent Builds]
- [Contributing to the Assets]
- [Using Assets in MuseScore's Build]

## Justification
[Justification]: #justification

Why compile assets? Why not just commit PNG & ICO files into the repository?

- __Binary files do not give readable diffs when they are edited.__
  - It is hard to see what has changed and to understand why it was necessary.
- __Compressed files are not stored efficiently by `git`.__
  - Make a small change to a text file or uncompressed binary file and only
    the changes (i.e. the diff) need to be stored.
  - Compression mixes the changed and unchanged parts of a file, so if you
    make a small change to a PNG then a whole new file will be stored.
  - The old file will forever be part of the history, slowing checkouts and
    branch operations, and wasting disk space and bandwidth.
- __The GPL requires all source files to be distributed.__
  - Source files are resources in their most editable form, such as SVGs with
    editable text, not PNGs with text 'burned in' to the pixel data.
- __Building assets saves having to generate the files manually.__
  - Reduced potential for mistakes and human error.
  - No possibility of forgetting to update assets for one of the platforms.

## Structure
[Structure]: #structure

### Files
[Files]: #files

- `CMakeLists.txt`, `*.cmake` - scripts used by the CMake build system to
  generate the assets.
- `*.svg`, `*.jpg`, `*.png` - source SVG, JPEG and PNG images.
- [assets-manifest.txt] - list of files generated during the build process.
  - NOTE: Paths in this file are relative to the build directory.
- [CONTRIBUTING.md] -  read this before making any changes to the assets.

[assets-manifest.txt]: assets-manifest.txt "Assets manifest"
[CONTRIBUTING.md]: CONTRIBUTING.md "Information for contributors"

### Directories
[Directories]: #directories

- __[Resources]:__ Textures and basic shapes that are used in other assets
- __[Brand]:__ Logos
- __[Platform]:__ Application and document icons used by the operating system
- __[Splash]:__ Splash screen images displayed during MuseScore's startup
- *Glyphs:* in-app icons for MuseScore's buttons and menus are not currently
  part of the assets build. They are in [../mscore/data/icons][Glyphs].

[Resources]: resources "Textures and basic shapes used in other assets"
[Brand]: brand "Logos"
[Platform]: platform "App and document icons used by the operating system"
[Splash]: splash "Splash screen images displayed during MuseScore's startup"
[Glyphs]: ../mscore/data/icons "Icons for MuseScore's buttons and menus"

The [Platform] directory produces assets that must integrate into the user's
desktop environment (file manager and system menu). This means there are extra
requirements for those files, as explained in the [Platform README].

[Platform README]: platform/README.md

## Building the assets
[Building the assets]: #building-the-assets

The assets can be built automatically as part of MuseScore's build, or they
can be built separately (i.e. without building MuseScore). This is useful for
development and testing purposes.

Like MuseScore, the assets build uses the [CMake build system], and the steps
are pretty much the same as for any CMake project.

[CMake build system]: https://cmake.org/

### Building with MuseScore
[Building with MuseScore]: #building-with-musescore

Follow the [instructions to compile MuseScore][MuseScore compilation] and
ensure it is building and running correctly.

[MuseScore compilation]: https://musescore.org/en/handbook/developers-handbook/compilation "MuseScore Developers Handbook - Compilation"

MuseScore has a build option called `DOWNLOAD_ASSETS` which disables the
assets build and causes assets to be download pre-built. This option is
enabled by default to save time and reduce the number of dependencies
developers have to install on their machines.

If you want to build the assets simply set MuseScore's `DOWNLOAD_ASSETS` build
option to `OFF` and then run the build again. The build will probably fail now
due to missing programs or fonts, so see the [Dependencies section] to get it
working again.

When building with MuseScore the assets are built inside the assets sub-folder
of MuseScore's build directory (i.e. `build.debug/assets`).

[Dependencies section]: #dependencies

### Building Alone
[Building Alone]: #building-alone

MuseScore's build takes much longer than the assets build alone, so you may
want to build the assets without building MuseScore. This saves time when
working on the assets and can help you to gain experience with CMake.

Note that you will still need MuseScore's source code (since the assets are
part of it), and it will be very helpful if you have already managed to build
MuseScore in the past, with or without the assets.

Thanks to the fact CMake does out of source builds (i.e. builds are done in a
separate directory to the source code) you can continue to use the same source
tree for MuseScore's builds, assets builds, and combined builds.

#### The First Build
[The First Build]: #the-first-build

Assuming you have already downloaded MuseScore's source code and managed to
compile it, to access the assets source code all you need to do is change
directory into the assets folder:

```bash
cd /path/to/MuseScore/assets
```

Once inside the assets source directory you can ignore everything outside it
and simply (try to) build the assets as you would any other CMake project:

```bash
mkdir build      # create a directory for the assets build
cd build
cmake ..         # this is the `configure` step (notice the two dots/periods)
cmake --build .  # this is the `build` step (notice the single dot/period)
```

These commands are __likely to fail__ at the `configure` step, but don't worry
as we will try to get it working in the next couple of sections.

__Tip:__ Recent versions of CMake have a `-j` option to specify the number of
parallel build jobs to run. The more jobs you run the faster the build, up to
a point, but if you run too many the build will go more slowly and your
computer may hang (become unresponsive) until the build is complete. The
optimum number of build jobs is usually the same as the number of physical CPU
cores in your machine, or twice that number if your CPU has hyper-threading:

```
cmake --build -j8 . # `build` step with 4 cores + HT (8 logical cores)
```

Most systems report the number of logical cores rather than the number of
physical cores, so you don't need to double it.

##### CMake Generators
[CMake Generators]: #cmake-generators

Read this section if you see an error like this when trying to run CMake:

- `CMake Error: CMake was unable to find a build program corresponding to "[name of generator]".`

CMake doesn't actually perform the `build` step itself. Instead, it calls a
[native build tool] to do the work. CMake's job is to _generate_ instructions
for the native build tool (it does this during the `configure` step).

[native build tool]: https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html "Native build tools supported by CMake"

The error message means that CMake couldn't find the native build tool it was
trying to generate instructions for. You could solve this issue by installing
the program mentioned in the message, or by telling CMake to use another build
tool that you know is already installed.

If you have managed to compile MuseScore before then __you already have a
build tool installed__. Run `cmake --help` and look under "generators" to see
a list of build tools supported on your system. Compare this list with the
programs you installed while following [MuseScore's compilation guide] to work
out which build tool you have installed.

[MuseScore's compilation guide]: https://musescore.org/en/handbook/developers-handbook/compilation "MuseScore Developers Handbook - Compilation"

Once you know which build tool you have, you can use CMake's `-G` (generator)
option to specify the build tool to generate for during the `configure` step:

```bash
cmake "-GXcode" ..  # `configure` step for Xcode on macOS
```

Sometimes you also need to tell CMake what the build tool is called on the
command line. You do this by defining (with `-D`) the CMake variable
[`CMAKE_MAKE_PROGRAM`][CMAKE_MAKE_PROGRAM] during the `configure` step:

[CMAKE_MAKE_PROGRAM]: https://cmake.org/cmake/help/latest/variable/CMAKE_MAKE_PROGRAM.html "CMAKE_MAKE_PROGRAM CMake Variable"

```bash
# `configure` step for MinGW Make on Windows
cmake "-GMinGW" "-DCMAKE_MAKE_PROGRAM=mingw32-make.exe" ..
```

The `build` step is always the same regardless of which generator you use,
though CMake will call a different build tool behind the scenes. (You could
even call the native build tool yourself if you know what you are doing.)

##### Dependencies
[Dependencies]: #dependencies

The first time you try to build the assets project you are likely to get an
error message saying the build has failed due to a missing dependency:

- `Program not found: [name of program]`
- `Font not found: [name of font]`
- `Variable not defined: [name of variable]`

###### Fonts and Programs

For missing fonts and programs, simply install the required program or font
and run CMake again. If the program has been installed and still is not found
then make sure its location is included in your `PATH` environment variable.

###### Variables

If variables are not defined then you can set the `STRICT_VARIABLES` option to
`OFF` to allow the build to proceed using default values. This can be done by
calling CMake with the `-D` (define) option during the `configure` step:

```bash
cmake "-DSTRICT_VARIABLES=OFF" ..  # `configure` step with option specified
```

This is OK for development and testing, but for production use you should
leave the `STRICT_VARIABLES` option enabled and instead give appropriate
values to the variables, again using the `-D` flag:

```bash
cmake "-DVAR1=Val 1" "-DVAR2=Val 2" ..  # `configure` step with variables set
```

You shouldn't see the `Variable not defined` message when building the assets
as part of MuseScore's build. If you do see it then make sure the relevant
variables are set in [MuseScore's CMakeLists.txt] *before* the assets project
gets called (i.e. before the line `add_subdirectory(assets)`).

[MuseScore's CMakeLists.txt]: ../CMakeLists.txt

#### Subsequent Builds
[Subsequent Builds]: #subsequent-builds

The `configure` step only has to be done on the first build. On subsequent
builds it is enough to simply run `cmake --build .` to rebuild any files that
have changed.

If you run into problems you can try running `cmake --build --target clean .`
to delete all generated files, thereby forcing them all to be built again the
next time you run `cmake --build .`. (You can also use the all-in-one command
`cmake --build --clean-first .`.)

If cleaning doesn't fix the problem then you can try starting from scratch by
making a fresh build directory and doing the `configure` step again.

## Contributing to the Assets
[Contributing to the Assets]: #contributing-to-the-assets

The assets sub-project is very different to the rest of MuseScore's source
code, so make sure you read the [CONTRIBUTING file][CONTRIBUTING.md] before
you make any changes to the code.

## Using Assets in MuseScore's Build and Other Projects
[Using Assets in MuseScore's Build]: #using-assets-in-musescores-build

When it comes to the assets, MuseScore's build must only make use of:

- The assets CMakeLists.txt (used to trigger the assets build).
- Generated files listed in [assets-manifest.txt].
- The assets archive (ZIP file that contains all assets in the manifest).

Those files represent the API for the assets project. Everything else is an
implementation detail and off-limits to projects using the assets.

MuseScore's build shouldn't modify the asset files in any way, except to:

- Move or rename the files as necessary to install them on the user's system.
- Change their timestamps or file permissions as necessary to bundle them for
  distribution.
- Embed them in an installation package or binary executable.

All other modifications, such as format conversions, should be done as part of
the assets build.


[MuseScore's CMakeLists.txt] defines a variable called `ASSETS_BINARY_DIR`,
which points to where the asset files are built or downloaded. Simply look for
the file you need in [assets-manifest.txt] and reference its path like this:

```cmake
"${ASSETS_BINARY_DIR}/path/to/asset/file.ext"

```

Assets are built during the `build` step, which occurs when the native build
tool is run (usually either Make, Xcode or MSCV). This means the assets are
*not* available during the `configure` step when CMake is run, so you can't
use commands like [`file(COPY)`] with the assets. However, since you already
know what the asset filepaths will be you can use [`configure_file()`] to
embed asset paths in source files such as [Qt Resources] (`.qrc` files).
(Source files are not compiled until the `build` step, so the assets will be
available by then.)

[`file(COPY)`]: https://cmake.org/cmake/help/latest/command/file.html "CMake commands - file()"
[`configure_file()`]: https://cmake.org/cmake/help/latest/command/configure_file.html "CMake commands - configure_file()"
[Qt Resources]: https://doc.qt.io/qt-5/resources.html

You can also use the assets with custom commands inside build targets (see
[`add_custom_target()`] and [`add_custom_command()`]) as the targets do not
run until the `build` step, but make sure that the commands you use will work
on all platforms. (CMake provides a number of cross-platform command line
tools which you can access using the `-E` option, such as `cmake -E copy` to
copy files and `cmake -E rename` to move or rename them. See the [man page].)
However, it is best to avoid custom commands if possible as there are better
ways to do this. The only reason to use custom commands would be if you have
to do something that can *only* be done during the `build` step.

[man page]: https://cmake.org/cmake/help/latest/manual/cmake.1.html#command-line-tool-mode "CMake Command Line Options"

Naturally, it is safe to use the assets with any commands or scripts that run
after the `build` step is complete (e.g. in the `install` or `package` steps).
If all you need to do is copy an asset straight from where it was generated
to its final destination, without any modification other than renaming it or
changing file permissions, then this is best done with the [`install()`]
command.

[`add_custom_command()`]: https://cmake.org/cmake/help/latest/command/add_custom_command.html "CMake commands - add_custom_command()"
[`add_custom_target()`]: https://cmake.org/cmake/help/latest/command/add_custom_target.html "CMake commands - add_custom_target()"
[`install()`]: https://cmake.org/cmake/help/latest/command/install.html "CMake commands - install()"
