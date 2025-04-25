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

#include <QFile>
#include <QFileInfo>

#include "musx_import.h"
#include "musx/musx.h"

#include "global/io/file.h"

#include "engraving/dom/box.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/chordlist.h"
#include "engraving/dom/drumset.h"
#include "engraving/dom/factory.h"
#include "engraving/dom/harmony.h"
#include "engraving/dom/key.h"
#include "engraving/dom/keysig.h"
#include "engraving/dom/layoutbreak.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/mscore.h"
#include "engraving/dom/note.h"
#include "engraving/dom/part.h"
#include "engraving/dom/pitchspelling.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/slur.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/text.h"
#include "engraving/dom/tie.h"
#include "engraving/dom/utils.h"
#include "engraving/engravingerrors.h"

#include "log.h"

using namespace mu::engraving;
using namespace muse;

namespace mu::iex::musx {

//---------------------------------------------------------
//   importMusx
//---------------------------------------------------------

Err importMusx(MasterScore* score, const QString& name)
{
    io::File xmlFile(name);
    if (!xmlFile.exists()) {
        return Err::FileNotFound;
    }

    if (!xmlFile.open(io::IODevice::ReadOnly)) {
        LOGE() << "could not open MusicXML file: " << name;
        return Err::FileOpenError;
    }

    const ByteArray data = xmlFile.readAll();
    xmlFile.close();

    auto doc = ::musx::factory::DocumentFactory::create<::musx::xml::qt::Document>(std::vector<char>(data.constData(), data.constData() + data.size()));

    /*
    if (!bb.read(name)) {
        LOGD("Cannot open file <%s>", qPrintable(name));
        return engraving::Err::FileOpenError;
    }
    score->style().set(Sid::chordsXmlFile, true);
    score->chordList()->read(score->configuration()->appDataPath(), u"chords.xml");
    *(score->sigmap()) = bb.siglist();

    QList<BBTrack*>* tracks = bb.tracks();
    int ntracks = tracks->size();
    if (ntracks == 0) {             // no events?
        ntracks = 1;
    }
    for (int i = 0; i < ntracks; ++i) {
        Part* part = new Part(score);
        Staff* s = Factory::createStaff(part);
        score->appendStaff(s);
        score->appendPart(part);
    }

    //---------------------------------------------------
    //  create measures
    //---------------------------------------------------

    for (int i = 0; i < bb.measures(); ++i) {
        Measure* measure  = Factory::createMeasure(score->dummy()->system());
        Fraction tick = Fraction::fromTicks(score->sigmap()->bar2tick(i, 0));
        measure->setTick(tick);
        Fraction ts = score->sigmap()->timesig(tick.ticks()).timesig();
        measure->setTimesig(ts);
        measure->setTicks(ts);
        score->measures()->add(measure);
    }

    //---------------------------------------------------
    //  create notes
    //---------------------------------------------------

    foreach (BBTrack* track, * tracks) {
        track->cleanup();
    }

    if (tracks->isEmpty()) {
        for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
            if (mb->type() != ElementType::MEASURE) {
                continue;
            }
            Measure* measure = (Measure*)mb;
            Segment* s = measure->getSegment(SegmentType::ChordRest, measure->tick());
            Rest* rest = Factory::createRest(s, TDuration(DurationType::V_MEASURE));
            rest->setTicks(measure->ticks());
            rest->setTrack(0);
            s->add(rest);
        }
    } else {
        int staffIdx = 0;
        foreach (BBTrack* track, * tracks) {
            bb.convertTrack(score, track, staffIdx++);
        }
    }

    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (mb->type() != ElementType::MEASURE) {
            continue;
        }
        Measure* measure = (Measure*)mb;
        Segment* s = measure->findSegment(SegmentType::ChordRest, measure->tick());
        if (s == 0) {
            Rest* rest = Factory::createRest(s, TDuration(DurationType::V_MEASURE));
            rest->setTicks(measure->ticks());
            rest->setTrack(0);
            Segment* s1 = measure->getSegment(SegmentType::ChordRest, measure->tick());
            s1->add(rest);
        }
    }

    score->spell();

    //---------------------------------------------------
    //    create title
    //---------------------------------------------------

    MeasureBase* measureB = score->first();
    Text* text = Factory::createText(measureB, TextStyleType::TITLE);
    text->setPlainText(String::fromUtf8(bb.title()));

    if (measureB->type() != ElementType::VBOX) {
        measureB = Factory::createTitleVBox(score->dummy()->system());
        measureB->setNext(score->first());
        score->measures()->add(measureB);
    }
    measureB->add(text);

    //---------------------------------------------------
    //    create chord symbols
    //---------------------------------------------------

    static const int table[] = {
        //C  Db, D,  Eb,  E, F, Gb, G,  Ab, A,  Bb, B,  C#, D#, F#  G#  A#
        14, 9, 16, 11, 18, 13, 8, 15, 10, 17, 12, 19, 21, 23, 20, 22, 24
    };
    foreach (const BBChord& c, bb.chords()) {
        Fraction tick = Fraction(c.beat, 4);          // c.beat  * Constants::division;
// LOGD("CHORD %d %d", c.beat, tick);
        Measure* m = score->tick2measure(tick);
        if (m == 0) {
            LOGD("import BB: measure for tick %d not found", tick.ticks());
            continue;
        }
        Segment* s = m->getSegment(SegmentType::ChordRest, tick);
        Harmony* h = Factory::createHarmony(s);
        h->setTrack(0);
        h->setRootTpc(table[c.root - 1]);
        if (c.bass > 0) {
            h->setBaseTpc(table[c.bass - 1]);
        } else {
            h->setBaseTpc(Tpc::TPC_INVALID);
        }
        h->setId(c.extension);
        h->getDescription();
        h->render();
        s->add(h);
    }

    //---------------------------------------------------
    //    insert layout breaks
    //    add chorus repeat
    //---------------------------------------------------

    int startChorus = bb.startChorus() - 1;
    int endChorus   = bb.endChorus() - 1;

    int n = 0;
    for (MeasureBase* mb = score->first(); mb; mb = mb->next()) {
        if (mb->type() != ElementType::MEASURE) {
            continue;
        }
        Measure* measure = (Measure*)mb;
        if (n && (n % 4) == 0) {
            LayoutBreak* lb = Factory::createLayoutBreak(measure);
            lb->setLayoutBreakType(LayoutBreakType::LINE);
            measure->add(lb);
        }
        if (startChorus == n) {
            measure->setRepeatStart(true);
        } else if (endChorus == n) {
            measure->setRepeatEnd(true);
            measure->setRepeatCount(bb.repeats());
        }
        ++n;
    }

    for (Staff* staff : score->staves()) {
        Fraction tick = Fraction(0, 1);
        KeySigEvent ke;
        Key key = Key(bb.key());
        Key cKey = key;
        Interval v = staff->part()->instrument(tick)->transpose();
        if (!v.isZero() && !score->style().styleB(Sid::concertPitch)) {
            cKey = transposeKey(key, v);
            // if there are more than 6 accidentals in transposing key, it cannot be PreferSharpFlat::AUTO
            if ((key > 6 || key < -6) && staff->part()->preferSharpFlat() == PreferSharpFlat::AUTO) {
                staff->part()->setPreferSharpFlat(PreferSharpFlat::NONE);
            }
        }
        ke.setConcertKey(cKey);
        ke.setKey(key);
        staff->setKey(tick, ke);
        Measure* mks = score->tick2measure(tick);
        Segment* sks = mks->getSegment(SegmentType::KeySig, tick);
        KeySig* keysig = Factory::createKeySig(sks);
        keysig->setTrack((static_cast<int>(score->staffIdx(staff->part())) + staff->rstaff()) * VOICES);
        keysig->setKey(cKey, key);
        sks->add(keysig);
    }
*/
    score->setUpTempoMap();
    return engraving::Err::NoError;
}

}
