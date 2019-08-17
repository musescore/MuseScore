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
#include "inspector/inspector.h"
#include "palette.h"
#include "palettebox.h"
#include "scoretab.h"
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
      if (type == SCRIPT_INSPECTOR)
            return InspectorScriptEntry::deserialize(tokens);
      if (type == SCRIPT_EXCERPT_CHANGE)
            return ExcerptChangeScriptEntry::deserialize(tokens);
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
//   serializePropertyValue
//---------------------------------------------------------

static const QString whitespaceRepr("&#032");

static QString serializePropertyValue(Pid pid, const QVariant& val)
      {
      return propertyToString(pid, val, /* mscx */ false).replace(' ', whitespaceRepr);
      }

//---------------------------------------------------------
//   deserializePropertyValue
//---------------------------------------------------------

static QVariant deserializePropertyValue(Pid pid, const QString& val)
      {
      return propertyFromString(pid, QString(val).replace(whitespaceRepr, " "));
      }


//---------------------------------------------------------
//   deserializePropertyId
//---------------------------------------------------------

static Pid deserializePropertyId(ElementType type, const QString& name)
      {
      // HACK: create a temporary element to make use of the virtual
      // ScoreElement::propertyId() function.
      std::unique_ptr<Element> tmpEl(Element::create(type, gscore));
      if (!tmpEl)
            return Pid::END;
      return tmpEl->propertyId(QStringRef(&name));
      }

//---------------------------------------------------------
//   PaletteCellInfo
//---------------------------------------------------------

struct PaletteCellInfo {
      Palette* palette;
      int idx;
      const Element* e;
      };

//---------------------------------------------------------
//   PaletteElementScriptEntry::_pids
//    List of PIDs used to distinguish palette elements,
//    sorted by their priority (PIDs from the beginning of
//    this list will be used first).
//---------------------------------------------------------

const std::initializer_list<Pid> PaletteElementScriptEntry::_pids {
      Pid::DYNAMIC_TYPE, // Above SUBTYPE as Dynamic has also SUBTYPE property assigned. TODO: deassign SUBTYPE from Dynamic?
      Pid::SUBTYPE,
      Pid::LABEL, // Segno, Coda and other markers. Should be above SUB_STYLE as the latter is redundant for Markers.
      Pid::JUMP_TO,     // Jumps
      Pid::PLAY_UNTIL,  // ...
      Pid::CONTINUE_AT, // (end Jumps)
      Pid::SUB_STYLE,
      Pid::ACTION, // Icons, e.g. grace notes, beam mode etc.
      Pid::BARLINE_TYPE,
      Pid::BARLINE_SPAN_FROM,
      Pid::BARLINE_SPAN_TO,
      Pid::LAYOUT_BREAK,
      Pid::ACCIDENTAL_TYPE,
      Pid::GLISS_TYPE,
      Pid::TIMESIG,
      Pid::TIMESIG_TYPE,
      Pid::SYMBOL,
      Pid::SYSTEM_BRACKET,
      Pid::HAIRPIN_TYPE,
      Pid::HEAD_GROUP,
      Pid::OTTAVA_TYPE,
      Pid::CLEF_TYPE_CONCERT,
      Pid::KEY,
      Pid::TRILL_TYPE,
      Pid::VIBRATO_TYPE,
      Pid::ARPEGGIO_TYPE,
      Pid::CHORD_LINE_TYPE,
      Pid::CHORD_LINE_STRAIGHT,
      Pid::TREMOLO_TYPE,

      // TextLineBase descendants
      Pid::BEGIN_HOOK_TYPE,
      Pid::END_HOOK_TYPE,
      Pid::BEGIN_TEXT,
      Pid::END_TEXT,
      Pid::CONTINUE_TEXT,

      Pid::TEXT,
      };

//---------------------------------------------------------
//   PaletteElementScriptEntry::paletteElements
//---------------------------------------------------------

static std::vector<PaletteCellInfo> getPaletteCells(ElementType elType, ScriptContext& ctx)
      {
      std::vector<PaletteCellInfo> cells;
      const PaletteBox* box = ctx.mscore()->getPaletteBox();
      if (!box)
            return cells;
      for (Palette* p : box->palettes()) {
            const int n = p->size();
            for (int i = 0; i < n; ++i) {
                  const Element* e = p->element(i);
                  if (e && e->type() == elType)
                        cells.push_back({ p, i, e });
                  }
            }
      return cells;
      }

