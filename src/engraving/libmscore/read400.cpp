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
#include "score.h"
#include "io/xml.h"
#include "masterscore.h"
#include "audio.h"
#include "text.h"
#include "part.h"
#include "spanner.h"
#include "excerpt.h"
#include "staff.h"

using namespace mu::engraving;
using namespace Ms;

bool Score::read400(XmlReader& e)
{
    if (!e.readNextStartElement()) {
        qDebug("%s: xml file is empty", qPrintable(e.getDocName()));
        return false;
    }

    if (e.name() == "museScore") {
        while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "programVersion") {
                e.skipCurrentElement();
            } else if (tag == "programRevision") {
                e.skipCurrentElement();
            } else if (tag == "Revision") {
                e.skipCurrentElement();
            } else if (tag == "Score") {
                if (!readScore400(e)) {
                    return false;
                }
            } else {
                e.skipCurrentElement();
            }
        }
    } else {
        qDebug("%s: invalid structure of xml file", qPrintable(e.getDocName()));
        return false;
    }

    return true;
}

bool Score::readScore400(XmlReader& e)
{
    // HACK
    // style setting compatibility settings for minor versions
    // this allows new style settings to be added
    // with different default values for older vs newer scores
    // note: older templates get the default values for older scores
    // these can be forced back in MuseScore::getNewFile() if necessary
    QString programVersion = masterScore()->mscoreVersion();
    bool disableHarmonyPlay = MScore::harmonyPlayDisableCompatibility && !MScore::testMode;
    if (!programVersion.isEmpty() && programVersion < "3.5" && disableHarmonyPlay) {
        style().set(Sid::harmonyPlay, false);
    }

    while (e.readNextStartElement()) {
        e.setTrack(-1);
        const QStringRef& tag(e.name());
        if (tag == "Staff") {
            readStaff(e);
        } else if (tag == "Omr") {
            e.skipCurrentElement();
        } else if (tag == "Audio") {
            _audio = new Audio;
            _audio->read(e);
        } else if (tag == "showOmr") {
            e.skipCurrentElement();
        } else if (tag == "playMode") {
            _playMode = PlayMode(e.readInt());
        } else if (tag == "LayerTag") {
            int id = e.intAttribute("id");
            const QString& t = e.attribute("tag");
            QString val(e.readElementText());
            if (id >= 0 && id < 32) {
                _layerTags[id] = t;
                _layerTagComments[id] = val;
            }
        } else if (tag == "Layer") {
            Layer layer;
            layer.name = e.attribute("name");
            layer.tags = e.attribute("mask").toUInt();
            _layer.append(layer);
            e.readNext();
        } else if (tag == "currentLayer") {
            _currentLayer = e.readInt();
        } else if (tag == "Synthesizer") {
            _synthesizerState.read(e);
        } else if (tag == "page-offset") {
            _pageNumberOffset = e.readInt();
        } else if (tag == "Division") {
            _fileDivision = e.readInt();
        } else if (tag == "showInvisible") {
            _showInvisible = e.readInt();
        } else if (tag == "showUnprintable") {
            _showUnprintable = e.readInt();
        } else if (tag == "showFrames") {
            _showFrames = e.readInt();
        } else if (tag == "showMargins") {
            _showPageborders = e.readInt();
        } else if (tag == "markIrregularMeasures") {
            _markIrregularMeasures = e.readInt();
        } else if (tag == "Style") {
            // Since version 400, the style is stored in a separate file
            e.skipCurrentElement();
        } else if (tag == "copyright" || tag == "rights") {
            Text* text = new Text(this);
            text->read(e);
            setMetaTag("copyright", text->xmlText());
            delete text;
        } else if (tag == "movement-number") {
            setMetaTag("movementNumber", e.readElementText());
        } else if (tag == "movement-title") {
            setMetaTag("movementTitle", e.readElementText());
        } else if (tag == "work-number") {
            setMetaTag("workNumber", e.readElementText());
        } else if (tag == "work-title") {
            setMetaTag("workTitle", e.readElementText());
        } else if (tag == "source") {
            setMetaTag("source", e.readElementText());
        } else if (tag == "metaTag") {
            QString name = e.attribute("name");
            setMetaTag(name, e.readElementText());
        } else if (tag == "Order") {
            ScoreOrder order;
            order.read(e);
            if (order.isValid()) {
                setScoreOrder(order);
            }
        } else if (tag == "Part") {
            Part* part = new Part(this);
            part->read(e);
            appendPart(part);
        } else if ((tag == "HairPin")
                   || (tag == "Ottava")
                   || (tag == "TextLine")
                   || (tag == "Volta")
                   || (tag == "Trill")
                   || (tag == "Slur")
                   || (tag == "Pedal")) {
            Spanner* s = toSpanner(Element::name2Element(tag, this));
            s->read(e);
            addSpanner(s);
        } else if (tag == "Excerpt") {
            // Since version 400, the Excerpts are stored in a separate file
            e.skipCurrentElement();
        } else if (e.name() == "Tracklist") {
            int strack = e.intAttribute("sTrack",   -1);
            int dtrack = e.intAttribute("dstTrack", -1);
            if (strack != -1 && dtrack != -1) {
                e.tracks().insert(strack, dtrack);
            }
            e.skipCurrentElement();
        } else if (tag == "Score") {
            // Since version 400, the Excerpts is stored in a separate file
            e.skipCurrentElement();
        } else if (tag == "name") {
            QString n = e.readElementText();
            if (!isMaster()) {     //ignore the name if it's not a child score
                excerpt()->setTitle(n);
            }
        } else if (tag == "layoutMode") {
            QString s = e.readElementText();
            if (s == "line") {
                setLayoutMode(LayoutMode::LINE);
            } else if (s == "system") {
                setLayoutMode(LayoutMode::SYSTEM);
            } else {
                qDebug("layoutMode: %s", qPrintable(s));
            }
        } else {
            e.unknown();
        }
    }
    e.reconnectBrokenConnectors();
    if (e.error() != QXmlStreamReader::NoError) {
        qDebug("%s: xml read error at line %lld col %lld: %s",
               qPrintable(e.getDocName()), e.lineNumber(), e.columnNumber(),
               e.name().toUtf8().data());
        if (e.error() == QXmlStreamReader::CustomError) {
            MScore::lastError = e.errorString();
        } else {
            MScore::lastError = QObject::tr("XML read error at line %1, column %2: %3").arg(e.lineNumber()).arg(e.columnNumber()).arg(
                e.name().toString());
        }
        return false;
    }

    connectTies();
    relayoutForStyles(); // force relayout if certain style settings are enabled

    _fileDivision = MScore::division;

    // Make sure every instrument has an instrumentId set.
    for (Part* part : parts()) {
        const InstrumentList* il = part->instruments();
        for (auto it = il->begin(); it != il->end(); it++) {
            static_cast<Instrument*>(it->second)->updateInstrumentId();
        }
    }

    fixTicks();

    for (Part* p : qAsConst(_parts)) {
        p->updateHarmonyChannels(false);
    }

    masterScore()->rebuildMidiMapping();
    masterScore()->updateChannel();

    for (Staff* staff : staves()) {
        staff->updateOttava();
    }

//      createPlayEvents();

    return true;
}
