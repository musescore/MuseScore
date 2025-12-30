/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "types/projecttypes.h" // IWYU pragma: keep

namespace mu::project {
namespace GenerateAudioTimePeriodTypeForeign {
Q_NAMESPACE;
QML_FOREIGN_NAMESPACE(mu::project::_GenerateAudioTimePeriodType);
QML_NAMED_ELEMENT(GenerateAudioTimePeriodType)
}

namespace MigrationTypeForeign {
Q_NAMESPACE;
QML_FOREIGN_NAMESPACE(mu::project::_MigrationType);
QML_NAMED_ELEMENT(MigrationType)
}

namespace SaveLocationTypeForeign {
Q_NAMESPACE;
QML_FOREIGN_NAMESPACE(mu::project::_SaveLocationType);
QML_NAMED_ELEMENT(SaveLocationType)
}
}
