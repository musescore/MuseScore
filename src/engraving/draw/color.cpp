#include "color.h"

static const struct RGBData {
    const char name[21];
    uint  value;
} rgbTbl[] = {
    { "aliceblue", mu::draw::rgb(240, 248, 255) },
    { "antiquewhite", mu::draw::rgb(250, 235, 215) },
    { "aqua", mu::draw::rgb( 0, 255, 255) },
    { "aquamarine", mu::draw::rgb(127, 255, 212) },
    { "azure", mu::draw::rgb(240, 255, 255) },
    { "beige", mu::draw::rgb(245, 245, 220) },
    { "bisque", mu::draw::rgb(255, 228, 196) },
    { "black", mu::draw::rgb( 0, 0, 0) },
    { "blanchedalmond", mu::draw::rgb(255, 235, 205) },
    { "blue", mu::draw::rgb( 0, 0, 255) },
    { "blueviolet", mu::draw::rgb(138, 43, 226) },
    { "brown", mu::draw::rgb(165, 42, 42) },
    { "burlywood", mu::draw::rgb(222, 184, 135) },
    { "cadetblue", mu::draw::rgb( 95, 158, 160) },
    { "chartreuse", mu::draw::rgb(127, 255, 0) },
    { "chocolate", mu::draw::rgb(210, 105, 30) },
    { "coral", mu::draw::rgb(255, 127, 80) },
    { "cornflowerblue", mu::draw::rgb(100, 149, 237) },
    { "cornsilk", mu::draw::rgb(255, 248, 220) },
    { "crimson", mu::draw::rgb(220, 20, 60) },
    { "cyan", mu::draw::rgb( 0, 255, 255) },
    { "darkblue", mu::draw::rgb( 0, 0, 139) },
    { "darkcyan", mu::draw::rgb( 0, 139, 139) },
    { "darkgoldenrod", mu::draw::rgb(184, 134, 11) },
    { "darkgray", mu::draw::rgb(169, 169, 169) },
    { "darkgreen", mu::draw::rgb( 0, 100, 0) },
    { "darkgrey", mu::draw::rgb(169, 169, 169) },
    { "darkkhaki", mu::draw::rgb(189, 183, 107) },
    { "darkmagenta", mu::draw::rgb(139, 0, 139) },
    { "darkolivegreen", mu::draw::rgb( 85, 107, 47) },
    { "darkorange", mu::draw::rgb(255, 140, 0) },
    { "darkorchid", mu::draw::rgb(153, 50, 204) },
    { "darkred", mu::draw::rgb(139, 0, 0) },
    { "darksalmon", mu::draw::rgb(233, 150, 122) },
    { "darkseagreen", mu::draw::rgb(143, 188, 143) },
    { "darkslateblue", mu::draw::rgb( 72, 61, 139) },
    { "darkslategray", mu::draw::rgb( 47, 79, 79) },
    { "darkslategrey", mu::draw::rgb( 47, 79, 79) },
    { "darkturquoise", mu::draw::rgb( 0, 206, 209) },
    { "darkviolet", mu::draw::rgb(148, 0, 211) },
    { "deeppink", mu::draw::rgb(255, 20, 147) },
    { "deepskyblue", mu::draw::rgb( 0, 191, 255) },
    { "dimgray", mu::draw::rgb(105, 105, 105) },
    { "dimgrey", mu::draw::rgb(105, 105, 105) },
    { "dodgerblue", mu::draw::rgb( 30, 144, 255) },
    { "firebrick", mu::draw::rgb(178, 34, 34) },
    { "floralwhite", mu::draw::rgb(255, 250, 240) },
    { "forestgreen", mu::draw::rgb( 34, 139, 34) },
    { "fuchsia", mu::draw::rgb(255, 0, 255) },
    { "gainsboro", mu::draw::rgb(220, 220, 220) },
    { "ghostwhite", mu::draw::rgb(248, 248, 255) },
    { "gold", mu::draw::rgb(255, 215, 0) },
    { "goldenrod", mu::draw::rgb(218, 165, 32) },
    { "gray", mu::draw::rgb(128, 128, 128) },
    { "green", mu::draw::rgb( 0, 128, 0) },
    { "greenyellow", mu::draw::rgb(173, 255, 47) },
    { "grey", mu::draw::rgb(128, 128, 128) },
    { "honeydew", mu::draw::rgb(240, 255, 240) },
    { "hotpink", mu::draw::rgb(255, 105, 180) },
    { "indianred", mu::draw::rgb(205, 92, 92) },
    { "indigo", mu::draw::rgb( 75, 0, 130) },
    { "ivory", mu::draw::rgb(255, 255, 240) },
    { "khaki", mu::draw::rgb(240, 230, 140) },
    { "lavender", mu::draw::rgb(230, 230, 250) },
    { "lavenderblush", mu::draw::rgb(255, 240, 245) },
    { "lawngreen", mu::draw::rgb(124, 252, 0) },
    { "lemonchiffon", mu::draw::rgb(255, 250, 205) },
    { "lightblue", mu::draw::rgb(173, 216, 230) },
    { "lightcoral", mu::draw::rgb(240, 128, 128) },
    { "lightcyan", mu::draw::rgb(224, 255, 255) },
    { "lightgoldenrodyellow", mu::draw::rgb(250, 250, 210) },
    { "lightgray", mu::draw::rgb(211, 211, 211) },
    { "lightgreen", mu::draw::rgb(144, 238, 144) },
    { "lightgrey", mu::draw::rgb(211, 211, 211) },
    { "lightpink", mu::draw::rgb(255, 182, 193) },
    { "lightsalmon", mu::draw::rgb(255, 160, 122) },
    { "lightseagreen", mu::draw::rgb( 32, 178, 170) },
    { "lightskyblue", mu::draw::rgb(135, 206, 250) },
    { "lightslategray", mu::draw::rgb(119, 136, 153) },
    { "lightslategrey", mu::draw::rgb(119, 136, 153) },
    { "lightsteelblue", mu::draw::rgb(176, 196, 222) },
    { "lightyellow", mu::draw::rgb(255, 255, 224) },
    { "lime", mu::draw::rgb( 0, 255, 0) },
    { "limegreen", mu::draw::rgb( 50, 205, 50) },
    { "linen", mu::draw::rgb(250, 240, 230) },
    { "magenta", mu::draw::rgb(255, 0, 255) },
    { "maroon", mu::draw::rgb(128, 0, 0) },
    { "mediumaquamarine", mu::draw::rgb(102, 205, 170) },
    { "mediumblue", mu::draw::rgb( 0, 0, 205) },
    { "mediumorchid", mu::draw::rgb(186, 85, 211) },
    { "mediumpurple", mu::draw::rgb(147, 112, 219) },
    { "mediumseagreen", mu::draw::rgb( 60, 179, 113) },
    { "mediumslateblue", mu::draw::rgb(123, 104, 238) },
    { "mediumspringgreen", mu::draw::rgb( 0, 250, 154) },
    { "mediumturquoise", mu::draw::rgb( 72, 209, 204) },
    { "mediumvioletred", mu::draw::rgb(199, 21, 133) },
    { "midnightblue", mu::draw::rgb( 25, 25, 112) },
    { "mintcream", mu::draw::rgb(245, 255, 250) },
    { "mistyrose", mu::draw::rgb(255, 228, 225) },
    { "moccasin", mu::draw::rgb(255, 228, 181) },
    { "navajowhite", mu::draw::rgb(255, 222, 173) },
    { "navy", mu::draw::rgb( 0, 0, 128) },
    { "oldlace", mu::draw::rgb(253, 245, 230) },
    { "olive", mu::draw::rgb(128, 128, 0) },
    { "olivedrab", mu::draw::rgb(107, 142, 35) },
    { "orange", mu::draw::rgb(255, 165, 0) },
    { "orangered", mu::draw::rgb(255, 69, 0) },
    { "orchid", mu::draw::rgb(218, 112, 214) },
    { "palegoldenrod", mu::draw::rgb(238, 232, 170) },
    { "palegreen", mu::draw::rgb(152, 251, 152) },
    { "paleturquoise", mu::draw::rgb(175, 238, 238) },
    { "palevioletred", mu::draw::rgb(219, 112, 147) },
    { "papayawhip", mu::draw::rgb(255, 239, 213) },
    { "peachpuff", mu::draw::rgb(255, 218, 185) },
    { "peru", mu::draw::rgb(205, 133, 63) },
    { "pink", mu::draw::rgb(255, 192, 203) },
    { "plum", mu::draw::rgb(221, 160, 221) },
    { "powderblue", mu::draw::rgb(176, 224, 230) },
    { "purple", mu::draw::rgb(128, 0, 128) },
    { "red", mu::draw::rgb(255, 0, 0) },
    { "rosybrown", mu::draw::rgb(188, 143, 143) },
    { "royalblue", mu::draw::rgb( 65, 105, 225) },
    { "saddlebrown", mu::draw::rgb(139, 69, 19) },
    { "salmon", mu::draw::rgb(250, 128, 114) },
    { "sandybrown", mu::draw::rgb(244, 164, 96) },
    { "seagreen", mu::draw::rgb( 46, 139, 87) },
    { "seashell", mu::draw::rgb(255, 245, 238) },
    { "sienna", mu::draw::rgb(160, 82, 45) },
    { "silver", mu::draw::rgb(192, 192, 192) },
    { "skyblue", mu::draw::rgb(135, 206, 235) },
    { "slateblue", mu::draw::rgb(106, 90, 205) },
    { "slategray", mu::draw::rgb(112, 128, 144) },
    { "slategrey", mu::draw::rgb(112, 128, 144) },
    { "snow", mu::draw::rgb(255, 250, 250) },
    { "springgreen", mu::draw::rgb( 0, 255, 127) },
    { "steelblue", mu::draw::rgb( 70, 130, 180) },
    { "tan", mu::draw::rgb(210, 180, 140) },
    { "teal", mu::draw::rgb( 0, 128, 128) },
    { "thistle", mu::draw::rgb(216, 191, 216) },
    { "tomato", mu::draw::rgb(255, 99, 71) },
    { "transparent", 0 },
    { "turquoise", mu::draw::rgb( 64, 224, 208) },
    { "violet", mu::draw::rgb(238, 130, 238) },
    { "wheat", mu::draw::rgb(245, 222, 179) },
    { "white", mu::draw::rgb(255, 255, 255) },
    { "whitesmoke", mu::draw::rgb(245, 245, 245) },
    { "yellow", mu::draw::rgb(255, 255, 0) },
    { "yellowgreen", mu::draw::rgb(154, 205, 50) }
};

