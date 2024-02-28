
/*
 * MULTI-CHANNEL SIGNED DISTANCE FIELD GENERATOR v1.5 (2017-07-23) - standalone console program
 * --------------------------------------------------------------------------------------------
 * A utility by Viktor Chlumsky, (c) 2014 - 2017
 *
 */

#ifdef MSDFGEN_STANDALONE

#define _USE_MATH_DEFINES
#include <cstdio>
#include <cmath>
#include <cstring>

#include "msdfgen.h"
#include "msdfgen-ext.h"

#ifdef _WIN32
    #pragma warning(disable:4996)
#endif

#define LARGE_VALUE 1e240

using namespace msdfgen;

enum Format {
    AUTO,
    PNG,
    BMP,
    TEXT,
    TEXT_FLOAT,
    BINARY,
    BINARY_FLOAT,
    BINART_FLOAT_BE
};

static char toupper(char c) {
    return c >= 'a' && c <= 'z' ? c-'a'+'A' : c;
}

static bool parseUnsigned(unsigned &value, const char *arg) {
    static char c;
    return sscanf(arg, "%u%c", &value, &c) == 1;
}

static bool parseInteger(int &value, const char *arg) {
    static char c;
    return sscanf(arg, "%d%c", &value, &c) == 1;
}

static bool parseUnsignedLL(unsigned long long &value, const char *arg) {
    static char c;
    return sscanf(arg, "%llu%c", &value, &c) == 1;
}

static bool parseUnsignedHex(unsigned &value, const char *arg) {
    static char c;
    return sscanf(arg, "%x%c", &value, &c) == 1;
}

static bool parseDouble(double &value, const char *arg) {
    static char c;
    return sscanf(arg, "%lf%c", &value, &c) == 1;
}

static bool parseUnicode(int &unicode, const char *arg) {
    unsigned uuc;
    if (parseUnsigned(uuc, arg)) {
        unicode = uuc;
        return true;
    }
    if (arg[0] == '0' && (arg[1] == 'x' || arg[1] == 'X') && parseUnsignedHex(uuc, arg+2)) {
        unicode = uuc;
        return true;
    }
    if (arg[0] == '\'' && arg[1] && arg[2] == '\'' && !arg[3]) {
        unicode = arg[1];
        return true;
    }
    return false;
}

static bool parseAngle(double &value, const char *arg) {
    char c1, c2;
    int result = sscanf(arg, "%lf%c%c", &value, &c1, &c2);
    if (result == 1)
        return true;
    if (result == 2 && (c1 == 'd' || c1 == 'D')) {
        value = M_PI*value/180;
        return true;
    }
    return false;
}

//static void parseColoring(Shape &shape, const char *edgeAssignment) {
//    unsigned c = 0, e = 0;
//    if (shape.contours.size() < c) return;
//    Contour *contour = &shape.contours[c];
//    bool change = false;
//    bool clear = true;
//    for (const char *in = edgeAssignment; *in; ++in) {
//        switch (*in) {
//            case ',':
//                if (change)
//                    ++e;
//                if (clear)
//                    while (e < contour->edges.size()) {
//                        contour->edges[e]->color = WHITE;
//                        ++e;
//                    }
//                ++c, e = 0;
//                if (shape.contours.size() <= c) return;
//                contour = &shape.contours[c];
//                change = false;
//                clear = true;
//                break;
//            case '?':
//                clear = false;
//                break;
//            case 'C': case 'M': case 'W': case 'Y': case 'c': case 'm': case 'w': case 'y':
//                if (change) {
//                    ++e;
//                    change = false;
//                }
//                if (e < contour->edges.size()) {
//                    contour->edges[e]->color = EdgeColor(
//                        (*in == 'C' || *in == 'c')*CYAN|
//                        (*in == 'M' || *in == 'm')*MAGENTA|
//                        (*in == 'Y' || *in == 'y')*YELLOW|
//                        (*in == 'W' || *in == 'w')*WHITE);
//                    change = true;
//                }
//                break;
//        }
//    }
//}

