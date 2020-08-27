//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "importgtp.h"
#include "libmscore/bracketItem.h"
#include <libmscore/instrtemplate.h>
#include <libmscore/part.h>
#include <libmscore/staff.h>
#include "thirdparty/qzip/qzipreader_p.h"

namespace Ms {
//---------------------------------------------------------
//   readTracks
//---------------------------------------------------------

void GuitarPro7::readTracks(QDomNode* track)
{
    QDomNode nextTrack = track->firstChild();
    int trackCounter   = 0;
    while (!nextTrack.isNull()) {
        QDomNode currentNode = nextTrack.firstChild();
        Part* part           = new Part(score);
        bool hasTuning       = false;
        Staff* s             = new Staff(score);
        s->setPart(part);
        part->insertStaff(s, -1);
        score->staves().push_back(s);
        while (!currentNode.isNull()) {
            QString nodeName = currentNode.nodeName();
            if (nodeName == "Name") {
                part->setPartName(currentNode.toElement().text());
            } else if (nodeName == "Color") {
            }
            // this is a typo is guitar pro - 'defaut' is correct here
            else if (nodeName == "SystemsDefautLayout") {
            } else if (nodeName == "SystemsLayout") {
            } else if (nodeName == "RSE") {
            } else if (nodeName == "PlaybackState") {
            } else if (nodeName == "PlayingStyle") {
            } else if (nodeName == "PageSetup") {
            } else if (nodeName == "MultiVoice") {
            } else if (nodeName == "ShortName") {
            } else if (nodeName == "Instrument") {
                QString ref = currentNode.attributes().namedItem("ref").toAttr().value();
                auto it     = instrumentMapping.find(ref);
                if (it != instrumentMapping.end()) {
                    part->setInstrument(Instrument::fromTemplate(Ms::searchTemplate(it->second)));
                } else {
                    qDebug() << "Unknown instrument: " << ref;
                }
                if (ref.endsWith("-gs") || ref.startsWith("2")) {         // grand staff
                    Staff* s2 = new Staff(score);
                    s2->setPart(part);
                    part->insertStaff(s2, -1);
                    score->staves().push_back(s2);
                    s->addBracket(new BracketItem(s->score(), BracketType::BRACE, 2));
                    s->setBarLineSpan(2);
                }
            } else if (nodeName == "Transpose") {
                part->instrument()->setTranspose(Interval(currentNode.firstChildElement("Octave").text().toInt() * 12));
            } else if (nodeName == "Sounds") {
                part->instrument()->channel(0)->setProgram(currentNode.firstChildElement("Sound").firstChildElement(
                                                               "MIDI").firstChildElement("Program").text().toInt());
            } else if (nodeName == "MidiConnection") {
                part->setMidiChannel(currentNode.firstChildElement("PrimaryChannel").text().toInt());
                if (part->midiChannel() == GP_DEFAULT_PERCUSSION_CHANNEL) {
                    part->instrument()->setDrumset(gpDrumset);
                    s->setStaffType(Fraction(0, 1), *StaffType::preset(StaffTypes::PERC_DEFAULT));
                }
            } else if (nodeName == "Staves") {
                QDomNode staff = currentNode.firstChild();
                QDomNode properties = staff.firstChildElement("Properties");
                readTrackProperties(properties, part, trackCounter, hasTuning);
            }
            currentNode = currentNode.nextSibling();
        }

        // add in a new part
        score->appendPart(part);
        trackCounter++;
        nextTrack = nextTrack.nextSibling();

        if (!hasTuning) {
            tunings.push_back("");
        }
    }

    previousDynamic = new int[score->staves().length() * VOICES];
    // initialise the dynamics to 0
    for (int i = 0; i < score->staves().length() * VOICES; i++) {
        previousDynamic[i] = 0;
    }
    // set the number of staves we need
    staves = score->staves().length();
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

bool GuitarPro7::read(QFile* fp)
{
    f = fp;
    previousTempo = -1;
    MQZipReader zip(fp);
    QByteArray fileData = zip.fileData("Content/score.gpif");
    zip.close();
    readGpif(&fileData);
    return true;
}
}
