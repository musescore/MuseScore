/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef MU_GLOBAL_STRING_H
#define MU_GLOBAL_STRING_H

#include <memory>
#include <cstring>
#include <vector>
#include <string>
#include <string_view>

#include "containers.h"
#include "bytearray.h"
#include "global/logstream.h"

namespace mu {
enum CaseSensitivity {
    CaseInsensitive = 0,
    CaseSensitive = 1
};

enum SplitBehavior {
    KeepEmptyParts = 0,
    SkipEmptyParts = 1,
};

// ============================
// AsciiChar (ASCII)
// ============================
struct AsciiChar
{
public:
    AsciiChar() = default;
    explicit AsciiChar(char c)
        : m_ch(c) {}

    inline char ascii() const noexcept { return m_ch; }
    inline char16_t unicode() const noexcept { return char16_t(m_ch); }
    inline char toLower() const { return toLower(m_ch); }
    inline char toUpper() const { return toUpper(m_ch); }

    static char toLower(char ch);
    static char toUpper(char ch);

private:
    char m_ch = 0;
};

// ============================
// Char (UTF-16)
// ============================
class Char
{
public:

    Char() = default;
    Char(char16_t c)
        : m_ch(c) {}
    Char(AsciiChar c)
        : m_ch(c.unicode()) {}

    Char(QChar c)
        : m_ch(c.unicode()) {}
    operator QChar() const {
        return QChar(m_ch);
    }

    inline bool operator ==(Char c) const { return m_ch == c.m_ch; }
    inline bool operator !=(Char c) const { return !operator ==(c); }
    inline bool operator ==(char16_t c) const { return m_ch == c; }
    inline bool operator !=(char16_t c) const { return !operator ==(c); }
    inline bool operator ==(AsciiChar c) const { return m_ch == c.unicode(); }
    inline bool operator !=(AsciiChar c) const { return !operator ==(c); }

    inline bool operator >(Char c) const { return m_ch > c.m_ch; }
    inline bool operator >(char16_t c) const { return m_ch > c; }
    inline bool operator <(Char c) const { return m_ch < c.m_ch; }
    inline bool operator <(char16_t c) const { return m_ch < c; }

    inline bool operator >=(Char c) const { return m_ch >= c.m_ch; }
    inline bool operator >=(char16_t c) const { return m_ch >= c; }
    inline bool operator <=(Char c) const { return m_ch <= c.m_ch; }
    inline bool operator <=(char16_t c) const { return m_ch <= c; }

    inline char16_t unicode() const { return m_ch; }

    inline bool isNull() const { return m_ch == 0; }

    inline bool isAscii() const { return isAscii(m_ch); }
    static inline bool isAscii(char16_t c) { return c <= 0xff; }
    inline char toAscii(bool* ok = nullptr) const { return toAscii(m_ch, ok); }
    static char toAscii(char16_t c, bool* ok = nullptr);
    static inline Char fromAscii(char c) { return static_cast<char16_t>(c); }

    inline bool isLetter() const { return isLetter(m_ch); }
    static bool isLetter(char16_t c);
    inline bool isSpace() const { return isSpace(m_ch); }
    static bool isSpace(char16_t c);
    inline bool isDigit() const { return isDigit(m_ch); }
    static bool isDigit(char16_t c);
    int digitValue() const;

    inline bool isLower() const { return toLower() == m_ch; }
    inline Char toLower() const { return toLower(m_ch); }
    static char16_t toLower(char16_t ch);
    inline bool isUpper() const { return toUpper() == m_ch; }
    inline Char toUpper() const { return toUpper(m_ch); }
    static char16_t toUpper(char16_t ch);

    inline bool isHighSurrogate() const { return isHighSurrogate(m_ch); }
    inline bool isLowSurrogate() const { return isLowSurrogate(m_ch); }
    inline bool isSurrogate() const { return isSurrogate(m_ch); }

