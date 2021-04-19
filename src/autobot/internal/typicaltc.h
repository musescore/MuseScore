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
#ifndef MU_AUTOBOT_TYPICALTC_H
#define MU_AUTOBOT_TYPICALTC_H

#include "../itestcase.h"

namespace mu::autobot {
class TypicalTC : public ITestCase
{
public:
    TypicalTC() = default;
    TypicalTC(const std::string& name, std::vector<ITestStep*> steps);

    std::string name() const override;
    const std::vector<ITestStepPtr>& steps() const override;

private:
    std::string m_name;
    std::vector<ITestStepPtr> m_steps;
};
}

#endif // MU_AUTOBOT_TYPICALTC_H
