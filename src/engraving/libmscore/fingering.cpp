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

#include "fingering.h"

#include "translation.h"

#include "beam.h"
#include "chord.h"
#include "measure.h"
#include "note.h"
#include "part.h"
#include "skyline.h"
#include "staff.h"
#include "stem.h"
#include "system.h"

using namespace mu;

namespace mu::engraving {
//---------------------------------------------------------
//   fingeringStyle
//---------------------------------------------------------

static const ElementStyle fingeringStyle {
    { Sid::fingeringPlacement, Pid::PLACEMENT },
    { Sid::fingeringMinDistance, Pid::MIN_DISTANCE },
};

//---------------------------------------------------------
//   Fingering
//      EngravingItem(Score* = 0, ElementFlags = ElementFlag::NOTHING);
//---------------------------------------------------------

Fingering::Fingering(Note* parent, TextStyleType tid, ElementFlags ef)
    : TextBase(ElementType::FINGERING, parent, tid, ef)
{
    setPlacement(PlacementV::ABOVE);
    initElementStyle(&fingeringStyle);
}

Fingering::Fingering(Note* parent, ElementFlags ef)
    : Fingering(parent, TextStyleType::FINGERING, ef)
{
}

//---------------------------------------------------------
//   layoutType
//---------------------------------------------------------

ElementType Fingering::layoutType() const
{
    switch (textStyleType()) {
    case TextStyleType::FINGERING:
    case TextStyleType::RH_GUITAR_FINGERING:
    case TextStyleType::STRING_NUMBER:
        return ElementType::CHORD;
    default:
        return ElementType::NOTE;
    }
}

//---------------------------------------------------------
//   calculatePlacement
//---------------------------------------------------------

PlacementV Fingering::calculatePlacement() const
{
    Note* n = note();
    if (!n) {
        return PlacementV::ABOVE;
    }
    Chord* chord = n->chord();
    Staff* staff = chord->staff();
    Part* part   = staff->part();
    size_t nstaves  = part->nstaves();
    bool voices  = chord->measure()->hasVoices(staff->idx(), chord->tick(), chord->actualTicks());
    bool below   = voices ? !chord->up() : (nstaves > 1) && (staff->rstaff() == nstaves - 1);
    return below ? PlacementV::BELOW : PlacementV::ABOVE;
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Fingering::layout()
{
    if (explicitParent()) {
        Fraction tick = parentItem()->tick();
        const Staff* st = staff();
        _skipDraw = false;
        if (st && st->isTabStaff(tick) && (!st->staffType(tick)->showTabFingering() || textStyleType() == TextStyleType::STRING_NUMBER)) {
            _skipDraw = true;
            return;
        }
    }

    TextBase::layout();
    setPosY(0.0);      // handle placement below

    if (autoplace() && note()) {
        Note* n      = note();
        Chord* chord = n->chord();
        bool voices  = chord->measure()->hasVoices(chord->staffIdx(), chord->tick(), chord->actualTicks());
        bool tight   = voices && chord->notes().size() == 1 && !chord->beam() && textStyleType() != TextStyleType::STRING_NUMBER;

        double headWidth = n->bboxRightPos();

        // update offset after drag
        double rebase = 0.0;
        if (offsetChanged() != OffsetChange::NONE && !tight) {
            rebase = rebaseOffset();
        }

        // temporarily exclude self from chord shape
        setAutoplace(false);

        if (layoutType() == ElementType::CHORD) {
            bool above = placeAbove();
            Stem* stem = chord->stem();
            Segment* s = chord->segment();
            Measure* m = s->measure();
            double sp = spatium();
            double md = minDistance().val() * sp;
            SysStaff* ss = m->system()->staff(chord->vStaffIdx());
            Staff* vStaff = chord->staff();           // TODO: use current height at tick

            if (n->mirror()) {
                movePosX(-n->ipos().x());
            }
            movePosX(headWidth * .5);
            if (above) {
                if (tight) {
                    if (chord->stem()) {
                        movePosX(-0.8 * sp);
                    }
                    movePosY(-1.5 * sp);
                } else {
                    RectF r = bbox().translated(m->pos() + s->pos() + chord->pos() + n->pos() + pos());
                    SkylineLine sk(false);
                    sk.add(r.x(), r.bottom(), r.width());
                    double d = sk.minDistance(ss->skyline().north());
                    double yd = 0.0;
                    if (d > 0.0 && isStyled(Pid::MIN_DISTANCE)) {
                        yd -= d + height() * .25;
                    }
                    // force extra space above staff & chord (but not other fingerings)
                    double top;
                    if (chord->up() && chord->beam() && stem) {
                        top = stem->y() + stem->bbox().top();
                    } else {
                        Note* un = chord->upNote();
                        top = std::min(0.0, un->y() + un->bbox().top());
                    }
                    top -= md;
                    double diff = (bbox().bottom() + ipos().y() + yd + n->y()) - top;
                    if (diff > 0.0) {
                        yd -= diff;
                    }
                    if (offsetChanged() != OffsetChange::NONE) {
                        // user moved element within the skyline
                        // we may need to adjust minDistance, yd, and/or offset
                        bool inStaff = above ? r.bottom() + rebase > 0.0 : r.top() + rebase < staff()->height();
                        rebaseMinDistance(md, yd, sp, rebase, above, inStaff);
                    }
                    movePosY(yd);
                }
            } else {
                if (tight) {
                    if (chord->stem()) {
                        movePosX(0.8 * sp);
                    }
                    movePosY(1.5 * sp);
                } else {
                    RectF r = bbox().translated(m->pos() + s->pos() + chord->pos() + n->pos() + pos());
                    SkylineLine sk(true);
                    sk.add(r.x(), r.top(), r.width());
                    double d = ss->skyline().south().minDistance(sk);
                    double yd = 0.0;
                    if (d > 0.0 && isStyled(Pid::MIN_DISTANCE)) {
                        yd += d + height() * .25;
                    }
                    // force extra space below staff & chord (but not other fingerings)
                    double bottom;
                    if (!chord->up() && chord->beam() && stem) {
                        bottom = stem->y() + stem->bbox().bottom();
                    } else {
                        Note* dn = chord->downNote();
                        bottom = std::max(vStaff->height(), dn->y() + dn->bbox().bottom());
                    }
                    bottom += md;
                    double diff = bottom - (bbox().top() + ipos().y() + yd + n->y());
                    if (diff > 0.0) {
                        yd += diff;
                    }
                    if (offsetChanged() != OffsetChange::NONE) {
                        // user moved element within the skyline
                        // we may need to adjust minDistance, yd, and/or offset
                        bool inStaff = above ? r.bottom() + rebase > 0.0 : r.top() + rebase < staff()->height();
                        rebaseMinDistance(md, yd, sp, rebase, above, inStaff);
                    }
                    movePosY(yd);
                }
            }
        } else if (textStyleType() == TextStyleType::LH_GUITAR_FINGERING) {
            // place to left of note
            double left = n->shape().left();
            if (left - n->x() > 0.0) {
                movePosX(-left);
            } else {
                movePosX(-n->x());
            }
        }
        // for other fingering styles, do not autoplace

        // restore autoplace
        setAutoplace(true);
    } else if (offsetChanged() != OffsetChange::NONE) {
        // rebase horizontally too, as autoplace may have adjusted it
        rebaseOffset(false);
    }
    setOffsetChanged(false);
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Fingering::draw(mu::draw::Painter* painter) const
{
    TRACE_ITEM_DRAW;
    TextBase::draw(painter);
}

bool Fingering::isEditAllowed(EditData& ed) const
{
    if (isTextNavigationKey(ed.key, ed.modifiers)) {
        return false;
    }

    return TextBase::isEditAllowed(ed);
}

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

String Fingering::accessibleInfo() const
{
    String rez = EngravingItem::accessibleInfo();
    if (textStyleType() == TextStyleType::STRING_NUMBER) {
        rez += u' ' + mtrc("engraving", "String number");
    }
    return String(u"%1: %2").arg(rez, plainText());
}

bool Fingering::isOnCrossBeamSide() const
{
    Chord* chord = note() ? note()->chord() : nullptr;
    if (!chord) {
        return false;
    }
    return layoutType() == ElementType::CHORD
           && chord->beam() && (chord->beam()->cross() || chord->staffMove() != 0)
           && placeAbove() == chord->up();
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

engraving::PropertyValue Fingering::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::PLACEMENT:
        return calculatePlacement();
    case Pid::TEXT_STYLE:
        return TextStyleType::FINGERING;
    default:
        return TextBase::propertyDefault(id);
    }
}
}
