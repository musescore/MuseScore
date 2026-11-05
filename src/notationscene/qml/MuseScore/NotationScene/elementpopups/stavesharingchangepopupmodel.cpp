/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

#include "stavesharingchangepopupmodel.h"

#include "engraving/dom/mscore.h"
#include "engraving/dom/part.h"
#include "engraving/dom/score.h"
#include "engraving/dom/sharedpart.h"
#include "engraving/dom/stavesharingchange.h"
#include "engraving/editing/editstavesharing.h"
#include "engraving/editing/transaction/transaction.h"

using namespace mu::notation;
using namespace mu::engraving;

TrackMappingItem::TrackMappingItem(QObject* parent)
    : QObject(parent)
{
}

QString TrackMappingItem::originPartName() const
{
    return m_originPartName;
}

void TrackMappingItem::setOriginPartName(const QString& name)
{
    m_originPartName = name;
}

int TrackMappingItem::staveIndex() const
{
    return m_staveIndex;
}

void TrackMappingItem::setStaveIndex(int staveIndex)
{
    if (m_staveIndex == staveIndex) {
        return;
    }

    m_staveIndex = staveIndex;
    emit staveIndexChanged();
}

int TrackMappingItem::voiceIndex() const
{
    return m_voiceIndex;
}

void TrackMappingItem::setVoiceIndex(int voiceIndex)
{
    if (m_voiceIndex == voiceIndex) {
        return;
    }

    m_voiceIndex = voiceIndex;
    emit voiceIndexChanged();
}

StaveSharingChangePopupModel::StaveSharingChangePopupModel(QObject* parent)
    : AbstractElementPopupModel(PopupModelType::TYPE_STAVE_SHARING_CHANGE, parent)
{
}

QString StaveSharingChangePopupModel::sharedPartName() const
{
    if (!m_item || !m_item->part()) {
        return QString();
    }

    return m_item->part()->partName().toQString();
}

bool StaveSharingChangePopupModel::isUserMapValid() const
{
    if (!m_item) {
        return false;
    }

    StaveSharingChange* change = toStaveSharingChange(m_item);
    const StaveSharingState* state = change ? change->staveSharingState() : nullptr;

    return state && (state->isReset() || state->isUserMapValid());
}

bool StaveSharingChangePopupModel::resetToDefault() const
{
    if (!m_item) {
        return true;
    }
    StaveSharingChange* change = toStaveSharingChange(m_item);

    if (!change->staveSharingState()) {
        return true;
    }

    return change->staveSharingState()->isReset();
}

void StaveSharingChangePopupModel::setResetToDefault(bool value)
{
    if (!m_item || resetToDefault() == value) {
        return;
    }

    StaveSharingChange* change = toStaveSharingChange(m_item);

    beginCommand(muse::TranslatableString("undoableAction", "Reset stave sharing change"));

    Transaction& tx = m_item->score()->transactionManager()->currentOrDummyTransaction();
    EditStaveSharing::setIsReset(tx, change, value);

    endCommand();
    updateNotation();

    emit resetToDefaultChanged();
    emit isUserMapValidChanged();

    refreshTrackMappingItems();
}

SharedPart* StaveSharingChangePopupModel::sharedPart() const
{
    Part* part = m_item ? m_item->part() : nullptr;
    if (!part || !part->isSharedPart()) {
        return nullptr;
    }

    return toSharedPart(part);
}

QList<TrackMappingItem*> StaveSharingChangePopupModel::trackMappings() const
{
    return m_trackMappings;
}

QVariantList StaveSharingChangePopupModel::staveOptions() const
{
    QVariantList options;

    SharedPart* part = sharedPart();
    size_t staveCount = part ? part->nstaves() : 0;

    for (size_t i = 0; i < staveCount; ++i) {
        QVariantMap option;
        option.insert("text", QString::number(i + 1));
        option.insert("value", static_cast<int>(i + 1));
        options << option;
    }

    return options;
}

