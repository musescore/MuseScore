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
#ifndef MU_GLOBAL_JSON_H
#define MU_GLOBAL_JSON_H

#include <memory>

#include "types/bytearray.h"
#include "types/string.h"

namespace mu {
struct JsonData;
using JsonDataPtr = std::shared_ptr<JsonData>;
class JsonArray;
class JsonObject;
class JsonValue
{
public:
    JsonValue() = delete;

    bool isNull() const;
    void setNull();

    bool isBool() const;
    bool toBool() const;
    JsonValue& operator=(bool v);

    bool isInt() const;
    int toInt() const;
    JsonValue& operator=(int v);

    bool isDouble() const;
    double toDouble() const;
    JsonValue& operator=(double v);

    bool isString() const;
    String toString() const;
    JsonValue& operator=(const String& str);
    JsonValue& operator=(const char16_t* str);
    JsonValue& operator=(const char* str);

    bool isArray() const;
    JsonArray toArray() const;

    bool isObject() const;
    JsonObject toObject() const;

private:
    friend class JsonObject;
    friend class JsonArray;

    JsonValue(JsonDataPtr data, uintptr_t value);

    JsonDataPtr m_data = nullptr;
    uintptr_t m_value;
};

class JsonArray
{
public:

    size_t size() const;
    JsonValue at(size_t i) const;

    JsonArray& append(bool v);
    JsonArray& append(int v);
    JsonArray& append(double v);
    JsonArray& append(const String& str);
    JsonArray& append(const char16_t* str);
    JsonArray& append(const char* str);

private:

    friend class JsonValue;

    JsonArray(JsonDataPtr data, uintptr_t value);

    JsonDataPtr m_data = nullptr;
    uintptr_t m_value;
};

class JsonObject
{
public:
    JsonObject() = delete;

    size_t size() const;
    std::vector<AsciiStringView> keys() const;
    bool contains(const AsciiStringView& key) const;
    JsonValue value(const AsciiStringView& key) const;
    JsonValue operator [](const AsciiStringView& key);

private:
    friend class JsonDocument;
    friend class JsonValue;

    JsonObject(JsonDataPtr data, uintptr_t value);

    JsonDataPtr m_data = nullptr;
    uintptr_t m_value;
};

class JsonDocument
{
public:
    JsonDocument();

    enum class Format {
        Indented,
        Compact
    };

    bool isObject() const;
    JsonObject rootObject() const;
    JsonObject rootObject();

    JsonObject newObject();

    ByteArray toJson(Format format = Format::Indented) const;
    static JsonDocument fromJson(const ByteArray& ba, bool* ok = nullptr);

private:
    JsonDataPtr m_data = nullptr;
};
}

#endif // MU_GLOBAL_JSON_H
