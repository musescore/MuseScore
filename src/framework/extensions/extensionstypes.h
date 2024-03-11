/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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
#ifndef MU_EXTENSIONS_EXTENSIONSTYPES_H
#define MU_EXTENSIONS_EXTENSIONSTYPES_H

#include <vector>
#include "global/types/uri.h"
#include "global/types/string.h"

namespace mu::extensions {
/*
manifest.json
{

"uri": String,                    // Example: musescore://module/target/name
"type": String,                   // Values: form
"title": String,                  //
"apiversion": String              // Optional default 1
"enabled": Boolean,               // Optional default true
"visible": Boolean,               // Optional default true
}*/

enum class Type {
    Undefined = 0,
    Form
};

static inline Type typeFromString(const std::string& str)
{
    if (str == "form") {
        return Type::Form;
    }
    return Type::Undefined;
}

static inline std::string typeToString(const Type& type)
{
    switch (type) {
    case Type::Undefined: return "undefined";
    case Type::Form: return "form";
    }
    return std::string();
}

struct Manifest {
    Uri uri;
    Type type = Type::Undefined;
    String title;
    int apiversion = 1;
    bool enabled = true;
    bool visible = true;

    bool isValid() const { return type != Type::Undefined && uri.isValid(); }
};

using ManifestList = std::vector<Manifest>;
}

#endif // MU_EXTENSIONS_EXTENSIONSTYPES_H
