Contributing to MuseScore's Assets
==================================

- [Basic Requirements]
  - [Requirements for source files]
    - [CMake files]
    - [Source SVGs]
    - [Source rasters]
  - [Requirements for generated files]
    - [Generated SVGs]
    - [Generated rasters]
    - [Generated icon files]
- [Coding Techniques]
  - [DRY: Don't Repeat Yourself!]
- [SVG Best Practice]
  - [Editing SVGs]
  - [Re-using SVG content]
  - [Fonts and SVG text]
    - [Font licensing]
  - [SVG Compatibility]
- [Understanding the build process]
  - [Why CMake?]
  - [CMake files]
  - [Where the build begins]
  - [CMake functions and macros]
  - [Command execution order]

## Basic Requirements
[Basic Requirements]: #basic-requirements

### Requirements for source files
[Requirements for source files]: #requirements-for-source-files

These requirements apply to all files that get checked into the repository.

#### CMake files
[CMake files]: #cmake-files

Files named `CMakeLists.txt` or `*.cmake` contain the code used by the CMake
build system to generate the assets. Try to employ good [Coding Techniques].

#### Source SVGs
[Source SVGs]: #source-svgs

SVG files in the repository must be fully editable "master" files. This means:

- Any text visible in an SVG image must be stored as editable text (i.e. not
  converted to paths).
- Any [images included in the file][Source rasters] are linked to rather than
  embedded (use relative paths for links!).
- No duplicated content (see [Re-using SVG content]).

If the source file requires some form of pre-processing as part of the build
process then you should indicate this through the file extension. Keep the
existing `.svg` extension and append `.xml` for files that contain XML syntax
that is not part of the SVG specification, and `.in` for files that must be
configured with CMake.

#### Source rasters
[Source rasters]: #source-rasters

You should avoid using raster images (PNGs, JPEG, etc.) as source files if at
all possible. If they are necessary (e.g. as a background texture) then they
should be optimized to reduce the file size as much as possible *without loss
of quality* and stored in the [resources] folder.

[resources]: resources "Textures and basic shapes used in other assets"

- Use JPEG for photographs and textures
- Use PNG for computer generated images (e.g. screenshots)

Raster images can be included in SVGs using the `<image>` element:

```svg
<image width="200" height="100" xlink:href="relative/path/to/file.jpg"/>
```

Technically this also works to include other SVG files, but you shouldn't use
it for that purpose as there are better ways to do this.

### Requirements for generated files
[Requirements for generated files]: #requirements-for-generated-files

These requirements apply to all files intended for external use (i.e. the ones
listed in the [assets manifest]).

[assets manifest]: assets-manifest.txt

The CMake files already define helper functions to get generated files to meet
these requirements automatically, so it's usually just a case of making sure
the source SVG is listed in one of the CMakeLists.txt files and that it is
passed through the right helper functions.

There are additional requirements for files that must integrate with the
operating system. These are explained in the [platform README].

[platform README]: platform/README.md

#### Generated SVGs
[Generated SVGs]: #generated-svgs

Generated SVGs must also be optimized in terms of file size, while remaining
fully portable and self-contained (standalone). This means:

- All `<text>` elements converted to paths to ensure they are displayed
  correctly regardless of whether the user has the right fonts installed.
- All raster images embedded into the SVG as [Base64 Data URIs] to ensure the
  files never become separated.
- No linking elements that point to external files (e.g. `<use>`, `<image>`).
- Formatting stored as XML attributes (`attr="val"`) rather than CSS styles
  (`style="attr:val"`) for compatibility reasons.
- No specialist SVG syntax that is not widely supported by SVG viewers. See
  [SVG Compatibility].

[Base64 Data URIs]: https://en.wikipedia.org/wiki/Data_URI_scheme#SVG

__Helper functions:__

- `standalone_svg()`
- `optimize_svg()`

### Generated rasters
[Generated rasters]: #generated-rasters

These should be generated from SVGs at each required size. Do not generate one
PNG at the largest size and then scale it down to the other required sizes;
this leads to reduced quality and increased file size compared to generating
each size of PNG straight from the SVG. All PNGs should be run through
`pngcrush` to optimize the file size, and this must be done before the PNG is
used for other things, such as embedding into icon files.

__Helper functions:__

-  `rasterize_svg()`
-  `optimize_png()`

### Generated icon files
[Generated icon files]: #generated-icon-files

ICO and ICNS icon files contain the same image at multiple image sizes. Many
icon tools allow you to create an icon from a single PNG file, but you __must
not__ use this feature. Instead, generate a separate PNG at each required size
as outlined in the section above, and then embed all the sizes into the icon.

Additional requirements for icon files are given in the [platform README].

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

- [Re-using SVG content]
- [CMake functions and macros]

See the [splash images](splash) and associated build rules for examples of all
the techniques explained in those sections.

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
copy the relevant part of the SVG code (`d="..."`) back into the source file.
However, it is preferable to define paths manually where possible.

For help with paths, see:

- https://www.w3schools.com/graphics/svg_path.asp
- https://css-tricks.com/svg-path-syntax-illustrated-guide/
- https://www.w3.org/TR/SVG11/paths.html#PathData

#### Re-using SVG content
[Re-using SVG content]: #re-using-svg-content

##### Cloning elements within an SVG file
[Cloning elements within an SVG file]: #cloning-elements-within-an-svg-file

If any SVG group or shape is given an ID (e.g. `<circle id="my-circle" ...`)
then it can be cloned elsewhere in the file with the `<use>` element
(`<use xlink:href="#my-circle"/>`). You can assign different properties to the
clone, such as giving it a different fill or stroke, and apply transformations
like `translate` (move), `scale` and `rotate`. If you make a change to the
original shape then all of the clones will be updated automatically.

##### Cloning elements from other SVG files
[Cloning elements from other SVG files]: #cloning-elements-from-other-svg-files

The `<use>` element also allows you to clone elements defined in other SVG
files (`<use xlink:href="relative/path/to/file.svg#my-circle"/>`), and you can
even insert an entire SVG using the `<image>` element. However, __you mustn't
use either of these features__ because it means the SVG is no longer a
standalone file (there are ways around this but they are less than ideal) and
there are other limitations besides.

Instead, if two SVGs share some of the same content, use [XInclude] to grab
the content from one file and dump it in the other at build time. (This makes
use of the fact that SVG files are also XML files.)

[XInclude]: https://en.wikipedia.org/wiki/XInclude

__Include an entire SVG (or XML) file:__

```xml
<xi:include href="relative/path/to/file.svg" xmlns:xi="http://www.w3.org/2001/XInclude"/>
```

If you only want to include part of a file, you can use an [XPointer]. The
most intuitive ways to use XPointers with XInclude are as follows:

[XPointer]: https://en.wikipedia.org/wiki/XPointer

__Include everything inside the outermost SVG (or XML) tag:__

```xml
<xi:include href="relative/path/to/file.svg" xpointer="xpointer(*/*)" xmlns:xi="http://www.w3.org/2001/XInclude"/>
```

__Include the SVG (or XML) element with attribute `id="name"`:__

```xml
<xi:include href="relative/path/to/file.svg" xpointer="xpointer(//*[@id='name'])" xmlns:xi="http://www.w3.org/2001/XInclude"/>
```

__Include everything inside the SVG (or XML) element with attribute `id="name"`:__

```xml
<xi:include href="relative/path/to/file.svg" xpointer="xpointer(//*[@id='name'])" xmlns:xi="http://www.w3.org/2001/XInclude"/>
```

IDs are stripped during the build as part of the SVG optimization process, so
you can't use the `[@id='name']` expression on files that have been optimized.
There should be equivalent optimized and non-optimized versions of every SVG
generated during the build. The non-optimized version of `file.svg` should be
called `file-standalone.svg`. SVGs in the [resources] folder do not get
optimized, so you can include those with `file.svg`.

Most SVG programs don't understand XInclude syntax, so you won't be able to
see the included elements in viewers until after you have run the build.

### Fonts and SVG text
[Fonts and SVG text]: #fonts-and-svg-text

If the SVG contains text then the text must specify a font, like this:

```xml
<text font-family="Raleway" font-size="36" font-style="italic" font-weight="bold">Hello World!</text>
```

Other font attributes are available, such as `font-variant="small-caps"`, and
you can see the full list [here][font properties in SVG]. Be aware that most
fonts don't provide glyphs for every possible combination of attribute values.
For example, a particular font might provide **bold** and *italic* but not
***bold italic***. You can use Inkscape to see which styles are available for
your font, and you can use Inkscape's built-in [XML viewer/editor] to check
the syntax for the particular text attributes you are interested in (or save a
copy and open it in a text editor) but be aware that most of the information
in the file will be redundant.

[font properties in SVG]: https://www.w3.org/TR/SVG11/text.html#FontPropertiesUsedBySVG
[XML viewer/editor]: http://tavmjong.free.fr/INKSCAPE/MANUAL/html/XML-Basic.html

__Tip:__ you can use the CMake function [`configure_file()`] to substitute
CMake variables inside SVG source files. This allows you to add things like
version numbers to SVGs and have them update automatically, or to use a single
SVG as a template from which multiple other SVGs can be created.

[`configure_file()`]: https://cmake.org/cmake/help/latest/command/configure_file.html

#### Font licensing
[Font licensing]: #font-licensing

The rules around font licensing are extremely complicated. To make it easy:

- Try to only use fonts available from [Google Fonts].
- The font __MUST__ use the [Open Font License] by SIL.

[Google Fonts]: https://fonts.google.com/
[Open Font License]: https://scripts.sil.org/cms/scripts/page.php?site_id=nrsi&item_id=OFL

Not all Google Fonts use the Open Font License, so make sure you check. The
license is stated at the bottom right of the font page for each font family.

If you have already used a proprietary font in your design then try to find a
replacement. Many proprietary fonts have an open source equivalent that can be
used as a drop-in replacement. Simply do an online search for "open source
alternative to ..." and then check to see if it uses the Open Font License.

In case you are interested, the main licensing requirement is obviously that
the font is free and open source. However, the license must also:

- Allow the font to be embedded in documents.
- Allow the font glyphs to vectorized (converted to outlines/paths).
- Apply only to the font itself and not to any documents that use the font,
  either in embedded or vector form.

This excludes pretty much any font covered by any of the familiar open source
software licenses, including the GPL, as those licenses were not written with
fonts in mind. This can make it difficult to distribute fonts with open source
software, but we can still distribute documents that *use* the font as long as
the font's license meets the above requirements.

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

As always, you should include a comment at the relevant place in the source
file if compatibility issues force you to do anything that is not obvious.

## The Build Process
[The build process]: #the-build-process

### Why CMake?
[Why CMake?]: #why-cmake

The CMake build system is used to generate the assets. It uses a strange
syntax which is very verbose and not particularly friendly for beginners.
We could generate the assets using simple Bash scripts, but there are
advantages to using CMake, as it:

- Behaves more consistently than Bash on certain systems (i.e. Windows).
- Integrates well with native build tools and IDEs.
- Is already used for MuseScore's build.

Finally, CMake provides various features that Bash doesn't (Bash isn't a build
tool after all), such as:

