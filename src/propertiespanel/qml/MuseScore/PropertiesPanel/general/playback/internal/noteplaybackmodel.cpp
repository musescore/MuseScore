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
#include "noteplaybackmodel.h"

#include <algorithm>
#include <cmath>

#include "translation.h"
#include "dataformatter.h"

#include "engraving/dom/note.h"
#include "engraving/types/types.h"

#include "mpe/mpetypes.h"

#include "notation/imasternotation.h"
#include "notation/inotationplayback.h"

using namespace mu::propertiespanel;

NotePlaybackModel::NotePlaybackModel(QObject* parent, const muse::modularity::ContextPtr& iocCtx, IElementRepositoryService* repository)
    : PropertiesPanelAbstractModel(parent, iocCtx, repository)
{
    setTitle(muse::qtrc("propertiespanel", "Notes"));
    setModelType(PropertiesPanelModelType::TYPE_NOTE);

    createProperties();
}

void NotePlaybackModel::createProperties()
{
    m_tuning = buildPropertyItem(mu::engraving::Pid::TUNING);

    // Redirected to a dedicated callback instead of the default setPropertyValue() (which only
    // ever writes the one Pid it's given) - dragging the on-canvas velocity bar always ends up as
    // an absolute VeloType::USER_VAL (see NotationNoteVelocityController::onBarDragged()), and
    // this mirrors that here too. Without it, typing a value into this spinbox for a
    // VeloType::OFFSET_VAL note (userVelocity() is a *percentage* nudge on the dynamics-derived
    // context for that type, not an absolute value) would leave VELO_TYPE untouched, silently
    // reinterpreting the just-typed absolute number as a percentage the next time it's read.
    auto onVelocityChanged = [this](const mu::engraving::Pid pid, const QVariant& newValue) {
        if (m_elementList.empty()) {
            return;
        }

        beginCommand(muse::TranslatableString("undoableAction", "Change note velocity"));

        for (mu::engraving::EngravingItem* item : m_elementList) {
            IF_ASSERT_FAILED(item) {
                continue;
            }
            mu::engraving::Note* note = item->isNote() ? mu::engraving::toNote(item) : nullptr;
            if (!note) {
                continue;
            }

            if (note->getProperty(mu::engraving::Pid::VELO_TYPE).value<mu::engraving::VeloType>()
                != mu::engraving::VeloType::USER_VAL) {
                note->undoChangeProperty(mu::engraving::Pid::VELO_TYPE, mu::engraving::VeloType::USER_VAL,
                                         mu::engraving::PropertyFlags::NOSTYLE);
            }

            mu::engraving::PropertyFlags ps = item->propertyFlags(pid);
            if (ps == mu::engraving::PropertyFlags::STYLED) {
                ps = mu::engraving::PropertyFlags::UNSTYLED;
            }
            item->undoChangeProperty(pid, valueToElementUnits(pid, newValue, item), ps);
        }

        updateNotation();
        endCommand();

        loadProperties();
    };
    m_velocity = buildPropertyItem(mu::engraving::Pid::USER_VELOCITY, onVelocityChanged);

    // Redirected to each note's own chain head (see headNoteElements()) instead of the default
    // callback, which would write to the exact selected note.
    auto onOffsetChanged = [this](const mu::engraving::Pid pid, const QVariant& newValue) {
        setPropertyValue(headNoteElements(), pid, newValue);
        loadProperties();
    };
    m_playbackStartOffset = buildPropertyItem(mu::engraving::Pid::PLAYBACK_START_OFFSET, onOffsetChanged);
    m_playbackDurationOffset = buildPropertyItem(mu::engraving::Pid::PLAYBACK_DURATION_OFFSET, onOffsetChanged);
}

void NotePlaybackModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(mu::engraving::ElementType::NOTEHEAD);
}

void NotePlaybackModel::loadProperties()
{
    loadPropertyItem(m_tuning, formatDoubleFunc);
    loadVelocityProperty();
    loadPropertyItem(m_playbackStartOffset, headNoteElements());
    loadPropertyItem(m_playbackDurationOffset, headNoteElements());
}

