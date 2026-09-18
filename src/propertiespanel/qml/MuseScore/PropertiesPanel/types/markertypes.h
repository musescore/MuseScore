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

#include "engraving/types/types.h"

namespace mu::propertiespanel {
namespace MarkerTypes {
Q_NAMESPACE;
QML_ELEMENT;

enum class Type {
    TYPE_SEGNO = int(engraving::MarkerType::SEGNO),
    TYPE_VARSEGNO = int(engraving::MarkerType::VARSEGNO),
    TYPE_CODA = int(engraving::MarkerType::CODA),
    TYPE_VARCODA = int(engraving::MarkerType::VARCODA),
    TYPE_CODETTA = int(engraving::MarkerType::CODETTA),
    TYPE_FINE = int(engraving::MarkerType::FINE),
    TYPE_TOCODA = int(engraving::MarkerType::TOCODA),
    TYPE_USER = int(engraving::MarkerType::USER)
};

Q_ENUM_NS(Type)
}
}
