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

#include "script.h"

#include "musescore.h"
#include "scoreview.h"
#include "scriptentry.h"
#include "testscript.h"

#include "libmscore/score.h"

namespace Ms {

//---------------------------------------------------------
//   ScriptContext
//---------------------------------------------------------

ScriptContext::ScriptContext(MuseScore* context)
   : _mscore(context), _cwd(QDir::current())
      {
      }

//---------------------------------------------------------
//   ScriptContext::execLog
//    Returns logging stream to be used by script entries
//    at execution time.
//    Current implementation returns a stream writing to
//    stdout performing its initialization if necessary.
//---------------------------------------------------------

QTextStream& ScriptContext::execLog()
      {
      if (!_execLog)
            _execLog.reset(new QTextStream(stdout));
      return *_execLog;
      }

//---------------------------------------------------------
//   Script::execute
//---------------------------------------------------------

bool Script::execute(ScriptContext& ctx) const
      {
      bool success = true;
      for (const auto& e : _entries) {
            if (e) {
                  if (!e->execute(ctx)) {
                        ctx.execLog() << "Script::execute: operation failed:" << e->serialize() << endl;
                        if (ctx.stopOnError())
                              return false;
                        success = false;
                        }
                  }
            }
      return success;
      }

//---------------------------------------------------------
//   Script::addFromLine
//---------------------------------------------------------

void Script::addFromLine(const QString& line)
      {
      addEntry(ScriptEntry::deserialize(line));
      }

//---------------------------------------------------------
//   Script::fromFile
//---------------------------------------------------------

std::unique_ptr<Script> Script::fromFile(QString fileName)
      {
      QFile file(fileName);
      if (!file.open(QIODevice::ReadOnly))
            return nullptr;
      QTextStream stream(&file);

      QString line;
      auto script = std::unique_ptr<Script>(new Script());
      while (stream.readLineInto(&line))
            script->addFromLine(line);
      return script;
      }

//---------------------------------------------------------
//   Script::writeToFile
//---------------------------------------------------------

void Script::writeToFile(QString fileName) const
      {
      QFile file(fileName);
      if (!file.open(QIODevice::WriteOnly))
            return;
      QTextStream stream(&file);

      for (const auto& e : _entries) {
            if (e)
                  stream << e->serialize() << '\n';
            }
      }

//---------------------------------------------------------
//   ScriptRecorder::ensureFileOpen
//---------------------------------------------------------

bool ScriptRecorder::ensureFileOpen()
      {
      if (_file.isOpen())
            return true;
      if (_file.open(QIODevice::WriteOnly)) {
            _stream.setDevice(&_file);
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   ScriptRecorder::syncRecord
//---------------------------------------------------------

void ScriptRecorder::syncRecord()
      {
      if (!_recording)
            return;
      if (!ensureFileOpen())
            return;
      for (; _recorded < _script.nentries(); ++_recorded)
            _stream << _script.entry(_recorded).serialize() << endl;
      }

//---------------------------------------------------------
//   ScriptRecorder::recordInitState
//---------------------------------------------------------

void ScriptRecorder::recordInitState()
      {
      if (_recording)
            _script.addEntry(new InitScriptEntry(_ctx));
      syncRecord();
      }

//---------------------------------------------------------
//   ScriptRecorder::recordCommand
//---------------------------------------------------------

void ScriptRecorder::recordCommand(const QString& name)
      {
      if (_recording)
            _script.addCommand(name);
      syncRecord();
      }

//---------------------------------------------------------
//   ScriptRecorder::recordPaletteElement
//---------------------------------------------------------

void ScriptRecorder::recordPaletteElement(Element* e)
      {
      if (_recording)
            _script.addEntry(PaletteElementScriptEntry::fromContext(e, _ctx));
      syncRecord();
      }

//---------------------------------------------------------
//   ScriptRecorder::recordCurrentScoreChange
//---------------------------------------------------------

void ScriptRecorder::recordCurrentScoreChange()
      {
      if (_recording)
            _script.addEntry(ExcerptChangeScriptEntry::fromContext(_ctx));
      syncRecord();
      }

//---------------------------------------------------------
//   ScriptRecorder::recordInspectorValueChange
//---------------------------------------------------------

void ScriptRecorder::recordInspectorValueChange(const Element* e, const InspectorItem& ii, const QVariant& value)
      {
      if (_recording)
            _script.addEntry(InspectorScriptEntry::fromContext(e, ii, value));
      syncRecord();
      }

//---------------------------------------------------------
//   ScriptRecorder::recordScoreTest
//---------------------------------------------------------

void ScriptRecorder::recordScoreTest(QString scoreName)
      {
      if (_recording)
            _script.addEntry(ScoreTestScriptEntry::fromContext(_ctx, scoreName));
      syncRecord();
      }
}
