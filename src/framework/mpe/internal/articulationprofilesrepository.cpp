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

#include "articulationprofilesrepository.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>

#include "log.h"

#include "internal/stringutils.h"

using namespace mu;
using namespace mu::mpe;
using namespace mu::async;

static const QString SUPPORTED_FAMILIES = "supportedFamilies";
static const QString PATTERNS_KEY = "patterns";
static const QString PATTERN_POS_KEY = "patternPosition";
static const QString OFFSET_POS_KEY = "offsetPosition";
static const QString OFFSET_VAL_KEY = "offsetValue";

// Arrangement
static const QString ARRANGEMENT_PATTERN_KEY = "arrangementPattern";
static const QString DURATION_FACTOR_KEY = "durationFactor";
static const QString TIMESTAMP_OFFSET_KEY = "timestampOffset";

// Pitch
static const QString PITCH_PATTERN_KEY = "pitchPattern";
static const QString PITCH_OFFSETS_KEY = "pitchOffsets";

// Expression
static const QString EXPRESSION_PATTERN = "expressionPattern";
static const QString MAX_AMPLITUDE_LEVEL_KEY = "maxAmplitudeLevel";
static const QString AMPLITUDE_TIME_SHIFT = "amplitudeTimeShift";
static const QString DYNAMIC_OFFSETS_KEY = "dynamicOffsets";

ArticulationsProfilePtr ArticulationProfilesRepository::createNew() const
{
    return std::make_shared<ArticulationsProfile>();
}

ArticulationsProfilePtr ArticulationProfilesRepository::defaultProfile(const ArticulationFamily family) const
{
    NOT_IMPLEMENTED;
    UNUSED(family);
    return nullptr;
}

ArticulationsProfilePtr ArticulationProfilesRepository::loadProfile(const io::path& path) const
{
    RetVal<QByteArray> fileReading = fileSystem()->readFile(path);

    if (!fileReading.ret) {
        LOGE() << "Unable to read profile, path: " << path;
        return nullptr;
    }

    QJsonParseError err;

    QJsonDocument file = QJsonDocument::fromJson(fileReading.val, &err);
    if (err.error != QJsonParseError::NoError) {
        LOGE() << err.errorString();
        return nullptr;
    }

    ArticulationsProfilePtr result = std::make_shared<ArticulationsProfile>();

    QJsonObject rootObj = file.object();

    result->supportedFamilies = supportedFamiliesFromJson(rootObj.value(SUPPORTED_FAMILIES).toArray());

    QJsonObject articulationPatterns = rootObj.value(PATTERNS_KEY).toObject();

    for (const QString& key : articulationPatterns.keys()) {
        result->updatePatterns(articulationTypeFromString(key),
                               patternsScopeFromJson(articulationPatterns.value(key).toArray()));
    }

    return result;
}

void ArticulationProfilesRepository::saveProfile(const io::path& path, const ArticulationsProfilePtr profilePtr)
{
    IF_ASSERT_FAILED(profilePtr) {
        return;
    }

    QJsonObject rootObj;

    rootObj.insert(SUPPORTED_FAMILIES, supportedFamiliesToJson(profilePtr->supportedFamilies));

    QJsonObject articulationPatterns;

    for (const auto& pair : profilePtr->data()) {
        articulationPatterns.insert(articulationTypeToString(pair.first),
                                    patternsScopeToJson(pair.second));
    }

    rootObj.insert(PATTERNS_KEY, articulationPatterns);

    Ret fileWriting = fileSystem()->writeToFile(path, QJsonDocument(rootObj).toJson());

    if (!fileWriting) {
        LOGE() << "Unable to write MPE Articulation Profile, err: " << fileWriting.toString();
        return;
    }

    m_profileChanged.send(path);
}

Channel<io::path> ArticulationProfilesRepository::profileChanged() const
{
    return m_profileChanged;
}

std::vector<ArticulationFamily> ArticulationProfilesRepository::supportedFamiliesFromJson(const QJsonArray& array) const
{
    std::vector<ArticulationFamily> result;
    result.reserve(array.size());

    for (const QJsonValue& val : array) {
        result.push_back(articulationFamilyFromString(val.toString()));
    }

    return result;
}

QJsonArray ArticulationProfilesRepository::supportedFamiliesToJson(const std::vector<ArticulationFamily>& families) const
{
    QJsonArray result;

    for (const auto& family : families) {
        result.append(articulationFamilyToString(family));
    }

    return result;
}

