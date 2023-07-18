/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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
#ifndef MU_ENGRAVING_PADDINGTABLE_H
#define MU_ENGRAVING_PADDINGTABLE_H

#include <array>

#include "types/types.h"

namespace mu::engraving {
class MStyle;

template<typename T>
struct PaddingVector : std::array<T, TOT_ELEMENT_TYPES>
{
    T& operator [](size_t i) { return std::array<T, TOT_ELEMENT_TYPES>::operator [](i); }
    T& operator [](ElementType et) { return std::array<T, TOT_ELEMENT_TYPES>::operator [](static_cast<size_t>(et)); }
    const T& at(ElementType et) const { return std::array<T, TOT_ELEMENT_TYPES>::at(static_cast<size_t>(et)); }
};

struct PaddingTable : public PaddingVector<PaddingVector<double> >
{
public:

    void setMinimumPaddingUnit(double val) { m_minimumPaddingUnit = val; }
    double minimumPaddingUnit() const { return m_minimumPaddingUnit; }

    void createTable(const MStyle& style);

private:
    double m_minimumPaddingUnit = 0.0;
};
}

#endif // MU_ENGRAVING_PADDINGTABLE_H
