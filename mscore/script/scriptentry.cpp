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
#include "palette.h"
#include "palettebox.h"
#include "script.h"
#include "testscript.h"

#include "libmscore/clef.h"
#include "libmscore/element.h"
#include "libmscore/icon.h"
#include "libmscore/keysig.h"
#include "libmscore/property.h"
#include "libmscore/score.h"
#include "libmscore/scoreElement.h"

namespace Ms {

//---------------------------------------------------------
//   ScriptEntry::deserialize
//---------------------------------------------------------

std::unique_ptr<ScriptEntry> ScriptEntry::deserialize(const QString& line)
      {
      if (line.isEmpty() || line.startsWith('#')) // comment or empty line
            return nullptr;
      QStringList tokens = line.split(' ');
      const QString& type = tokens[0];
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
      if (type == SCRIPT_PALETTE)
            return PaletteElementScriptEntry::deserialize(tokens);
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
      ctx.mscore()->cmd(getAction(_command.constData()));
      return true;
      }

//---------------------------------------------------------
//   PaletteElementScriptEntry::getElementSubtype
//---------------------------------------------------------

QString PaletteElementScriptEntry::getElementSubtype(Element* e)
      {
      if (!e)
            return QString();
      if (e->isIcon()) // we are interested in action really
            return toIcon(e)->action();
      else if (e->isClef()) // it is not too trivial to assign Pids to the properties of the following types
            return toClef(e)->clefTypeName();
      else if (e->isKeySig())
            return QString::number(int(toKeySig(e)->key()));
      Pid pid = e->subtypePid();
      QVariant subtype = e->getProperty(pid);
      QString subtypeName = propertyWritableValue(pid, subtype);
      return subtypeName;
      }

//---------------------------------------------------------
//   PaletteElementScriptEntry::execute
//---------------------------------------------------------

bool PaletteElementScriptEntry::execute(ScriptContext& ctx) const
      {
      PaletteBox* box = ctx.mscore()->getPaletteBox();
      if (!box)
            return false;
      for (Palette* p : box->palettes()) {
            const int n = p->size();
            for (int i = 0; i < n; ++i) {
                  Element* e = p->element(i);
                  if (e && e->type() == _type && getElementSubtype(e) == _subtype) {
                        p->setCurrentIdx(i);
                        p->applyPaletteElement();
                        return true;
                        }
                  }
            }
      return false;
      }

//---------------------------------------------------------
//   PaletteElementScriptEntry::serialize
//---------------------------------------------------------

QString PaletteElementScriptEntry::serialize() const
      {
      QString info = QString("%1 %2")
         .arg(ScoreElement::name(_type))
         .arg(_subtype);
      return entryTemplate(SCRIPT_PALETTE).arg(info);
      }

//---------------------------------------------------------
//   PaletteElementScriptEntry::deserialize
//---------------------------------------------------------

std::unique_ptr<ScriptEntry> PaletteElementScriptEntry::deserialize(const QStringList& tokens)
      {
      if (tokens.size() < 2) {
            qWarning("palette: unexpected number of tokens: %d", tokens.size());
            return nullptr;
            }
      ElementType type = ScoreElement::name2type(QStringRef(&tokens[1]));
      QString subtype;
      if (tokens.size() >= 3)
            subtype = tokens[2];
      return std::unique_ptr<ScriptEntry>(new PaletteElementScriptEntry(type, subtype));
      }
}
