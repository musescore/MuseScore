/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "apistructs.h"

#include "engraving/types/fraction.h"

#include "log.h"

using namespace mu::engraving::apiv1;

Fraction* Fraction::reduced() const
{
    return wrap(fraction().reduced());
}

Fraction* Fraction::inverse() const
{
    return wrap(fraction().inverse());
}

Fraction* Fraction::absValue() const
{
    return wrap(fraction().absValue());
}

Fraction* Fraction::plus(Fraction* other)
{
    return wrap(fraction() + other->fraction());
}

Fraction* Fraction::minus(Fraction* other)
{
    return wrap(fraction() - other->fraction());
}

Fraction* Fraction::times(Fraction* other)
{
    return wrap(fraction() * other->fraction());
}

Fraction* Fraction::times(int v)
{
    return wrap(fraction() * v);
}

Fraction* Fraction::dividedBy(Fraction* other)
{
    return wrap(fraction() / other->fraction());
}

Fraction* Fraction::dividedBy(int v)
{
    return wrap(fraction() / v);
}

bool Fraction::greaterThan(Fraction* other)
{
    return fraction() > other->fraction();
}

bool Fraction::lessThan(Fraction* other)
{
    return fraction() < other->fraction();
}

bool Fraction::equals(Fraction* other)
{
    return fraction() == other->fraction();
}

bool Fraction::identical(Fraction* other)
{
    return fraction().identical(other->fraction());
}
