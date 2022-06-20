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
#include "json.h"

#include "thirdparty/rapidjson/document.h"
#include "thirdparty/rapidjson/writer.h"
#include "thirdparty/rapidjson/prettywriter.h"
#include "thirdparty/rapidjson/stringbuffer.h"

#include "log.h"

using namespace mu;
using namespace rapidjson;

struct mu::JsonData {
    Document doc;
};

// =======================================
// JsonValue
// =======================================

static Document::ValueType& value_cast(uintptr_t v)
{
    return *reinterpret_cast<Document::ValueType*>(v);
}

JsonValue::JsonValue(JsonDataPtr data, uintptr_t value)
    : m_data(data), m_value(value)
{
}

bool JsonValue::isNull() const
{
    return value_cast(m_value).IsNull();
}

void JsonValue::setNull()
{
    value_cast(m_value).SetNull();
}

bool JsonValue::isBool() const
{
    return value_cast(m_value).IsBool();
}

bool JsonValue::toBool() const
{
    return value_cast(m_value).GetBool();
}

JsonValue& JsonValue::operator=(bool v)
{
    value_cast(m_value).SetBool(v);
    return *this;
}

bool JsonValue::isInt() const
{
    return value_cast(m_value).IsInt();
}

int JsonValue::toInt() const
{
    return value_cast(m_value).GetInt();
}

JsonValue& JsonValue::operator=(int v)
{
    value_cast(m_value).SetInt(v);
    return *this;
}

bool JsonValue::isDouble() const
{
    return value_cast(m_value).IsDouble();
}

double JsonValue::toDouble() const
{
    return value_cast(m_value).GetDouble();
}

JsonValue& JsonValue::operator=(double v)
{
    value_cast(m_value).SetDouble(v);
    return *this;
}

bool JsonValue::isString() const
{
    return value_cast(m_value).IsString();
}

String JsonValue::toString() const
{
    return String::fromUtf8(value_cast(m_value).GetString());
}

JsonValue& JsonValue::operator=(const String& str)
{
    ByteArray ba = str.toUtf8();
    value_cast(m_value).SetString(ba.constChar(), ba.size(), m_data->doc.GetAllocator());
    return *this;
}

JsonValue& JsonValue::operator=(const char16_t* str)
{
    ByteArray ba = String(str).toUtf8();
    value_cast(m_value).SetString(ba.constChar(), ba.size(), m_data->doc.GetAllocator());
    return *this;
}

JsonValue& JsonValue::operator=(const char* str)
{
    value_cast(m_value).SetString(str, std::strlen(str), m_data->doc.GetAllocator());
    return *this;
}

bool JsonValue::isArray() const
{
    return value_cast(m_value).IsArray();
}

JsonArray JsonValue::toArray() const
{
    if (!value_cast(m_value).IsArray()) {
        value_cast(m_value).SetArray();
    }
    return JsonArray(m_data, m_value);
}

bool JsonValue::isObject() const
{
    return value_cast(m_value).IsObject();
}

JsonObject JsonValue::toObject() const
{
    if (!value_cast(m_value).IsObject()) {
        value_cast(m_value).SetObject();
    }
    return JsonObject(m_data, m_value);
}

// =======================================
// JsonArray
// =======================================

JsonArray::JsonArray(JsonDataPtr data, uintptr_t value)
    : m_data(data), m_value(value)
{
}

size_t JsonArray::size() const
{
    return value_cast(m_value).Size();
}

JsonValue JsonArray::at(size_t i) const
{
    Document::ValueType& val = value_cast(m_value)[static_cast<int>(i)];
    return JsonValue(m_data, reinterpret_cast<uintptr_t>(&val));
}

JsonArray& JsonArray::append(bool v)
{
    value_cast(m_value).PushBack(v, m_data->doc.GetAllocator());
    return *this;
}

JsonArray& JsonArray::append(int v)
{
    value_cast(m_value).PushBack(v, m_data->doc.GetAllocator());
    return *this;
}

