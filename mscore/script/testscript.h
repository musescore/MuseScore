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

#ifndef __TESTSCRIPT_H__
#define __TESTSCRIPT_H__

#include "scriptentry.h"

namespace Ms {

class ScriptContext;

//---------------------------------------------------------
//   TestScriptEntry
//---------------------------------------------------------

class TestScriptEntry : public ScriptEntry {
   protected:
      static QString entryTemplate(const char* testType) { return ScriptEntry::entryTemplate(SCRIPT_TEST).arg(testType) + " %1"; }
   public:
      static constexpr const char* TEST_SCORE = "score";

      static std::unique_ptr<ScriptEntry> deserialize(const QStringList& tokens);
      };

//---------------------------------------------------------
//   ScoreTestScriptEntry
//---------------------------------------------------------

class ScoreTestScriptEntry : public TestScriptEntry {
      QString _refPath;
   public:
      ScoreTestScriptEntry(QString refPath) : _refPath(refPath) {}
      bool execute(ScriptContext& ctx) const override;
      QString serialize() const override { return entryTemplate(TEST_SCORE).arg(_refPath); }
      static std::unique_ptr<ScriptEntry> fromContext(const ScriptContext& ctx, QString fileName = QString());
      };

}     // namespace Ms
#endif
