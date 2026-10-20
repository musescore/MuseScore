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

#include <QList>
#include <QObject>
#include <QVariantList>
#include <qqmlintegration.h>

#include "engraving/types/types.h"

#include "../abstractelementpopupmodel.h"

namespace mu::engraving {
class SharedPart;
}

namespace mu::notation {
class TrackMappingItem : public QObject
{
    Q_OBJECT
    QML_ELEMENT;

    Q_PROPERTY(QString originPartName READ originPartName CONSTANT)
    Q_PROPERTY(int staveIndex READ staveIndex NOTIFY staveIndexChanged)
    Q_PROPERTY(int voiceIndex READ voiceIndex NOTIFY voiceIndexChanged)

public:
    explicit TrackMappingItem(QObject* parent = nullptr);

    QString originPartName() const;
    void setOriginPartName(const QString& name);

    int staveIndex() const;
    void setStaveIndex(int staveIndex);

    int voiceIndex() const;
    void setVoiceIndex(int voiceIndex);

    mu::engraving::track_idx_t originTrack = muse::nidx;

signals:
    void staveIndexChanged();
    void voiceIndexChanged();

private:
    QString m_originPartName;
    int m_staveIndex = 1;
    int m_voiceIndex = 1;
};

class StaveSharingChangePopupModel : public AbstractElementPopupModel
{
    Q_OBJECT

    Q_PROPERTY(QString sharedPartName READ sharedPartName NOTIFY sharedPartNameChanged)
    Q_PROPERTY(bool isUserMapValid READ isUserMapValid NOTIFY isUserMapValidChanged)
    Q_PROPERTY(bool resetToDefault READ resetToDefault WRITE setResetToDefault NOTIFY resetToDefaultChanged)
    Q_PROPERTY(QList<TrackMappingItem*> trackMappings READ trackMappings NOTIFY trackMappingsChanged)
    Q_PROPERTY(QVariantList staveOptions READ staveOptions NOTIFY staveOptionsChanged)
    Q_PROPERTY(QVariantList voiceOptions READ voiceOptions CONSTANT)

    QML_ELEMENT

public:
    explicit StaveSharingChangePopupModel(QObject* parent = nullptr);

    QString sharedPartName() const;
    bool isUserMapValid() const;
    bool resetToDefault() const;
    QList<TrackMappingItem*> trackMappings() const;
    QVariantList staveOptions() const;
    QVariantList voiceOptions() const;

    Q_INVOKABLE void init() override;
    Q_INVOKABLE void setTrackMappingStave(int row, int staveIndex);
    Q_INVOKABLE void setTrackMappingVoice(int row, int voiceIndex);

public slots:
    void setResetToDefault(bool value);

signals:
    void sharedPartNameChanged();
    void isUserMapValidChanged();
    void resetToDefaultChanged();
    void trackMappingsChanged();
    void staveOptionsChanged();

private:
    mu::engraving::SharedPart* sharedPart() const;
    void refreshTrackMappingItems();
    void setTrackMapping(TrackMappingItem* item, mu::engraving::track_idx_t sharedTrack);

    QList<TrackMappingItem*> m_trackMappings;
};
}
