/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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

#include "global/iglobalconfiguration.h"
#include "iinteractive.h"

#include "audio/iplayback.h"
#include "audio/audiotypes.h"
#include "midi/miditypes.h"

#include "abstractaudioresourceitem.h"

namespace mu::playback {
class InputResourceItem : public AbstractAudioResourceItem
{
    Q_OBJECT

    INJECT(muse::IGlobalConfiguration, globalConfiguration)
    INJECT(muse::IInteractive, interactive)
    INJECT(muse::audio::IPlayback, playback)

public:
    explicit InputResourceItem(QObject* parent);

    void requestAvailableResources() override;
    void handleMenuItem(const QString& menuItemId) override;

    const muse::audio::AudioInputParams& params() const;
    void setParams(const muse::audio::AudioInputParams& newParams);
    void setParamsRecourceMeta(const muse::audio::AudioResourceMeta& newMeta);

    QString title() const override;
    bool isBlank() const override;
    bool isActive() const override;
    bool hasNativeEditorSupport() const override;

signals:
    void inputParamsChanged();
    void inputParamsChangeRequested(const muse::audio::AudioResourceMeta& newMeta);

private:
    using ResourceByVendorMap = std::map<muse::audio::AudioResourceVendor, muse::audio::AudioResourceMetaList>;

    QVariantMap buildMuseMenuItem(const ResourceByVendorMap& resourcesByVendor) const;
    QVariantMap buildVstMenuItem(const ResourceByVendorMap& resourcesByVendor) const;
    QVariantMap buildSoundFontsMenuItem(const ResourceByVendorMap& resourcesByVendor) const;
    QVariantMap buildMsBasicMenuItem(const muse::audio::AudioResourceMetaList& availableResources, bool isCurrentSoundFont,
                                     const std::optional<muse::midi::Program>& currentPreset) const;
    QVariantMap buildSoundFontMenuItem(const muse::String& soundFont, const muse::audio::AudioResourceMetaList& availableResources,
                                       bool isCurrentSoundFont, const std::optional<muse::midi::Program>& currentPreset) const;

    void updateAvailableResources(const muse::audio::AudioResourceMetaList& availableResources);

    std::map<muse::audio::AudioResourceType, ResourceByVendorMap > m_availableResourceMap;
    muse::audio::AudioInputParams m_currentInputParams;
};
}

#endif // MU_PLAYBACK_INPUTRESOURCEITEM_H
