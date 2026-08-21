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

#include "translation.h"
#include "dataformatter.h"

#include "engraving/dom/note.h"

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
    m_velocity = buildPropertyItem(mu::engraving::Pid::USER_VELOCITY);

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
    loadPropertyItem(m_velocity, [](const QVariant& value) {
        //! NOTE: display 64 instead of 0 in the Velocity field to avoid confusing the user
        return value.toInt() == 0 ? 64 : value;
    });
    loadPropertyItem(m_playbackStartOffset, headNoteElements());
    loadPropertyItem(m_playbackDurationOffset, headNoteElements());
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
