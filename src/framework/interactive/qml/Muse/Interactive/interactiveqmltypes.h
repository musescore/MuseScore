/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <qqmlintegration.h>

#include "interactivetypes.h"

namespace muse::interactive {
namespace ContainerType {
Q_NAMESPACE;
QML_ELEMENT;

enum Type {
    Undefined = ContainerMeta::Type::Undefined,
    PrimaryPage = ContainerMeta::Type::PrimaryPage,
    QmlDialog = ContainerMeta::Type::QmlDialog,
    QWidgetDialog = ContainerMeta::Type::QWidgetDialog
};
Q_ENUM_NS(Type)
}
}
