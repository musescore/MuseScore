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
#ifndef MU_FRAMEWORK_VAL_H
#define MU_FRAMEWORK_VAL_H

#include <string>

#ifndef NO_QT_SUPPORT
#include <QColor>
#include <QVariant>
#endif

namespace mu {
struct Val
{
    enum class Type {
        Undefined = 0,
        Bool,
        Int,
        Double,
        String,
        Color
    };

    std::string val;    //! NOTE In C++17 can be replaced by std::any or std::variant
    Type type = Type::Undefined;

    Val() = default;

    explicit Val(const char* str);
    explicit Val(const std::string& str);
    explicit Val(std::string&& str);
    explicit Val(double val);
    explicit Val(bool val);
    explicit Val(int val);
    explicit Val(QColor val);

    bool isNull() const;
    const std::string& toString() const;
    double toDouble() const;
    bool toBool() const;
    int toInt() const;

#ifndef NO_QT_SUPPORT
    QColor toQColor() const;

    QVariant toQVariant() const;
    static Val fromQVariant(const QVariant& var);
#endif
};
}

#endif // MU_FRAMEWORK_VAL_H
