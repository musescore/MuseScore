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

#include "scriptentry.h"

#include "musescore.h"
#include "script.h"
#include "testscript.h"

#include "libmscore/score.h"

namespace Ms {

//---------------------------------------------------------
//   ScriptEntry::deserialize
//---------------------------------------------------------

std::unique_ptr<ScriptEntry> ScriptEntry::deserialize(const QString& line)
      {
      QStringList tokens = line.split(' ');
      const QString& type = tokens[0];
      if (tokens.empty()) {
            qWarning("Script line is empty");
            return nullptr;
            }
      if (type == SCRIPT_CMD) {
            if (tokens.size() != 2) {
                  qWarning("Unexpected number of tokens in command: %d", tokens.size());
                  return nullptr;
                  }
            return std::unique_ptr<ScriptEntry>(new CommandScriptEntry(tokens[1]));
            }
      if (type == SCRIPT_INIT)
            return InitScriptEntry::deserialize(tokens);
      if (type == SCRIPT_TEST)
            return TestScriptEntry::deserialize(tokens);
      qWarning() << "Unsupported action type:" << type;
      return nullptr;
      }

//---------------------------------------------------------
//   InitScriptEntry::InitScriptEntry
//---------------------------------------------------------

InitScriptEntry::InitScriptEntry(const ScriptContext& ctx)
      {
      MasterScore* score = ctx.mscore()->currentScore()->masterScore();
      _filePath = score->importedFilePath();
      if (ctx.relativePaths())
            _filePath = ctx.relativeFilePath(_filePath);
      // TODO: handle excerpts
      }

//---------------------------------------------------------
//   InitScriptEntry::execute
//---------------------------------------------------------

bool InitScriptEntry::execute(ScriptContext& ctx) const
      {
      return ctx.mscore()->openScore(ctx.absoluteFilePath(_filePath));
      }

//---------------------------------------------------------
//   InitScriptEntry::execute
//---------------------------------------------------------

QString InitScriptEntry::serialize() const
      {
      // TODO: handle spaces in file path!
      return entryTemplate(SCRIPT_INIT).arg(_filePath);
      }

//---------------------------------------------------------
//   InitScriptEntry::deserialize
//---------------------------------------------------------

std::unique_ptr<ScriptEntry> InitScriptEntry::deserialize(const QStringList& tokens)
      {
      if (tokens.size() != 2) {
            qWarning("init: unexpected number of tokens: %d", tokens.size());
            return nullptr;
            }
      return std::unique_ptr<ScriptEntry>(new InitScriptEntry(tokens[1]));
      }

//---------------------------------------------------------
//   CommandScriptEntry::execute
//---------------------------------------------------------

bool CommandScriptEntry::execute(ScriptContext& ctx) const
      {
//       ctx.mscore()->cmd(getAction(_command.constData()), _command);
      Script::execCmd(ctx.mscore(), getAction(_command.constData()), _command);
      return true;
      }

}