//---------------------------------------------------------
//   PaletteElementScriptEntry::execute
//---------------------------------------------------------

bool PaletteElementScriptEntry::execute(ScriptContext& ctx) const
      {
      const auto cells(getPaletteCells(_type, ctx));

      for (const PaletteCellInfo& c : cells) {
            bool match = true;
            for (const auto& p : _props) {
                  const Pid pid = p.first;
                  const QVariant pVal = c.e->getProperty(pid);
                  if (serializePropertyValue(pid, pVal) != p.second) {
                        match = false;
                        break;
                        }
                  }

            if (match) {
                  c.palette->setCurrentIdx(c.idx);
                  c.palette->applyPaletteElement();
                  return true;
                  }
            }

      return false;
      }

//---------------------------------------------------------
//   PaletteElementScriptEntry::fromContext
//---------------------------------------------------------

std::unique_ptr<ScriptEntry> PaletteElementScriptEntry::fromContext(const Element* e, ScriptContext& ctx)
      {
      const ElementType type = e->type();
      std::vector<std::pair<Pid, QString>> props;
      auto cells(getPaletteCells(type, ctx));

      if (cells.empty())
            return nullptr;

      if (cells.size() > 1) {
            for (Pid pid : PaletteElementScriptEntry::_pids) {
                  const QVariant val = e->getProperty(pid);
                  bool sameValue = false;
                  if (!val.isValid())
                        continue;
                  for (PaletteCellInfo& c : cells) {
                        if (c.e == e || !c.e)
                              continue;
                        if (c.e->getProperty(pid) == val)
                              sameValue = true;
                        else
                              c.e = nullptr; // excude it from further comparisons
                        }

                  props.emplace_back(pid, serializePropertyValue(pid, val));
                  if (!sameValue)
                        break;
                  }
            }

      return std::unique_ptr<ScriptEntry>(new PaletteElementScriptEntry(type, std::move(props)));
      }

//---------------------------------------------------------
//   PaletteElementScriptEntry::serialize
//---------------------------------------------------------

QString PaletteElementScriptEntry::serialize() const
      {
      QStringList tokens(ScoreElement::name(_type));
      for (const auto& p : _props)
            tokens << propertyName(p.first) << p.second;
      return entryTemplate(SCRIPT_PALETTE).arg(tokens.join(' '));
      }

//---------------------------------------------------------
//   PaletteElementScriptEntry::deserialize
//---------------------------------------------------------

std::unique_ptr<ScriptEntry> PaletteElementScriptEntry::deserialize(const QStringList& tokens)
      {
      const int ntokens = tokens.size();
      if (ntokens < 2) {
            qWarning("palette: unexpected number of tokens: %d", ntokens);
            return nullptr;
            }
      const ElementType type = ScoreElement::name2type(QStringRef(&tokens[1]));
      std::vector<std::pair<Pid, QString>> props;
      const int propTokenIdx = 2;
      if (ntokens > propTokenIdx) {
            if ((ntokens - propTokenIdx) % 2) {
                  qWarning("palette: unexpected number of tokens: %d", ntokens);
                  return nullptr;
                  }
            for (int i = propTokenIdx; i < ntokens; i += 2) {
                  const Pid pid = deserializePropertyId(type, tokens[i]);
                  const QString value = tokens[i+1];
                  props.emplace_back(pid, value);
                  }
            }
      return std::unique_ptr<ScriptEntry>(new PaletteElementScriptEntry(type, std::move(props)));
      }


//---------------------------------------------------------
//   InspectorScriptEntry::execute
//---------------------------------------------------------

