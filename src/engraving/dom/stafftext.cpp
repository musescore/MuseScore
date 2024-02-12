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

#include "stafftext.h"

#include "soundflag.h"
#include "segment.h"
#include "score.h"

using namespace mu;

namespace mu::engraving {
//---------------------------------------------------------
//   staffStyle
//---------------------------------------------------------

static const ElementStyle staffStyle {
    { Sid::staffTextPlacement, Pid::PLACEMENT },
    { Sid::staffTextMinDistance, Pid::MIN_DISTANCE },
};

//---------------------------------------------------------
//   StaffText
//---------------------------------------------------------

StaffText::StaffText(Segment* parent, TextStyleType tid)
    : StaffTextBase(ElementType::STAFF_TEXT, parent, tid, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
    initElementStyle(&staffStyle);
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

engraving::PropertyValue StaffText::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::TEXT_STYLE:
        return TextStyleType::STAFF;
    default:
        return StaffTextBase::propertyDefault(id);
    }
}

void StaffText::scanElements(void* data, void (* func)(void*, EngravingItem*), bool all)
{
    for (EngravingObject* child: scanChildren()) {
        child->scanElements(data, func, all);
    }
    if (all || visible() || score()->isShowInvisible()) {
        func(data, this);
    }
}

EngravingObjectList StaffText::scanChildren() const
{
    EngravingObjectList children;

    if (m_soundFlag) {
        children.push_back(m_soundFlag);
    }

    return children;
}

void StaffText::add(EngravingItem* e)
{
    e->setParent(this);
    e->setTrack(track());

    switch (e->type()) {
    case ElementType::SOUND_FLAG:
        setSoundFlag(toSoundFlag(e));
        e->added();
        return;
    default:
        break;
    }

    StaffTextBase::add(e);
}

void StaffText::remove(EngravingItem* e)
{
    switch (e->type()) {
    case ElementType::SOUND_FLAG: {
        if (soundFlag() == e) {
            setSoundFlag(nullptr);
            e->removed();
            return;
        } else {
            LOGD("StaffText::remove: %s %p there is already another sound flag", e->typeName(), e);
        }
        break;
    }
    default:
        break;
    }

    StaffTextBase::remove(e);
}

void StaffText::setTrack(track_idx_t idx)
{
    StaffTextBase::setTrack(idx);

    if (m_soundFlag) {
        m_soundFlag->setTrack(idx);
    }
}

bool StaffText::hasSoundFlag() const
{
    return m_soundFlag != nullptr;
}

SoundFlag* StaffText::soundFlag() const
{
    return m_soundFlag;
}

void StaffText::setSoundFlag(SoundFlag* flag)
{
    if (m_soundFlag == flag) {
        return;
    }

    m_soundFlag = flag;

    if (m_soundFlag) {
        m_soundFlag->setParent(this);
        m_soundFlag->setTrack(track());
    }
}
}
