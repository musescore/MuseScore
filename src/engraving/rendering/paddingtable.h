/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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
#pragma once

#include <array>

#include "../types/types.h"

namespace mu::engraving {
class EngravingItem;
class MStyle;
struct ParenPaddingTable;

using ParenPaddingTablePtr = std::unique_ptr<ParenPaddingTable>;

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
    void initPaddingTable();
    double m_minimumPaddingUnit = 0.0;
};

struct ParenPaddingTable
{
public:
    void setMinimumPaddingUnit(double val) { m_minimumPaddingUnit = val; }
    double minimumPaddingUnit() const { return m_minimumPaddingUnit; }

    virtual void createTable(const MStyle& style) = 0;
    double padding(ElementType type1, ElementType type2);

    static ParenPaddingTablePtr getPaddingTable(const EngravingItem* parent);

protected:
    void initPaddingTable();
    PaddingVector<double> m_parenBefore;
    PaddingVector<double> m_parenAfter;

private:
    double m_minimumPaddingUnit = 0.0;
};

struct NoteParenPaddingTable : public ParenPaddingTable {
    void createTable(const MStyle& style) override;
};

struct KeySigParenPaddingTable : public ParenPaddingTable {
    void createTable(const MStyle& style) override;
};

struct TimeSigParenPaddingTable : public ParenPaddingTable {
    void createTable(const MStyle& style) override;
};

struct ClefParenPaddingTable : public ParenPaddingTable {
    void createTable(const MStyle& style) override;
};
}
