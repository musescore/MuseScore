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
#ifndef MU_UI_INTERACTIVEURIREGISTER_H
#define MU_UI_INTERACTIVEURIREGISTER_H

#include "iinteractiveuriregister.h"

namespace mu::ui {
class InteractiveUriRegister : public IInteractiveUriRegister
{
public:
    void registerUri(const Uri& uri, const ContainerMeta& meta) override;
    ContainerMeta meta(const Uri& uri) const override;

private:
    QHash<Uri, ContainerMeta> m_uriHash;
};
}

namespace mu {
#ifdef MU_QT5_COMPAT
inline uint qHash(const Uri& uri)
{
    return qHash(QString::fromStdString(uri.toString()));
}

#else
inline size_t qHash(const Uri& uri)
{
    return qHash(QString::fromStdString(uri.toString()));
}

#endif
}

#endif // MU_UI_INTERACTIVEURIREGISTER_H