- Checking whether targets have already been generated and only building the
  ones for which the source files have changed.
- Running builds in parallel (i.e. building more than one target at a time).
- Handling dependencies (i.e. when some targets must be built before others).

These abilities help to save a lot of time on successive builds and reduce the
number of writes made to disk. These features could be written in Bash, but we
would have to write them ourselves whereas CMake gives us them for free.

### Editing CMake Files
[CMake files]: #cmake-files

CMake builds the assets based on the instructions in the `CMakeList.txt` and
`*.cmake` files. Each directory has its own CMake files that contain the rules
needed to build all the assets in that directory and any subdirectories.

- If you are just making changes within an existing SVG then it probably won't
  be necessary to make any changes to the CMake files at all.

- If you are adding new SVGs to an existing directory then you just need to
  modify the `CMakeLists.txt` file in that directory. Use its existing rules
  as a guide.

- If you create a new directory then you need to create a new `CMakeLists.txt`
  file to go inside it. Use an existing `CMakeLists.txt` as a template.

None of those situations require detailed knowledge of how the build process
works or of CMake in general. However, if you are trying to do something a bit
more complicated then it is helpful if you know a bit about how CMake works.

### Understanding the build process
[Understanding the build process]: #understanding-the-build-process

The most difficult thing to understand is the order in which things happen.
The build rules are defined in the CMake files, but events don't necessarily
take place in the order in which they appear in those files. To see why this
is, you first need to understand that CMake doesn't actually build anything
itself. Instead, the building is done by a native build tool (usually Make,
Xcode or MSCV). CMake simply gets things ready for the native build tool.

