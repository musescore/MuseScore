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

#ifndef __SCRIPT_H__
#define __SCRIPT_H__

#include "scriptentry.h"

namespace Ms {

class MasterScore;
class MuseScore;
class ScoreView;

//---------------------------------------------------------
//   ScriptContext
//---------------------------------------------------------

class ScriptContext {
      MuseScore* _mscore;
      QDir _cwd; // current working directory
      bool _relativePaths = true;
      bool _stopOnError = true;
      std::unique_ptr<QTextStream> _execLog;

   public:
      ScriptContext(MuseScore* mscore);

      MuseScore* mscore() { return _mscore; }
      const MuseScore* mscore() const { return _mscore; }

      const QDir& cwd() const { return _cwd; }
      void setCwd(QDir dir) { _cwd = dir; }
      bool relativePaths() const { return _relativePaths; }
      void setRelativePaths(bool rel) { _relativePaths = rel; }
      bool stopOnError() const { return _stopOnError; }
      void setStopOnError(bool stop) { _stopOnError = stop; }

      QString absoluteFilePath(const QString& filePath) const { return _cwd.absoluteFilePath(filePath); }
      QString relativeFilePath(const QString& filePath) const { return _cwd.relativeFilePath(filePath); }
      QTextStream& execLog(); // logging stream to be used while executing scripts
      };

//---------------------------------------------------------
//   Script
//---------------------------------------------------------

class Script final {
      std::vector<std::unique_ptr<ScriptEntry>> _entries;

   public:
      const ScriptEntry& entry(size_t n) const { return *_entries[n]; }
      const ScriptEntry& lastEntry() const { return *_entries.back(); }
      size_t nentries() const { return _entries.size(); }
      bool empty() const { return _entries.empty(); }

      bool execute(ScriptContext& ctx) const;

      void clear() { _entries.clear(); }
      void addEntry(std::unique_ptr<ScriptEntry>&& e) { if (e) _entries.push_back(std::move(e)); }
      void addEntry(ScriptEntry* e) { _entries.emplace_back(e); }
      void addCommand(const QByteArray& cmd) { _entries.emplace_back(new CommandScriptEntry(cmd)); }
      void addCommand(const QString& cmd) { addCommand(cmd.toLatin1()); }

      void addFromLine(const QString& line);

      static std::unique_ptr<Script> fromFile(QString fileName);
      void writeToFile(QString fileName) const;
      };

//---------------------------------------------------------
//   ScriptRecorder
//    Records script writing it to a file just on the fly
//    so that the script is preserved in case actions
//    series wasn't finished properly (e.g. in case of
//    MuseScore crash).
//---------------------------------------------------------

class ScriptRecorder {
      QFile _file;
      QTextStream _stream;
      Script _script;
      ScriptContext _ctx;

      bool _recording = false;
      size_t _recorded = 0;

      bool ensureFileOpen();
      void syncRecord();

   public:
      ScriptRecorder(MuseScore* context) : _ctx(context) {}

      bool isRecording() const { return _recording; }
      void setRecording(bool r) { _recording = r; }

      void close() { setRecording(false); _file.close(); _script.clear(); _recorded = 0; }
      void setFile(QString fileName) { close(); _file.setFileName(fileName); }
      ScriptContext& context() { return _ctx; }
      const ScriptContext& context() const { return _ctx; }

      void recordInitState();
      void recordCommand(const QString& name);
      void recordPaletteElement(Element* e);
      void recordInspectorValueChange(const Element*, const InspectorItem&, const QVariant& value);
      void recordCurrentScoreChange();
      void recordScoreTest(QString scoreName = QString());
      };

}     // namespace Ms
#endif
