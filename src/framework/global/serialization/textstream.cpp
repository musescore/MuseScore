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
#include "textstream.h"

#include <array>
#include <charconv>

#include "global/io/iodevice.h"
#include "global/types/bytearray.h"
#include "global/types/string.h"

#include "global/log.h"

using namespace muse;

static constexpr int TEXTSTREAM_BUFFERSIZE = 16384;

TextStream::TextStream(io::IODevice* device)
    : m_device(device)
{
    m_buf.reserve(TEXTSTREAM_BUFFERSIZE);
}

TextStream::~TextStream()
{
    flush();
}

void TextStream::setDevice(io::IODevice* device)
{
    m_device = device;
}

void TextStream::flush()
{
    if (m_device && m_device->isOpen()) {
        m_device->write(m_buf.data(), m_buf.size());
        m_buf.clear();
    }
}

TextStream& TextStream::operator<<(char ch)
{
    write(&ch, 1);
    return *this;
}

TextStream& TextStream::operator<<(const int32_t val)
{
    // ceil(log_10(2^31)) = 10 (+ sign)
    std::array<char, 11> buf{};
    const auto [last, ec] = std::to_chars(buf.data(), buf.data() + buf.size(), val);
    IF_ASSERT_FAILED(ec == std::errc {}) {
        return *this;
    }

    write(buf.data(), static_cast<size_t>(last - buf.data()));

    return *this;
}

TextStream& TextStream::operator<<(const uint32_t val)
{
    // ceil(log_10(2^32)) = 10
    std::array<char, 10> buf{};
    const auto [last, ec] = std::to_chars(buf.data(), buf.data() + buf.size(), val);
    IF_ASSERT_FAILED(ec == std::errc {}) {
        return *this;
    }

    write(buf.data(), static_cast<size_t>(last - buf.data()));
    return *this;
}

TextStream& TextStream::operator<<(double val)
{
    // macOS: requires macOS 13.3 at runtime
    // linux: requires at least libstdc++ 11 (we compile with libstdc++ 10) or libc++ 14
#if !defined(Q_OS_MAC) && (defined(_MSC_VER) \
    || (defined(_LIBCPP_VERSION) && _LIBCPP_VERSION >= 14000) \
    || (defined(_GLIBCXX_RELEASE) && _GLIBCXX_RELEASE >= 11))
    std::array<char, 24> buf{};
    const auto [last, ec] = std::to_chars(buf.data(), buf.data() + buf.size(), val);
    IF_ASSERT_FAILED(ec == std::errc {}) {
        return *this;
    }
    write(buf.data(), static_cast<size_t>(last - buf.data()));
#else
    std::stringstream ss;
    ss << val;
    std::string buf = ss.str();
    write(buf.data(), buf.size());
#endif

    return *this;
}

TextStream& TextStream::operator<<(const int64_t val)
{
    // ceil(log_10(2^63)) = 20 (+ sign)
    std::array<char, 21> buf{};
    const auto [last, ec] = std::to_chars(buf.data(), buf.data() + buf.size(), val);
    IF_ASSERT_FAILED(ec == std::errc {}) {
        return *this;
    }

    write(buf.data(), static_cast<size_t>(last - buf.data()));

    return *this;
}

TextStream& TextStream::operator<<(const uint64_t val)
{
    // ceil(log_10(2^64)) = 20
    std::array<char, 20> buf{};
    const auto [last, ec] = std::to_chars(buf.data(), buf.data() + buf.size(), val);
    IF_ASSERT_FAILED(ec == std::errc {}) {
        return *this;
    }

    write(buf.data(), static_cast<size_t>(last - buf.data()));

    return *this;
}

TextStream& TextStream::operator<<(const char* s)
{
    return operator<<(std::string_view { s });
}

TextStream& TextStream::operator<<(const std::string_view str)
{
    write(str.data(), str.size());
    return *this;
}

#ifndef NO_QT_SUPPORT
TextStream& TextStream::operator<<(const QString& s)
{
    QByteArray ba = s.toUtf8();
    write(ba.constData(), ba.length());
    return *this;
}

#endif

TextStream& TextStream::operator<<(const ByteArray& b)
{
    write(reinterpret_cast<const char*>(b.constData()), b.size());
    return *this;
}

TextStream& TextStream::operator<<(const String& s)
{
    ByteArray b = s.toUtf8();
    write(reinterpret_cast<const char*>(b.constData()), b.size());
    return *this;
}

void TextStream::write(const char* ch, size_t len)
{
    m_buf.insert(m_buf.end(), reinterpret_cast<const uint8_t*>(ch), reinterpret_cast<const uint8_t*>(ch + len));
    if (m_device && m_buf.size() > TEXTSTREAM_BUFFERSIZE) {
        flush();
    }
}
