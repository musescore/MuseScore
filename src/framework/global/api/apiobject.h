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
#ifndef MUSE_API_APIOBJECT_H
#define MUSE_API_APIOBJECT_H

#include <QObject>

#include "iapiengine.h"
#include "modularity/ioc.h"

//! NOTE This class requires a `cpp` file.
//! If we move it to the `api` module, we will have to link it to all other modules.
//! That’s why it’s located here, because the `global` module links to everything.

namespace muse::api {
class ApiObject : public QObject, public Injectable
{
    Q_OBJECT

public:
    explicit ApiObject(IApiEngine* e);

    IApiEngine* engine() const;

private:

    IApiEngine* m_engine = nullptr;
};
}

#endif // MUSE_API_APIOBJECT_H