static void invertColor(Bitmap<FloatRGB> &bitmap) {
    for (int y = 0; y < bitmap.height(); ++y)
        for (int x = 0; x < bitmap.width(); ++x) {
            bitmap(x, y).r = 1.f-bitmap(x, y).r;
            bitmap(x, y).g = 1.f-bitmap(x, y).g;
            bitmap(x, y).b = 1.f-bitmap(x, y).b;
        }
}

static void invertColor(Bitmap<float> &bitmap) {
    for (int y = 0; y < bitmap.height(); ++y)
        for (int x = 0; x < bitmap.width(); ++x)
            bitmap(x, y) = 1.f-bitmap(x, y);
}

static void invertColor(Bitmap<uint8_t> &bitmap) {
	for (int y = 0; y < bitmap.height(); ++y)
		for (int x = 0; x < bitmap.width(); ++x)
			bitmap(x, y) = 255 - bitmap(x, y);
}

static bool writeTextBitmap(FILE *file, const float *values, int cols, int rows) {
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            int v = clamp(int((*values++)*0x100), 0xff);
            fprintf(file, col ? " %02X" : "%02X", v);
        }
        fprintf(file, "\n");
    }
    return true;
}

static bool writeTextBitmapFloat(FILE *file, const float *values, int cols, int rows) {
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            fprintf(file, col ? " %g" : "%g", *values++);
        }
        fprintf(file, "\n");
    }
    return true;
}

static bool writeBinBitmap(FILE *file, const float *values, int count) {
    for (int pos = 0; pos < count; ++pos) {
        unsigned char v = clamp(int((*values++)*0x100), 0xff);
        fwrite(&v, 1, 1, file);
    }
    return true;
}

#ifdef __BIG_ENDIAN__
static bool writeBinBitmapFloatBE(FILE *file, const float *values, int count)
#else
static bool writeBinBitmapFloat(FILE *file, const float *values, int count)
#endif
{
    return fwrite(values, sizeof(float), count, file) == count;
}

#ifdef __BIG_ENDIAN__
static bool writeBinBitmapFloat(FILE *file, const float *values, int count)
#else
static bool writeBinBitmapFloatBE(FILE *file, const float *values, int count)
#endif
{
    for (int pos = 0; pos < count; ++pos) {
        const unsigned char *b = reinterpret_cast<const unsigned char *>(values++);
        for (int i = sizeof(float)-1; i >= 0; --i)
            fwrite(b+i, 1, 1, file);
    }
    return true;
}

static bool cmpExtension(const char *path, const char *ext) {
    for (const char *a = path+strlen(path)-1, *b = ext+strlen(ext)-1; b >= ext; --a, --b)
        if (a < path || toupper(*a) != toupper(*b))
            return false;
    return true;
}