static const int rgbTblSize = sizeof(rgbTbl) / sizeof(RGBData);

constexpr inline int fromHex(uint c) noexcept {
    return ((c >= '0') && (c <= '9')) ? int(c - '0') :
        ((c >= 'A') && (c <= 'F')) ? int(c - 'A' + 10) :
        ((c >= 'a') && (c <= 'f')) ? int(c - 'a' + 10) :
        -1;
}

static inline int hex2int(const char *s, int n) {
    if (n < 0)
        return -1;
    int result = 0;
    for (; n > 0; --n) {
        result = result * 16;
        const int h = fromHex(*s++);
        if (h < 0)
            return -1;
        result += h;
    }
    return result;
}

static bool get_hex_rgb(const char *name, size_t len, mu::draw::Rgb *rgb) {
    if (name[0] != '#')
        return false;
    name++;
    --len;
    int a, r, g, b;
    a = 65535;
    if (len == 12) {
        r = hex2int(name + 0, 4);
        g = hex2int(name + 4, 4);
        b = hex2int(name + 8, 4);
    } else if (len == 9) {
        r = hex2int(name + 0, 3);
        g = hex2int(name + 3, 3);
        b = hex2int(name + 6, 3);
        if (r == -1 || g == -1 || b == -1)
            return false;
        r = (r << 4) | (r >> 8);
        g = (g << 4) | (g >> 8);
        b = (b << 4) | (b >> 8);
    } else if (len == 8) {
        a = hex2int(name + 0, 2) * 0x101;
        r = hex2int(name + 2, 2) * 0x101;
        g = hex2int(name + 4, 2) * 0x101;
        b = hex2int(name + 6, 2) * 0x101;
    } else if (len == 6) {
        r = hex2int(name + 0, 2) * 0x101;
        g = hex2int(name + 2, 2) * 0x101;
        b = hex2int(name + 4, 2) * 0x101;
    } else if (len == 3) {
        r = hex2int(name + 0, 1) * 0x1111;
        g = hex2int(name + 1, 1) * 0x1111;
        b = hex2int(name + 2, 1) * 0x1111;
    } else {
        r = g = b = -1;
    }
    if ((uint)r > 65535 || (uint)g > 65535 || (uint)b > 65535 || (uint)a > 65535) {
        *rgb = 0;
        return false;
    }
    *rgb = mu::draw::rgba(r, g ,b, a);
    return true;
}

