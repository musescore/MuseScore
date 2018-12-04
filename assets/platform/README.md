MuseScore's Desktop Assets
==========================

Assets produced in these directories integrate into the user's home screen or
desktop environment. This includes:

- __[app-icons]__ Application icons displayed in app stores, launchers and
  system menus.
- __[file-icons]__ Document filetype/mimetype icons displayed in file managers.

These assets are usually displayed by the operating system rather than by
MuseScore.

[app-icons]: app-icons "Application icons displayed in system menus"
[file-icons]: file-icons "Document icons displayed in file managers"

## Platform requirements

Platforms often have [technical requirements] and [style guidelines] for
assets that integrate with their systems. These requirements change frequently
to follow (or set) the latest fashions in UX design so they are not documented
here in any detail. Check the comments in the code for specific details in
places where platform requirements have affected the design and build process,
as well as links to the sources of those requirements.

[technical requirements]: #technical-requirements
[style guidelines]: #style-guidelines

### Technical Requirements

These include requirements to do with the file format used for icons as well
as the dimensions of the images they contain. Assets must meet at least the
most basic of these requirements otherwise they will not be displayed
correctly or at all. However, it may be safe to ignore some of the more
specialist requirements if they are not relevant to MuseScore.

Some requirements are not dealt with as part of the assets build and are
instead handled as part of MuseScore's build or installation process. This
includes any requirements to do with final names asset files must be given and
the locations they must be placed in for the operating system to be able to
find them. The assets build is purely concerned with generating the files.

Technical requirements mainly affect the build rules in the CMake files, so
relevant details should be mentioned in those files where they have influenced
the build process.

### Style Guidelines

These relate to the artistic design of assets. Many platforms recommend that
designs are restricted to a particular set of shapes or colors to achieve a
consistent look and feel for all applications on that platform. While we don't
want MuseScore to look out of place on any platform, we must balance this
desire with our own requirement that MuseScore should look and feel the same
on all platforms.

- Where platforms have guidelines that are harmonious, or at least compatible,
  then we clearly should do our best to follow them.

- Where a particular recommendation from one platform is neither encouraged or
  discouraged on another platform it may nevertheless be safe to follow the
  guideline on both platforms.

- Where platforms have conflicting guidelines it may be better to adopt a
  neutral style rather than favoring one set of guidelines over the other.

Style Guidelines mainly affect the content of the SVGs, so relevant details
should be mentioned as comments in those files where they have influenced the
design.

## Icon Formats

The two most common icons formats are ICO (Windows) and ICNS (macOS). Linux
and BSD used to use the XPM format for icons but this has been almost
universally replaced with ordinary PNG and SVG images on those systems (more
on this below).

The ICO and ICNS formats are containers for holding multiple images, or more
commonly, multiple versions of the same image. Each version can have different
image properties, enabling the system to pick the best version of the image to
use in any given situation.

### Image Sizes

The most common use of icons is to provide the same image so that the system
can display the size best suited to the monitor's size and resolution, the
average viewing distance, the user's display settings, and where the icon is
to be used (e.g. tree views use smaller icons than list views, File Open and
File Save dialogs use smaller icons than in the ordinary file manager, etc.).
Icons usually provide a range of image sizes from 16x16 pixels up to 256x256
or higher. The smaller sizes are still regularly used even on modern devices.

#### Pixel Densities

The resolution or "pixel density" of a display is measured in Pixels Per Inch
(PPI) or Pixels Per CentiMeter (PPCM). These are commonly referred to as Dots
Per Inch (DPI) or Dots Per Centimeter (DPCM) respectively, although those
terms technically refer to printer resolutions. Also, the word "resolution" is
often incorrectly used to mean the pixel size of a display, but that's another
story...

[DPI standards]: https://en.wikipedia.org/wiki/Dots_per_inch#Computer_monitor_DPI_standards

For many years, all monitors had basically the same pixel density of 72 DPI.
[Wikipedia explains][DPI standards] why Apple thought 72 DPI was such a great
idea, why they were wrong, and how Microsoft tried to fix it by pretending 72
DPI equals 96 DPI, which created even more problems. The upshot is that many
computer programs on Windows, including Qt, now report two DPIs:

