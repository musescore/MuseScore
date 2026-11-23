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
#pragma once

#include <qqmlintegration.h>

#include <QVariantList>

#include "propertiespanelabstractmodel.h"

#include "engraving/playback/noteplaybackinfo.h"

namespace mu::propertiespanel {
//! Read-only summary of what playback will do with a single selected note
class NotePlaybackInfoModel : public PropertiesPanelAbstractModel
{
    Q_OBJECT
    QML_ELEMENT;
    QML_UNCREATABLE("Not creatable from QML")

    Q_PROPERTY(QVariantList passes READ passes NOTIFY infoChanged)
    Q_PROPERTY(bool isMuted READ isMuted NOTIFY infoChanged)
    Q_PROPERTY(QString muteReason READ muteReason NOTIFY infoChanged)
    Q_PROPERTY(int velocityOverride READ velocityOverride NOTIFY infoChanged)
    Q_PROPERTY(bool isTiedContinuation READ isTiedContinuation NOTIFY infoChanged)

public:
    explicit NotePlaybackInfoModel(
        QObject* parent, const muse::modularity::ContextPtr& iocCtx, IElementRepositoryService* repository);

    QVariantList passes() const { return m_passes; }
    bool isMuted() const { return m_isMuted; }
    QString muteReason() const { return m_muteReason; }
    int velocityOverride() const { return m_velocityOverride; }
    bool isTiedContinuation() const { return m_isTiedContinuation; }

    bool isEmpty() const override
    {
        // Single note only
        return m_elementList.size() != 1;
    }

    bool shouldUpdateOnEmptyPropertyAndStyleIdSets() const override
    {
        // A dynamic/tempo change anywhere before the note can change what it plays at
        return true;
    }

    void createProperties() override {}
    void requestElements() override;
    void loadProperties() override;
    void onCurrentNotationChanged() override;
    void onNotationChanged(const mu::engraving::PropertyIdSet&, const mu::engraving::StyleIdSet&) override;

signals:
    void infoChanged();

private:
    const mu::engraving::Note* selectedNote() const;
    void reload();
    void updateMuteState(const mu::engraving::Note* note);

    QVariantList m_passes;
    bool m_isMuted = false;
    QString m_muteReason;
    int m_velocityOverride = 0;
    bool m_isTiedContinuation = false;
};
} // namespace mu::propertiespanel