    static inline char32_t surrogateToUcs4(char16_t high, char16_t low) { return (char32_t(high) << 10) + low - 0x35fdc00; }
    static inline char32_t surrogateToUcs4(Char high, Char low) { return surrogateToUcs4(high.m_ch, low.m_ch); }
    static inline bool isHighSurrogate(char32_t ucs4) { return (ucs4 & 0xfffffc00) == 0xd800; }
    static inline bool isLowSurrogate(char32_t ucs4) { return (ucs4 & 0xfffffc00) == 0xdc00; }
    static inline bool isSurrogate(char32_t ucs4) { return ucs4 - 0xd800u < 2048u; }
    static inline char16_t highSurrogate(char32_t ucs4) { return char16_t((ucs4 >> 10) + 0xd7c0); }
    static inline char16_t lowSurrogate(char32_t ucs4) { return char16_t(ucs4 % 0x400 + 0xdc00); }
    static inline bool requiresSurrogates(char32_t ucs4) { return ucs4 >= 0x10000; }

private:
    char16_t m_ch = 0;
};

// ============================
// UtfCodec
// ============================
class UtfCodec
{
public:
    static void utf8to16(std::string_view src, std::u16string& dst);
    static void utf16to8(std::u16string_view src, std::string& dst);
    static void utf8to32(std::string_view src, std::u32string& dst);
    static void utf32to8(std::u32string_view src, std::string& dst);
};

// ============================
// String (UTF-16)
// ============================
class StringList;
class AsciiStringView;
class String
{
public:

    String();
    String(const char16_t* str);
    String(const Char& ch);
    String(const Char* unicode, size_t size = mu::nidx);

    String& operator=(const char16_t* str);
    void reserve(size_t i);

    inline bool operator ==(const String& s) const { return constStr() == s.constStr(); }
    inline bool operator !=(const String& s) const { return !operator ==(s); }
    inline bool operator ==(const QString& s) const { return toQString() == s; }
    inline bool operator !=(const QString& s) const { return !operator ==(s); }
    bool operator ==(const AsciiStringView& s) const;
    inline bool operator !=(const AsciiStringView& s) const { return !operator ==(s); }
    inline bool operator ==(const char16_t* s) const { return constStr() == s; }
    inline bool operator !=(const char16_t* s) const { return !operator ==(s); }
    bool operator ==(const char* s) const;
    inline bool operator !=(const char* s) const { return !operator ==(s); }

    inline bool operator <(const String& s) const { return constStr() < s.constStr(); }
    inline bool operator >(const String& s) const { return constStr() > s.constStr(); }
    inline bool operator <=(const String& s) const { return constStr() <= s.constStr(); }
    inline bool operator >=(const String& s) const { return constStr() >= s.constStr(); }

    inline String& operator +=(const String& s) { return append(s); }
    String& operator +=(const char16_t* s);
    inline String& operator +=(char16_t s) { return append(s); }

    inline String operator+(const mu::String& s) const { String t(*this); t += s; return t; }
    inline String operator+(const char16_t* s) const { String t(*this); t += s; return t; }
    inline String operator+(char16_t s) const { String t(*this); t += s; return t; }

    String& append(Char ch);
    String& append(const String& s);
    String& prepend(Char ch);
    String& prepend(const String& s);

    static String fromUtf8(const char* str);
    ByteArray toUtf8() const;

    static String fromAscii(const char* str, size_t size = mu::nidx);
    ByteArray toAscii(bool* ok = nullptr) const;

    static String fromStdString(const std::string& str);
    std::string toStdString() const;
    std::u16string toStdU16String() const;
    static String fromUcs4(const char32_t* str, size_t size = mu::nidx);
    std::u32string toStdU32String() const;

//#ifndef NO_QT_SUPPORT
    String(const QString& str) { *this = fromQString(str); }
    operator QString() const {
        return this->toQString();
    }

