//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2018 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "testscript.h"

#include "musescore.h"
#include "script.h"

#include "libmscore/scorediff.h"

namespace Ms {

//---------------------------------------------------------
//   TestScriptEntry::deserialize
//---------------------------------------------------------

std::unique_ptr<ScriptEntry> TestScriptEntry::deserialize(const QStringList& tokens)
      {
      // assume that 0th token is just a "test" statement
      if (tokens.size() < 2) {
            qDebug("test: unexpected tokens size: %d", tokens.size());
            return nullptr;
            }

      const QString& type = tokens[1];
      if (type == TEST_SCORE) {
            if (tokens.size() != 3) {
                  qDebug("test: unexpected tokens size: %d", tokens.size());
                  return nullptr;
                  }
            return std::unique_ptr<ScriptEntry>(new ScoreTestScriptEntry(tokens[2]));
            }

      qDebug() << "test: unsupported type:" << tokens[1];
      return nullptr;
      }

//---------------------------------------------------------
//   ScoreTestScriptEntry::fromContext
//---------------------------------------------------------

std::unique_ptr<ScriptEntry> ScoreTestScriptEntry::fromContext(const ScriptContext& ctx, QString fileName)
      {
      MasterScore* score = ctx.mscore()->currentScore()->masterScore();
      // TODO: handle excerpts

      if (fileName.isEmpty()) {
            int scoreNum = 1;
            const QString templ("%1.mscx");
            fileName = templ.arg(QString::number(scoreNum));
            while (QFileInfo::exists(fileName))
                  fileName = templ.arg(QString::number(++scoreNum));
            }

      QString filePath = ctx.absoluteFilePath(fileName);
      QFileInfo fi(filePath);
      score->Score::saveFile(fi);

      if (ctx.relativePaths())
            filePath = fileName;

      return std::unique_ptr<ScriptEntry>(new ScoreTestScriptEntry(filePath));
      }

//---------------------------------------------------------
//   ScoreTestScriptEntry::execute
//---------------------------------------------------------

bool ScoreTestScriptEntry::execute(ScriptContext& ctx) const
      {
      MasterScore* curScore = ctx.mscore()->currentScore()->masterScore();
      if (!curScore) {
            ctx.execLog() << "ScoreTestScriptEntry: no current score" << endl;
            return false;
            }

      QString refFilePath = ctx.absoluteFilePath(_refPath);
      std::unique_ptr<MasterScore> refScore(ctx.mscore()->readScore(refFilePath));
      if (!refScore) {
            ctx.execLog() << "reference score loaded with errors: " << refFilePath << endl;
            return false;
            }

      ScoreDiff diff(curScore, refScore.get(), /* textDiffOnly */ true);
      if (!diff.equal()) {
            ctx.execLog() << "ScoreTestScriptEntry: fail\n" << diff.rawDiff() << endl;
            return false;
            }
      return true;
      }
}
