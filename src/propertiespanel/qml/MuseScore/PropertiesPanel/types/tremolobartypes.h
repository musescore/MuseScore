/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

#include "engraving/dom/tremolobar.h"

namespace mu::propertiespanel {
namespace TremoloBarTypes {
Q_NAMESPACE;
QML_ELEMENT;

enum class TremoloBarType {
    TYPE_DIP = int(engraving::TremoloBarType::DIP),
    TYPE_DIVE = int(engraving::TremoloBarType::DIVE),
    TYPE_RELEASE_UP = int(engraving::TremoloBarType::RELEASE_UP),
    TYPE_INVERTED_DIP = int(engraving::TremoloBarType::INVERTED_DIP),
    TYPE_RETURN = int(engraving::TremoloBarType::RETURN),
    TYPE_RELEASE_DOWN = int(engraving::TremoloBarType::RELEASE_DOWN),
    TYPE_CUSTOM = int(engraving::TremoloBarType::CUSTOM)
};

Q_ENUM_NS(TremoloBarType)
}
}
