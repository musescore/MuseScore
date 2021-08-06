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

#ifndef MU_VST_VSTPLUGINLISTMODELEXAMPLE_H
#define MU_VST_VSTPLUGINLISTMODELEXAMPLE_H

#include <QObject>
#include <QVector>
#include <QList>
#include <QStringList>
#include <QString>

#include "async/asyncable.h"
#include "modularity/ioc.h"
#include "iinteractive.h"
#include "audio/itracks.h"
#include "audio/iaudiooutput.h"
#include "audio/iplayback.h"

#include "internal/vstplugin.h"
#include "ivstmodulesrepository.h"

namespace mu::vst {
class VstPluginListModelExample : public QObject, public async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(int currentSequenceId READ currentSequenceId WRITE setCurrentSequenceId NOTIFY currentSequenceIdChanged)
    Q_PROPERTY(int currentTrackId READ currentTrackId WRITE setCurrentTrackId NOTIFY currentTrackIdChanged)
    Q_PROPERTY(QString currentSynthResource READ currentSynthResource WRITE setCurrentSynthResource NOTIFY currentSynthResourceChanged)
    Q_PROPERTY(QString currentFxResource READ currentFxResource WRITE setCurrentFxResource NOTIFY currentFxResourceChanged)

    Q_PROPERTY(QVariantList sequenceIdList READ sequenceIdList WRITE setSequenceIdList NOTIFY sequenceIdListChanged)
    Q_PROPERTY(QVariantList trackIdList READ trackIdList WRITE setTrackIdList NOTIFY trackIdListChanged)
    Q_PROPERTY(QVariantList availableFxResources READ availableFxResources NOTIFY availableFxResourcesChanged)
    Q_PROPERTY(QVariantList availableSynthResources READ availableSynthResources NOTIFY availableSynthResourcesChanged)

    INJECT(vst, audio::IPlayback, playback)
    INJECT(vst, framework::IInteractive, interactive)
public:
    explicit VstPluginListModelExample(QObject* parent = nullptr);

    Q_INVOKABLE void load();
    Q_INVOKABLE void showSynthPluginEditor();
    Q_INVOKABLE void showFxPluginEditor();

    const QVariantList& sequenceIdList() const;
    void setSequenceIdList(const QVariantList& newSequenceIdList);

    const QVariantList& trackIdList() const;
    void setTrackIdList(const QVariantList& newTrackIdList);

    const QVariantList& availableFxResources() const;
    void setAvailableFxResources(const QVariantList& newAvailableFxResources);

    const QVariantList& availableSynthResources() const;
    void setAvailableSynthResources(const QVariantList& newAvailableSynthResources);

    int currentSequenceId() const;
    void setCurrentSequenceId(int newCurrentSequenceId);

    int currentTrackId() const;
    void setCurrentTrackId(int newCurrentTrackId);

    const QString& currentSynthResource() const;
    void setCurrentSynthResource(const QString& newCurrentSynthResource);

    const QString& currentFxResource() const;
    void setCurrentFxResource(const QString& newCurrentFxResource);

signals:
    void sequenceIdListChanged();
    void trackIdListChanged();
    void availableFxResourcesChanged();
    void availableSynthResourcesChanged();

    void currentSequenceIdChanged();
    void currentTrackIdChanged();
    void currentSynthResourceChanged();
    void currentFxResourceChanged();

private:
    void applyNewInputParams();
    void applyNewOutputParams();

    QVariantList m_sequenceIdList;
    QVariantList m_trackIdList;
    QVariantList m_availableFxResources;
    QVariantList m_availableSynthResources;
    int m_currentSequenceId = -1;
    int m_currentTrackId = -1;
    QString m_currentSynthResource;
    QString m_currentFxResource;
};
}

#endif // VSTPLUGINLISTMODELEXAMPLE_H
