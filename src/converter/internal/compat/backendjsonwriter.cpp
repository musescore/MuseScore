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
#include "backendjsonwriter.h"

#include <QIODevice>

using namespace mu::converter;

BackendJsonWriter::BackendJsonWriter(QIODevice* destinationDevice)
{
    m_destinationDevice = destinationDevice;
    m_destinationDevice->open(QIODevice::WriteOnly);
    m_destinationDevice->write("{\n");
}

BackendJsonWriter::~BackendJsonWriter()
{
    m_destinationDevice->write("\n}\n");
    m_destinationDevice->close();
}

void BackendJsonWriter::addKey(const char* arrayName)
{
    m_destinationDevice->write("\"");
    m_destinationDevice->write(arrayName);
    m_destinationDevice->write("\": ");
}

void BackendJsonWriter::addValue(const QByteArray& data, bool addSeparator, bool isJson)
{
    if (!isJson) {
        m_destinationDevice->write("\"");
    }
    m_destinationDevice->write(data);
    if (!isJson) {
        m_destinationDevice->write("\"");
    }
    if (addSeparator) {
        m_destinationDevice->write(",\n");
    }
}

void BackendJsonWriter::openArray()
{
    m_destinationDevice->write(" [");
}

void BackendJsonWriter::closeArray(bool addSeparator)
{
    m_destinationDevice->write("]");
    if (addSeparator) {
        m_destinationDevice->write(",");
    }
    m_destinationDevice->write("\n");
}
