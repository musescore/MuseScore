/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "scoreelement.h"

#include "engraving/dom/measure.h"
#include "engraving/dom/range.h"
#include "engraving/dom/engravingobject.h"
#include "engraving/dom/score.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/spanner.h"
#include "engraving/dom/staff.h"

// api
#include "apitypes.h"
#include "apistructs.h"
#include "score.h"
#include "part.h"
#include "elements.h"

#include "log.h"

using namespace mu::engraving::apiv1;

ScoreElement::~ScoreElement()
{
    if (m_ownership == Ownership::PLUGIN) {
        delete e;
    }
}

int ScoreElement::apiversion() const
{
    if (m_apiversion == -1) {
        QJSEngine* engine = qjsEngine(this);
        IF_ASSERT_FAILED(engine) {
            return -1;
        }

        m_apiversion = engine->property("apiversion").toInt();
        IF_ASSERT_FAILED(m_apiversion > 0) {
            m_apiversion = -1;
        }
    }
    return m_apiversion;
}

QString ScoreElement::name() const
{
    return QString(e->typeName());
}

QJSValue ScoreElement::type() const
{
    if (apiversion() == 1) {
        return QJSValue(int(e->type()));
    } else {
        static const QMetaEnum meta = QMetaEnum::fromType<enums::ElementType>();
        return QJSValue(QString(meta.valueToKey(int(e->type()))));
    }
}

//---------------------------------------------------------
//   ScoreElement::userName
///   \brief Human-readable element type name
///   \returns Name of the element type, translated
///   according to the current MuseScore locale settings.
//---------------------------------------------------------

QString ScoreElement::userName() const
{
    return e->translatedTypeUserName();
}

//---------------------------------------------------------
//   ScoreElement::spatium
//---------------------------------------------------------

qreal ScoreElement::spatium() const
{
    return e->isEngravingItem() ? toEngravingItem(e)->spatium() : e->score()->style().spatium();
}

//---------------------------------------------------------
//   ScoreElement::get
///   Returns an engraving property converted to a plugin-readable value.
//---------------------------------------------------------

QVariant ScoreElement::get(mu::engraving::Pid pid) const
{
    if (!e) {
        return QVariant();
    }
    const PropertyValue val = e->getProperty(pid);
    switch (val.type()) {
    case P_TYPE::FRACTION: {
        const mu::engraving::Fraction f(val.value<mu::engraving::Fraction>());
        return QVariant::fromValue(wrap(f));
    }
    case P_TYPE::ORNAMENT_INTERVAL: {
        const OrnamentInterval o(val.value<OrnamentInterval>());
        return QVariant::fromValue(wrap(o));
    }
    case P_TYPE::POINT:
        return val.value<PointF>().toQPointF() / spatium();
    case P_TYPE::PAIR_REAL: {
        const PairF pair = val.value<PairF>();
        return QVariantList { pair.first, pair.second };
    }
    case P_TYPE::ABSOLUTE:
        return val.toReal() / spatium();
    case P_TYPE::SPATIUM:
        return val.value<Spatium>().val();
    default:
        break;
    }
    return val.toQVariant();
}

static mu::engraving::AlignH alignHFromApiValue(enums::Align apiValue)
{
    switch (apiValue & enums::Align::HMASK) {
    case enums::Align::LEFT: return mu::engraving::AlignH::LEFT;
    case enums::Align::RIGHT: return mu::engraving::AlignH::RIGHT;
    case enums::Align::HCENTER: return mu::engraving::AlignH::HCENTER;
    default:
        break;
    }

    return mu::engraving::AlignH::LEFT;
}

