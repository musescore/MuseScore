/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <qqmlintegration.h>

#include "types/migrationtypes.h"
#include "types/projecttypes.h"
#include "types/savelocation.h"

namespace mu::project {
namespace GenerateAudioTimePeriodTypeQml {
Q_NAMESPACE;
QML_NAMED_ELEMENT(GenerateAudioTimePeriodType)

enum class Type {
    Never = static_cast<int>(GenerateAudioTimePeriodType::Never),
    Always = static_cast<int>(GenerateAudioTimePeriodType::Always),
    AfterCertainNumberOfSaves = static_cast<int>(GenerateAudioTimePeriodType::AfterCertainNumberOfSaves)
};
Q_ENUM_NS(Type)
}

namespace MigrationTypeQml {
Q_NAMESPACE;
QML_NAMED_ELEMENT(MigrationType)

enum class Type {
    Unknown = static_cast<int>(MigrationType::Unknown),
    Pre_3_6 = static_cast<int>(MigrationType::Pre_3_6),
    Ver_3_6 = static_cast<int>(MigrationType::Ver_3_6)
};
Q_ENUM_NS(Type)
}

namespace SaveLocationTypeQml {
Q_NAMESPACE;
QML_NAMED_ELEMENT(SaveLocationType)

enum class Type {
    Undefined = static_cast<int>(SaveLocationType::Undefined),
    Local = static_cast<int>(SaveLocationType::Local),
    Cloud = static_cast<int>(SaveLocationType::Cloud)
};
Q_ENUM_NS(Type)
}
}