inline bool operator<(const char *name, const RGBData &data)
{ return strcmp(name, data.name) < 0; }
inline bool operator<(const RGBData &data, const char *name)
{ return strcmp(data.name, name) < 0; }

static bool get_named_rgb_no_space(const char *name_no_space, mu::draw::Rgb *rgb) {
    const RGBData *r = std::lower_bound(rgbTbl, rgbTbl + rgbTblSize, name_no_space);
    if ((r != rgbTbl + rgbTblSize) && !(name_no_space < *r)) {
        *rgb = r->value;
        return true;
    }
    return false;
}

static bool get_named_rgb(const char *name, int len, mu::draw::Rgb* rgb) {
    if (len > 255)
        return false;
    char name_no_space[256];
    int pos = 0;
    for (int i = 0; i < len; i++) {
        if (name[i] != '\t' && name[i] != ' ')
            name_no_space[pos++] = tolower(name[i]);
    }
    name_no_space[pos] = 0;

    return get_named_rgb_no_space(name_no_space, rgb);
}

mu::draw::Color::Color(const Color &other)
    : _rgb(other._rgb) {

}

mu::draw::Color::Color(int red, int green, int blue, int alpha /* = 255*/)
    : _rgb(rgba(red, green, blue, alpha)) {
}

mu::draw::Color::Color(GlobalColor color) {
#define RGB(r, g, b) \
    Rgb(((0xffu << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff)))
#define RGBA(r, g, b, a) \
    Rgb(((a & 0xff) << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff))

    static const Rgb global_colors[] = {
        RGB(255, 255, 255), // Qt::color0
        RGB(  0,   0,   0), // Qt::color1
        RGB(  0,   0,   0), // black
        RGB(255, 255, 255), // white
        RGB(128, 128, 128), // index 248   medium gray
        RGB(160, 160, 164), // index 247   light gray
        RGB(192, 192, 192), // index 7     light gray
        RGB(255,   0,   0), // index 249   red
        RGB(  0, 255,   0), // index 250   green
        RGB(  0,   0, 255), // index 252   blue
        RGB(  0, 255, 255), // index 254   cyan
        RGB(255,   0, 255), // index 253   magenta
        RGB(255, 255,   0), // index 251   yellow
        RGB(128,   0,   0), // index 1     dark red
        RGB(  0, 128,   0), // index 2     dark green
        RGB(  0,   0, 128), // index 4     dark blue
        RGB(  0, 128, 128), // index 6     dark cyan
        RGB(128,   0, 128), // index 5     dark magenta
        RGB(128, 128,   0), // index 3     dark yellow
        RGBA(0, 0, 0, 0)    //             transparent
    };
#undef RGB
#undef RGBA

    setRgb(getRed(global_colors[color]),
           getGreen(global_colors[color]),
           getBlue(global_colors[color]),
           getAlpha(global_colors[color]));
}