static mu::engraving::AlignV alignVFromApiValue(enums::Align apiValue)
{
    switch (apiValue & enums::Align::VMASK) {
    case enums::Align::TOP: return mu::engraving::AlignV::TOP;
    case enums::Align::BOTTOM: return mu::engraving::AlignV::BOTTOM;
    case enums::Align::VCENTER: return mu::engraving::AlignV::VCENTER;
    case enums::Align::BASELINE: return mu::engraving::AlignV::BASELINE;
    default:
        break;
    }

    return mu::engraving::AlignV::TOP;
}

// Only annotations and the lines the app puts in a voice can change voice this way:
// notes, rests and other items in a segment need the voice commands, other lines are
// in voice 1, and system items stay on the staves that show them
static bool canChangeVoice(const mu::engraving::EngravingItem* item)
{
    if (item->systemFlag()) {
        return false;
    }

    if (item->isSpanner()) {
        const mu::engraving::Spanner* spanner = mu::engraving::toSpanner(item);
        return spanner->isVoiceSpecific() || spanner->hasVoiceAssignmentProperties();
    }

    const mu::engraving::EngravingItem* parent = item->ownershipParentItem();
    if (!parent || !parent->isSegment()) {
        return false;
    }
    const std::vector<mu::engraving::EngravingItem*>& annotations = mu::engraving::toSegment(parent)->annotations();
    return std::find(annotations.begin(), annotations.end(), item) != annotations.end();
}

static bool canSetIndexInScore(const mu::engraving::EngravingItem* item, mu::engraving::Pid pid, size_t index)
{
    using mu::engraving::Pid;

    switch (pid) {
    case Pid::VOICE:
        return canChangeVoice(item);
    case Pid::TRACK:
        // only within its staff
        return canChangeVoice(item) && mu::engraving::track2staff(index) == item->staffIdx();
    case Pid::SPANNER_TRACK2: {
        if (!item->isSpanner()) {
            return false;
        }
        const mu::engraving::Spanner* spanner = mu::engraving::toSpanner(item);
        if (!spanner->canBeCrossStaff()) {
            // it ends on its start track
            return index == spanner->track();
        }
        if (spanner->anchor() != mu::engraving::Spanner::Anchor::SEGMENT
            && spanner->anchor() != mu::engraving::Spanner::Anchor::CHORDREST) {
            return false;
        }
        const mu::engraving::Score* score = spanner->score();
        if (index >= score->ntracks() || score->staff(mu::engraving::track2staff(index))->part() != spanner->part()) {
            return false;
        }
        // layout ends the spanner on the chord or rest in that track
        const mu::engraving::Segment* endSegment
            = score->tick2segmentMM(spanner->tick2(), false, mu::engraving::SegmentType::ChordRest);
        return endSegment && endSegment->element(index);
    }
    default:
        return true;
    }
}

//---------------------------------------------------------
//   ScoreElement::set
//---------------------------------------------------------