    String& operator=(const QString& str) { *this = fromQString(str); return *this; }
    static String fromQString(const QString& str);
    QString toQString() const;
//#endif

    size_t size() const;
    bool empty() const;
    inline bool isEmpty() const { return empty(); }
    void clear();
    Char at(size_t i) const;
    bool contains(const Char& ch) const;
    bool contains(const String& str, CaseSensitivity cs = CaseSensitive) const;
    int count(const Char& ch) const;
    size_t indexOf(const Char& ch) const;
    size_t indexOf(const char16_t* str) const;
    size_t lastIndexOf(const Char& ch) const;

    //! NOTE Now implemented only compare with ASCII
    bool startsWith(const String& str, CaseSensitivity cs = CaseSensitive) const;
    bool startsWith(char16_t ch, CaseSensitivity cs = CaseSensitive) const;
    bool endsWith(const String& str, CaseSensitivity cs = CaseSensitive) const;
    bool endsWith(char16_t ch, CaseSensitivity cs = CaseSensitive) const;

    StringList split(const Char& ch, SplitBehavior behavior = KeepEmptyParts) const;
    String& replace(const String& before, const String& after);
    String& replace(char16_t before, char16_t after);
    String& insert(size_t position, const String& str);
    String& remove(const String& str) { return replace(str, String()); }
    String& remove(const Char& ch);
    String& remove(char16_t ch);
    String& remove(size_t position, size_t n = mu::nidx);
    void truncate(size_t position);

    String arg(const String& val) const;
    String arg(const String& val1, const String& val2) const;
    String arg(const String& val1, const String& val2, const String& val3) const;
    String arg(const String& val1, const String& val2, const String& val3, const String& val4) const;
    String arg(const String& val1, const String& val2, const String& val3, const String& val4, const String& val5) const;

    String arg(int val) const { return arg(number(val)); }
    String arg(int val1, int val2) const { return arg(number(val1), number(val2)); }
    String arg(int val1, int val2, int val3) const { return arg(number(val1), number(val2), number(val3)); }

    String arg(int64_t val) const { return arg(number(val)); }
    String arg(int64_t val1, int64_t val2) const { return arg(number(val1), number(val2)); }
    String arg(int64_t val1, int64_t val2, int64_t val3) const { return arg(number(val1), number(val2), number(val3)); }

    String arg(size_t val) const { return arg(number(val)); }
    String arg(size_t val1, size_t val2) const { return arg(number(val1), number(val2)); }
    String arg(size_t val1, size_t val2, size_t val3) const { return arg(number(val1), number(val2), number(val3)); }

    String arg(double val) const { return arg(number(val)); }
    String arg(double val1, double val2) const { return arg(number(val1), number(val2)); }
    String arg(double val1, double val2, double val3) const { return arg(number(val1), number(val2), number(val3)); }

    String mid(size_t pos, size_t count = mu::nidx) const;
    String left(size_t n) const;
    String right(size_t n) const;

    String trimmed() const;
    String simplified() const;
    String toXmlEscaped() const;
    static String toXmlEscaped(const String& str);
    static String toXmlEscaped(char16_t c);

    String toLower() const;
    String toUpper() const;

    int toInt(bool* ok = nullptr, int base = 10) const;
    double toDouble(bool* ok = nullptr) const;

    static String number(int n, int base = 10);
    static String number(int64_t n);
    static String number(size_t n);
    static String number(double n, int prec = 6);

    inline size_t hash() const { return std::hash<std::u16string> {}(constStr()); }

private:
    const std::u16string& constStr() const;
    std::u16string& mutStr();
    void detach();
    void doArgs(std::u16string& out, const std::vector<std::u16string_view>& args) const;

    std::shared_ptr<std::u16string> m_data;
};

class StringList : public std::vector<String>
{
public:
    StringList() = default;
    StringList(const QStringList& l);

    StringList& operator <<(const String& s) { push_back(s); return *this; }
    StringList& append(const String& s) { push_back(s); return *this; }