- __Physical DPI:__ the actual number of pixels per inch that the monitor has.
- __Logical DPI:__ the number Windows pretends the monitor has (usually 96).

These days monitors have a range of physical DPIs, from mobile devices



The size of a GUI element has traditionally been measured in pixels. This was
fine while everybody sat two feet away from 1024x768 monitors, but it creates
problems on modern HDPI displays (called "retina displays" by Apple) and on
mobile devices where the average viewing distance is very different to that of
a traditional desktop arrangement. A 100 pixel line will appear much smaller
on an HDPI display than on a standard display, while it

and the problem is made even worse on mobile devices where the

is even worse on mobile devices, where the the average viewing
distance can be very different to the traditional desktop

Apple's ICNS format allows different images to be used on HDPI ("retina")
displays compared to on standard displays. However, this does not mean that
the retina icons have a higher resolution


 This means that a single icon file
might contain two images with 128x128 pixels, one for use on standard

Other image properties that can vary between images within an icon:

- __device pixel ratio__ - whether the image is intended for use on standard
  displays (DPR = 1) or HDPI/"retina" displays (DPR > 1).
- __color mode__ - monochrome, grayscale, indexed, full color (RGB or RGBA).
  - Grayscale images have a single color channel (gray).
  - Full color images have 3 channels, or 4 if there is an alpha channel.
  - Indexed images have no channels. Instead they have a palette of up to 256
    colors, and each pixel stores a number corresponding to a color in the
    palette. You might be able to pick the colors available in the palette (an
    "optimum" palette), or you might be forced to using an existing palette of
    usually 16 (4 bit) or 256 (8 bit) colors (e.g the "web-safe" palette).
  - Monochrome images can be regarded as indexed with a 2 color palette
    consisting of only black and white, or as grayscale (single channel)
    images with a bit depth of 1.
- __bit depth__ - affects the number of colors that can appear in the image.
  - Confusingly, bit depth can be given *per color channel* or *per pixel*
    (total for all channels), and it may or may not include the alpha channel.
  - The most common value *per channel* is 8 (256 levels), but with the advent
    of HDR displays we can expect to see 10 (1024 levels) become the new norm.
  - A *per-channel bit depth* of 8 gives a *total bit depth per pixel* of 24
    (16.7 million colors) or 32 (4.3 billion) including the alpha channel.
  - Apart from 24 and 32, the most common *total bit depths per pixel* are 1
    (monochrome), 4 (16 colors indexed) and 8 (256 colors indexed or grayscale).
  - Notice that the phrase "bit depth of 8" is very ambiguous!
- __alpha bit depth__ - bit depth of the alpha channel.
    - This can be different to the bit depth of the other channels.
    - Common values are 0 (no alpha channel), 1 (binary mask: each pixel is
      fully transparent or fully opaque), and 8 (256 levels of transparency).
- __encoding__ - type of image compression.

 This is often used to provide an image
at multiple sizes, ranging from 16x16 pixels up to 256x256 pixels or higher,
to allow the system to display the most appropriate size based on the monitor
resolution, display settings, and the type of item view it is being displayed
in (list view, grid view, tree view, etc.).

Size is not the only attribute that can change. Images can also have different
color depths (number of colors), encoding schemes, alpha masks, etc.

- 16x16 pixels for use on small displays


The
most common use is to hold multiple copies of the same image rendered at
different sizes.

Icon files are special image files that can contain multiple images optimized
for different scenarios. The most obvious

Windows and macOS have special image formats specifically for icons, called
ICO and ICNS respectively. Both formats are capable of

Icon files are capable of containing multiple raster images at various sizes,
pixel densities, bit depths (number of colors) and encodings. The idea is that
the operating system only has to look in one place to find all the available
versions of an image, and it will pick the most appropriate image to use for
any given situation.

Icon files (ICO, ICNS) should be generated from separate PNG files at each
required size. Do not generate them from a single PNG (not even a large one)
as this leads to reduced quality and increased file size.
