//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MU_AUTOBOT_IABCONTEXT_H
#define MU_AUTOBOT_IABCONTEXT_H

#include <memory>
#include <vector>
#include <map>
#include <any>

#include "ret.h"

namespace mu::autobot {
class IAbContext
{
public:
    virtual ~IAbContext() = default;

    enum class Key {
        Undefined = 0,
        FilePath,
        FileIndex,
        ViewZoom,
        CurDrawData,
        RefDrawData,
        DiffDrawData
    };

    using Val = std::any;

    struct StepContext
    {
        std::string name;
        std::map<Key, Val> vals;
        Ret ret;
    };

    virtual const std::vector<StepContext>& steps() const = 0;
    virtual const StepContext& step(const std::string& name) const = 0;

    virtual void setGlobalVal(const Key& key, const Val& val) = 0;
    virtual Val globalVal(const Key& key) const = 0;

    // work with current (last) step
    virtual void addStep(const std::string& name) = 0;  // become current
    virtual const StepContext& currentStep() const = 0; // last step
    virtual void setStepVal(const Key& key, const Val& val) = 0;
    virtual void setStepRet(const Ret& ret) = 0;

    virtual Val stepVal(const std::string& stepName, const Key& key) const = 0;
    virtual Ret stepRet(const std::string& stepName) const = 0;

    virtual Val findVal(const Key& key) const = 0;

    virtual Ret completeRet() const = 0;

    // helpful
    template<typename T>
    T globalVal(const Key& key) const
    {
        return std::any_cast<T>(globalVal(key));
    }

    template<typename T>
    T findVal(const Key& key) const
    {
        return std::any_cast<T>(findVal(key));
    }
};
using IAbContextPtr = std::shared_ptr<IAbContext>;
}

#endif // MU_AUTOBOT_IABCONTEXT_H