bool InspectorScriptEntry::execute(ScriptContext& ctx) const
      {
      Inspector* i = ctx.mscore()->inspector();
      if (!i)
            return false;
      InspectorBase* ib = i->ie;
      if (!ib)
            return false;

      const auto iiList = ib->iList;
      for (int idx = 0; idx < int(iiList.size()); ++idx) {
            const InspectorItem& ii = iiList[idx];
            if (ii.t == _pid && ii.parent == _parentLevel) {
                  const Score* score = ctx.mscore()->currentScore();
                  if (!score) {
                        qWarning() << "InspectorScriptEntry: no score!";
                        return false;
                        }
                  const auto scoreState = score->state();
                  ib->setValue(ii, _val);
                  if (scoreState == score->state()) // probably valueChanged wasn't triggered
                        ib->valueChanged(idx);
                  return true;
                  }
            }
      return false;
      }

//---------------------------------------------------------
//   InspectorScriptEntry::serialize
//---------------------------------------------------------

QString InspectorScriptEntry::serialize() const
      {
      QStringList tokens;
      tokens << ScoreElement::name(_type) << propertyName(_pid) << serializePropertyValue(_pid, _val);
      if (_parentLevel)
            tokens << "parent" << QString::number(_parentLevel);
      return entryTemplate(SCRIPT_INSPECTOR).arg(tokens.join(' '));
      }

//---------------------------------------------------------
//   InspectorScriptEntry::deserialize
//---------------------------------------------------------

std::unique_ptr<ScriptEntry> InspectorScriptEntry::deserialize(const QStringList& tokens)
      {
      if (tokens.size() < 4) {
            qWarning("palette: unexpected number of tokens: %d", tokens.size());
            return nullptr;
            }

      const ElementType type = ScoreElement::name2type(tokens[1]);
      const Pid pid = deserializePropertyId(type, tokens[2]);
      const QVariant val = deserializePropertyValue(pid, tokens[3]);
      int parentLevel = 0;
      if (tokens.size() > 5 && tokens[4] == "parent")
            parentLevel = tokens[5].toInt();

      return std::unique_ptr<ScriptEntry>(new InspectorScriptEntry(type, parentLevel, pid, val));
      }

//---------------------------------------------------------
//   InspectorScriptEntry::fromContext
//---------------------------------------------------------

std::unique_ptr<ScriptEntry> InspectorScriptEntry::fromContext(const Element* e, const InspectorItem& ii, const QVariant& value)
      {
      for (int i = 0; i < ii.parent; ++i)
            e = e->parent();
      return std::unique_ptr<ScriptEntry>(new InspectorScriptEntry(e->type(), ii.parent, ii.t, value));
      }

//---------------------------------------------------------
//   ExcerptChangeScriptEntry::execute
//---------------------------------------------------------

bool ExcerptChangeScriptEntry::execute(ScriptContext& ctx) const
      {
      ctx.mscore()->currentScoreTab()->setExcerpt(_index);
      return true;
      }

//---------------------------------------------------------
//   ExcerptChangeScriptEntry::serialize
//---------------------------------------------------------

QString ExcerptChangeScriptEntry::serialize() const
      {
      return entryTemplate(SCRIPT_EXCERPT_CHANGE).arg(QString::number(_index));
      }

//---------------------------------------------------------
//   ExcerptChangeScriptEntry::deserialize
//---------------------------------------------------------

std::unique_ptr<ScriptEntry> ExcerptChangeScriptEntry::deserialize(const QStringList& tokens)
      {
      if (tokens.size() < 2) {
            qWarning("excerpt change: unexpected number of tokens: %d", tokens.size());
            return nullptr;
            }

      bool ok = false;
      const int index = tokens[1].toInt(&ok);
      if (!ok) {
            qWarning("excerpt change: argument is not a number");
            return nullptr;
            }

      return std::unique_ptr<ScriptEntry>(new ExcerptChangeScriptEntry(index));
      }

//---------------------------------------------------------
//   ExcerptChangeScriptEntry::fromContext
//---------------------------------------------------------

std::unique_ptr<ScriptEntry> ExcerptChangeScriptEntry::fromContext(const ScriptContext& ctx)
      {
      const Score* s = ctx.mscore()->currentScore();
      int index = 0;
      if (!s->isMaster()) {
            const auto& scores = s->masterScore()->scoreList();
            index = std::find(scores.begin(), scores.end(), s) - scores.begin();
            }
      return std::unique_ptr<ScriptEntry>(new ExcerptChangeScriptEntry(index));
      }
}
