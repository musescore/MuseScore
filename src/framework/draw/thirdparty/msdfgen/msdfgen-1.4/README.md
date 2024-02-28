# Multi-channel signed distance field generator

This is a utility for generating signed distance fields from vector shapes and font glyphs,
which serve as a texture representation that can be used in real-time graphics to efficiently reproduce said shapes.
Although it can also be used to generate conventional signed distance fields best known from
[this Valve paper](http://www.valvesoftware.com/publications/2007/SIGGRAPH2007_AlphaTestedMagnification.pdf)
and pseudo-distance fields, its primary purpose is to generate multi-channel distance fields,
using a method I have developed. Unlike monochrome distance fields, they have the ability
to reproduce sharp corners almost perfectly by utilizing all three color channels.

The following comparison demonstrates the improvement in image quality.

![demo-msdf16](https://cloud.githubusercontent.com/assets/18639794/14770355/14cda9f8-0a70-11e6-8346-2bd14b5b832f.png)
![demo-sdf16](https://cloud.githubusercontent.com/assets/18639794/14770360/20c51156-0a70-11e6-8f03-ed7632d07997.png)
![demo-sdf32](https://cloud.githubusercontent.com/assets/18639794/14770361/251a4406-0a70-11e6-95a7-e30e235ac729.png)

## New in version 1.4
 - The procedure of how contours are combined together has been reworked, and now supports overlapping contours,
   which are often present in fonts with auto-generated accented glyphs. Since this is a major change to the core algorithm,
   the original versions of all functions in [msdfgen.h](msdfgen.h) have been preserved with `_legacy` suffix,
   and can be enabled in the command line tool with **-legacy** switch.
 - A major bug has been fixed in the evaluation of signed distance of cubic curves, in which at least one of the control points
   lies at the endpoint. If you use an older version, you should update now.
 - In the standalone program, the orientation of the input is now being automatically detected by sampling the signed distance
   at an arbitrary point outside the shape's bounding box, and the output adjusted accordingly. This can be disabled
   by new option **-keeporder** or the pre-existing **-reverseorder**.

## Getting started

The project can be used either as a library or as a console program. It is divided into two parts, **[core](core)**
and **[extensions](ext)**. The core module has no dependencies and only uses bare C++. It contains all
key data structures and algorithms, which can be accessed through the [msdfgen.h](msdfgen.h) header.
Extensions contain utilities for loading fonts and SVG files, as well as saving PNG images.
Those are exposed by the [msdfgen-ext.h](msdfgen-ext.h) header. This module uses
[FreeType](http://www.freetype.org/),
[TinyXML2](http://www.grinninglizard.com/tinyxml2/),
and [LodePNG](http://lodev.org/lodepng/).

Additionaly, there is the [main.cpp](main.cpp), which wraps the functionality into
a comprehensive standalone console program. To start using the program immediately,
there is a Windows binary available for download in the "Releases" section.

## Console commands

The standalone program is executed as
```
msdfgen.exe <mode> <input> <options>
```
where only the input specification is required.

Mode can be one of:
 - **sdf** &ndash; generates a conventional monochrome signed distance field.
 - **psdf** &ndash; generates a monochrome signed pseudo-distance field.
 - **msdf** (default) &ndash; generates a multi-channel signed distance field using my new method.

The input can be specified as one of:
 - **-font \<filename.ttf\> \<character code\>** &ndash; to load a glyph from a font file.
   Character code can be expressed as either a decimal (63) or hexadecimal (0x3F) Unicode value, or an ASCII character
   in single quotes ('?').
 - **-svg \<filename.svg\>** &ndash; to load an SVG file. Note that only the last vector path in the file will be used.
 - **-shapedesc \<filename.txt\>**, -defineshape \<definition\>, -stdin &ndash; to load a text description of the shape
   from either a file, the next argument, or the standard input, respectively. Its syntax is documented further down.

The complete list of available options can be printed with **-help**.
Some of the important ones are:
 - **-o \<filename\>** &ndash; specifies the output file name. The desired format will be deduced from the extension
   (png, bmp, txt, bin). Otherwise, use -format.
 - **-size \<width\> \<height\>** &ndash; specifies the dimensions of the output distance field (in pixels).
 - **-range \<range\>**, **-pxrange \<range\>** &ndash; specifies the width of the range around the shape
   between the minimum and maximum representable signed distance in shape units or distance field pixels, respectivelly.
 - **-autoframe** &ndash; automatically frames the shape to fit the distance field. If the output must be precisely aligned,
   you should manually position it using -translate and -scale instead.
 - **-scale \<scale\>** &ndash; sets the scale used to convert shape units to distance field pixels.
 - **-translate \<x\> \<y\>** &ndash; sets the translation of the shape in shape units. Otherwise the origin (0, 0)
   lies in the bottom left corner.
 - **-angle \<angle\>** &ndash; specifies the maximum angle to be considered a corner.
   Can be expressed in radians (3.0) or degrees with D at the end (171.9D).
 - **-testrender \<filename.png\> \<width\> \<height\>** - tests the generated distance field by using it to render an image
   of the original shape into a PNG file with the specified dimensions. Alternatively, -testrendermulti renders
   an image without combining the color channels, and may give you an insight in how the multi-channel distance field works.
 - **-exportshape \<filename.txt\>** - saves the text description of the shape with edge coloring to the specified file.
   This can be later edited and used as input through -shapedesc.
 - **-printmetrics** &ndash; prints some useful information about the shape's layout.

For example,
```
msdfgen.exe msdf -font C:\Windows\Fonts\arialbd.ttf 'M' -o msdf.png -size 32 32 -pxrange 4 -autoframe -testrender render.png 1024 1024
```

will take the glyph capital M from the Arial Bold typeface, generate a 32&times;32 multi-channel distance field
with a 4 pixels wide distance range, store it into msdf.png, and create a test render of the glyph as render.png.

## Library API

If you choose to use this utility inside your own program, there are a few simple steps you need to perform
in order to generate a distance field. Please note that all classes and functions are in the `msdfgen` namespace.

 - Acquire a `Shape` object. You can either load it via `loadGlyph` or `loadSvgShape`, or construct it manually.
   It consists of closed contours, which in turn consist of edges. An edge is represented by a `LinearEdge`, `QuadraticEdge`,
   or `CubicEdge`. You can construct them from two endpoints and 0 to 2 Bézier control points.
 - Normalize the shape using its `normalize` method and assign colors to edges if you need a multi-channel SDF.
   This can be performed automatically using the `edgeColoringSimple` heuristic, or manually by setting each edge's
   `color` member. Keep in mind that at least two color channels must be turned on in each edge, and iff two edges meet
   at a sharp corner, they must only have one channel in common.
 - Call `generateSDF`, `generatePseudoSDF`, or `generateMSDF` to generate a distance field into a floating point
   `Bitmap` object. This can then be worked with further or saved to a file using `saveBmp` or `savePng`.
 - You may also render an image from the distance field using `renderSDF`. Consider calling `simulate8bit`
   on the distance field beforehand to simulate the standard 8 bits/channel image format.

Example:
```c++
#include "msdfgen.h"
#include "msdfgen-ext.h"

using namespace msdfgen;

int main() {
    FreetypeHandle *ft = initializeFreetype();
    if (ft) {
        FontHandle *font = loadFont(ft, "C:\\Windows\\Fonts\\arialbd.ttf");
        if (font) {
            Shape shape;
            if (loadGlyph(shape, font, 'A')) {
                shape.normalize();
                //                      max. angle
                edgeColoringSimple(shape, 3.0);
                //           image width, height
                Bitmap<FloatRGB> msdf(32, 32);
                //                     range, scale, translation
                generateMSDF(msdf, shape, 4.0, 1.0, Vector2(4.0, 4.0));
                savePng(msdf, "output.png");
            }
            destroyFont(font);
        }
        deinitializeFreetype(ft);
    }
    return 0;
}

```

## Using a multi-channel distance field

Using a multi-channel distance field generated by this program is similarly simple to how a monochrome distance field is used.
The only additional operation is computing the **median** of the three channels inside the fragment shader,
right after sampling the distance field. This signed distance value can then be used the same way as usual.

The following is an example GLSL fragment shader including anti-aliasing:

```glsl
in vec2 pos;
out vec4 color;
uniform sampler2D msdf;
uniform float pxRange;
uniform vec4 bgColor;
uniform vec4 fgColor;

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

void main() {
    vec2 msdfUnit = pxRange/vec2(textureSize(msdf, 0));
    vec3 sample = texture(msdf, pos).rgb;
    float sigDist = median(sample.r, sample.g, sample.b) - 0.5;
    sigDist *= dot(msdfUnit, 0.5/fwidth(pos));
    float opacity = clamp(sigDist + 0.5, 0.0, 1.0);
    color = mix(bgColor, fgColor, opacity);
}
```

## Shape description syntax

The text shape description has the following syntax.
 - Each closed contour is enclosed by braces: `{ <contour 1> } { <contour 2> }`
 - Each point (and control point) is written as two real numbers separated by a comma.
 - Points in a contour are separated with semicolons.
 - The last point of each contour must be equal to the first, or the symbol `#` can be used, which represents the first point.
 - There can be an edge segment specification between any two points, also separated by semicolons.
   This can include the edge's color (`c`, `m`, `y` or `w`) and/or one or two Bézier curve control points inside parentheses.
   
For example,
```
{ -1, -1; m; -1, +1; y; +1, +1; m; +1, -1; y; # }
```
would represent a square with magenta and yellow edges,
```
{ 0, 1; (+1.6, -0.8; -1.6, -0.8); # }
```
is a teardrop shape formed by a single cubic Bézier curve.
