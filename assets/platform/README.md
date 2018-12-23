MuseScore Assets - Platform
===========================

Assets produced in these directories integrate into the user's home screen or
desktop environment. This includes:

- __[app-icons]:__ Application icons displayed in app stores, launchers and
  system menus.
- __[file-icons]:__ Document mime/filetype icons displayed in file managers.

These assets are usually displayed by the operating system rather than by
MuseScore.

[app-icons]: app-icons "Application icons displayed in system menus"
[file-icons]: file-icons "Document icons displayed in file managers"

## Platform Requirements

Platforms often have [technical requirements] and [style guidelines] for
assets that integrate with their systems. These requirements change frequently
to follow (or set) the latest fashions in UX design so they are not documented
here in any detail. Check the comments in the code for specific details in
places where platform requirements have affected the design and build process,
as well as links to the sources of those requirements.

[technical requirements]: #technical-requirements
[style guidelines]: #style-guidelines

### Technical requirements

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

### Style guidelines

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
image properties, such as different sizes, enabling the system to pick the
best version of the image to use depending on:

- The monitor's size and resolution
- The average viewing distance
- The user's display settings
- Where the icon is to be used, such as:
  - Item views (tree views use smaller icons than list views)
  - File Open and File Save dialogs
  - File managers
  - Launcher menus

Icons usually provide a range of image sizes from 16x16 pixels up to 256x256
or higher. The smaller sizes are still regularly used even on modern devices.

Image size is not the only property that can differ between images in the same
icon file. Other properties include bit depth (number of colors), encoding
(type of image compression), alpha transparency, or pretty much any other
property, though size is the most common one to change.

#### HDPI / Retina displays

Modern devices pack more pixels into the same area of screen than older
displays, so icons need to have more pixels to compensate otherwise they will
appear too small on the screen. Most platforms simply use one of the larger
image sizes when they detect that the user has a High DPI display, but Apple's
ICNS format allows special images to be embedded specifically for use with
HDPI displays (Apple calls them "Retina" displays). This means that a single
icon file might contain some sizes twice, one for use on normal displays and
one for Retina displays. The non-Retina version of a 256 x 256 pixel icon is
labelled "256x256", while the Retina version is labelled "128x128@2x". Most of
Apple's Retina displays have double the DPI of the standard DPI displays they
replaced, so 256 pixels on a Retina "@2x" display is the same physical length
(in inches) as 128 pixels on a non-Retina display.

Of course, the ability to use different images for Retina displays is only
beneficial if you do in fact use different images. Since our images are all
generated from the same SVGs there is literally no difference between the
256x256 image and the 128x128@2x image - and there doesn't really need to be.
Apple argue that it can be beneficial to increase the stroke width in Retina
icons compared to non-Retina icons, but our icons don't make use of strokes,
and there is certainly no benefit to be had from specialization in regions of
solid color or simple gradients.
