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
#ifndef MUSE_CLOUD_CLOUDQMLTYPES_H
#define MUSE_CLOUD_CLOUDQMLTYPES_H

#include "cloudtypes.h"

namespace muse::cloud {
class QMLCloudVisibility
{
    Q_GADGET

public:
    enum CloudVisibility {
        Public = int(cloud::Visibility::Public),
        Unlisted = int(cloud::Visibility::Unlisted),
        Private = int(cloud::Visibility::Private)
    };
    Q_ENUM(CloudVisibility);
};

class QMLSaveToCloudResponse
{
    Q_GADGET

public:
    enum SaveToCloudResponse {
        Cancel,
        Ok,
        SaveLocallyInstead
    };
    Q_ENUM(SaveToCloudResponse);
};
}

#endif // MUSE_CLOUD_CLOUDQMLTYPES_H
