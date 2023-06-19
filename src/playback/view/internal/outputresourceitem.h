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

#ifndef MU_PLAYBACK_OUTPUTRESOURCEITEM_H
#define MU_PLAYBACK_OUTPUTRESOURCEITEM_H

#include <QObject>
#include <QString>
#include <map>

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "audio/iaudiooutput.h"
#include "audio/iplayback.h"
#include "audio/audiotypes.h"

#include "abstractaudioresourceitem.h"

#if (defined(_MSCVER) || defined(_MSC_VER))
// unreferenced function with internal linkage has been removed
#pragma warning(disable: 4505)
#endif

namespace mu::playback {
class OutputResourceItem : public AbstractAudioResourceItem, public async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(QString id READ id NOTIFY fxParamsChanged)
    Q_PROPERTY(bool isActive READ isActive WRITE setIsActive NOTIFY isActiveChanged)

    INJECT(audio::IPlayback, playback)

public:
    explicit OutputResourceItem(QObject* parent, const audio::AudioFxParams& params);

    void requestAvailableResources() override;
    void handleMenuItem(const QString& menuItemId) override;

    const audio::AudioFxParams& params() const;
    void setParams(const audio::AudioFxParams& params);

    QString title() const override;
    bool isBlank() const override;
    bool isActive() const override;
    bool hasNativeEditorSupport() const override;

    QString id() const;

public slots:
    void setIsActive(bool newIsActive);

signals:
    void fxParamsChanged();

private:
    void updateCurrentFxParams(const audio::AudioResourceMeta& newMeta);
    void updateAvailableFxVendorsMap(const audio::AudioResourceMetaList& availableFxResources);

    std::map<audio::AudioResourceVendor, audio::AudioResourceMetaList> m_fxByVendorMap;

    audio::AudioFxParams m_currentFxParams;
};
}

#endif // MU_PLAYBACK_OUTPUTRESOURCEITEM_H
