//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef MU_FRAMEWORK_RET_H
#define MU_FRAMEWORK_RET_H

#include <string>

namespace mu {
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

        LanguagesFirst = 700,
        LanguagesLast  = 799,

        NotationFirst   = 1000,
        NotationLast    = 1299,

        ConverterFirst  = 1300,
        ConverterLast   = 1399,

        VstFirst        = 1400,
        VstLast         = 1499
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

    inline Ret& operator=(int c) { m_code = c; return *this; }
    inline operator bool() const {
        return success();
    }
    inline bool operator!() const { return !success(); }

    std::string toString() const;

private:
    int m_code = int(Code::Undefined);
    std::string m_text;
};

inline mu::Ret make_ret(Ret::Code e)
{
    return Ret(static_cast<int>(e));
}

inline mu::Ret make_ret(Ret::Code e, const std::string& text)
{
    return Ret(static_cast<int>(e), text);
}

inline bool check_ret(const Ret& r, Ret::Code c)
{
    return r.code() == int(c);
}
}

#endif // MU_FRAMEWORK_RET_H
