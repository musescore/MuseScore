//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <stdio.h>
#include "all.h"

static QFile logFile;
static int processed = 0;
static int failed = 0;

const char* tests[] = {
//      "libmscore/compat/tst_compat",          // expected to not work
      "libmscore/element/tst_element",
      "libmscore/note/tst_note",
      "libmscore/keysig/tst_keysig",
      "libmscore/barline/tst_barline",
      "libmscore/clef/tst_clef",
//      "libmscore/timesig/tst_timesig",
      "libmscore/repeat/tst_repeat",
      "libmscore/dynamic/tst_dynamic",
      "libmscore/breath/tst_breath",
      "libmscore/tuplet/tst_tuplet",
      "libmscore/hairpin/tst_hairpin",
//      "libmscore/chordsymbol/tst_chordsymbol",
      "libmscore/text/tst_text",
//      "libmscore/measure/tst_measure",
      "libmscore/beam/tst_beam",
//      "libmscore/spanners/tst_spanners",
//      "libmscore/clef_courtesy/tst_clef_courtesy",

//      "libmscore/midimapping/tst_midimapping",      // probably ok

      "libmscore/layout/tst_benchmark",
//      "libmscore/selectionrangedelete/tst_selectionrangedelete",
      "libmscore/instrumentchange/tst_instrumentchange",
//      "libmscore/join/tst_join",
      "libmscore/transpose/tst_transpose",
      "libmscore/copypaste/tst_copypaste",

//      "libmscore/parts/tst_parts",                                // crash
      "libmscore/concertpitch/tst_concertpitchbenchmark",
      "libmscore/selectionfilter/tst_selectionfilter",
//      "libmscore/earlymusic/tst_earlymusic",

//      "libmscore/midi/tst_midi",
//      "libmscore/tools/tst_tools",
//      "libmscore/splitstaff/tst_splitstaff",
//      "libmscore/split/tst_split",
      "libmscore/plugins/tst_plugins",
      "libmscore/album/tst_album",
//      "libmscore/copypastesymbollist/tst_copypastesymbollist",
      "scripting/tst_scripting"

        // import/export
      "guitarpro/tst_guitarpro",
      "biab/tst_biab",
//      "testoves/structure/tst_ove_structure",
//      "testoves/ove3/tst_ove_ove3",
//      "testoves/bdat/tst_ove_bdat",
      "capella/io/tst_capella_io",
      "importmidi/tst_importmidi",
//      "musicxml/io/tst_mxml_io",
      };

//---------------------------------------------------------
//   process
//---------------------------------------------------------

static void process(const QString& cmd)
      {
      QStringList args;
      int rv = QProcess::execute(cmd, args);
      if (rv != 0) {
            // seems not to be reliable
            printf("========mtest process <%s> returns %d\n", qPrintable(cmd), rv);
            failed++;
            }
      processed++;
      }

//---------------------------------------------------------
//   scanDir
//---------------------------------------------------------

static void scanDir(QDir d)
      {
      QFileInfoList l = d.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);
      foreach(const QFileInfo& fi, l) {
            if (fi.isDir()) {
                  scanDir(QDir(fi.filePath()));
                  }
            else if (fi.isExecutable()) {
                  QString s(fi.filePath());
                  if (fi.completeBaseName().startsWith("tst_"))
                        process(s);
                  }
            }
      }

//---------------------------------------------------------
//   main
//---------------------------------------------------------

int main(int argc, char* argv[])
      {
#if 0
      logFile.setFileName("mtest.log");
      if (!logFile.open(QIODevice::WriteOnly)) {
            fprintf(stderr, "mtest: cannot open log file <mtest.log>\n");
            exit(-1);
            }
#endif
      QDir wd(QDir::current());
#ifdef Q_OS_MAC
      wd.cdUp();
#endif

#if 0
      scanDir(wd);
#else
      for (const char* s : tests)
            process(s);
#endif

      printf("\n");
      printf("================\n");
      printf("  processed %d  -- failed %d\n", processed, failed);
      printf("================\n");
      return 0;
      }