    size_t indexOf(const String& s) const { return mu::indexOf(*this, s); }
    bool contains(const String& s) const { return mu::contains(*this, s); }
    StringList filter(const String& str) const;
    String join(const String& sep) const;

    void insert(size_t idx, const String& str);
    void replace(size_t idx, const String& str);
    bool removeAll(const String& str);
    void removeAt(size_t i);
};

// ============================
// AsciiStringView (ASCII)
// Be carefully!!, this class just hold pointer to string (no copy), so source string should be present, while use view
// ============================
class AsciiStringView
{
public:

    constexpr AsciiStringView() = default;
    constexpr AsciiStringView(const char* str)
        : m_size(str ? std::char_traits<char>::length(str) : 0), m_data(str) {}
    constexpr AsciiStringView(const char* str, size_t size)
        : m_size(size), m_data(str) {}

//#ifndef NO_QT_SUPPORT
    static AsciiStringView fromQLatin1String(const QLatin1String& str) { return AsciiStringView(str.latin1(), str.size()); }
    QLatin1String toQLatin1String() const { return QLatin1String(m_data, static_cast<int>(m_size)); }
//#endif

    inline bool operator ==(const AsciiStringView& s) const { return m_size == s.m_size && std::memcmp(m_data, s.m_data, m_size) == 0; }
    inline bool operator !=(const AsciiStringView& s) const { return !this->operator ==(s); }
    inline bool operator ==(const char* s) const
    {
        size_t sz = (s ? std::char_traits<char>::length(s) : 0);
        return m_size == sz && (s ? std::memcmp(m_data, s, m_size) == 0 : true);
    }

    inline bool operator !=(const char* s) const { return !this->operator ==(s); }

    inline bool operator <(const AsciiStringView& s) const
    {
        if (m_size != s.m_size) {
            return m_size < s.m_size;
        }
        return std::memcmp(m_data, s.m_data, m_size) < 0;
    }

    const char* ascii() const;
    size_t size() const;
    bool empty() const;
    AsciiChar at(size_t i) const;
    bool contains(char ch) const;
    size_t indexOf(char ch) const;

    int toInt(bool* ok = nullptr, int base = 10) const;
    double toDouble(bool* ok = nullptr) const;

private:
    size_t m_size = 0;
    const char* m_data = nullptr;
};

inline String operator+(char16_t s1, const String& s2) { String t(s1); t += s2; return t; }
inline String operator+(const char16_t* s1, const String& s2) { String t(s1); t += s2; return t; }
}

// ============================
// Char (UTF-16)
// ============================
inline bool operator ==(const char16_t c1, const mu::Char c2) { return c2 == c1; }
inline bool operator !=(const char16_t c1, const mu::Char c2) { return c2 != c1; }

// ============================
// String (UTF-16)
// ============================
inline bool operator ==(const char16_t* s1, const mu::String& s2) { return s2 == s1; }
inline bool operator !=(const char16_t* s1, const mu::String& s2) { return s2 != s1; }

template<>
struct std::hash<mu::String>
{
    std::size_t operator()(const mu::String& s) const noexcept { return s.hash(); }
};

inline mu::logger::Stream& operator<<(mu::logger::Stream& s, const mu::String& str)
{
    s << str.toUtf8().constChar();
    return s;
}

// ============================
// AsciiStringView (ASCII)
// ============================
inline bool operator ==(const char* s1, const mu::AsciiStringView& s2) { return s2 == s1; }
inline bool operator !=(const char* s1, const mu::AsciiStringView& s2) { return s2 != s1; }

inline mu::logger::Stream& operator<<(mu::logger::Stream& s, const mu::AsciiStringView& str)
{
    s << str.ascii();
    return s;
}

#ifndef muPrintable
#  define muPrintable(string) string.toUtf8().constChar()
#endif

#endif // MU_GLOBAL_STRING_H
