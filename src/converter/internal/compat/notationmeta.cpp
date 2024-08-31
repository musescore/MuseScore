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
#include "notationmeta.h"

#include <cmath>

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

#include "engraving/dom/tempotext.h"
#include "engraving/dom/text.h"

#include "log.h"

using namespace muse;
using namespace mu::converter;
using namespace mu::engraving;

static QString boolToString(bool b)
{
    return b ? "true" : "false";
}

RetVal<std::string> NotationMeta::metaJson(notation::INotationPtr notation)
{
    IF_ASSERT_FAILED(notation) {
        return make_ret(Ret::Code::UnknownError);
    }

    mu::engraving::Score* score = notation->elements()->msScore();

    IF_ASSERT_FAILED(score) {
        return make_ret(Ret::Code::UnknownError);
    }

    QJsonObject json;

    json["title"] =  title(score);
    json["subtitle"] =  subtitle(score);
    json["composer"] =  composer(score);
    json["poet"] =  poet(score);
    json["mscoreVersion"] =  score->mscoreVersion().toQString();
    json["fileVersion"] =  score->mscVersion();
    json["pages"] =  static_cast<int>(score->npages()); // no = operator for size_t
    json["measures"] = static_cast<int>(score->nmeasures()); // no = operator for size_t
    json["hasLyrics"] =  boolToString(score->hasLyrics());
    json["hasHarmonies"] =  boolToString(score->hasHarmonies());
    json["keysig"] =  score->keysig();
    json["previousSource"] =  score->metaTag(u"source").toQString();
    json["timesig"] =  timesig(score);
    json["duration"] =  score->duration();
    json["lyrics"] =  score->extractLyrics().toQString();

    auto _tempo = tempo(score);
    json["tempo"] =  _tempo.first;
    json["tempoText"] =  _tempo.second;

    json["parts"] =  partsJsonArray(score);
    json["pageFormat"] = pageFormatJson(score->style());
    json["textFramesData"] =  typeDataJson(score);

    RetVal<std::string> result;
    result.ret = make_ret(Ret::Code::Ok);
    result.val = QJsonDocument(json).toJson().toStdString();

    return result;
}

QString NotationMeta::title(const mu::engraving::Score* score)
{
    QString title;
    const mu::engraving::Text* text = score->getText(mu::engraving::TextStyleType::TITLE);
    if (text) {
        title = text->plainText();
    }

    if (title.isEmpty()) {
        title = score->metaTag(u"workTitle");
    }

    if (title.isEmpty()) {
        title = score->name();
    }

    return title;
}

QString NotationMeta::subtitle(const mu::engraving::Score* score)
{
    QString subtitle;
    const mu::engraving::Text* text = score->getText(mu::engraving::TextStyleType::SUBTITLE);
    if (text) {
        subtitle = text->plainText();
    }

    return subtitle;
}

QString NotationMeta::composer(const mu::engraving::Score* score)
{
    QString composer;
    const mu::engraving::Text* text = score->getText(mu::engraving::TextStyleType::COMPOSER);
    if (text) {
        composer = text->plainText();
    }

    if (composer.isEmpty()) {
        composer = score->metaTag(u"composer");
    }

    return composer;
}

QString NotationMeta::poet(const mu::engraving::Score* score)
{
    QString poet;
    const mu::engraving::Text* text = score->getText(mu::engraving::TextStyleType::LYRICIST);
    if (text) {
        poet = text->plainText();
    }

    if (poet.isEmpty()) {
        poet = score->metaTag(u"lyricist");
    }

    return poet;
}

QString NotationMeta::timesig(const mu::engraving::Score* score)
{
    size_t staves = score->nstaves();
    size_t tracks = staves * mu::engraving::VOICES;
    const mu::engraving::Segment* timeSigSegment = score->firstSegmentMM(mu::engraving::SegmentType::TimeSig);
    if (!timeSigSegment) {
        return QString();
    }

    QString timeSig;
    const mu::engraving::EngravingItem* element = nullptr;
    for (size_t track = 0; track < tracks; ++track) {
        element = timeSigSegment->element(static_cast<int>(track));
        if (element) {
            break;
        }
    }

    if (element && element->isTimeSig()) {
        const mu::engraving::TimeSig* ts = mu::engraving::toTimeSig(element);
        timeSig = QString("%1/%2").arg(ts->numerator()).arg(ts->denominator());
    }

    return timeSig;
}