mu::draw::Color::Color(const char* color) {
    setNamedColor(color);
}

#ifndef NO_QT_SUPPORT
mu::draw::Color::Color(const QColor &color)
    : _rgb(color.rgba()) {
}

mu::draw::Color::Color(Qt::GlobalColor color)
    : Color(QColor(color)) {

}
#endif

mu::draw::Color &mu::draw::Color::operator=(const Color &other) {
    _rgb = other._rgb;
    return *this;
}

mu::draw::Color &mu::draw::Color::operator=(mu::draw::GlobalColor color) {
    *this = Color(color);
    return *this;
}

#ifndef NO_QT_SUPPORT
mu::draw::Color &mu::draw::Color::operator=(const QColor& other) {
    *this = fromQColor(other);
    return *this;
}

mu::draw::Color &mu::draw::Color::operator=(Qt::GlobalColor color) {
    *this = Color::fromQColor(color);
    return *this;
}
#endif

bool mu::draw::Color::operator==(const Color &other) const noexcept {
    return _rgb == other._rgb;
}


bool mu::draw::Color::operator!=(const Color &other) const noexcept {
    return _rgb != other._rgb;
}

bool mu::draw::Color::operator==(const GlobalColor &color) const noexcept {
    return *this == Color(color);
}

bool mu::draw::Color::operator!=(const GlobalColor &color) const noexcept {
    return *this != Color(color);
}

