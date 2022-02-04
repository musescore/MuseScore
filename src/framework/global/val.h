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
#ifndef MU_FRAMEWORK_VAL_H
#define MU_FRAMEWORK_VAL_H

#include <string>

#ifndef NO_QT_SUPPORT
#include <QColor>
#include <QVariant>
#endif

#include "io/path.h"

namespace mu {
class Val
{
public:
    enum class Type {
        Undefined = 0,
        Bool,
        Int,
        Double,
        String,
        Color,
        Variant
    };

    Val() = default;

    explicit Val(const char* str);
    explicit Val(const std::string& str);
    explicit Val(std::string&& str);
    explicit Val(double val);
    explicit Val(bool val);
    explicit Val(int val);
    explicit Val(const io::path& path);

    template<class E, typename = std::enable_if_t<std::is_enum_v<E> > >
    explicit Val(E val)
        : Val{static_cast<std::underlying_type_t<E> >(val)}
    {
    }

    void setType(Type t);
    Type type() const;

    bool isNull() const;
    std::string toString() const;
    double toDouble() const;
    float toFloat() const;
    bool toBool() const;
    int toInt() const;
    io::path toPath() const;

    template<class E, typename = std::enable_if_t<std::is_enum_v<E> > >
    E toEnum() const
    {
        return static_cast<E>(toInt());
    }

#ifndef NO_QT_SUPPORT
    explicit Val(QColor color);
    explicit Val(QVariant val);
    explicit Val(QString val);

    QColor toQColor() const;
    QString toQString() const;
    QVariant toQVariant() const;

    static Val fromQVariant(const QVariant& var);
#endif

    bool operator ==(const Val& v) const;
    bool operator <(const Val& v) const;

private:
    QVariant m_val; //! NOTE In C++17 can be replaced by std::any or std::variant
    Type m_type = Type::Undefined;
};
}

#endif // MU_FRAMEWORK_VAL_H
