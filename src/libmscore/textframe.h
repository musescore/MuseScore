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
    TBox(Score* score);
    TBox(const TBox&);
    ~TBox();

    // Score Tree functions
    ScoreElement* treeParent() const override;
    ScoreElement* treeChild(int idx) const override;
    int treeChildCount() const override;

    virtual TBox* clone() const override { return new TBox(*this); }
    virtual ElementType type() const override { return ElementType::TBOX; }
    virtual void write(XmlWriter&) const override;
    using VBox::write;
    virtual void read(XmlReader&) override;
    virtual Element* drop(EditData&) override;
    virtual void add(Element* e) override;
    virtual void remove(Element* el) override;

    virtual void layout() override;
    virtual QString accessibleExtraInfo() const override;
    Text* text() { return _text; }

    EditBehavior normalModeEditBehavior() const override { return EditBehavior::SelectOnly; }
};
}     // namespace Ms
#endif
