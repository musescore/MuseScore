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

#include <stdio.h>
#include <unistd.h>

#include "xmlreader.h"
#include "midifile.h"

static const char versionString[] = "0.1";
static bool debugMode = false;

//---------------------------------------------------------
//   printVersion
//---------------------------------------------------------

static void printVersion()
{
    printf("This is xml2smf version %s\n", versionString);
}

//---------------------------------------------------------
//   usage
//---------------------------------------------------------

static void usage()
{
    printVersion();
    printf("Usage: xml2smf [args] [infile] [outfile]\n");
    printf("   args:\n"
           "      -v      print version\n"
           "      -d      debug mode\n"
           );
}

//---------------------------------------------------------
//   main
//---------------------------------------------------------

int main(int argc, char* argv[])
{
    int c;
    while ((c = getopt(argc, argv, "vdD:r")) != EOF) {
        switch (c) {
        case 'v':
            printVersion();
            return 0;
        case 'd':
            debugMode = true;
            break;
        default:
            usage();
            return -1;
        }
    }
    QIODevice* in = 0;
    QIODevice* out = 0;

    switch (argc - optind) {
    case 2:
        out = new QFile(argv[1 + optind]);
        if (!out->open(QIODevice::WriteOnly)) {
            fprintf(stderr, "Cannot open output file <%s>: %s\n", argv[optind + 1], strerror(errno));
            return -3;
        }
    case 1:
        in = new QFile(argv[optind]);
        if (!in->open(QIODevice::ReadOnly)) {
            fprintf(stderr, "Cannot open input file <%s>: %s\n", argv[optind], strerror(errno));
            return -4;
        }
        break;
    case 0:
        break;
    default:
        usage();
        return -2;
        break;
    }
    if (in == 0) {
        in = new QFile;
        ((QFile*)in)->open(stdin, QIODevice::ReadOnly);
    }
    if (out == 0) {
        out = new QFile;
        ((QFile*)out)->open(stdout, QIODevice::WriteOnly);
    }

    MidiFile mf;
    mf.setFormat(1);

    XmlReader e(in);
    while (e.readNextStartElement()) {
        const QStringView tag(e.name());
        if (tag == "SMF") {
            while (e.readNextStartElement()) {
                const QStringView tag(e.name());
                if (tag == "Track") {
                    MidiTrack* track = new MidiTrack(&mf);
                    while (e.readNextStartElement()) {
                        const QStringView tag(e.name());
                        if (tag == "NoteOff") {
                            MidiEventType t = MidiEventType::NOTEOFF;
                            int tick        = e.intAttribute("tick");
                            uchar c         = e.intAttribute("c");
                            uchar a         = e.intAttribute("a", 0, 16);
                            uchar b         = e.intAttribute("b", 0, 16);
                            track->events().insert(std::pair<int, MidiEvent>(tick, MidiEvent(t, c, a, b)));
                            e.skipCurrentElement();
                        } else if (tag == "NoteOn") {
                            MidiEventType t = MidiEventType::NOTEON;
                            int tick        = e.intAttribute("tick");
                            uchar c         = e.intAttribute("c");
                            uchar a         = e.intAttribute("a", 0, 16);
                            uchar b         = e.intAttribute("b", 0, 16);
                            track->events().insert(std::pair<int, MidiEvent>(tick, MidiEvent(t, c, a, b)));
                            e.skipCurrentElement();
                        } else if (tag == "Ctrl") {
                            MidiEventType t = MidiEventType::CONTROLLER;
                            int tick        = e.intAttribute("tick");
                            uchar c         = e.intAttribute("c");
                            uchar a         = e.intAttribute("a", 0, 16);
                            uchar b         = e.intAttribute("b", 0, 16);
                            track->events().insert(std::pair<int, MidiEvent>(tick, MidiEvent(t, c, a, b)));
                            e.skipCurrentElement();
                        } else if (tag == "Program") {
                            MidiEventType t = MidiEventType::PROGRAM;
                            int tick        = e.intAttribute("tick");
                            uchar c         = e.intAttribute("c");
                            uchar a         = e.intAttribute("a", 0, 16);
                            track->events().insert(std::pair<int, MidiEvent>(tick, MidiEvent(t, c, a, 0)));
                            e.skipCurrentElement();
                        } else if (tag == "Event") {
                            uchar t         = e.intAttribute("t");
                            int tick        = e.intAttribute("tick");
                            uchar c         = e.intAttribute("c");
                            uchar a         = e.intAttribute("a", 0, 16);
                            uchar b         = e.intAttribute("b", 0, 16);
                            track->events().insert(std::pair<int, MidiEvent>(tick,
                                                                             MidiEvent(MidiEventType(t), c, a, b)));
                            e.skipCurrentElement();
                        } else {
                            e.unknown();
                        }
                    }
                    mf.tracks().push_back(track);
                } else if (tag == "division") {
                    mf.setDivision(e.readInt());
                } else if (tag == "format") {
                    mf.setFormat(e.readInt());
                } else {
                    e.unknown();
                }
            }
        } else {
            e.unknown();
        }
    }

    mf.write(out);

    delete out;
    delete in;
    return 0;
}
