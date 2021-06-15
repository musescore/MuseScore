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
#include "notationmetawriter.h"

#include <cmath>

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

#include "libmscore/tempotext.h"

#include "log.h"
#include "global/xmlwriter.h"

using namespace mu::notation;
using namespace mu::framework;

static std::string boolToString(bool b)
{
    return b ? "true" : "false";
}

mu::Ret NotationMetaWriter::write(INotationPtr notation, mu::system::IODevice& destinationDevice, const INotationWriter::Options&)
{
    IF_ASSERT_FAILED(notation) {
        return make_ret(Ret::Code::UnknownError);
    }

    Ms::Score* score = notation->elements()->msScore();

    IF_ASSERT_FAILED(score) {
        return make_ret(Ret::Code::UnknownError);
    }

    XmlWriter writer(&destinationDevice);

    writer.writeStartDocument();

    writer.writeAttribute("title", title(score));
    writer.writeAttribute("subtitle", subtitle(score));
    writer.writeAttribute("composer", composer(score));
    writer.writeAttribute("poet", poet(score));
    writer.writeAttribute("mscoreVersion", score->mscoreVersion().toStdString());
    writer.writeAttribute("fileVersion", std::to_string(score->mscVersion()));
    writer.writeAttribute("pages", std::to_string(score->npages()));
    writer.writeAttribute("measures", std::to_string(score->nmeasures()));
    writer.writeAttribute("hasLyrics", boolToString(score->hasLyrics()));
    writer.writeAttribute("hasHarmonies", boolToString(score->hasHarmonies()));
    writer.writeAttribute("keysig", std::to_string(score->keysig()));
    writer.writeAttribute("previousSource", score->metaTag("source").toStdString());
    writer.writeAttribute("timesig", timesig(score));
    writer.writeAttribute("duration", std::to_string(score->duration()));
    writer.writeAttribute("lyrics", score->extractLyrics().toStdString());

    auto tempo = this->tempo(score);
    writer.writeAttribute("tempo", tempo.first);
    writer.writeAttribute("tempoText", tempo.second);

    writer.writeAttribute("parts", parts(score));
    writer.writeAttribute("pageFormat", pageFormat(score));
    writer.writeAttribute("textFramesData", typeData(score));

    writer.writeEndDocument();

    if (!writer.success()) {
        LOGE() << "failed write xml";
    }

    return writer.success();
}

std::string NotationMetaWriter::title(const Ms::Score* score) const
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

    return title.toStdString();
}

std::string NotationMetaWriter::subtitle(const Ms::Score* score) const
{
    QString subtitle;
    const Ms::Text* text = score->getText(Ms::Tid::SUBTITLE);
    if (text) {
        subtitle = text->plainText();
    }

    return subtitle.toStdString();
}

std::string NotationMetaWriter::composer(const Ms::Score* score) const
{
    QString composer;
    const Ms::Text* text = score->getText(Ms::Tid::COMPOSER);
    if (text) {
        composer = text->plainText();
    }

    if (composer.isEmpty()) {
        composer = score->metaTag("composer");
    }

    return composer.toStdString();
}

std::string NotationMetaWriter::poet(const Ms::Score* score) const
{
    QString poet;
    const Ms::Text* text = score->getText(Ms::Tid::POET);
    if (text) {
        poet = text->plainText();
    }

    if (poet.isEmpty()) {
        poet = score->metaTag("lyricist");
    }

    return poet.toStdString();
}

std::string NotationMetaWriter::timesig(const Ms::Score* score) const
{
    int staves = score->nstaves();
    int tracks = staves * VOICES;
    const Ms::Segment* timeSigSegment = score->firstSegmentMM(Ms::SegmentType::TimeSig);
    if (!timeSigSegment) {
        return std::string();
    }

    QString timeSig;
    const Element* element = nullptr;
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

    return timeSig.toStdString();
}

std::pair<std::string, std::string> NotationMetaWriter::tempo(const Ms::Score* score) const
{
    int tempo = 0;
    QString tempoText;
    for (const Ms::Segment* segment = score->firstSegmentMM(Ms::SegmentType::All); segment; segment = segment->next1MM()) {
        auto annotations = segment->annotations();
        for (const Element* anotation : annotations) {
            if (anotation && anotation->isTempoText()) {
                const Ms::TempoText* tt = toTempoText(anotation);
                tempo = round(tt->tempo() * 60);
                tempoText = tt->xmlText();
            }
        }
    }

    return { std::to_string(tempo), tempoText.toStdString() };
}

std::string NotationMetaWriter::parts(const Ms::Score* score) const
{
    QJsonArray jsonPartsArray;
    for (const Part* part : score->parts()) {
        QJsonObject jsonPart;
        jsonPart.insert("name", part->longName().replace("\n", ""));
        int midiProgram = part->midiProgram();
        jsonPart.insert("program", midiProgram);
        jsonPart.insert("instrumentId", part->instrumentId());
        jsonPart.insert("lyricCount", part->lyricCount());
        jsonPart.insert("harmonyCount", part->harmonyCount());
        jsonPart.insert("hasPitchedStaff", QString::fromStdString(boolToString(part->hasPitchedStaff())));
        jsonPart.insert("hasTabStaff", QString::fromStdString(boolToString(part->hasTabStaff())));
        jsonPart.insert("hasDrumStaff", QString::fromStdString(boolToString(part->hasDrumStaff())));
        jsonPart.insert("isVisible", QString::fromStdString(boolToString(part->show())));
        jsonPartsArray.append(jsonPart);
    }

    return QJsonDocument(jsonPartsArray).toJson(QJsonDocument::Compact).toStdString();
}

std::string NotationMetaWriter::pageFormat(const Ms::Score* score) const
{
    QJsonObject format;
    format.insert("height", round(score->styleD(Ms::Sid::pageHeight) * Ms::INCH));
    format.insert("width", round(score->styleD(Ms::Sid::pageWidth) * Ms::INCH));
    format.insert("twosided", QString::fromStdString(boolToString(score->styleB(Ms::Sid::pageTwosided))));

    return QJsonDocument(format).toJson(QJsonDocument::Compact).toStdString();
}

static void findTextByType(void* data, Element* element)
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

std::string NotationMetaWriter::typeData(Ms::Score* score)
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

    return QJsonDocument(typesData).toJson(QJsonDocument::Compact).toStdString();
}