std::pair<int, QString> NotationMeta::tempo(const mu::engraving::Score* score)
{
    int tempo = 0;
    QString tempoText;
    for (const mu::engraving::Segment* segment = score->firstSegmentMM(mu::engraving::SegmentType::All); segment;
         segment = segment->next1MM()) {
        auto annotations = segment->annotations();
        for (const mu::engraving::EngravingItem* annotation : annotations) {
            if (annotation && annotation->isTempoText()) {
                const mu::engraving::TempoText* tt = toTempoText(annotation);
                tempo = round(tt->tempo().toBPM().val);
                tempoText = tt->xmlText();
            }
        }
    }

    return { tempo, tempoText };
}

QJsonArray NotationMeta::partsJsonArray(const mu::engraving::Score* score)
{
    QJsonArray jsonPartsArray;
    for (const mu::engraving::Part* part : score->parts()) {
        QJsonObject jsonPart;
        jsonPart.insert("name", part->longName().replace(u"\n", u"").toQString());
        int midiProgram = part->midiProgram();
        jsonPart.insert("program", midiProgram);
        jsonPart.insert("instrumentId", part->instrumentId().toQString());
        jsonPart.insert("lyricCount", part->lyricCount());
        jsonPart.insert("harmonyCount", part->harmonyCount());
        jsonPart.insert("hasPitchedStaff", boolToString(part->hasPitchedStaff()));
        jsonPart.insert("hasTabStaff", boolToString(part->hasTabStaff()));
        jsonPart.insert("hasDrumStaff", boolToString(part->hasDrumStaff()));
        jsonPart.insert("isVisible", boolToString(part->show()));
        jsonPartsArray.append(jsonPart);
    }

    return jsonPartsArray;
}

QJsonObject NotationMeta::pageFormatJson(const mu::engraving::MStyle& style)
{
    QJsonObject format;
    format.insert("height", round(style.styleD(mu::engraving::Sid::pageHeight) * mu::engraving::INCH));
    format.insert("width", round(style.styleD(mu::engraving::Sid::pageWidth) * mu::engraving::INCH));
    format.insert("twosided", boolToString(style.styleB(mu::engraving::Sid::pageTwosided)));

    return format;
}

static void findTextByType(void* data, mu::engraving::EngravingItem* element)
{
    if (!element->isTextBase()) {
        return;
    }

    const mu::engraving::TextBase* text = toTextBase(element);
    auto* typeStringsData = static_cast<std::pair<TextStyleType, QStringList*>*>(data);
    if (text->textStyleType() == typeStringsData->first) {
        QStringList* titleStrings = typeStringsData->second;
        Q_ASSERT(titleStrings);
        titleStrings->append(text->plainText());
    }
}

QJsonObject NotationMeta::typeDataJson(mu::engraving::Score* score)
{
    QJsonObject typesData;
    static const std::vector<std::pair<QString, TextStyleType> > namesTypesList {
        { "titles", TextStyleType::TITLE },
        { "subtitles", TextStyleType::SUBTITLE },
        { "composers", TextStyleType::COMPOSER },
        { "poets", TextStyleType::LYRICIST }
    };

    for (auto nameType : namesTypesList) {
        QJsonArray typeData;
        QStringList typeTextStrings;
        std::pair<TextStyleType, QStringList*> extendedTitleData = std::make_pair(nameType.second, &typeTextStrings);
        score->scanElements(&extendedTitleData, findTextByType);
        for (auto typeStr : typeTextStrings) {
            typeData.append(typeStr);
        }
        typesData.insert(nameType.first, typeData);
    }

    return typesData;
}
