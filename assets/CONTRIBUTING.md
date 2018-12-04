Contributing to MuseScore's Assets
==================================

- [Basic Requirements]
  - [Requirements for Source Files]
    - [Source SVGs]
    - [Source Rasters]
  - [Requirements for Generated Files]
    - [Generated SVGs]
    - [Generated Rasters]
    - [Generated Icon Files]
- [Coding Techniques]
  - [DRY: Don't Repeat Yourself]
- [SVG Best Practice]
  - [Editing SVGs]
  - [Re-using SVG Content]
  - [Fonts]
  - [SVG Compatibility]
- [Understanding the Build Process]
  - [Why CMake?]
  - [CMake Files]
  - [Where the Build Begins]
  - [CMake Functions and Macros]
  - [Command Execution Order]

## Basic Requirements
[Basic Requirements]: #basic-requirements

### Requirements for Source Files
[Requirements for Source Files]: #requirements-for-source-files

These requirements apply to all files that get checked into the repository.

#### Source SVGs
[Source SVGs]: #source-svgs

Files in the repository must be fully editable "master" files. This means:

- Any text visible in an SVG image must be stored as editable text (i.e. not
  converted to paths).
- Any images included in the file stored as "xlinks" to other files (use
  relative paths for links!).
- No duplicated content (see [Avoiding Copy and Paste]).

[Avoiding Copy and Paste]: #avoiding-copy-and-paste

#### Source Rasters
[Source Rasters]: #source-rasters

You should avoid using raster images (PNGs, JPEG, etc.) as source files if at
all possible. If they are necessary (e.g. as a background texture) then they
should be optimized to reduce the file size as much as possible *without loss
of quality* and stored in the resources folder.

- Use JPEG for photographs and textures
- Use PNG for computer generated images (e.g screenshots)

### Requirements for Generated Files
[Requirements for Generated Files]: #requirements-for-generated-files

These requirements apply to all files intended for external use (i.e. the ones
listed in the [assets manifest]).

[assets manifest]: assets-manifest.txt

Note that you often need to run the source files through multiple programs
before they are ready for external use, so there will be intermediate files
created that don't meet either set of requirements. This is fine as long as
they don't appear in the manifest.

There are additional requirements for file that must integrate with the
operating system. These are explained in <platform/README.md>.

#### Generated SVGs
[Generated SVGs]: #generated-svgs

Generated SVGs must be fully portable and self-contained (standalone). This
means:

- All `<text>` elements converted to paths to ensure they are displayed
  correctly regardless of whether the user has the right fonts installed.
- All raster images embedded into the SVG as [Base64 Data URIs] to ensure the
  files never become separated.
- No linking elements that point to external files (e.g. `<use>`, `<image>`).
- Formatting stored as XML attributes (`attr="val"`) rather than CSS styles
  (`style="attr:val"`) for compatibility reasons.
- No specialist SVG syntax that is not widely supported by SVG viewers.

[Base64 Data URIs]: https://en.wikipedia.org/wiki/Data_URI_scheme#SVG

See [SVG Compatibility](#svg-compatibility) below.

### Generated Rasters
[Generated Rasters]: #generated-rasters

These should be generated from SVGs at each required size. Do not generate one
PNG at the largest size and then scale it down to the other required sizes;
this leads to reduced quality and increased file size compared to generating
each size of PNG straight from the SVG. All PNGs should be run through
`pngcrush` to optimize the file size, and this must be done before the PNG is
used for other things, such as embedding into icon files.

### Generated Icon Files
[Generated Icon Files]: #generated-icon-files

ICO and ICNS icon files contain the same image at multiple image sizes. Many
icon tools allow you to create an icon from a single PNG file, but you __must
not__ use this feature. Instead, generate a separate PNG at each required size
as outlined in the section above, and then embed all the sizes into the icon.

Additional requirements for icon files are given in <platform/README.md>.

## Coding Techniques
[Coding Techniques]: #coding-techniques

### DRY: Don't Repeat Yourself!
[DRY: Don't Repeat Yourself!]: #dry-dont-repeat-yourself

Try to avoid copying and pasting code as much as possible as this makes things
harder to understand and maintain. Instead, make use of functions and links
that you define once and then call upon elsewhere in the code. This means that
changes can be made easily by updating the definition; there's no need to go
hunting for other places where the same code is used.

These sections explain how to apply this principle in various parts of the
code:

- [Re-using SVG Content]
- [CMake Functions and Macros]

See the [splash images](splash) and associated build rules for examples of all
the techniques explained in those sections, and they also show how you can use
[`configure_file()`] to substitute CMake variables inside SVG files.

[`configure_file()`]: https://cmake.org/cmake/help/latest/command/configure_file.html "CMake Commands - configure_file()"

## SVG Best Practice
[SVG Best Practice]: #svg-best-practice

### Editing SVGs
[Editing SVGs]: #editing-svgs

You should create and edit SVGs manually in a text editor. Do not use a
graphical SVG editing program as these tend to add unnecessary bloat to the
SVG code. They also tend to convert simple shapes like circles to complicated
Bézier paths, making the code difficult to understand and edit later on.

However, you may use a graphical editor to help you edit complicated paths,
where necessary. Simply save the edited file under a different name and then
copy the relevant part of the code back into source file.

#### Re-using SVG Content
[Re-using SVG Content]: #re-using-svg-content

If any SVG group or shape is given an ID (e.g. `<circle id="my-circle" ...`)
then it can be cloned elsewhere in the file with the `<use>` element
(`<use xlink:href="#my-circle"/>`). You can assign different properties to the
clone, such as giving it a different fill or stroke, and apply transformations
like `translate` (move), `scale` and `rotate`. If you make a change to the
original shape then all of the clones will be updated automatically.

The `<use>` element even allows you to clone elements defined in other SVG
files (`<use xlink:href="path/to/file.svg#my-circle"/>`). If two SVGs share
some of the same content, consider moving the shared content to its own SVG
in the [resources folder] and referencing it with `<use>`. You can also use
`<use>` with content from SVGs that are generated as part of the build, but be
aware that IDs are stripped as part of the optimization process, so you can
only `<use>` SVGs that have not been optimized.

[resources folder]: resources "Textures and basic shapes used in other assets"

Unfortunately the `<use>` element is quite limited (e.g. it doesn't work with
gradients). When `<use>` doesn't work try `<xinclude>` instead as this allows
you to simply dump content from one XML file inside another. However, you will
need to process the file with `xmllint` as part of the build as `<xinclude>`
is not part of the SVG specification.

Do not use the `<image>` element to include one SVG inside another as this
is inefficient in terms of file size and has other limitations besides. The
`<image>` element must only be used for embedding JPEG and PNG images (and
only when it is [appropriate to do so](#source-rasters)).

### Fonts
[Fonts]: #fonts

To avoid most licensing issues, only use fonts that are open source and free.
This excludes pretty much any font included with Windows or macOS, or shipped
with a proprietary design program such as Adobe Illustrator. [Google Fonts] is
a good place to look for free fonts, but be sure to check that the license of
your chosen font is [compatible with the GPLv2][GPL compatible licenses].

[Google Fonts]: https://fonts.google.com/
[GPL compatible licenses]: https://www.gnu.org/licenses/license-list.html#GPLCompatibleLicenses

If you have already used a proprietary font in your design then try to find a
replacement. Simply do an online search for "open source alternative to ...".
Many proprietary fonts have an open source equivalent that can be used as a
drop-in replacement.

### SVG Compatibility
[SVG Compatibility]: #svg-compatibility

Support for SVG features varies widely between SVG viewer applications.
Fortunately, pretty much all viewers support a core set of features, and most
of the specialist features can be "faked" using the core set.

A good way to test for compatibility is to see how the SVG looks when opened
in [Qt's example SVG Viewer][QT-svg-viewer] application. The viewer can be
loaded and built [from the Welcome screen in Qt Creator][Qt-examples].

[QT-svg-viewer]: https://doc.qt.io/qt-5/qtsvg-svgviewer-example.html
[Qt-examples]: https://doc.qt.io/qt-5/qtexamplesandtutorials.html

Qt only supports a subset of the SVG specification called SVG Tiny, and its
implementation appears to be somewhat buggy and incomplete. SVGs that are
displayed within MuseScore are rendered using Qt, so it is essential that they
are displayed correctly in the viewer.

### Understanding the Build Process
[Understanding the Build Process]: #understanding-the-build-process

#### Why CMake?
[Why CMake?]: #why-cmake

The CMake build system is used to generate the assets. It uses a strange
syntax which is very verbose and not particularly friendly for beginners.
We could generate the assets using simple Bash scripts, but there are
advantages to using CMake:

- CMake behaves more consistently than Bash on certain systems (i.e. Windows).
- CMake integrates well with native build tools and IDEs.
- CMake is already used for MuseScore's build.

Finally, CMake provides various features that Bash doesn't (Bash isn't a build
tool after all), such as:

- Checking whether assets have already been generated and only building the
  ones that have changed.
- Running builds in parallel (i.e. building more than one asset at a time).
- Handling dependencies (i.e. this asset must be built before that one, etc.).

These abilities help to save a lot of time on successive builds and reduce the
number of writes made to disk. These features could be written in Bash, but we
would have to write them ourselves whereas CMake gives us them for free.

#### CMake Files
[CMake Files]: #cmake-files

CMake builds the assets based on the instructions in the `CMakeList.txt` and
`*.cmake` files, which are together known as the "CMake files". Each directory
has its own CMake files that contain the rules needed to build all the assets
in that directory and any subdirectories.

- If you are just making changes to the existing source SVGs then it might not
  be necessary to make any changes to the build process at all.

- If you are adding new SVGs to an existing directory then you just need to
  modify the `CMakeLists.txt` file in that directory. Use its existing rules
  as a guide.

- If you create a new directory then you need to create a new `CMakeLists.txt`
  file to go inside it. Use an existing `CMakeLists.txt` as a template.

None of those situations require detailed knowledge of how the build process
works or of CMake in general. However, if you are trying to do something a bit
more complicated then it is useful to understand a bit about how CMake works.

### Where the Build Begins
[Where the Build Begins]: #where-the-build-begins

The assets build begins with the [top-level CMakeList.txt], and this is the
file you should look at first. The code in this file sets the default options
and creates the

XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

for the assets build and pulls in some function definitions from the `*.cmake`
files to use later in the build.

 and from `CMakeList.txt`
files in the subdirectories (`add_subdirectory(DIRECTORY)`).

[top-level CMakeLists.txt]: CMakeLists.txt
[image-functions.cmake]: image-functions.cmake

The assets build is a separate project to MuseScore as far as CMake is
concerned. Within assets CMake files the variables `${PROJECT_SOURCE_DIR}`
and `${PROJECT_BINARY_DIR}` always refer to the assets' source and build
directories, not MuseScore's, even when building the assets with MuseScore.

### CMake Functions and Macros
[CMake Functions and Macros]: #cmake-functions-and-macros

CMake allows you to create [functions] and [macros] that you can use later in
the project. This avoids having to write out the code in the function multiple
times, which is considered [bad coding practice][DRY: Don't Repeat Yourself!].

[functions]: https://cmake.org/cmake/help/latest/command/function.html "Custom CMake functions"
[macros]: https://cmake.org/cmake/help/latest/command/macro.html "Custom CMake macros"

__Tip:__ the difference between a macro and a function is that variables set
in functions are not available outside the function (not unless you ask them
to be by setting them with the [`PARENT_SCOPE`] keyword). Variables set in
macros are *always* available outside the macro, which means they overwrite
other variables with the same name. *Always use a function unless you have a
specific need for a macro.* If you do need to create a macro then make sure
you leave a comment to say why a function wouldn't do the job.

[`PARENT_SCOPE`]: https://cmake.org/cmake/help/latest/command/set.html#set-normal-variable

### Command Execution Order
[Command Execution Order]: #command-execution-order

The key point to understand about CMake is that it is a build generator, not
a complete build system. CMake's job is to take the set of build rules we
wrote in the CMake files and use these to generate a new set of rules (called
targets) for another build tool to follow in doing the actual build.

The build happens in two stages:

- `configure` - this is when you run CMake on the build rules we wrote.
- `build` - this is when the build tool is run on the rules CMake generated.

(Other projects sometimes have more stages after these, often called `install`
and `package`, but these stages are not used for the assets project.)

The upshot of all this is that __commands are not necessarily executed in the
order in which they appear in the CMake files__. Some commands are executed
immediately by CMake during the `configure` step, while other commands tell
CMake to set up a target for the native system to build in the `build` step.
CMake sets up the target straight away, but the code inside the target is not
executed until the native tool is run in the `build` step.

If you understand this then you understand the most difficult thing about
using CMake, and indeed any build system.