#### The `configure` step

##### Types of CMake command

The `configure` step is performed entirely by CMake based on commands in the
CMake files. It basically has two jobs to do:

1. Perform any necessary housekeeping (e.g. with `configure_file()`).
2. Generate code (called "targets") for the native build tool.
    - CMake generates this code immediately, but the code is not executed
      until the native build tool runs in the `build` step.

This means there are essentially two types of command you will come across in
the CMake files:

1. Things CMake does itself (i.e. immediately).
2. Things it tells the native build tool to do later.

In general, commands that CMake does immediately can only interact with source
files (i.e. files in the Git repository) or configured files (i.e. files
created by CMake, usually with `file(WRITE)` or `configure_file()`). These
commands can get and set CMake variables, but they cannot interact with files
that are created during the `build` step because those files do not exist when
CMake is run.

On the other hand, commands that tell CMake to generate targets for the native
build tool are generally able to interact with all files, including files that
are created in the `build` step. However, these commands cannot be used to set
CMake variables because the `build` step is not performed by CMake (you can
still pass variables in as input to these commands).

Sometimes there are two commands that serve a similar purpose, but one is for
CMake and the other is for the native build tool. For an example of this see
[`execute_process()` vs `add_custom_target()`].

##### Order of command execution

When you run CMake in the `configure` step, it begins by looking at the code
in the [top-level CMakeList.txt]. The code in this file sets the project name
(we treat the assets as a separate project to MuseScore) and various options
and variables. CMake works its way through this file, running each command in
turn, until it reaches an `include()` or `add_subdirectory()` command, which
tell it to run some code from one of the other CMake files in the project. The
code in the other file is also executed in-turn, and then CMake returns to the
first file and picks up where it left off. Once it reaches the end of the
top-level CMakeLists.txt CMake exits and the `configure` step is complete.

