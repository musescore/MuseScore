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

FractionWrapper* FractionWrapper::reduced() const
{
    return wrap(fraction().reduced());
}

FractionWrapper* FractionWrapper::inverse() const
{
    return wrap(fraction().inverse());
}

FractionWrapper* FractionWrapper::absValue() const
{
    return wrap(fraction().absValue());
}

FractionWrapper* FractionWrapper::plus(FractionWrapper* other)
{
    return wrap(fraction() + other->fraction());
}

FractionWrapper* FractionWrapper::minus(FractionWrapper* other)
{
    return wrap(fraction() - other->fraction());
}

FractionWrapper* FractionWrapper::times(FractionWrapper* other)
{
    return wrap(fraction() * other->fraction());
}

FractionWrapper* FractionWrapper::times(int v)
{
    return wrap(fraction() * v);
}

FractionWrapper* FractionWrapper::dividedBy(FractionWrapper* other)
{
    return wrap(fraction() / other->fraction());
}

FractionWrapper* FractionWrapper::dividedBy(int v)
{
    return wrap(fraction() / v);
}

bool FractionWrapper::greaterThan(FractionWrapper* other)
{
    return fraction() > other->fraction();
}

bool FractionWrapper::lessThan(FractionWrapper* other)
{
    return fraction() < other->fraction();
}

bool FractionWrapper::equals(FractionWrapper* other)
{
    return fraction() == other->fraction();
}

bool FractionWrapper::identical(FractionWrapper* other)
{
    return fraction().identical(other->fraction());
}
