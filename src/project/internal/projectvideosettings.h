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

#include <QJsonObject>

#include "types/ret.h"
#include "engraving/infrastructure/mscreader.h"
#include "engraving/infrastructure/mscwriter.h"

#include "../iprojectvideosettings.h"

namespace mu::project {
class ProjectVideoSettings : public IProjectVideoSettings
{
public:
    const VideoAttachmentSettings& attachment() const override;
    void setAttachment(const VideoAttachmentSettings& attachment) override;
    void clearAttachment() override;

    muse::async::Notification settingsChanged() const override;

    muse::Ret read(const engraving::MscReader& reader);
    muse::Ret write(engraving::MscWriter& writer) const;

    void makeDefault();

private:
    VideoHitPointSettings hitPointFromJson(const QJsonObject& object) const;
    QJsonObject hitPointToJson(const VideoHitPointSettings& hitPoint) const;
    VideoAttachmentSettings attachmentFromJson(const QJsonObject& object) const;
    QJsonObject attachmentToJson(const VideoAttachmentSettings& attachment) const;

    VideoAttachmentSettings m_attachment;
    muse::async::Notification m_settingsChanged;
};

using ProjectVideoSettingsPtr = std::shared_ptr<ProjectVideoSettings>;
}