QVariantList StaveSharingChangePopupModel::voiceOptions() const
{
    QVariantList options;

    for (size_t i = 0; i < VOICES; ++i) {
        QVariantMap option;
        option.insert("text", QString::number(i + 1));
        option.insert("value", static_cast<int>(i + 1));
        options << option;
    }

    return options;
}

void StaveSharingChangePopupModel::refreshTrackMappingItems()
{
    SharedPart* part = sharedPart();
    if (!part || !m_item) {
        return;
    }

    StaveSharingChange* change = toStaveSharingChange(m_item);
    if (!change->staveSharingState()) {
        return;
    }
    const SharedTrackMap map = change->staveSharingState()->userTrackMap().value_or(SharedTrackMap());

    staff_idx_t sharedFirstStaff = track2staff(part->trackRange().startTrack);

    bool rebuild = m_trackMappings.size() != static_cast<int>(part->originParts().size());
    if (rebuild) {
        qDeleteAll(m_trackMappings);
        m_trackMappings.clear();
    }

    int row = 0;
    for (Part* originPart : part->originParts()) {
        track_idx_t originTrack = originPart->trackRange().startTrack;

        TrackMappingItem* item = rebuild ? new TrackMappingItem(this) : m_trackMappings.at(row);
        item->originTrack = originTrack;
        item->setOriginPartName(originPart->partName().toQString());

        auto it = map.find(originTrack);
        if (it != map.end()) {
            staff_idx_t destStaff = track2staff(it->second);
            item->setStaveIndex(static_cast<int>(destStaff - sharedFirstStaff) + 1);
            item->setVoiceIndex(static_cast<int>(track2voice(it->second)) + 1);
        }

        if (rebuild) {
            m_trackMappings.push_back(item);
        }

        ++row;
    }

    if (rebuild) {
        emit trackMappingsChanged();
    }

    emit staveOptionsChanged();
}

void StaveSharingChangePopupModel::setTrackMapping(TrackMappingItem* item, track_idx_t sharedTrack)
{
    if (!m_item || !item) {
        return;
    }

    StaveSharingChange* change = toStaveSharingChange(m_item);

    beginCommand(muse::TranslatableString("undoableAction", "Change stave sharing mapping"));

    Transaction& tx = m_item->score()->transactionManager()->currentOrDummyTransaction();
    EditStaveSharing::setTrackMapping(tx, change->segment(), change->part(), item->originTrack, sharedTrack);

    endCommand();
    updateNotation();

    emit resetToDefaultChanged();
    emit isUserMapValidChanged();

    refreshTrackMappingItems();
}

void StaveSharingChangePopupModel::setTrackMappingStave(int row, int staveIndex)
{
    if (row < 0 || row >= m_trackMappings.size()) {
        return;
    }

    SharedPart* part = sharedPart();
    if (!part) {
        return;
    }

    TrackMappingItem* item = m_trackMappings.at(row);

    staff_idx_t sharedFirstStaff = track2staff(part->trackRange().startTrack);
    staff_idx_t destStaff = sharedFirstStaff + static_cast<staff_idx_t>(staveIndex - 1);
    voice_idx_t voice = static_cast<voice_idx_t>(item->voiceIndex() - 1);

    setTrackMapping(item, staff2track(destStaff, voice));
}

void StaveSharingChangePopupModel::setTrackMappingVoice(int row, int voiceIndex)
{
    if (row < 0 || row >= m_trackMappings.size()) {
        return;
    }

    SharedPart* part = sharedPart();
    if (!part) {
        return;
    }

    TrackMappingItem* item = m_trackMappings.at(row);

    staff_idx_t sharedFirstStaff = track2staff(part->trackRange().startTrack);
    staff_idx_t destStaff = sharedFirstStaff + static_cast<staff_idx_t>(item->staveIndex() - 1);
    voice_idx_t voice = static_cast<voice_idx_t>(voiceIndex - 1);

    setTrackMapping(item, staff2track(destStaff, voice));
}

void StaveSharingChangePopupModel::init()
{
    AbstractElementPopupModel::init();

    emit sharedPartNameChanged();
    emit isUserMapValidChanged();
    emit resetToDefaultChanged();

    refreshTrackMappingItems();
}
