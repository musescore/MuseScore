/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#ifndef MU_NOTATION_IEXCERPTNOTATION_H
#define MU_NOTATION_IEXCERPTNOTATION_H

#include "inotation.h"

namespace mu::notation {
class IExcerptNotation;
using IExcerptNotationPtr = std::shared_ptr<IExcerptNotation>;

class IExcerptNotation
{
public:
    virtual ~IExcerptNotation() = default;

    virtual bool isInited() const = 0;
    virtual bool isCustom() const = 0;
    virtual bool isEmpty() const = 0;

    virtual QString name() const = 0;
    virtual void setName(const QString& name) = 0; // not undoable
    virtual void undoSetName(const QString& name) = 0; // undoable
    virtual muse::async::Notification nameChanged() const = 0;

    virtual bool hasFileName() const = 0;
    virtual const muse::String& fileName() const = 0;

    virtual INotationPtr notation() = 0;
    virtual IExcerptNotationPtr clone() const = 0;
};
}

#endif // MU_NOTATION_IEXCERPTNOTATION_H
