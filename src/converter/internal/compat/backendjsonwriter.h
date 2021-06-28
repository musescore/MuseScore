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
#ifndef MU_CONVERTER_BACKENDJSONWRITER_H
#define MU_CONVERTER_BACKENDJSONWRITER_H

#include "io/path.h"
#include "io/device.h"

namespace mu::converter {
class BackendJsonWriter
{
public:
    BackendJsonWriter(io::Device* destinationDevice);
    ~BackendJsonWriter();

    void addKey(const char* arrayName);
    void addValue(const QByteArray& data, bool addSeparator = false, bool isJson = false);

    void openArray();
    void closeArray(bool addSeparator = false);

private:
    io::Device* m_destinationDevice = nullptr;
};
}

#endif // MU_CONVERTER_BACKENDJSONWRITER_H
