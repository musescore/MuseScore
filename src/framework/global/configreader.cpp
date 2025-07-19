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
#include "configreader.h"

#include <string>

#include "io/file.h"
#include "serialization/json.h"
#include "types/bytearray.h"
#include "types/ret.h"
#include "types/val.h"

#include "log.h"

using namespace muse;

static void fillMap(ValMap& v, const std::string& key, const JsonValue& jv);
static void fillList(ValList& l, const JsonValue& jv)
{
    if (jv.isNull()) {
        return;
    } else if (jv.isBool()) {
        l.push_back(Val(jv.toBool()));
    } else if (jv.isNumber()) {
        l.push_back(Val(jv.toDouble()));
    } else if (jv.isString()) {
        l.push_back(Val(jv.toStdString()));
    } else if (jv.isArray()) {
        ValList list;
        JsonArray arr = jv.toArray();
        for (size_t i = 0; i < arr.size(); ++i) {
            JsonValue av = arr.at(i);
            fillList(list, av);
        }
        l.push_back(Val(list));
    } else if (jv.isObject()) {
        ValMap map;
        JsonObject obj = jv.toObject();
        for (const std::string& k : obj.keys()) {
            fillMap(map, k, obj.value(k));
        }
        l.push_back(Val(map));
    }
}

static void fillMap(ValMap& v, const std::string& key, const JsonValue& jv)
{
    if (jv.isNull()) {
        return;
    } else if (jv.isBool()) {
        v[key] = Val(jv.toBool());
    } else if (jv.isNumber()) {
        v[key] = Val(jv.toDouble());
    } else if (jv.isString()) {
        v[key] = Val(jv.toStdString());
    } else if (jv.isArray()) {
        ValList list;
        JsonArray arr = jv.toArray();
        for (size_t i = 0; i < arr.size(); ++i) {
            JsonValue av = arr.at(i);
            fillList(list, av);
        }
        v[key] = Val(list);
    } else if (jv.isObject()) {
        ValMap map;
        JsonObject obj = jv.toObject();
        for (const std::string& k : obj.keys()) {
            fillMap(map, k, obj.value(k));
        }
        v[key] = Val(map);
    }
}

Config ConfigReader::read(const io::path_t& path)
{
    ByteArray data;
    Ret ret = io::File::readFile(path, data);
    if (!ret) {
        LOGE() << "failed read file: " << path << ", err: " << ret.toString();
        return Config();
    }

    std::string err;
    JsonObject obj = JsonDocument::fromJson(data, &err).rootObject();
    if (!err.empty()) {
        LOGE() << "failed parse file: " << path << ", err: " << err;
        return Config();
    }

    ValMap cv;
    for (const std::string& k : obj.keys()) {
        fillMap(cv, k, obj.value(k));
    }

    return Config(cv);
}
