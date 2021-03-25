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
#ifndef MU_AUTOBOT_ABCONTEXT_H
#define MU_AUTOBOT_ABCONTEXT_H

#include "../iabcontext.h"

namespace mu::autobot {
struct AbContext : public IAbContext
{
public:
    AbContext() = default;

    const std::vector<StepContext>& steps() const override;
    const StepContext& step(const std::string& name) const override;

    void setGlobalVal(const Key& key, const Val& val) override;
    Val globalVal(const Key& key) const override;

    void addStep(const std::string& name) override;
    const StepContext& currentStep() const override;
    void setStepVal(const Key& key, const Val& val) override;
    void setStepRet(const Ret& ret) override;

    Val stepVal(const std::string& stepName, const Key& key) const override;
    Ret stepRet(const std::string& stepName) const override;

    Val findVal(const Key& key) const override;

    Ret completeRet() const override;

private:

    std::map<Key, Val > m_globalVals;
    std::vector<StepContext> m_steps;
};
}
#endif // MU_AUTOBOT_ABCONTEXT_H
