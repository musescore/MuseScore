/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
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
#pragma once

#include "../imnxconfiguration.h"

#include "async/asyncable.h"

namespace mu::iex::mnxio {
class MnxConfiguration : public IMnxConfiguration, public muse::async::Asyncable
{
public:
    void init();

    int mnxIndentSpaces() const override;
    void setMnxIndentSpaces(int value) override;

    bool mnxExportBeams() const override;
    void setMnxExportBeams(bool value) override;

    bool mnxExportRestPositions() const override;
    void setMnxExportRestPositions(bool value) override;

    bool mnxRequireExactSchemaValidation() const override;
    void setMnxRequireExactSchemaValidation(bool value) override;
};
}