JsonArray& JsonArray::append(double v)
{
    value_cast(m_value).PushBack(v, m_data->doc.GetAllocator());
    return *this;
}

JsonArray& JsonArray::append(const String& str)
{
    ByteArray ba = str.toUtf8();
    value_cast(m_value).PushBack(GenericStringRef(ba.constChar(), ba.size()), m_data->doc.GetAllocator());
    return *this;
}

JsonArray& JsonArray::append(const char16_t* str)
{
    ByteArray ba = String(str).toUtf8();
    value_cast(m_value).PushBack(GenericStringRef(ba.constChar(), ba.size()), m_data->doc.GetAllocator());
    return *this;
}

JsonArray& JsonArray::append(const char* str)
{
    value_cast(m_value).PushBack(GenericStringRef(str), m_data->doc.GetAllocator());
    return *this;
}

// =======================================
// JsonObject
// =======================================

JsonObject::JsonObject(JsonDataPtr data, uintptr_t value)
    : m_data(data), m_value(value)
{
}

size_t JsonObject::size() const
{
    return value_cast(m_value).MemberCount();
}

bool JsonObject::contains(const AsciiStringView& key) const
{
    return value_cast(m_value).HasMember(key.ascii());
}

JsonValue JsonObject::value(const AsciiStringView& key) const
{
    Document::ValueType& val = value_cast(m_value)[key.ascii()];
    return JsonValue(m_data, reinterpret_cast<uintptr_t>(&val));
}

JsonValue JsonObject::operator[](const AsciiStringView& key)
{
    Document::ValueType& val = value_cast(m_value);
    Document::MemberIterator member = val.FindMember(key.ascii());
    if (member != val.MemberEnd()) {
        return JsonValue(m_data, reinterpret_cast<uintptr_t>(&member->value));
    }
    static Document::ValueType null;
    value_cast(m_value).AddMember(GenericStringRef(key.ascii()), null, m_data->doc.GetAllocator());
    member = val.FindMember(key.ascii());
    if (member != val.MemberEnd()) {
        return JsonValue(m_data, reinterpret_cast<uintptr_t>(&member->value));
    }

    UNREACHABLE;
    return JsonValue(m_data, reinterpret_cast<uintptr_t>(&null));
}

std::vector<AsciiStringView> JsonObject::keys() const
{
    Document::ValueType& val = value_cast(m_value);
    std::vector<AsciiStringView> vec;
    vec.reserve(val.MemberCount());
    Document::MemberIterator member = val.MemberBegin();
    for (; member != val.MemberEnd(); ++member) {
        vec.push_back(member->name.GetString());
    }
    return vec;
}

// =======================================
// JsonDocument
// =======================================

JsonDocument::JsonDocument()
{
    m_data = std::make_shared<JsonData>();
}

ByteArray JsonDocument::toJson(Format format) const
{
    StringBuffer buffer;

    if (format == Format::Indented) {
        PrettyWriter<StringBuffer> writer(buffer);
        m_data->doc.Accept(writer);
    } else {
        Writer<StringBuffer> writer(buffer);
        m_data->doc.Accept(writer);
    }

    const char* str = buffer.GetString();
    return ByteArray(str);
}

JsonDocument JsonDocument::fromJson(const ByteArray& ba, bool* ok)
{
    JsonDocument d;
    bool myOk = d.m_data->doc.Parse(ba.constChar(), ba.size()).HasParseError();
    if (ok) {
        *ok = myOk;
    }
    return d;
}

bool JsonDocument::isObject() const
{
    return m_data->doc.IsObject();
}

JsonObject JsonDocument::rootObject() const
{
    return JsonObject(m_data, reinterpret_cast<uintptr_t>(&m_data->doc));
}

JsonObject JsonDocument::rootObject()
{
    if (m_data->doc.IsNull()) {
        m_data->doc.SetObject();
    }
    return JsonObject(m_data, reinterpret_cast<uintptr_t>(&m_data->doc));
}
