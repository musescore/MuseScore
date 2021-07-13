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
#ifndef MU_CONTEXT_IGLOBALCONTEXT_H
#define MU_CONTEXT_IGLOBALCONTEXT_H

#include "modularity/imoduleexport.h"
#include "notation/inotationproject.h"
#include "async/notification.h"
#include "io/path.h"

namespace mu::context {
class IGlobalContext : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(mu::context::IGlobalContext)

public:
    virtual ~IGlobalContext() = default;

    virtual void addNotationProject(const notation::INotationProjectPtr& project) = 0;
    virtual void removeNotationProject(const notation::INotationProjectPtr& project) = 0;
    virtual const std::vector<notation::INotationProjectPtr>& notationProjects() const = 0;
    virtual bool containsNotationProject(const io::path& path) const = 0;

    virtual void setCurrentNotationProject(const notation::INotationProjectPtr& project) = 0;
    virtual notation::INotationProjectPtr currentNotationProject() const = 0;
    virtual async::Notification currentNotationProjectChanged() const = 0;

    virtual notation::IMasterNotationPtr currentMasterNotation() const = 0;
    virtual async::Notification currentMasterNotationChanged() const = 0;

    virtual void setCurrentNotation(const notation::INotationPtr& notation) = 0;
    virtual notation::INotationPtr currentNotation() const = 0;
    virtual async::Notification currentNotationChanged() const = 0;
};
}

#endif // MU_CONTEXT_IGLOBALCONTEXT_H