[top-level CMakeLists.txt]: CMakeLists.txt

So in reality everything does happen in the order we would expect. There is of
course some added complexity due to the presence of control statements like
`if()` and `foreach()`, and [CMake functions and macros], but this is pretty
much the same as in any programming language. The only difference is that some
commands don't just contain instructions for CMake; they also contain
instructions for the native build tool. This can make it look like CMake is
ignoring your commands, when in reality it did what it was supposed to do and
added them to the code it generates for the native build tool.

##### The `build` step

The `build` step is performed entirely by the native build tool based on the
instructions given to it by CMake. Everything in the `configure` step either
happened in the order we would expect or it was deferred until the `build`
step. Unfortunately, the `build` step is not so simple.

Recall that some CMake commands created code (called "targets") for the native
build tool to perform. Targets are not necessarily run in the order in which
they were defined in the CMake files. In fact, the native build tool will
ideally build multiple targets simultaneously to save time, and this can lead
to the order being essentially random, so you have no way of knowing when
things will happen!

Sometimes a target cannot run until another target is complete. For example,
there might be a target to generate an SVG and another target to convert the
SVG into a PNG. Clearly the PNG cannot be created before the SVG has been
generated, so the PNG *depends* on the SVG. It is up to you to tell the build
system that this dependency exists otherwise it could try to build the targets
in the wrong order. Notice that this means you can still encounter errors in
the `build` step even if the `configure` step appeared to be successful.

The native build tool's code is generated by CMake so we don't want to go
editing it ourselves. The only (sensible) way to declare a dependency is
through the CMake files. This is explained in
[`add_custom_target()` vs `add_custom_command()`].

### Similar commands

#### `execute_process()` vs `add_custom_target()`
[`execute_process()` vs `add_custom_target()`]: #execute_process-vs-add_custom_target

Both of these are commands accept a `COMMAND` argument which is used to run
a program on the command line with the specified options. The difference is
that `execute_process()` *tells CMake* to run a program on the command line,
whereas `add_custom_target()` *tells CMake to tell the native build tool* to
run a program on the command line. This can be a bit confusing, but the
distinction is important because it affects when the program gets run and what
what you are able do with it.

In the case of `execute_process()`, CMake runs the program straight away and
then waits for it to finish. There is an option to store the program's return
status and outputs (from `stdout` and `stderr`) in variables to use later in
the CMake code. However, since the program is run during the `configure` step,
it is only able to interact with source files and configured files (e.g. those
created with `configure_file()`), because files created during the `build`
step will not exist yet.

