/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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

#ifndef MU_PLAYBACK_INPUTRESOURCEITEM_H
#define MU_PLAYBACK_INPUTRESOURCEITEM_H

#include <map>
#include <optional>

#include <QObject>
#include <QString>

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "audio/iplayback.h"
#include "audio/audiotypes.h"
#include "midi/miditypes.h"

#include "abstractaudioresourceitem.h"

namespace mu::playback {
class InputResourceItem : public AbstractAudioResourceItem, public async::Asyncable
{
    Q_OBJECT

    INJECT(audio::IPlayback, playback)

public:
    explicit InputResourceItem(QObject* parent);

    void requestAvailableResources() override;
    void handleMenuItem(const QString& menuItemId) override;

    const audio::AudioInputParams& params() const;
    void setParams(const audio::AudioInputParams& newParams);

    QString title() const override;
    bool isBlank() const override;
    bool isActive() const override;
    bool hasNativeEditorSupport() const override;

signals:
    void inputParamsChanged();

private:
    using ResourceByVendorMap = std::map<audio::AudioResourceVendor, audio::AudioResourceMetaList>;

    QVariantMap buildMuseMenuItem(const ResourceByVendorMap& resourcesByVendor) const;
    QVariantMap buildVstMenuItem(const ResourceByVendorMap& resourcesByVendor) const;
    QVariantMap buildSoundFontsMenuItem(const ResourceByVendorMap& resourcesByVendor) const;
    QVariantMap buildMsBasicMenuItem(const audio::AudioResourceMetaList& availableResources, bool isCurrentSoundFont,
                                     const std::optional<midi::Program>& currentPreset) const;

    void updateCurrentParams(const audio::AudioResourceMeta& newMeta);
    void updateAvailableResources(const audio::AudioResourceMetaList& availableResources);

    std::map<audio::AudioResourceType, ResourceByVendorMap > m_availableResourceMap;
    audio::AudioInputParams m_currentInputParams;
};
}

#endif // MU_PLAYBACK_INPUTRESOURCEITEM_H
