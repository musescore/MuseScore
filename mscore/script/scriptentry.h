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

class ScriptContext;

//---------------------------------------------------------
//   ScriptEntry
//---------------------------------------------------------

class ScriptEntry {
   protected:
      static constexpr const char* SCRIPT_INIT = "init";
      static constexpr const char* SCRIPT_CMD = "cmd";
      static constexpr const char* SCRIPT_PALETTE = "palette";
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
      QString _subtype;

      static QString getElementSubtype(Element* e);
   public:
      PaletteElementScriptEntry(ElementType type, QString subtype) : _type(type), _subtype(subtype) {}
      explicit PaletteElementScriptEntry(Element* e) : _type(e->type()), _subtype(getElementSubtype(e)) {}
      bool execute(ScriptContext& ctx) const override;
      QString serialize() const override;
      static std::unique_ptr<ScriptEntry> deserialize(const QStringList& tokens);
      };

}     // namespace Ms
#endif