void ScoreElement::set(mu::engraving::Pid pid, const QVariant& val)
{
    if (!e) {
        return;
    }

    PropertyValue newValue;

    switch (propertyType(pid)) {
    case P_TYPE::FRACTION: {
        Fraction* f = val.value<Fraction*>();
        if (!f) {
            LOGW() << "trying to assign value of wrong type to fractional property";
            return;
        }
        // Pid::TIMESIG_ACTUAL is only set when we change the time signature,
        // aside from that it's read-only. What the user intends to do here is
        // change the actual length of the measure, so we do that instead.
        if (pid == Pid::TIMESIG_ACTUAL && e->isMeasure() && m_ownership == Ownership::SCORE) {
            mu::engraving::Measure* m = toMeasure(e);
            if (m->ticks() != f->fraction()) {
                mu::engraving::ScoreRange range;
                range.read(m->first(), m->last());
                m->adjustToLen(f->fraction());
            }
            return;
        }
        newValue = f->fraction();
    }
    break;
    case P_TYPE::ORNAMENT_INTERVAL: {
        OrnamentIntervalWrapper* o = val.value<OrnamentIntervalWrapper*>();
        if (!o) {
            LOGW() << "trying to assign value of wrong type to fractional property";
            return;
        }
        newValue = o->ornamentInterval();
    }
    break;
    case P_TYPE::POINT:
        newValue = PointF::fromQPointF(val.toPointF() * spatium());
        break;
    case P_TYPE::ABSOLUTE:
        newValue = val.toReal() * spatium();
        break;
    case P_TYPE::SIZE_T: {
        // track, voice, spannerTrack2, bracketColumn
        const int index = val.toInt();
        if (index < 0 || (pid == Pid::VOICE && index >= static_cast<int>(mu::engraving::MAX_VOICES))) {
            LOGW() << "trying to assign an invalid value to an index property: " << index;
            return;
        }
        if (m_ownership != Ownership::SCORE || !e->isEngravingItem()) {
            newValue = PropertyValue::fromQVariant(val, P_TYPE::SIZE_T);
            break;
        }
        mu::engraving::EngravingItem* item = toEngravingItem(e);
        if (!canSetIndexInScore(item, pid, static_cast<size_t>(index))) {
            LOGW() << "can't set " << propertyName(pid) << " of " << item->typeName() << " to " << index;
            return;
        }
        if (pid == Pid::TRACK) {
            // as a voice, which reaches parts and shared staves
            pid = Pid::VOICE;
            newValue = PropertyValue(static_cast<size_t>(index) % mu::engraving::VOICES);
        } else {
            newValue = PropertyValue::fromQVariant(val, P_TYPE::SIZE_T);
        }
    } break;
    case P_TYPE::ALIGN: {
        apiv1::enums::Align apiValue = apiv1::enums::Align(val.toInt());

        newValue = Align { alignHFromApiValue(apiValue), alignVFromApiValue(apiValue) };
    } break;

    default:
        newValue = PropertyValue::fromQVariant(val, propertyType(pid));
        break;
    }

    const PropertyFlags f = e->propertyFlags(pid);
    const PropertyFlags newFlags = (f == PropertyFlags::NOSTYLE) ? f : PropertyFlags::UNSTYLED;

    if (m_ownership == Ownership::SCORE) {
        e->undoChangeProperty(pid, newValue, newFlags);
        // as the voice commands do
        if (pid == Pid::VOICE && e->isEngravingItem() && toEngravingItem(e)->hasVoiceAssignmentProperties()) {
            e->undoChangeProperty(Pid::VOICE_ASSIGNMENT, mu::engraving::VoiceAssignment::CURRENT_VOICE_ONLY);
        }
    } else { // not added to a score so no need (and dangerous) to deal with undo stack
        e->setProperty(pid, newValue);
        e->setPropertyFlags(pid, newFlags);
    }
}

void ScoreElement::reset(mu::engraving::Pid pid)
{
    if (!e) {
        return;
    }

    if (m_ownership == Ownership::SCORE) {
        e->undoResetProperty(pid);
    } else {
        e->resetProperty(pid);
    }
}

//---------------------------------------------------------
//   wrap
///   \cond PLUGIN_API \private \endcond
///   Wraps mu::engraving::ScoreElement choosing the correct wrapper
///   type at runtime based on the actual element type.
//---------------------------------------------------------

ScoreElement* mu::engraving::apiv1::wrap(mu::engraving::EngravingObject* se, Ownership own)
{
    if (!se) {
        return nullptr;
    }
    if (se->isEngravingItem()) {
        return wrap(toEngravingItem(se), own);
    }

    using mu::engraving::ElementType;
    switch (se->type()) {
    case ElementType::SCORE:
        return wrap<mu::engraving::apiv1::Score>(toScore(se), own);
    case ElementType::PART:
        return wrap<mu::engraving::apiv1::Part>(toPart(se), own);
    case ElementType::STAFF:
        return wrap<mu::engraving::apiv1::Staff>(toStaff(se), own);
    default:
        break;
    }
    return wrap<ScoreElement>(se, own);
}