void NotePlaybackModel::loadVelocityProperty()
{
    // loadPropertyItem()'s convertElementPropertyValueFunc only ever receives the already-read
    // property value, with no way back to which element it came from - not enough to compute a
    // per-note contextual fallback, so this walks m_elementList directly instead.
    if (m_elementList.isEmpty()) {
        m_velocity->setIsEnabled(false);
        return;
    }

    QVariant value;
    bool isUndefined = false;
    bool isModified = false;

    for (mu::engraving::EngravingItem* item : m_elementList) {
        IF_ASSERT_FAILED(item) {
            continue;
        }

        mu::engraving::Note* note = item->isNote() ? mu::engraving::toNote(item) : nullptr;
        if (!note) {
            continue;
        }

        const int elementValue = effectiveVelocity(note);

        if (!value.isValid()) {
            value = elementValue;
        } else if (!isUndefined && value.toInt() != elementValue) {
            isUndefined = true;
        }

        if (!isModified && note->userVelocity() != 0) {
            isModified = true;
        }
    }

    // The displayed number alone can't distinguish "still following the dynamic context" from
    // "just pinned explicitly to the same number that context happened to produce" - e.g. dragging
    // a forte note's velocity bar to exactly 96 doesn't change what's displayed (96 both before and
    // after), so the plain value-equality check in updateCurrentValue() would otherwise skip
    // notifying entirely. Force the notification through whenever isModified is about to flip, so
    // the spinbox never silently disagrees with the (always-correct) isModified-driven color.
    const bool forceNotify = m_velocity->isModified() != isModified;

    m_velocity->setIsEnabled(value.isValid());
    m_velocity->updateCurrentValue(isUndefined ? QVariant() : value, forceNotify);
    m_velocity->setIsModified(isModified);
}

int NotePlaybackModel::contextVelocity(const mu::engraving::Note* note) const
{
    // What the dynamics-marking/hairpin context alone would produce at this note's tick, with no
    // per-note override - falls back to a flat constant only when there's no playback available
    // to ask (mirrors NotationNoteVelocityController::contextVelocity()).
    const notation::IMasterNotationPtr masterNotation = context()->currentMasterNotation();
    const notation::INotationPlaybackPtr playback = masterNotation ? masterNotation->playback() : nullptr;
    if (!playback) {
        return 64;
    }

    const muse::mpe::dynamic_level_t level = playback->appliableDynamicLevel(note->track(), note->tick().ticks());
    const double ratio = muse::mpe::dynamicLevelToVelocityRatio(level);
    return std::clamp(static_cast<int>(std::lround(ratio * 127.0)), 0, 127);
}

int NotePlaybackModel::effectiveVelocity(const mu::engraving::Note* note) const
{
    if (!note) {
        return 64;
    }

    const int userVelocity = note->userVelocity();
    if (userVelocity == 0) {
        // No explicit velocity set on this note - fall back to the same dynamics-derived value
        // the on-canvas velocity-bar overlay already shows, instead of a flat constant that
        // ignores whatever dynamic (piano, forte...) actually applies.
        return contextVelocity(note);
    }

    // Note::customizeVelocity(): VeloType::USER_VAL means userVelocity() IS the absolute value,
    // but VeloType::OFFSET_VAL means it's a *percentage* nudge applied on top of the dynamic
    // context (velo += velo * userVelocity() / 100) - treating it as absolute here would show a
    // value with no relation to either the percentage or what actually plays, and disagree with
    // NotationNoteVelocityController::displayedVelocity(), which this is meant to mirror.
    const mu::engraving::VeloType veloType = note->getProperty(mu::engraving::Pid::VELO_TYPE).value<mu::engraving::VeloType>();
    if (veloType == mu::engraving::VeloType::USER_VAL) {
        return userVelocity;
    }

    const int context = contextVelocity(note);
    const int offset = static_cast<int>(std::lround(context * userVelocity / 100.0));
    return std::clamp(context + offset, 0, 127);
}

void NotePlaybackModel::onNotationChanged(const mu::engraving::PropertyIdSet&, const mu::engraving::StyleIdSet&)
{
    loadProperties();
}

QList<mu::engraving::EngravingItem*> NotePlaybackModel::headNoteElements() const
{
    QList<mu::engraving::EngravingItem*> result;
    result.reserve(m_elementList.size());

    for (mu::engraving::EngravingItem* item : m_elementList) {
        mu::engraving::Note* note = item && item->isNote() ? mu::engraving::toNote(item) : nullptr;
        if (!note) {
            result.push_back(item);
            continue;
        }

        mu::engraving::Note* head = note->firstTiedNote(/*ignorePlayback*/ false);
        mu::engraving::Note* tail = note->lastTiedNote(/*ignorePlayback*/ false);

        // A note buried in the middle of a longer tie chain (neither the chain's head nor its
        // tail) owns neither edge of the overlay's rectangle for that chain - it's excluded here
        // entirely, rather than merely redirected, so both spinboxes read as disabled instead of
        // silently editing a value this note has no visual handle for.
        if (note != head && note != tail) {
            continue;
        }

        result.push_back(head);
    }

    return result;
}

PropertyItem* NotePlaybackModel::tuning() const
{
    return m_tuning;
}

PropertyItem* NotePlaybackModel::velocity() const
{
    return m_velocity;
}

PropertyItem* NotePlaybackModel::playbackStartOffset() const
{
    return m_playbackStartOffset;
}

PropertyItem* NotePlaybackModel::playbackDurationOffset() const
{
    return m_playbackDurationOffset;
}
