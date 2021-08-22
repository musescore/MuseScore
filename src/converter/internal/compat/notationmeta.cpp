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
#include "notationmeta.h"

#include <cmath>

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

#include "libmscore/tempotext.h"

#include "log.h"
#include "global/xmlwriter.h"

using namespace mu::converter;

static QString boolToString(bool b)
{
    return b ? "true" : "false";
}

mu::RetVal<std::string> NotationMeta::metaJson(notation::INotationPtr notation)
{
    IF_ASSERT_FAILED(notation) {
        return make_ret(Ret::Code::UnknownError);
    }

    Ms::Score* score = notation->elements()->msScore();

    IF_ASSERT_FAILED(score) {
        return make_ret(Ret::Code::UnknownError);
    }

    QJsonObject json;

    json["title"] =  title(score);
    json["subtitle"] =  subtitle(score);
    json["composer"] =  composer(score);
    json["poet"] =  poet(score);
    json["mscoreVersion"] =  score->mscoreVersion();
    json["fileVersion"] =  score->mscVersion();
    json["pages"] =  score->npages();
    json["measures"] =  score->nmeasures();
    json["hasLyrics"] =  boolToString(score->hasLyrics());
    json["hasHarmonies"] =  boolToString(score->hasHarmonies());
    json["keysig"] =  score->keysig();
    json["previousSource"] =  score->metaTag("source");
    json["timesig"] =  timesig(score);
    json["duration"] =  score->duration();
    json["lyrics"] =  score->extractLyrics();

    auto _tempo = tempo(score);
    json["tempo"] =  _tempo.first;
    json["tempoText"] =  _tempo.second;

    json["parts"] =  parts(score);
    json["pageFormat"] =  pageFormat(score);
    json["textFramesData"] =  typeData(score);

    RetVal<std::string> result;
    result.ret = make_ret(Ret::Code::Ok);
    result.val = QJsonDocument(json).toJson(QJsonDocument::Compact).toStdString();

    return result;
}

QString NotationMeta::title(const Ms::Score* score)
{
    QString title;
    const Ms::Text* text = score->getText(Ms::Tid::TITLE);
    if (text) {
        title = text->plainText();
    }

    if (title.isEmpty()) {
        title = score->metaTag("workTitle");
    }

    if (title.isEmpty()) {
        title = score->title();
    }

    return title;
}

QString NotationMeta::subtitle(const Ms::Score* score)
{
    QString subtitle;
    const Ms::Text* text = score->getText(Ms::Tid::SUBTITLE);
    if (text) {
        subtitle = text->plainText();
    }

    return subtitle;
}

QString NotationMeta::composer(const Ms::Score* score)
{
    QString composer;
    const Ms::Text* text = score->getText(Ms::Tid::COMPOSER);
    if (text) {
        composer = text->plainText();
    }

    if (composer.isEmpty()) {
        composer = score->metaTag("composer");
    }

    return composer;
}

QString NotationMeta::poet(const Ms::Score* score)
{
    QString poet;
    const Ms::Text* text = score->getText(Ms::Tid::POET);
    if (text) {
        poet = text->plainText();
    }

    if (poet.isEmpty()) {
        poet = score->metaTag("lyricist");
    }

    return poet;
}

QString NotationMeta::timesig(const Ms::Score* score)
{
    int staves = score->nstaves();
    int tracks = staves * Ms::VOICES;
    const Ms::Segment* timeSigSegment = score->firstSegmentMM(Ms::SegmentType::TimeSig);
    if (!timeSigSegment) {
        return QString();
    }

    QString timeSig;
    const Ms::Element* element = nullptr;
    for (int track = 0; track < tracks; ++track) {
        element = timeSigSegment->element(track);
        if (element) {
            break;
        }
    }

    if (element && element->isTimeSig()) {
        const Ms::TimeSig* ts = Ms::toTimeSig(element);
        timeSig = QString("%1/%2").arg(ts->numerator()).arg(ts->denominator());
    }

    return timeSig;
}

std::pair<int, QString> NotationMeta::tempo(const Ms::Score* score)
{
    int tempo = 0;
    QString tempoText;
    for (const Ms::Segment* segment = score->firstSegmentMM(Ms::SegmentType::All); segment; segment = segment->next1MM()) {
        auto annotations = segment->annotations();
        for (const Ms::Element* anotation : annotations) {
            if (anotation && anotation->isTempoText()) {
                const Ms::TempoText* tt = toTempoText(anotation);
                tempo = round(tt->tempo() * 60);
                tempoText = tt->xmlText();
            }
        }
    }

    return { tempo, tempoText };
}

QString NotationMeta::parts(const Ms::Score* score)
{
    QJsonArray jsonPartsArray;
    for (const Ms::Part* part : score->parts()) {
        QJsonObject jsonPart;
        jsonPart.insert("name", part->longName().replace("\n", ""));
        int midiProgram = part->midiProgram();
        jsonPart.insert("program", midiProgram);
        jsonPart.insert("instrumentId", part->instrumentId());
        jsonPart.insert("lyricCount", part->lyricCount());
        jsonPart.insert("harmonyCount", part->harmonyCount());
        jsonPart.insert("hasPitchedStaff", boolToString(part->hasPitchedStaff()));
        jsonPart.insert("hasTabStaff", boolToString(part->hasTabStaff()));
        jsonPart.insert("hasDrumStaff", boolToString(part->hasDrumStaff()));
        jsonPart.insert("isVisible", boolToString(part->show()));
        jsonPartsArray.append(jsonPart);
    }

    return QJsonDocument(jsonPartsArray).toJson(QJsonDocument::Compact);
}

QString NotationMeta::pageFormat(const Ms::Score* score)
{
    QJsonObject format;
    format.insert("height", round(score->styleD(Ms::Sid::pageHeight) * Ms::INCH));
    format.insert("width", round(score->styleD(Ms::Sid::pageWidth) * Ms::INCH));
    format.insert("twosided", boolToString(score->styleB(Ms::Sid::pageTwosided)));

    return QJsonDocument(format).toJson(QJsonDocument::Compact);
}

static void findTextByType(void* data, Ms::Element* element)
{
    if (!element->isTextBase()) {
        return;
    }

    const Ms::TextBase* text = toTextBase(element);
    auto* typeStringsData = static_cast<std::pair<Ms::Tid, QStringList*>*>(data);
    if (text->tid() == typeStringsData->first) {
        QStringList* titleStrings = typeStringsData->second;
        Q_ASSERT(titleStrings);
        titleStrings->append(text->plainText());
    }
}

QString NotationMeta::typeData(Ms::Score* score)
{
    QJsonObject typesData;
    static std::vector<std::pair<QString, Ms::Tid> > namesTypesList {
        { "titles", Ms::Tid::TITLE },
        { "subtitles", Ms::Tid::SUBTITLE },
        { "composers", Ms::Tid::COMPOSER },
        { "poets", Ms::Tid::POET }
    };

    for (auto nameType : namesTypesList) {
        QJsonArray typeData;
        QStringList typeTextStrings;
        std::pair<Ms::Tid, QStringList*> extendedTitleData = std::make_pair(nameType.second, &typeTextStrings);
        score->scanElements(&extendedTitleData, findTextByType);
        for (auto typeStr : typeTextStrings) {
            typeData.append(typeStr);
        }
        typesData.insert(nameType.first, typeData);
    }

    return QJsonDocument(typesData).toJson(QJsonDocument::Compact);
}
