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

#ifndef MUSE_GLOBAL_RET_H
#define MUSE_GLOBAL_RET_H

#include <string>
#include <map>
#include <any>
#include <optional>

#ifndef NO_QT_SUPPORT
#include <QString>
#endif

#include "global/log.h"

namespace muse {
class Ret
{
public:

    enum class Code {
        Undefined       = -1,
        Ok              = 0,
        UnknownError    = 1,

        // not error, just codes
        Cancel          = 3,  // abort by user

        NotSupported    = 4,
        NotImplemented  = 5,

        // Global errors
        GlobalFirst     = 20,
        InternalError   = 21,
        GlobalLast      = 99,

        UiFirst         = 100,
        UiLast          = 199,

        ExtensionsFirst = 200,
        ExtensionsLast  = 299,

        AudioFirst      = 300,
        AudioLast       = 399,

        SystemFirst     = 400,
        SystemLast      = 499,

        NetworkFirst    = 500,
        NetworkLast     = 599,

        MidiFirst       = 600,
        MidiLast        = 699,

        LanguagesFirst  = 700,
        LanguagesLast   = 799,

        NotationFirst   = 1000,
        NotationLast    = 1299,

        ConverterFirst  = 1300,
        ConverterLast   = 1399,

        VstFirst        = 1400,
        VstLast         = 1499,

        WorkspaceFirst  = 1500,
        WorkspaceLast   = 1599,

        LearnFirst      = 1600,
        LearnLast       = 1699,

        UpdateFirst  = 1700,
        UpdateLast   = 1799,

        CloudFirst   = 1800,
        CloudLast    = 1899,

        EngravingFirst  = 2000,
        EngravingLast   = 2999,

        ProjectFirst  = 3000,
        ProjectLast   = 3999,

        DiagnosticsFirst = 4000,
        DiagnosticsLast = 4999,

        MuseSoundsFirst = 5000,
        MuseSoundsLast = 5999
    };

    Ret() = default;
    Ret(bool arg);
    explicit Ret(int c);
    explicit Ret(Code c);
    explicit Ret(const int& c, const std::string& text);

    void setCode(int c);
    int code() const;
    bool valid() const;
    bool success() const;
    void setText(const std::string& s);
    const std::string& text() const;
    void setData(const std::string& key, const std::any& val);
    std::any data(const std::string& key) const;

    template<typename DataType>
    std::optional<DataType> data(const std::string& key, const std::optional<DataType>& defaultValue = std::nullopt) const
    {
        static_assert(!std::is_reference_v<DataType>, "DataType must not be a reference");
        static_assert(!std::is_pointer_v<DataType>, "DataType must not be a pointer");

        const auto it = m_data.find(key);
        if (it == m_data.end()) {
            IF_ASSERT_FAILED_X(false, "Ret::data<" + std::string(typeid(DataType).name()) + ">: key not found: '" + key + "'") {
                return defaultValue;
            }
        }

        if (it->second.type() == typeid(DataType)) {
            try
            {
                return std::any_cast<DataType>(it->second);
            }
            catch (const std::bad_any_cast& e)
            {
                IF_ASSERT_FAILED_X(false, "Ret::data<" + std::string(typeid(DataType).name())
                                   + ">: bad_any_cast for key '" + key + "': " + e.what()) {
                    return defaultValue;
                }
            }
        } else {
            IF_ASSERT_FAILED_X(false, "Ret::data<" + std::string(typeid(DataType).name())
                               + ">: type mismatch for key '" + key
                               + "', stored type is " + it->second.type().name()) {
                return defaultValue;
            }
        }

        return defaultValue;
    }

    inline Ret& operator=(int c) { m_code = c; return *this; }
    inline Ret& operator=(bool arg) { m_code = arg ? int(Code::Ok) : int(Code::UnknownError); return *this; }
    inline operator bool() const {
        return success();
    }
    inline bool operator!() const { return !success(); }

    std::string toString() const;

private:
    int m_code = int(Code::Undefined);
    std::string m_text;
    std::map<std::string, std::any> m_data;
};

inline muse::Ret make_ok()
{
    return Ret(static_cast<int>(Ret::Code::Ok));
}

inline muse::Ret make_ret(Ret::Code e)
{
    return Ret(static_cast<int>(e));
}

inline muse::Ret make_ret(Ret::Code e, const std::string& text)
{
    return Ret(static_cast<int>(e), text);
}

inline muse::Ret make_ret(int e, const std::string& text)
{
    return Ret(e, text);
}

#ifndef NO_QT_SUPPORT
inline muse::Ret make_ret(Ret::Code e, const QString& text)
{
    return Ret(static_cast<int>(e), text.toStdString());
}

#endif

inline bool check_ret(const Ret& r, Ret::Code c)
{
    return r.code() == int(c);
}
}

#endif // MUSE_GLOBAL_RET_H
