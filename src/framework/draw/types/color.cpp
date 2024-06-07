#include "color.h"

#include <sstream>
#include <iomanip>

namespace muse::draw {
static constexpr int fromHex(char c);
static int hex2int(const char* s, int n);
static bool getHexRgb(const char* name, size_t len, Rgba* rgba);
static std::string rgb2hex(int r, int g, int b, int a = Color::DEFAULT_ALPHA);
static std::string rgb2hex(Rgba rgba);
static void insertHexComponent(int num, std::stringstream& ss);

Color::Color()
{
    m_isValid = false;
}

Color::Color(const Color& other)
    : m_rgba(other.m_rgba), m_isValid(other.m_isValid)
{
}

Color::Color(const char* color)
{
    setNamedColor(color);
}

#ifndef NO_QT_SUPPORT
Color::Color(const QColor& color)
    : m_rgba(color.rgba()), m_isValid(color.isValid())
{
}

#endif

Color& Color::operator=(const Color& other)
{
    m_rgba = other.m_rgba;
    m_isValid = other.m_isValid;
    return *this;
}

#ifndef NO_QT_SUPPORT
Color& Color::operator=(const QColor& other)
{
    *this = fromQColor(other);
    return *this;
}

#endif

bool Color::operator==(const Color& other) const
{
    return m_rgba == other.m_rgba && m_isValid == other.m_isValid;
}

bool Color::operator!=(const Color& other) const
{
    return !operator==(other);
}

bool Color::operator<(const Color& other) const
{
    return m_rgba < other.m_rgba;
}

std::string Color::toString() const
{
    return rgb2hex(m_rgba);
}

Color Color::fromString(const std::string& str)
{
    Color c;
    c.setNamedColor(str);
    return c;
}

Color Color::fromString(const char* str)
{
    Color c;
    c.setNamedColor(str);
    return c;
}

Color Color::fromString(const String& str)
{
    return fromString(str.toStdString());
}

bool Color::isValid() const
{
    return m_isValid;
}

int Color::red() const
{
    return getRed(m_rgba);
}

int Color::green() const
{
    return getGreen(m_rgba);
}

int Color::blue() const
{
    return getBlue(m_rgba);
}

int Color::alpha() const
{
    return getAlpha(m_rgba);
}

#ifndef NO_QT_SUPPORT
Color Color::fromQColor(const QColor& color)
{
    Color c;
    c.m_rgba = color.rgba();
    c.m_isValid = color.isValid();
    return c;
}

QColor Color::toQColor() const
{
    return QColor::fromRgba(m_rgba);
}

#endif

void Color::setNamedColor(const std::string& color)
{
    if (color.empty()) {
        m_isValid = false;
        return;
    }

    Rgba rgba;
    if (color[0] == '#') {
        if (getHexRgb(color.data(), color.size(), &rgba)) {
            setRgba(rgba);
            m_isValid = true;
            return;
        }
    }

    m_isValid = false;
}

void Color::setNamedColor(const char* color)
{
    if (!color) {
        return;
    }
    setNamedColor(std::string(color));
}

void Color::setRed(int value)
{
    setRgba(value, green(), blue(), alpha());
}

void Color::setGreen(int value)
{
    setRgba(red(), value, blue(), alpha());
}

void Color::setBlue(int value)
{
    setRgba(red(), green(), value, alpha());
}

void Color::setAlpha(int value)
{
    setRgba(red(), green(), blue(), value);
}

void Color::applyTint(double tint)
{
    setRgba(red() + (255 - red()) * tint, green() + (255 - green()) * tint, blue() + (255 - blue()) * tint, alpha());
}

void Color::setRgba(int r, int g, int b, int a)
{
    if (!isRgbaValid(r, g, b, a)) {
        m_isValid = false;
        return;
    }

    m_rgba = rgba(r, g, b, a);
    m_isValid = true;
}

void Color::setRgba(Rgba rgba)
{
    if (!isRgbaValid(getRed(rgba), getGreen(rgba), getBlue(rgba), getAlpha(rgba))) {
        m_isValid = false;
        return;
    }

    m_rgba = rgba;
    m_isValid = true;
}

Color Color::inverted() const
{
    int m = std::min(red() < green() ? red() : green(), blue());
    int M = std::max(red() > green() ? red() : green(), blue());
    int x = 255 - m - M;
    return Color(red() + x, green() + x, blue() + x, alpha());
}

static constexpr int fromHex(char c)
{
    return ((c >= '0') && (c <= '9')) ? int(c - '0')
           : ((c >= 'A') && (c <= 'F')) ? int(c - 'A' + 10)
           : ((c >= 'a') && (c <= 'f')) ? int(c - 'a' + 10)
           : -1;
}

static int hex2int(const char* s, int n)
{
    if (n < 0) {
        return -1;
    }
    int result = 0;
    for (; n > 0; --n) {
        result = result * 16;
        const int h = fromHex(*s++);
        if (h < 0) {
            return -1;
        }
        result += h;
    }
    return result;
}

static bool getHexRgb(const char* name, size_t len, Rgba* rgba)
{
    if (name[0] != '#') {
        return false;
    }
    name++;
    --len;
    int a, r, g, b;
    a = 255;
    if (len == 8) {
        a = hex2int(name + 0, 2);
        r = hex2int(name + 2, 2);
        g = hex2int(name + 4, 2);
        b = hex2int(name + 6, 2);
    } else if (len == 6) {
        r = hex2int(name + 0, 2);
        g = hex2int(name + 2, 2);
        b = hex2int(name + 4, 2);
    } else if (len == 3) {
        r = hex2int(name + 0, 1) * 0x11;
        g = hex2int(name + 1, 1) * 0x11;
        b = hex2int(name + 2, 1) * 0x11;
    } else {
        r = g = b = -1;
    }

    if (!isRgbaValid(r, g, b, a)) {
        *rgba = 0;
        return false;
    }

    *rgba = muse::draw::rgba(r, g, b, a);
    return true;
}

static void insertHexComponent(int num, std::stringstream& ss)
{
    ss << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << num;
}

static std::string rgb2hex(int r, int g, int b, int a)
{
    std::stringstream ss;

    ss << "#";
    insertHexComponent(r, ss);
    insertHexComponent(g, ss);
    insertHexComponent(b, ss);
    if (a != Color::DEFAULT_ALPHA) {
        insertHexComponent(a, ss);
    }
    return ss.str();
}

static std::string rgb2hex(Rgba rgba)
{
    return rgb2hex(getRed(rgba), getGreen(rgba), getBlue(rgba), getAlpha(rgba));
}
}
