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
#ifndef MU_DRAW_DRAWDATAJSON_H
#define MU_DRAW_DRAWDATAJSON_H

#include "../types/drawdata.h"
#include "global/types/retval.h"

namespace mu {
class JsonObject;
}

namespace mu::draw {
class DrawDataJson
{
public:

    static ByteArray toJson(const DrawDataPtr& data, bool prettify = true);
    static RetVal<DrawDataPtr> fromJson(const ByteArray& json);

    static ByteArray diffToJson(const Diff& diff);
    static RetVal<Diff> diffFromJson(const ByteArray& json);

private:
    static void toJson(JsonObject& root, const DrawDataPtr& data);
    static void fromJson(const JsonObject& root, DrawDataPtr& data);
};
}
#endif // MU_DRAW_DRAWDATAJSON_H
