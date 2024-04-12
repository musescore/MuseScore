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
#include <QFile>

#include "xmlwriter.h"
#include "midifile.h"

static const char versionString[] = "0.1";

bool debugMode = false;

//---------------------------------------------------------
//   printVersion
//---------------------------------------------------------

static void printVersion()
{
    printf("This is smf2xml version %s\n", versionString);
}

//---------------------------------------------------------
//   usage
//---------------------------------------------------------

static void usage()
{
    printVersion();
    printf("Usage: smf2xml [args] [infile] [outfile]\n");
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
    while ((c = getopt(argc, argv, "vd")) != EOF) {
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
            fprintf(stderr, "Cannot open output file <%s>: %s\n", argv[2], strerror(errno));
            return -3;
        }
    case 1:
        in = new QFile(argv[0 + optind]);
        if (!in->open(QIODevice::ReadOnly)) {
            fprintf(stderr, "Cannot open input file <%s>: %s\n", argv[1], strerror(errno));
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
    mf.read(in);

    XmlWriter xml(out);
    xml.header();
    xml.stag("SMF");
    xml.tag("division", mf.division());
    xml.tag("format", mf.format());

    for (MidiTrack* t : mf.tracks()) {
        xml.stag("Track");
        for (auto i = t->events().begin(); i != t->events().end(); ++i) {
            switch (i->second.type()) {
            case MidiEventType::NOTEOFF:
                xml.tagE(QString("NoteOff tick=\"%1\" c=\"%2\" a=\"%3\" b=\"%4\"")
                         .arg(i->first)
                         .arg(int(i->second.channel()), 0, 10)
                         .arg(int(i->second.dataA()), 2, 16)
                         .arg(int(i->second.dataB()), 2, 16)
                         );
                break;
            case MidiEventType::NOTEON:
                xml.tagE(QString("NoteOn  tick=\"%1\" c=\"%2\" a=\"%3\" b=\"%4\"")
                         .arg(i->first)
                         .arg(int(i->second.channel()), 0, 10)
                         .arg(int(i->second.dataA()), 2, 16)
                         .arg(int(i->second.dataB()), 2, 16)
                         );
                break;
            case MidiEventType::CONTROLLER:
                xml.tagE(QString("Ctrl    tick=\"%1\" c=\"%2\" a=\"%3\" b=\"%4\"")
                         .arg(i->first)
                         .arg(int(i->second.channel()), 0, 10)
                         .arg(int(i->second.dataA()), 2, 16)
                         .arg(int(i->second.dataB()), 2, 16)
                         );
                break;
            case MidiEventType::PROGRAM:
                xml.tagE(QString("Program tick=\"%1\" c=\"%2\" a=\"%3\"")
                         .arg(i->first)
                         .arg(int(i->second.channel()), 0, 10)
                         .arg(int(i->second.dataA()), 2, 16)
                         );
                break;
            default:
                fprintf(stderr, "unknown event\n");
                xml.tagE(QString("Event   tick=\"%1\" t=\"%2\" c=\"%3\" a=\"%4\" b=\"%5\"")
                         .arg(i->first)
                         .arg(int(i->second.type()), 2, 16)
                         .arg(int(i->second.channel()), 0, 10)
                         .arg(int(i->second.dataA()), 2, 16)
                         .arg(int(i->second.dataB()), 2, 16)
                         );
                break;
            }
        }
        xml.etag();
    }

    xml.etag();
    delete in;
    delete out;

    return 0;
}
