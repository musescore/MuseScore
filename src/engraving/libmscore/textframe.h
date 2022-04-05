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

#ifndef __TEXTFRAME_H__
#define __TEXTFRAME_H__

#include "box.h"

namespace Ms {
class Text;

//---------------------------------------------------------
//   @@ TBox
///    Text frame.
//---------------------------------------------------------

class TBox : public VBox
{
    Text* _text;

public:
    TBox(System* parent);
    TBox(const TBox&);
    ~TBox();

    Text* text() const { return _text; }

    // Score Tree functions
    EngravingObject* scanParent() const override;
    EngravingObject* scanChild(int idx) const override;
    int scanChildCount() const override;

    TBox* clone() const override { return new TBox(*this); }

    void write(XmlWriter&) const override;
    using VBox::write;
    void read(XmlReader&) override;
    EngravingItem* drop(EditData&) override;
    void add(EngravingItem* e) override;
    void remove(EngravingItem* el) override;

    void layout() override;
    QString accessibleExtraInfo() const override;

    int gripsCount() const override;
    Grip initialEditModeGrip() const override;
    Grip defaultGrip() const override;

    bool needStartEditingAfterSelecting() const override { return false; }
};
}     // namespace Ms
#endif
