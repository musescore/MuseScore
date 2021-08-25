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
#include "interactiveuriregister.h"

#include "log.h"

using namespace mu::ui;

void InteractiveUriRegister::registerUri(const Uri& uri, const ContainerMeta& meta)
{
    IF_ASSERT_FAILED(!m_uriHash.contains(uri)) {
        LOGW() << "URI " << uri.toString() << " already register. Will be rewrite";
    }

    m_uriHash[uri] = meta;
}

ContainerMeta InteractiveUriRegister::meta(const Uri& uri) const
{
    if (!m_uriHash.contains(uri)) {
        LOGW() << "URI " << uri.toString() << " not registered";
        return ContainerMeta();
    }

    return m_uriHash[uri];
}