template <typename T>
static const char * writeOutput(const Bitmap<T> &bitmap, const std::string &pnn, Format format) {
    const char *filename = pnn.size() ? pnn.c_str() : nullptr;

    if (filename) {
        if (format == AUTO) {
            if (cmpExtension(filename, ".png")) format = PNG;
            else if (cmpExtension(filename, ".bmp")) format = BMP;
            else if (cmpExtension(filename, ".txt")) format = TEXT;
            else if (cmpExtension(filename, ".bin")) format = BINARY;
            else
                return "Could not deduce format from output file name.";
        }
        switch (format) {
            case PNG: return savePng(bitmap, filename) ? NULL : "Failed to write output PNG image.";
            case BMP: return saveBmp(bitmap, filename) ? NULL : "Failed to write output BMP image.";
            case TEXT: case TEXT_FLOAT: {
                FILE *file = fopen(filename, "w");
                if (!file) return "Failed to write output text file.";
                if (format == TEXT)
                    writeTextBitmap(file, reinterpret_cast<const float *>(&bitmap(0, 0)), sizeof(T)/sizeof(float)*bitmap.width(), bitmap.height());
                else if (format == TEXT_FLOAT)
                    writeTextBitmapFloat(file, reinterpret_cast<const float *>(&bitmap(0, 0)), sizeof(T)/sizeof(float)*bitmap.width(), bitmap.height());
                fclose(file);
                return NULL;
            }
            case BINARY: case BINARY_FLOAT: case BINART_FLOAT_BE: {
                FILE *file = fopen(filename, "wb");
                if (!file) return "Failed to write output binary file.";
                if (format == BINARY)
                    writeBinBitmap(file, reinterpret_cast<const float *>(&bitmap(0, 0)), sizeof(T)/sizeof(float)*bitmap.width()*bitmap.height());
                else if (format == BINARY_FLOAT)
                    writeBinBitmapFloat(file, reinterpret_cast<const float *>(&bitmap(0, 0)), sizeof(T)/sizeof(float)*bitmap.width()*bitmap.height());
                else if (format == BINART_FLOAT_BE)
                    writeBinBitmapFloatBE(file, reinterpret_cast<const float *>(&bitmap(0, 0)), sizeof(T)/sizeof(float)*bitmap.width()*bitmap.height());
                fclose(file);
                return NULL;
            }
            default:
                break;
        }
    } else {
        if (format == AUTO || format == TEXT)
            writeTextBitmap(stdout, reinterpret_cast<const float *>(&bitmap(0, 0)), sizeof(T)/sizeof(float)*bitmap.width(), bitmap.height());
        else if (format == TEXT_FLOAT)
            writeTextBitmapFloat(stdout, reinterpret_cast<const float *>(&bitmap(0, 0)), sizeof(T)/sizeof(float)*bitmap.width(), bitmap.height());
        else
            return "Unsupported format for standard output.";
    }
    return NULL;
}

static const char *helpText =
    "\n"
    "Multi-channel signed distance field generator by Viktor Chlumsky v" MSDFGEN_VERSION "\n"
    "---------------------------------------------------------------------\n"
    "  Usage: msdfgen"
    #ifdef _WIN32
        ".exe"
    #endif
        " <mode> <input specification> <options>\n"
    "\n"
    "MODES\n"
    "  sdf - Generate conventional monochrome signed distance field.\n"
    "  psdf - Generate monochrome signed pseudo-distance field.\n"
    "  msdf - Generate multi-channel signed distance field. This is used by default if no mode is specified.\n"
    "  metrics - Report shape metrics only.\n"
    "\n"
    "INPUT SPECIFICATION\n"
    "  -defineshape <definition>\n"
        "\tDefines input shape using the ad-hoc text definition.\n"
    "  -font <filename.ttf> <character code>\n"
        "\tLoads a single glyph from the specified font file. Format of character code is '?', 63 or 0x3F.\n"
    "  -shapedesc <filename.txt>\n"
        "\tLoads text shape description from a file.\n"
    "  -stdin\n"
        "\tReads text shape description from the standard input.\n"
    "  -svg <filename.svg>\n"
        "\tLoads the last vector path found in the specified SVG file.\n"
    "\n"
    "OPTIONS\n"
    "  -angle <angle>\n"
        "\tSpecifies the minimum angle between adjacent edges to be considered a corner. Append D for degrees.\n"
    "  -ascale <x scale> <y scale>\n"
        "\tSets the scale used to convert shape units to pixels asymmetrically.\n"
    "  -autoframe\n"
        "\tAutomatically scales (unless specified) and translates the shape to fit.\n"
    "  -edgecolors <sequence>\n"
        "\tOverrides automatic edge coloring with the specified color sequence.\n"
    "  -errorcorrection <threshold>\n"
        "\tChanges the threshold used to detect and correct potential artifacts. 0 disables error correction.\n"
    "  -exportshape <filename.txt>\n"
        "\tSaves the shape description into a text file that can be edited and loaded using -shapedesc.\n"
    "  -format <png / bmp / text / textfloat / bin / binfloat / binfloatbe>\n"
        "\tSpecifies the output format of the distance field. Otherwise it is chosen based on output file extension.\n"
    "  -help\n"
        "\tDisplays this help.\n"
    "  -keeporder\n"
        "\tDisables the detection of shape orientation and keeps it as is.\n"
    "  -legacy\n"
        "\tUses the original (legacy) distance field algorithms.\n"
    "  -o <filename>\n"
        "\tSets the output file name. The default value is \"output.png\".\n"
    "  -printmetrics\n"
        "\tPrints relevant metrics of the shape to the standard output.\n"
    "  -pxrange <range>\n"
        "\tSets the width of the range between the lowest and highest signed distance in pixels.\n"
    "  -range <range>\n"
        "\tSets the width of the range between the lowest and highest signed distance in shape units.\n"
    "  -scale <scale>\n"
        "\tSets the scale used to convert shape units to pixels.\n"
    "  -size <width> <height>\n"
        "\tSets the dimensions of the output image.\n"
    "  -stdout\n"
        "\tPrints the output instead of storing it in a file. Only text formats are supported.\n"
    "  -testrender <filename.png> <width> <height>\n"
        "\tRenders an image preview using the generated distance field and saves it as a PNG file.\n"
    "  -testrendermulti <filename.png> <width> <height>\n"
        "\tRenders an image preview without flattening the color channels.\n"
    "  -tolerance <tolerance>  (Default: 0.01)\n"
    	"\tTolerance when checking for point equality. Helps avoid artifacts in noisy/inaccurate input shapes.\n"
    "  -translate <x> <y>\n"
        "\tSets the translation of the shape in shape units.\n"
    "  -reverseorder\n"
        "\tDisables the detection of shape orientation and reverses the order of its vertices.\n"
    "  -seed <n>\n"
        "\tSets the random seed for edge coloring heuristic.\n"
    "  -yflip\n"
        "\tInverts the Y axis in the output distance field. The default order is bottom to top.\n"
    "\n";