bool mu::draw::Color::isValid() const noexcept {
    return _isValid;
}

int mu::draw::Color::red() const noexcept {
    return getRed(_rgb);
}

int mu::draw::Color::green() const noexcept {
    return getGreen(_rgb);
}

int mu::draw::Color::blue() const noexcept {
    return getBlue(_rgb);
}

int mu::draw::Color::alpha() const noexcept {
    return getAlpha(_rgb);
}

std::string mu::draw::Color::name() const noexcept {
    return _name;
}

mu::draw::Color mu::draw::Color::lighter(int factor) const noexcept {
    // TODO
    return Color(*this);
}

#ifndef NO_QT_SUPPORT
mu::draw::Color mu::draw::Color::fromQColor(const QColor &color) {
    Color c;
    c._rgb = color.rgba();
    return c;
}

QColor mu::draw::Color::toQColor() const {
    return QColor(_rgb);
}
#endif

void mu::draw::Color::setNamedColor(const std::string& color) noexcept {
    if (color.size() == 0) {
        _isValid = false;
        return;
    }

    Rgb rgb;
    if (color[0] == '#') {
        if (get_hex_rgb(color.data(), color.size(), &rgb)) {
            setRgb(rgb);
        } else {
            _isValid = false;
        }

        return;
    }

    if (get_named_rgb(color.data(), color.size(), &rgb)) {
        setRgb(rgb);
    }
    else {
        _isValid = false;
    }
}

void mu::draw::Color::setNamedColor(const char* color) noexcept {
    setNamedColor(std::string(color));
}

void mu::draw::Color::setRed(int value) noexcept {
    setRgb(value, green(), blue(), alpha());
}

void mu::draw::Color::setGreen(int value) noexcept {
    setRgb(red(), value, blue(), alpha());
}

void mu::draw::Color::setBlue(int value) noexcept {
    setRgb(red(), green(), value, alpha());
}

void mu::draw::Color::setAlpha(int value) noexcept {
    setRgb(red(), green(), blue(), value);
}

void mu::draw::Color::setRgb(int r, int g, int b, int a)
{
    if (!isRgbaValid(r, g, b, a)) {
        _isValid = false;
        return;
    }

    _rgb = rgba(r, g, b, a);
}

void mu::draw::Color::setRgb(Rgb rgb)
{
    if (!isRgbaValid(getRed(rgb), getGreen(rgb), getBlue(rgb), getAlpha(rgb))) {
        _isValid = false;
        return;
    }

    _rgb = rgb;
}
