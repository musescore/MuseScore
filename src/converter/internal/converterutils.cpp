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
#include "converterutils.h"

#include <QJsonDocument>
#include <QJsonObject>

#include "convertercodes.h"

using namespace muse;
using namespace mu::converter;
using namespace mu::notation;

RetVal<TransposeOptions> ConverterUtils::parseTransposeOptions(const std::string& optionsJson)
{
    QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(optionsJson));
    if (!doc.isObject()) {
        LOGW() << "Transpose options JSON is not an object: " << optionsJson;
        return make_ret(Err::InvalidTransposeOptions);
    }

    QJsonObject optionsObj = doc.object();

    return parseTransposeOptions(optionsObj);
}

RetVal<TransposeOptions> ConverterUtils::parseTransposeOptions(const QJsonObject& optionsObj)
{
    TransposeOptions options;

    const QString modeName = optionsObj["mode"].toString();
    if (modeName == "by_key" || modeName == "to_key") { // "by_key" for backwards compatibility
        options.mode = TransposeMode::TO_KEY;
    } else if (modeName == "by_interval") {
        options.mode = TransposeMode::BY_INTERVAL;
    } else if (modeName == "diatonically") {
        options.mode = TransposeMode::DIATONICALLY;
    } else {
        LOGW() << "Transpose: invalid \"mode\" option: " << modeName;
        return make_ret(Err::InvalidTransposeOptions);
    }

    const QString directionName = optionsObj["direction"].toString();
    if (directionName == "up") {
        options.direction = TransposeDirection::UP;
    } else if (directionName == "down") {
        options.direction = TransposeDirection::DOWN;
    } else if (directionName == "closest") {
        options.direction = TransposeDirection::CLOSEST;
    } else {
        LOGW() << "Transpose: invalid \"direction\" option: " << directionName;
        return make_ret(Err::InvalidTransposeOptions);
    }

    constexpr int defaultKey = int(Key::INVALID);
    const Key targetKey = Key(optionsObj["targetKey"].toInt(defaultKey));
    if (options.mode == TransposeMode::TO_KEY) {
        const bool targetKeyValid = int(Key::MIN) <= int(targetKey) && int(targetKey) <= int(Key::MAX);
        if (!targetKeyValid) {
            LOGW() << "Transpose: invalid targetKey: " << int(targetKey);
            return make_ret(Err::InvalidTransposeOptions);
        }
    }

    const int transposeInterval = optionsObj["transposeInterval"].toInt(-1);
    constexpr int INTERVAL_LIST_SIZE = 26;

    if (options.mode != TransposeMode::TO_KEY) {
        const bool transposeIntervalValid = -1 < transposeInterval && transposeInterval < INTERVAL_LIST_SIZE;
        if (!transposeIntervalValid) {
            LOGW() << "Transpose: invalid transposeInterval: " << transposeInterval;
            return make_ret(Err::InvalidTransposeOptions);
        }
    }

    options.interval = transposeInterval;
    options.key = targetKey;
    options.needTransposeKeys = optionsObj["transposeKeySignatures"].toBool();
    options.needTransposeChordNames = optionsObj["transposeChordNames"].toBool();
    options.needTransposeDoubleSharpsFlats = optionsObj["useDoubleSharpsFlats"].toBool();

    RetVal<TransposeOptions> result;
    result.ret = make_ret(Ret::Code::Ok);
    result.val = options;

    return result;
}

Ret ConverterUtils::applyTranspose(const INotationPtr notation, const std::string& optionsJson)
{
    RetVal<TransposeOptions> options = parseTransposeOptions(optionsJson);
    if (!options.ret) {
        return options.ret;
    }

    return applyTranspose(notation, options.val);
}

Ret ConverterUtils::applyTranspose(const INotationPtr notation, const TransposeOptions& options)
{
    INotationInteractionPtr interaction = notation ? notation->interaction() : nullptr;
    if (!interaction) {
        return make_ret(Ret::Code::InternalError);
    }

    interaction->selectAll();

    bool ok = interaction->transpose(options);

    return ok ? make_ret(Ret::Code::Ok) : make_ret(Err::TransposeFailed);
}