In the case of `add_custom_target()`, CMake generates a target for the native
build tool to run during the `build` step. The target can specify a program to
run on the command line (but it doesn't have to). Since the program is not run
by CMake, you are not able to access its output or return status in CMake
variables. However, it is able to interact with files built during the `build`
step, but only if those files are built first (i.e. the target must specify a
dependency on those files and/or the targets that create them). It can of
course interact with any source files or configured files since those are
guaranteed to exist at this point in the build, but there is no need to
specify them as a dependency.

#### `add_custom_target()` vs `add_custom_command()`
[`add_custom_target()` vs `add_custom_command()`]: #add_custom_target-vs-add_custom_command

Both of these are commands can be used to run programs on the command line and
both of them do this during the `build` step (use `execute_process()` to run
a program during the `configure` step). This means the program (or programs)
is not run by CMake but by the native build tool. If any of the programs
return a non-zero exit status then the build is failed and an error message is
displayed.

In general, you should use `add_custom_command()` whenever you want to run a
program to *create or modify a file* on the filesystem. You should specify
this file as `OUTPUT` to the custom command, and any files that the program
needs look at first as dependencies (with the `DEPENDS` argument). The native
build tool will only run the command if the `OUTPUT` doesn't already exist
(i.e. on a clean build) or if one of the dependencies has changed since the
command was last run. This saves a lot of time on future builds, allowing the
build to complete more or less instantly if nothing has changed. However,
__the command will not run at all unless its `OUTPUT` is depended on by a
custom target or by another custom command__, and the same applies to the
other custom command, so there must be a custom target involved somewhere.

You should use `add_custom_target()` when you want to run a program that
doesn't modify any files on the filesystem. You might do this to run some kind
of code validation check that will return a non-zero status if it fails, thus
stopping the build (hopefully with a useful error message printed on the
console). However, the most common use of custom targets is simply to cause
custom commands to get built, which you do by specifying the `OUTPUT` of the
custom command as the `DEPENDS` argument to the custom target (there is no
need to specify a `COMMAND` argument in this case). The target itself will not
be built unless one of the following conditions is met:

- You specifically ask the native build tool to build the target:
  - e.g. for Make: `make TARGETNAME`
  - or in general: `cmake --build . --target=TARGETNAME`
- The target was defined with the `ALL` keyword and you don't specify any
  targets for the native tool, i.e. you build the `all`/`default` target:
  - e.g. for Make: `make`
  - or in general: `cmake --build .`
- The target is depended on by another target which is being built.
  - You do this with the `add_dependencies()` command in CMake.

Sometimes you might create a top-level target that simply serves as a shortcut
to build lots of other targets:

```cmake
add_custom_target(bigtarget)
add_dependencies(bigtarget smalltarget1 smalltarget2 smalltarget3 ...)
```

##### Depending on a custom command in another directory

The following situation can sometimes arise:

- In directory `A`:
  - the target `targetA` is used to run a custom command that outputs a file
    called `fileA`.
- In directory `B`:
  - the target `targetB` is used to run a custom command that outputs a file
    called `fileB`.

Let's say that the command for `fileB` needs to use `fileA` as input. It
should be enough to say this:

```cmake
custom_command(OUTPUT fileB DEPENDS ../A/fileA COMMAND ...)
```

However, due to some quirks of CMake and various native build tools, it is
actually necessary to __also__ say:

```cmake
add_dependencies(targetB targetA) # no need to specify a directory for targets
```

This ensures that `targetA` is built before `targetB` and that `fileB` will be
rebuilt if `fileA` changes.

### CMake Functions and Macros
[CMake functions and macros]: #cmake-functions-and-macros

CMake allows you to create [functions] and [macros] that you can use later in
the project. This avoids having to write out the code in the function multiple
times, which is considered [bad coding practice][DRY: Don't Repeat Yourself!].

[functions]: https://cmake.org/cmake/help/latest/command/function.html "Custom CMake functions"
[macros]: https://cmake.org/cmake/help/latest/command/macro.html "Custom CMake macros"

__Tip:__ the difference between a macro and a function is that variables set
in functions are not available outside the function (not unless you ask them
to be by setting them with the [`PARENT_SCOPE`] keyword). Variables set in
macros are *always* available outside the macro, which means they overwrite
other variables with the same name. __Always use a function unless you have a
specific need for a macro.__ If you ever find that you *do* need to create a
macro in order to perform a particular task then make sure you leave a comment
to say why a function wouldn't do the job.

[`PARENT_SCOPE`]: https://cmake.org/cmake/help/latest/command/set.html#set-normal-variable
