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
#include "textframe.h"

#include "draw/fontmetrics.h"

#include "layout/tlayout.h"

#include "box.h"
#include "factory.h"
#include "mscore.h"
#include "system.h"
#include "text.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   TBox
//---------------------------------------------------------

TBox::TBox(System* parent)
    : VBox(ElementType::TBOX, parent)
{
    setBoxHeight(Spatium(1));
    m_text  = Factory::createText(this, TextStyleType::FRAME);
    m_text->setLayoutToParentWidth(true);
    m_text->setParent(this);
}

TBox::TBox(const TBox& tbox)
    : VBox(tbox)
{
    m_text = Factory::copyText(*(tbox.m_text));
}

TBox::~TBox()
{
    delete m_text;
}

//---------------------------------------------------------
<<<<<<< HEAD
<<<<<<< HEAD
=======
//   layout
///   The text box layout() adjusts the frame height to text
///   height.
//---------------------------------------------------------

void TBox::layout()
{
    UNREACHABLE;
    LayoutContext ctx(score());
    v0::TLayout::layout(this, ctx);
}

//---------------------------------------------------------
>>>>>>> 4f8a1b6dd0... [engraving] replaced item->layout() to TLayout::layout
=======
>>>>>>> 11610ff2b5... [engraving] removed item->layout method
//   drop
//---------------------------------------------------------

EngravingItem* TBox::drop(EditData& data)
{
    EngravingItem* e = data.dropElement;
    switch (e->type()) {
    case ElementType::TEXT:
        m_text->undoChangeProperty(Pid::TEXT, toText(e)->xmlText());
        delete e;
        return m_text;
    default:
        return VBox::drop(data);
    }
}

//---------------------------------------------------------
//   add
///   Add new EngravingItem \a el to TBox
//---------------------------------------------------------

void TBox::add(EngravingItem* e)
{
    if (e->isText()) {
        // does not normally happen, since drop() handles this directly
        m_text->undoChangeProperty(Pid::TEXT, toText(e)->xmlText());
        e->setParent(this);
        e->added();
    } else {
        VBox::add(e);
    }
}

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void TBox::remove(EngravingItem* el)
{
    if (el == m_text) {
        // does not normally happen, since Score::deleteItem() handles this directly
        // but if it does:
        // replace with new empty text element
        // this keeps undo/redo happier than just clearing the text
        LOGD("TBox::remove() - replacing _text");
        m_text = Factory::createText(this, TextStyleType::FRAME);
        m_text->setLayoutToParentWidth(true);
        m_text->setParent(this);
        el->removed();
    } else {
        VBox::remove(el);
    }
}
}