ArticulationPattern ArticulationProfilesRepository::patternsScopeFromJson(const QJsonArray& array) const
{
    ArticulationPattern result;

    for (const QJsonValue& val : array) {
        QJsonObject patternObj = val.toObject();

        duration_percentage_t position = patternObj.value(PATTERN_POS_KEY).toDouble();

        ArrangementPattern arrangementPattern = arrangementPatternFromJson(patternObj.value(ARRANGEMENT_PATTERN_KEY).toObject());
        PitchPattern pitchPattern = pitchPatternFromJson(patternObj.value(PITCH_PATTERN_KEY).toObject());
        ExpressionPattern expressionPattern = expressionPatternFromJson(patternObj.value(EXPRESSION_PATTERN).toObject());

        ArticulationPatternSegment articulation;
        articulation.arrangementPattern = std::move(arrangementPattern);
        articulation.pitchPattern = std::move(pitchPattern);
        articulation.expressionPattern = std::move(expressionPattern);

        result.emplace(position, std::move(articulation));
    }

    return result;
}

QJsonArray ArticulationProfilesRepository::patternsScopeToJson(const ArticulationPattern& scope) const
{
    QJsonArray result;

    for (const auto& pair : scope) {
        QJsonObject pattern;
        pattern.insert(PATTERN_POS_KEY, pair.first);
        pattern.insert(ARRANGEMENT_PATTERN_KEY, arrangementPatternToJson(pair.second.arrangementPattern));
        pattern.insert(PITCH_PATTERN_KEY, pitchPatternToJson(pair.second.pitchPattern));
        pattern.insert(EXPRESSION_PATTERN, expressionPatternToJson(pair.second.expressionPattern));

        result.append(pattern);
    }

    return result;
}

ArrangementPattern ArticulationProfilesRepository::arrangementPatternFromJson(const QJsonObject& obj) const
{
    ArrangementPattern result;

    result.durationFactor = obj.value(DURATION_FACTOR_KEY).toDouble();
    result.timestampOffset = obj.value(TIMESTAMP_OFFSET_KEY).toDouble();

    return result;
}

QJsonObject ArticulationProfilesRepository::arrangementPatternToJson(const ArrangementPattern& pattern) const
{
    QJsonObject result;

    result.insert(DURATION_FACTOR_KEY, pattern.durationFactor);
    result.insert(TIMESTAMP_OFFSET_KEY, static_cast<int>(pattern.timestampOffset));

    return result;
}

PitchPattern ArticulationProfilesRepository::pitchPatternFromJson(const QJsonObject& obj) const
{
    PitchPattern result;

    QJsonArray offsets = obj.value(PITCH_OFFSETS_KEY).toArray();

    for (const QJsonValue& pitchOffset : offsets) {
        QJsonObject offsetObj = pitchOffset.toObject();

        result.pitchOffsetMap.emplace(offsetObj.value(OFFSET_POS_KEY).toDouble(),
                                      offsetObj.value(OFFSET_VAL_KEY).toDouble());
    }

    return result;
}

QJsonObject ArticulationProfilesRepository::pitchPatternToJson(const PitchPattern& pattern) const
{
    QJsonObject result;

    QJsonArray pitchOffsets;

    for (const auto& pair : pattern.pitchOffsetMap) {
        QJsonObject offsetObj;
        offsetObj.insert(OFFSET_POS_KEY, pair.first);
        offsetObj.insert(OFFSET_VAL_KEY, pair.second);

        pitchOffsets.append(std::move(offsetObj));
    }

    result.insert(PITCH_OFFSETS_KEY, pitchOffsets);

    return result;
}

ExpressionPattern ArticulationProfilesRepository::expressionPatternFromJson(const QJsonObject& obj) const
{
    ExpressionPattern result;

    result.maxAmplitudeLevel = obj.value(MAX_AMPLITUDE_LEVEL_KEY).toDouble();
    result.amplitudeTimeShift = obj.value(AMPLITUDE_TIME_SHIFT).toDouble();

    QJsonArray offsets = obj.value(DYNAMIC_OFFSETS_KEY).toArray();

    for (const QJsonValue& offset : offsets) {
        QJsonObject offsetObj = offset.toObject();
        result.dynamicOffsetMap.emplace(offsetObj.value(OFFSET_POS_KEY).toDouble(),
                                        offsetObj.value(OFFSET_VAL_KEY).toDouble());
    }

    return result;
}

QJsonObject ArticulationProfilesRepository::expressionPatternToJson(const ExpressionPattern& pattern) const
{
    QJsonObject result;

    result.insert(MAX_AMPLITUDE_LEVEL_KEY, pattern.maxAmplitudeLevel);
    result.insert(AMPLITUDE_TIME_SHIFT, pattern.amplitudeTimeShift);

    QJsonArray dynamicOffsets;

    for (const auto& pair : pattern.dynamicOffsetMap) {
        QJsonObject offsetObj;
        offsetObj.insert(OFFSET_POS_KEY, pair.first);
        offsetObj.insert(OFFSET_VAL_KEY, pair.second);

        dynamicOffsets.append(std::move(offsetObj));
    }

    result.insert(DYNAMIC_OFFSETS_KEY, dynamicOffsets);

    return result;
}
