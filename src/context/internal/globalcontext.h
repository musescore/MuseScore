//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_CONTEXT_GLOBALCONTEXT_H
#define MU_CONTEXT_GLOBALCONTEXT_H

#include "../iglobalcontext.h"

namespace mu::context {
class GlobalContext : public IGlobalContext
{
public:
    void addMasterNotation(const notation::IMasterNotationPtr& notation) override;
    void removeMasterNotation(const notation::IMasterNotationPtr& notation) override;
    const std::vector<notation::IMasterNotationPtr>& masterNotations() const override;
    bool containsMasterNotation(const io::path& path) const override;

    void setCurrentMasterNotation(const notation::IMasterNotationPtr& notation) override;
    notation::IMasterNotationPtr currentMasterNotation() const override;
    async::Notification currentMasterNotationChanged() const override;

    void setCurrentNotation(const notation::INotationPtr& notation) override;
    notation::INotationPtr currentNotation() const override;
    async::Notification currentNotationChanged() const override;

private:
    void doSetCurrentNotation(const notation::INotationPtr& notation);

    std::vector<notation::IMasterNotationPtr> m_masterNotations;

    notation::IMasterNotationPtr m_currentMasterNotation;
    async::Notification m_currentMasterNotationChanged;

    notation::INotationPtr m_currentNotation;
    async::Notification m_currentNotationChanged;
};
}

#endif // MU_CONTEXT_GLOBALCONTEXT_H
