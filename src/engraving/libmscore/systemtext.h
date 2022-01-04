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

#ifndef __SYSTEMTEXT_H__
#define __SYSTEMTEXT_H__

#include "stafftext.h"

namespace Ms {
//---------------------------------------------------------
//   SystemText
//---------------------------------------------------------

class SystemText final : public StaffTextBase
{
    void layout() override;
    mu::engraving::PropertyValue propertyDefault(Pid id) const override;

public:
    SystemText(Segment* parent, TextStyleType = TextStyleType::SYSTEM);

    SystemText* clone() const override { return new SystemText(*this); }
    Segment* segment() const { return (Segment*)explicitParent(); }
};
}     // namespace Ms
#endif
