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

#ifndef MU_ENGRAVING_FACTORY_H
#define MU_ENGRAVING_FACTORY_H

#include <memory>
#include <QStringRef>
#include <QString>

#include "engravingitem.h"

namespace mu::engraving {
class Factory
{
public:

    static Ms::ElementType name2type(const QStringRef& name, bool silent = false);
    static Ms::ElementType name2type(const QString& name) { return name2type(QStringRef(&name)); }
    static const char* name(Ms::ElementType type);
    static const char* userName(Ms::ElementType type);

    static Ms::EngravingItem* createItem(Ms::ElementType type, Ms::EngravingItem* parent);
    static Ms::EngravingItem* createItemByName(const QStringRef& name, Ms::EngravingItem* parent);

    static Ms::Accidental* createAccidental(Ms::EngravingItem* parent);
    static std::shared_ptr<Ms::Accidental> makeAccidental(Ms::EngravingItem* parent);

private:
    static Ms::EngravingItem* doCreateItem(Ms::ElementType type, Ms::EngravingItem* parent);
};
}

#endif // MU_ENGRAVING_FACTORY_H
