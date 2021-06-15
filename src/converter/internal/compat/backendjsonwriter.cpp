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
#include "backendjsonwriter.h"

using namespace mu::converter;

BackendJsonWriter::BackendJsonWriter(const io::path& filePath)
{
    jsonFormatFile.setFileName(filePath.toQString());
    jsonFormatFile.open(QIODevice::WriteOnly);
    jsonFormatFile.write("{\n");
}

BackendJsonWriter::~BackendJsonWriter()
{
    jsonFormatFile.write("\n}\n");
    jsonFormatFile.close();
}

void BackendJsonWriter::addKey(const char* arrayName)
{
    jsonFormatFile.write("\"");
    jsonFormatFile.write(arrayName);
    jsonFormatFile.write("\": ");
}

void BackendJsonWriter::addValue(const QByteArray& data, bool lastJsonElement, bool isJson)
{
    if (!isJson) {
        jsonFormatFile.write("\"");
    }
    jsonFormatFile.write(data);
    if (!isJson) {
        jsonFormatFile.write("\"");
    }
    if (!lastJsonElement) {
        jsonFormatFile.write(",\n");
    }
}

void BackendJsonWriter::openArray()
{
    jsonFormatFile.write(" [");
}

void BackendJsonWriter::closeArray(bool lastJsonElement)
{
    jsonFormatFile.write("]");
    if (!lastJsonElement) {
        jsonFormatFile.write(",");
    }
    jsonFormatFile.write("\n");
}
