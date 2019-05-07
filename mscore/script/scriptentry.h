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

#ifndef __SCRIPTENTRY_H__
#define __SCRIPTENTRY_H__

#include "libmscore/element.h"

namespace Ms {

struct InspectorItem;
class ScriptContext;

//---------------------------------------------------------
//   ScriptEntry
//---------------------------------------------------------

class ScriptEntry {
   protected:
      static constexpr const char* SCRIPT_INIT = "init";
      static constexpr const char* SCRIPT_CMD = "cmd";
      static constexpr const char* SCRIPT_PALETTE = "palette";
      static constexpr const char* SCRIPT_INSPECTOR = "inspector";
      static constexpr const char* SCRIPT_EXCERPT_CHANGE = "excerpt";
      static constexpr const char* SCRIPT_TEST = "test";

      static QString entryTemplate(const char* entryType) { return QString("%1 %2").arg(entryType); }

   public:
      virtual ~ScriptEntry() = default;
      virtual bool execute(ScriptContext& ctx) const = 0;
      virtual QString serialize() const = 0;
      static std::unique_ptr<ScriptEntry> deserialize(const QString& line);
      };

//---------------------------------------------------------
//   InitScriptEntry
//    Initializes the environment for the script.
//---------------------------------------------------------

class InitScriptEntry : public ScriptEntry {
      QString _filePath;
   public:
      explicit InitScriptEntry(const ScriptContext& ctx);
      explicit InitScriptEntry(const QString& filePath) : _filePath(filePath) {}
      bool execute(ScriptContext& ctx) const override;
      QString serialize() const override;
      static std::unique_ptr<ScriptEntry> deserialize(const QStringList& tokens);
      };

//---------------------------------------------------------
//   CommandScriptEntry
//---------------------------------------------------------

class CommandScriptEntry : public ScriptEntry {
      QByteArray _command;
   public:
      explicit CommandScriptEntry(const QByteArray& cmd) : _command(cmd) {}
      explicit CommandScriptEntry(const QString& cmd) : _command(cmd.toLatin1()) {}
      explicit CommandScriptEntry(const char* cmd) : _command(cmd) {}
      bool execute(ScriptContext& ctx) const override;
      QString serialize() const override { return entryTemplate(SCRIPT_CMD).arg(_command.constData()); };
      };

//---------------------------------------------------------
//   PaletteElementScriptEntry
//---------------------------------------------------------

class PaletteElementScriptEntry : public ScriptEntry {
      ElementType _type;
      std::vector<std::pair<Pid, QString>> _props;
      static const std::initializer_list<Pid> _pids;

      PaletteElementScriptEntry(ElementType type, std::vector<std::pair<Pid, QString>>&& props) : _type(type), _props(props) {}

   public:
      bool execute(ScriptContext& ctx) const override;
      static std::unique_ptr<ScriptEntry> fromContext(const Element* e, ScriptContext& ctx);
      QString serialize() const override;
      static std::unique_ptr<ScriptEntry> deserialize(const QStringList& tokens);
      };

//---------------------------------------------------------
//   InspectorScriptEntry
//---------------------------------------------------------

class InspectorScriptEntry : public ScriptEntry {
      ElementType _type;
      int _parentLevel;
      Pid _pid;
      QVariant _val;

   public:
      InspectorScriptEntry(ElementType type, int parentLevel, Pid pid, QVariant value)
         : _type(type), _parentLevel(parentLevel), _pid(pid), _val(std::move(value)) {}
      bool execute(ScriptContext& ctx) const override;
      QString serialize() const override;
      static std::unique_ptr<ScriptEntry> fromContext(const Element*, const InspectorItem&, const QVariant&);
      static std::unique_ptr<ScriptEntry> deserialize(const QStringList& tokens);
      };

//---------------------------------------------------------
//   ExcerptChangeEntry
///   Represents switching between excerpt tabs
//---------------------------------------------------------

class ExcerptChangeScriptEntry : public ScriptEntry {
      int _index;

   public:
      ExcerptChangeScriptEntry(int idx) : _index(idx) {}
      bool execute(ScriptContext& ctx) const override;
      QString serialize() const override;
      static std::unique_ptr<ScriptEntry> fromContext(const ScriptContext&);
      static std::unique_ptr<ScriptEntry> deserialize(const QStringList& tokens);
      };
}     // namespace Ms
#endif
