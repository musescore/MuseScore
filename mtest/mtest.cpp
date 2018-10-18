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
#if 1
      "libmscore/element/tst_element",
      "libmscore/note/tst_note",
      "libmscore/keysig/tst_keysig",
      "libmscore/barline/tst_barline",
      "libmscore/clef/tst_clef",
      "libmscore/timesig/tst_timesig",
      "libmscore/repeat/tst_repeat",
      "libmscore/rhythmicGrouping/tst_rhythmicGrouping",
      "libmscore/dynamic/tst_dynamic",
      "libmscore/durationtype/tst_durationtype",
      "libmscore/breath/tst_breath",
      "libmscore/tuplet/tst_tuplet",
      "libmscore/hairpin/tst_hairpin",
      "libmscore/chordsymbol/tst_chordsymbol",
      "libmscore/text/tst_text",
      "libmscore/measure/tst_measure",
      "libmscore/beam/tst_beam",
      "libmscore/layout/tst_benchmark",
      "libmscore/layout_elements/tst_layout_elements",
      "libmscore/instrumentchange/tst_instrumentchange",
      "libmscore/join/tst_join",
      "libmscore/transpose/tst_transpose",
      "libmscore/copypaste/tst_copypaste",
      "libmscore/concertpitch/tst_concertpitchbenchmark",
      "libmscore/selectionfilter/tst_selectionfilter",
      "libmscore/tools/tst_tools",                    // some tests disabled
      "libmscore/plugins/tst_plugins",
      "libmscore/album/tst_album",
      "scripting/tst_scripting"
      "guitarpro/tst_guitarpro",
      "biab/tst_biab",
      "capella/io/tst_capella_io",
      "importmidi/tst_importmidi",
      "libmscore/selectionrangedelete/tst_selectionrangedelete",
      "libmscore/parts/tst_parts",
#endif
#if 0
      "libmscore/spanners/tst_spanners",              // FAIL

      "libmscore/clef_courtesy/tst_clef_courtesy",    // FAIL
      "libmscore/midimapping/tst_midimapping",        // FAIL
      "libmscore/earlymusic/tst_earlymusic",          // FAIL

      "libmscore/midi/tst_midi",                      // FAIL
      "libmscore/splitstaff/tst_splitstaff",          // FAIL
      "libmscore/split/tst_split",                    // FAIL
      "libmscore/copypastesymbollist/tst_copypastesymbollist",    // FAIL

        // import/export
      "testoves/structure/tst_ove_structure",         // FAIL
      "testoves/ove3/tst_ove_ove3",                   // FAIL
      "testoves/bdat/tst_ove_bdat",
      "musicxml/io/tst_mxml_io",                      // FAIL
#endif
#ifdef OMR
      "omr/notes/tst_notes",
#endif
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
#if 0
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
#endif

//---------------------------------------------------------
//   main
//---------------------------------------------------------

int main(int argc, char* argv[])
      {
      Q_UNUSED(argc);
      Q_UNUSED(argv);
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

