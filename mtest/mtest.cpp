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

// static QFile log;
static int processed = 0;
static int failed = 0;

//---------------------------------------------------------
//   process
//---------------------------------------------------------

static void process(const QString& cmd)
      {
      QStringList args;
      int rv = QProcess::execute(cmd, args);
      if (rv != 0)
            failed++;
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
      log.setFileName("mtest.log");
      if (!log.open(QIODevice::WriteOnly)) {
            fprintf(stderr, "mtest: cannot open log file <mtest.log>\n");
            exit(-1);
            }
#endif
      QDir wd(QDir::current());
#ifdef Q_OS_MAC
      wd.cdUp();
#endif
      scanDir(wd);

      printf("\n");
      printf("================\n");
      printf("  processed %d  -- failed %d\n", processed, failed);
      printf("================\n");
      return 0;
      }