int main(int argc, const char * const *argv) {
    #define ABORT(msg) { puts(msg); getchar(); return 1; }

    // Parse command line arguments
    enum {
        NONE,
        SVG,
        FONT,
    } inputType = NONE;
    enum {
        SINGLE,
        PSEUDO,
        MULTI,
        METRICS
    } mode = MULTI;
    Format format = AUTO;
    const char *input = NULL;
    std::string outputName = "output.png";
    const char *testRender = NULL;
    const char *testRenderMulti = NULL;
    bool outputSpecified = false;
    bool outputOffsets = false;
    int unicode = 0;
    int svgPathIndex = 0;
    std::string outputPath;

    int width = 64, height = 64;
    int testWidth = 0, testHeight = 0;
    int testWidthM = 0, testHeightM = 0;
    bool autoFrame = false;
    enum {
        RANGE_UNIT,
        RANGE_PX
    } rangeMode = RANGE_PX;
    double range = 1;
    double pxRange = 2;
    Vector2 translate;
    Vector2 scale = 1;
    bool scaleSpecified = false;
    double angleThreshold = 3;
    double edgeThreshold = 1.00000001;
    const char *edgeAssignment = NULL;
    bool yFlip = false;
    bool printMetrics = false;
    bool skipColoring = false;
    enum {
        KEEP,
        REVERSE,
        GUESS
    } orientation = GUESS;
    unsigned long long coloringSeed = 0;

    int argPos = 1;
    bool suggestHelp = false;
    while (argPos < argc) {
        const char *arg = argv[argPos];
        #define ARG_CASE(s, p) if (!strcmp(arg, s) && argPos+(p) < argc)
        #define ARG_MODE(s, m) if (!strcmp(arg, s)) { mode = m; ++argPos; continue; }
        #define SETFORMAT(fmt, ext) \
            do { \
                format = fmt; \
                if (!outputSpecified) \
                    outputName = "output." ext; \
                else if (!strchr(outputName.c_str(), '.')) \
                    outputName += "." ext; \
            } while (false)

        ARG_MODE("sdf", SINGLE)
        ARG_MODE("psdf", PSEUDO)
        ARG_MODE("msdf", MULTI)
        ARG_MODE("metrics", METRICS)

        ARG_CASE("-svg", 1) {
            inputType = SVG;
            input = argv[argPos+1];
            argPos += 2;
            // Optional path specifier
            if( argPos+1 < argc && parseInteger(svgPathIndex, argv[argPos]) ) {
                argPos += 1;
            }
            continue;
        }
        ARG_CASE("-font", 2) {
            inputType = FONT;
            input = argv[argPos+1];
            parseUnicode(unicode, argv[argPos+2]);
            outputName = argv[argPos + 2];
            if (outputName[0] == '0' && (outputName[1] == 'x' || outputName[1] == 'X'))
            {
                outputName = outputName.substr(2);
            }
            outputSpecified = true;
            outputOffsets = true;
            argPos += 3;
            continue;
        }
        ARG_CASE("-path", 1) {
            outputPath = argv[argPos + 1];
            argPos += 2;
            continue;
        }
        ARG_CASE("-o", 1) {
            outputName = argv[argPos+1];
            outputSpecified = true;
            argPos += 2;
            continue;
        }
        ARG_CASE("-stdout", 0) {
            outputName.clear();
            argPos += 1;
            continue;
        }
        ARG_CASE("-format", 1) {
            if (!strcmp(argv[argPos+1], "auto")) format = AUTO;
            else if (!strcmp(argv[argPos+1], "png")) SETFORMAT(PNG, "png");
            else if (!strcmp(argv[argPos+1], "bmp")) SETFORMAT(BMP, "bmp");
            else if (!strcmp(argv[argPos+1], "text") || !strcmp(argv[argPos+1], "txt")) SETFORMAT(TEXT, "txt");
            else if (!strcmp(argv[argPos+1], "textfloat") || !strcmp(argv[argPos+1], "txtfloat")) SETFORMAT(TEXT_FLOAT, "txt");
            else if (!strcmp(argv[argPos+1], "bin") || !strcmp(argv[argPos+1], "binary")) SETFORMAT(BINARY, "bin");
            else if (!strcmp(argv[argPos+1], "binfloat") || !strcmp(argv[argPos+1], "binfloatle")) SETFORMAT(BINARY_FLOAT, "bin");
            else if (!strcmp(argv[argPos+1], "binfloatbe")) SETFORMAT(BINART_FLOAT_BE, "bin");
            else
                puts("Unknown format specified.");
            argPos += 2;
            continue;
        }
        ARG_CASE("-size", 2) {
            unsigned w, h;
            if (!parseUnsigned(w, argv[argPos+1]) || !parseUnsigned(h, argv[argPos+2]) || !w || !h)
                ABORT("Invalid size arguments. Use -size <width> <height> with two positive integers.");
            width = w, height = h;
            argPos += 3;
            continue;
        }
        ARG_CASE("-autoframe", 0) {
            autoFrame = true;
            argPos += 1;
            continue;
        }
        ARG_CASE("-range", 1) {
            double r;
            if (!parseDouble(r, argv[argPos+1]) || r < 0)
                ABORT("Invalid range argument. Use -range <range> with a positive real number.");
            rangeMode = RANGE_UNIT;
            range = r;
            argPos += 2;
            continue;
        }
        ARG_CASE("-pxrange", 1) {
            double r;
            if (!parseDouble(r, argv[argPos+1]) || r < 0)
                ABORT("Invalid range argument. Use -pxrange <range> with a positive real number.");
            rangeMode = RANGE_PX;
            pxRange = r;
            argPos += 2;
            continue;
        }
        ARG_CASE("-scale", 1) {
            double s;
            if (!parseDouble(s, argv[argPos+1]) || s <= 0)
                ABORT("Invalid scale argument. Use -scale <scale> with a positive real number.");
            scale = s;
            scaleSpecified = true;
            argPos += 2;
            continue;
        }
        ARG_CASE("-ascale", 2) {
            double sx, sy;
            if (!parseDouble(sx, argv[argPos+1]) || !parseDouble(sy, argv[argPos+2]) || sx <= 0 || sy <= 0)
                ABORT("Invalid scale arguments. Use -ascale <x> <y> with two positive real numbers.");
            scale.set(sx, sy);
            scaleSpecified = true;
            argPos += 3;
            continue;
        }
        ARG_CASE("-translate", 2) {
            double tx, ty;
            if (!parseDouble(tx, argv[argPos+1]) || !parseDouble(ty, argv[argPos+2]))
                ABORT("Invalid translate arguments. Use -translate <x> <y> with two real numbers.");
            translate.set(tx, ty);
            argPos += 3;
            continue;
        }
        ARG_CASE("-angle", 1) {
            double at;
            if (!parseAngle(at, argv[argPos+1]))
                ABORT("Invalid angle threshold. Use -angle <min angle> with a positive real number less than PI or a value in degrees followed by 'd' below 180d.");
            angleThreshold = at;
            argPos += 2;
            continue;
        }
        ARG_CASE("-errorcorrection", 1) {
            double et;
            if (!parseDouble(et, argv[argPos+1]) || et < 0)
                ABORT("Invalid error correction threshold. Use -errorcorrection <threshold> with a real number larger or equal to 1.");
            edgeThreshold = et;
            argPos += 2;
            continue;
        }
        ARG_CASE("-edgecolors", 1) {
            static const char *allowed = " ?,cmyCMY";
            for (int i = 0; argv[argPos+1][i]; ++i) {
                for (int j = 0; allowed[j]; ++j)
                    if (argv[argPos+1][i] == allowed[j])
                        goto ROLL_ARG;
                ABORT("Invalid edge coloring sequence. Use -assign <color sequence> with only the colors C, M, and Y. Separate contours by commas and use ? to keep the default assigment for a contour.");
            ROLL_ARG:;
            }
            edgeAssignment = argv[argPos+1];
            argPos += 2;
            continue;
        }
        ARG_CASE("-testrender", 3) {
            unsigned w, h;
            if (!parseUnsigned(w, argv[argPos+2]) || !parseUnsigned(h, argv[argPos+3]) || !w || !h)
                ABORT("Invalid arguments for test render. Use -testrender <output.png> <width> <height>.");
            testRender = argv[argPos+1];
            testWidth = w, testHeight = h;
            argPos += 4;
            continue;
        }
        ARG_CASE("-testrendermulti", 3) {
            unsigned w, h;
            if (!parseUnsigned(w, argv[argPos+2]) || !parseUnsigned(h, argv[argPos+3]) || !w || !h)
                ABORT("Invalid arguments for test render. Use -testrendermulti <output.png> <width> <height>.");
            testRenderMulti = argv[argPos+1];
            testWidthM = w, testHeightM = h;
            argPos += 4;
            continue;
        }
        ARG_CASE("-yflip", 0) {
            yFlip = true;
            argPos += 1;
            continue;
        }
        ARG_CASE("-printmetrics", 0) {
            printMetrics = true;
            argPos += 1;
            continue;
        }
        ARG_CASE("-keeporder", 0) {
            orientation = KEEP;
            argPos += 1;
            continue;
        }
        ARG_CASE("-reverseorder", 0) {
            orientation = REVERSE;
            argPos += 1;
            continue;
        }
        ARG_CASE("-guessorder", 0) {
            orientation = GUESS;
            argPos += 1;
            continue;
        }
        ARG_CASE("-seed", 1) {
            if (!parseUnsignedLL(coloringSeed, argv[argPos+1]))
                ABORT("Invalid seed. Use -seed <N> with N being a non-negative integer.");
            argPos += 2;
            continue;
        }
        ARG_CASE("-help", 0)
            ABORT(helpText);
        printf("Unknown setting or insufficient parameters: %s\n", arg);
        suggestHelp = true;
        ++argPos;
    }
    if (suggestHelp)
        printf("Use -help for more information.\n");

    // Load input
    Vector2 svgDims;
    double glyphAdvance = 0;
    if (!inputType || !input)
        ABORT("No input specified! Use either -svg <file.svg> or -font <file.ttf/otf> <character code>, or see -help.");
    Shape shape;
    switch (inputType) {
        case SVG: {
            if (!loadSvgShape(shape, input, svgPathIndex, &svgDims))
                ABORT("Failed to load shape from SVG file.");
            break;
        }
        case FONT: {
            if (!unicode)
                ABORT("No character specified! Use -font <file.ttf/otf> <character code>. Character code can be a number (65, 0x41), or a character in apostrophes ('A').");
            FreetypeHandle *ft = initializeFreetype();
            if (!ft) return -1;
            FontHandle *font = loadFont(ft, input);
            if (!font) {
                deinitializeFreetype(ft);
                ABORT("Failed to load font file.");
            }
            if (!loadGlyph(shape, font, unicode, &glyphAdvance)) {
                destroyFont(font);
                deinitializeFreetype(ft);
                ABORT("Failed to load glyph from font file.");
            }
            destroyFont(font);
            deinitializeFreetype(ft);
            break;
        }
        default:
            break;
    }

    // Validate and normalize shape
    if (!shape.validate())
        ABORT("The geometry of the loaded shape is invalid.");
    shape.normalize();
    if (!yFlip)
        shape.inverseYAxis = !shape.inverseYAxis;

    double avgScale = .5*(scale.x+scale.y);
    struct {
        double l, b, r, t;
    } bounds = {
        LARGE_VALUE, LARGE_VALUE, -LARGE_VALUE, -LARGE_VALUE
    };
    if (autoFrame || mode == METRICS || printMetrics || orientation == GUESS)
        shape.bounds(bounds.l, bounds.b, bounds.r, bounds.t);

    // Auto-frame
    if (autoFrame) {
        double l = bounds.l, b = bounds.b, r = bounds.r, t = bounds.t;
        Vector2 frame(width, height);
        if (rangeMode == RANGE_UNIT)
            l -= range, b -= range, r += range, t += range;
        else if (!scaleSpecified)
            frame -= 2*pxRange;
        if (l >= r || b >= t)
            l = 0, b = 0, r = 1, t = 1;
        if (frame.x <= 0 || frame.y <= 0)
            ABORT("Cannot fit the specified pixel range.");
        Vector2 dims(r-l, t-b);
        if (scaleSpecified)
            translate = .5*(frame/scale-dims)-Vector2(l, b);
        else {
            if (dims.x*frame.y < dims.y*frame.x) {
                translate.set(.5*(frame.x/frame.y*dims.y-dims.x)-l, -b);
                scale = avgScale = frame.y/dims.y;
            } else {
                translate.set(-l, .5*(frame.y/frame.x*dims.x-dims.y)-b);
                scale = avgScale = frame.x/dims.x;
            }
        }
        if (rangeMode == RANGE_PX && !scaleSpecified)
            translate += pxRange/scale;
    }

    if (rangeMode == RANGE_PX)
        range = pxRange/min(scale.x, scale.y);

    if (outputOffsets) {
        std::string extensionless = outputName;
        size_t extPos = outputName.find_first_of('.');
        if (extPos != std::string::npos)
        {
            extensionless = outputName.substr(0, extPos);
        }
        FILE *out = fopen((outputPath + extensionless + ".txt").c_str(), "wb");
        if (!out)
            ABORT("Failed to open output file for symbol offsets");
        if (scale.x != scale.y)
            ABORT("Unexpected difference between scale.x and scale.y");
        float scaleX = scale.x;
        fwrite(&scaleX, sizeof(float), 1, out);
        float translateX = translate.x;
        fwrite(&translateX, sizeof(float), 1, out);
        float translateY = translate.y;
        fwrite(&translateY, sizeof(float), 1, out);
        int pxRangeInteger = lroundl(pxRange);
        fwrite(&pxRangeInteger, sizeof(int), 1, out);
        fclose(out);
    }

    // Print metrics
    if (mode == METRICS || printMetrics) {
        FILE *out = stdout;
        if (mode == METRICS && outputSpecified)
            out = fopen(outputName.c_str(), "w");
        if (!out)
            ABORT("Failed to write output file.");
        if (shape.inverseYAxis)
            fprintf(out, "inverseY = true\n");
        if (bounds.r >= bounds.l && bounds.t >= bounds.b)
            fprintf(out, "bounds = %.12g, %.12g, %.12g, %.12g\n", bounds.l, bounds.b, bounds.r, bounds.t);
        if (svgDims.x != 0 && svgDims.y != 0)
            fprintf(out, "dimensions = %.12g, %.12g\n", svgDims.x, svgDims.y);
        if (glyphAdvance != 0)
            fprintf(out, "advance = %.12g\n", glyphAdvance);
        if (autoFrame) {
            if (!scaleSpecified)
                fprintf(out, "scale = %.12g\n", avgScale);
            fprintf(out, "translate = %.12g, %.12g\n", translate.x, translate.y);
        }
        if (rangeMode == RANGE_PX)
            fprintf(out, "range = %.12g\n", range);
        if (mode == METRICS && outputSpecified)
            fclose(out);
    }

    // Compute output
    Bitmap<uint8_t> sdf;
    Bitmap<FloatRGB> msdf;
    switch (mode) {
        case SINGLE: {
            sdf = Bitmap<uint8_t>(width, height);
            generateSDF(sdf, shape, bounds.l, range, scale, translate);
            break;
        }
        //case PSEUDO:
        //    generatePseudoSDF(sdf, shape, range, scale, translate);
        //    break;
        //}
        //case MULTI: {
        //    if (!skipColoring)
        //        edgeColoringSimple(shape, angleThreshold, coloringSeed);
        //    if (edgeAssignment)
        //        parseColoring(shape, edgeAssignment);
        //    msdf = Bitmap<FloatRGB>(width, height);
        //    generateMSDF(msdf, shape, range, scale, translate, edgeThreshold);
        //    break;
        //}
        default:
            break;
    }

    // This guess doesn't work when using fill rules, so skip it.
    if (orientation == GUESS) {
        // Get sign of signed distance outside bounds
        Point2 p(bounds.l-(bounds.r-bounds.l)-1, bounds.b-(bounds.t-bounds.b)-1);
		float minDistance = DBL_MAX;
        for (std::vector<Contour>::const_iterator contour = shape.contours.begin(); contour != shape.contours.end(); ++contour)
            for (std::vector<EdgeSegment>::const_iterator edge = contour->edges.begin(); edge != contour->edges.end(); ++edge) {
                float distance = edge->signedDistance(p);
                if (distance < minDistance)
                    minDistance = distance;
            }
        orientation = minDistance <= 0 ? KEEP : REVERSE;
    }
    if (orientation == REVERSE) {
        invertColor(sdf);
        invertColor(msdf);
    }

    const char *error = NULL;
    switch (mode) {
        case SINGLE:
        case PSEUDO:
            error = writeOutput(sdf, outputPath + outputName, format);
            if (error)
                ABORT(error);
            //if (testRenderMulti || testRender)
            //    simulate8bit(sdf);
            //if (testRenderMulti) {
            //    Bitmap<FloatRGB> render(testWidthM, testHeightM);
            //    renderSDF(render, sdf, avgScale*range);
            //    if (!savePng(render, testRenderMulti))
            //        puts("Failed to write test render file.");
            //}
        //    if (testRender) {
        //        Bitmap<float> render(testWidth, testHeight);
        //        renderSDF(render, sdf, avgScale*range);
        //        if (!savePng(render, testRender))
        //            puts("Failed to write test render file.");
        //    }
            break;
        //case MULTI:
        //    error = writeOutput(msdf, outputPath + outputName, format);
        //    if (error)
        //        ABORT(error);
        //    if (testRenderMulti || testRender)
        //        simulate8bit(msdf);
        //    if (testRenderMulti) {
        //        Bitmap<FloatRGB> render(testWidthM, testHeightM);
        //        renderSDF(render, msdf, avgScale*range);
        //        if (!savePng(render, testRenderMulti))
        //            puts("Failed to write test render file.");
        //    }
        //    if (testRender) {
        //        Bitmap<float> render(testWidth, testHeight);
        //        renderSDF(render, msdf, avgScale*range);
        //        if (!savePng(render, testRender))
        //            ABORT("Failed to write test render file.");
        //    }
        //    break;
        default:
            break;
    }

    return 0;
}

#endif
