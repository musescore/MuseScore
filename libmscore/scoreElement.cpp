//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2015 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "scoreElement.h"
#include "score.h"
#include "undo.h"
#include "xml.h"
#include "bracket.h"
#include "bracketItem.h"
#include "spanner.h"
#include "musescoreCore.h"

namespace Ms {

ElementStyle const ScoreElement::emptyStyle;

//
// list has to be synchronized with ElementType enum
//
static const ElementName elementNames[] = {
      { ElementType::INVALID,              "invalid",              QT_TRANSLATE_NOOP("elementName", "invalid") },
      { ElementType::BRACKET_ITEM,         "BracketItem",          QT_TRANSLATE_NOOP("elementName", "BracketItem") },
      { ElementType::PART,                 "Part",                 QT_TRANSLATE_NOOP("elementName", "Part") },
      { ElementType::STAFF,                "Staff",                QT_TRANSLATE_NOOP("elementName", "Staff") },
      { ElementType::SCORE,                "Score",                QT_TRANSLATE_NOOP("elementName", "Score") },
      { ElementType::SYMBOL,               "Symbol",               QT_TRANSLATE_NOOP("elementName", "Symbol") },
      { ElementType::TEXT,                 "Text",                 QT_TRANSLATE_NOOP("elementName", "Text") },
      { ElementType::MEASURE_NUMBER,       "MeasureNumber",        QT_TRANSLATE_NOOP("elementName", "Measure Number") },
      { ElementType::INSTRUMENT_NAME,      "InstrumentName",       QT_TRANSLATE_NOOP("elementName", "Instrument Name") },
      { ElementType::SLUR_SEGMENT,         "SlurSegment",          QT_TRANSLATE_NOOP("elementName", "Slur Segment") },
      { ElementType::TIE_SEGMENT,          "TieSegment",           QT_TRANSLATE_NOOP("elementName", "Tie Segment") },
      { ElementType::BAR_LINE,             "BarLine",              QT_TRANSLATE_NOOP("elementName", "Barline") },
      { ElementType::STAFF_LINES,          "StaffLines",           QT_TRANSLATE_NOOP("elementName", "Staff Lines") },
      { ElementType::SYSTEM_DIVIDER,       "SystemDivider",        QT_TRANSLATE_NOOP("elementName", "System Divider") },
      { ElementType::STEM_SLASH,           "StemSlash",            QT_TRANSLATE_NOOP("elementName", "Stem Slash") },
      { ElementType::ARPEGGIO,             "Arpeggio",             QT_TRANSLATE_NOOP("elementName", "Arpeggio") },
      { ElementType::ACCIDENTAL,           "Accidental",           QT_TRANSLATE_NOOP("elementName", "Accidental") },
      { ElementType::LEDGER_LINE,          "LedgerLine",           QT_TRANSLATE_NOOP("elementName", "Ledger Line") },
      { ElementType::STEM,                 "Stem",                 QT_TRANSLATE_NOOP("elementName", "Stem") },
      { ElementType::NOTE,                 "Note",                 QT_TRANSLATE_NOOP("elementName", "Note") },
      { ElementType::CLEF,                 "Clef",                 QT_TRANSLATE_NOOP("elementName", "Clef") },
      { ElementType::KEYSIG,               "KeySig",               QT_TRANSLATE_NOOP("elementName", "Key Signature") },
      { ElementType::AMBITUS,              "Ambitus",              QT_TRANSLATE_NOOP("elementName", "Ambitus") },
      { ElementType::TIMESIG,              "TimeSig",              QT_TRANSLATE_NOOP("elementName", "Time Signature") },
      { ElementType::REST,                 "Rest",                 QT_TRANSLATE_NOOP("elementName", "Rest") },
      { ElementType::BREATH,               "Breath",               QT_TRANSLATE_NOOP("elementName", "Breath") },
      { ElementType::REPEAT_MEASURE,       "RepeatMeasure",        QT_TRANSLATE_NOOP("elementName", "Repeat Measure") },
      { ElementType::TIE,                  "Tie",                  QT_TRANSLATE_NOOP("elementName", "Tie") },
      { ElementType::ARTICULATION,         "Articulation",         QT_TRANSLATE_NOOP("elementName", "Articulation") },
      { ElementType::FERMATA,              "Fermata",              QT_TRANSLATE_NOOP("elementName", "Fermata") },
      { ElementType::CHORDLINE,            "ChordLine",            QT_TRANSLATE_NOOP("elementName", "Chord Line") },
      { ElementType::DYNAMIC,              "Dynamic",              QT_TRANSLATE_NOOP("elementName", "Dynamic") },
      { ElementType::BEAM,                 "Beam",                 QT_TRANSLATE_NOOP("elementName", "Beam") },
      { ElementType::HOOK,                 "Hook",                 QT_TRANSLATE_NOOP("elementName", "Hook") },
      { ElementType::LYRICS,               "Lyrics",               QT_TRANSLATE_NOOP("elementName", "Lyrics") },
      { ElementType::FIGURED_BASS,         "FiguredBass",          QT_TRANSLATE_NOOP("elementName", "Figured Bass") },
      { ElementType::MARKER,               "Marker",               QT_TRANSLATE_NOOP("elementName", "Marker") },
      { ElementType::JUMP,                 "Jump",                 QT_TRANSLATE_NOOP("elementName", "Jump") },
      { ElementType::FINGERING,            "Fingering",            QT_TRANSLATE_NOOP("elementName", "Fingering") },
      { ElementType::TUPLET,               "Tuplet",               QT_TRANSLATE_NOOP("elementName", "Tuplet") },
      { ElementType::TEMPO_TEXT,           "Tempo",                QT_TRANSLATE_NOOP("elementName", "Tempo") },
      { ElementType::STAFF_TEXT,           "StaffText",            QT_TRANSLATE_NOOP("elementName", "Staff Text") },
      { ElementType::SYSTEM_TEXT,          "SystemText",           QT_TRANSLATE_NOOP("elementName", "System Text") },
      { ElementType::REHEARSAL_MARK,       "RehearsalMark",        QT_TRANSLATE_NOOP("elementName", "Rehearsal Mark") },
      { ElementType::INSTRUMENT_CHANGE,    "InstrumentChange",     QT_TRANSLATE_NOOP("elementName", "Instrument Change") },
      { ElementType::STAFFTYPE_CHANGE,     "StaffTypeChange",      QT_TRANSLATE_NOOP("elementName", "Staff Type Change") },
      { ElementType::HARMONY,              "Harmony",              QT_TRANSLATE_NOOP("elementName", "Chord Symbol") },
      { ElementType::FRET_DIAGRAM,         "FretDiagram",          QT_TRANSLATE_NOOP("elementName", "Fretboard Diagram") },
      { ElementType::BEND,                 "Bend",                 QT_TRANSLATE_NOOP("elementName", "Bend") },
      { ElementType::TREMOLOBAR,           "TremoloBar",           QT_TRANSLATE_NOOP("elementName", "Tremolo Bar") },
      { ElementType::VOLTA,                "Volta",                QT_TRANSLATE_NOOP("elementName", "Volta") },
      { ElementType::HAIRPIN_SEGMENT,      "HairpinSegment",       QT_TRANSLATE_NOOP("elementName", "Hairpin Segment") },
      { ElementType::OTTAVA_SEGMENT,       "OttavaSegment",        QT_TRANSLATE_NOOP("elementName", "Ottava Segment") },
      { ElementType::TRILL_SEGMENT,        "TrillSegment",         QT_TRANSLATE_NOOP("elementName", "Trill Segment") },
      { ElementType::LET_RING_SEGMENT,     "LetRingSegment",       QT_TRANSLATE_NOOP("elementName", "Let Ring Segment") },
      { ElementType::VIBRATO_SEGMENT,      "VibratoSegment",       QT_TRANSLATE_NOOP("elementName", "Vibrato Segment") },
      { ElementType::PALM_MUTE_SEGMENT,    "PalmMuteSegment",      QT_TRANSLATE_NOOP("elementName", "Palm Mute Segment") },
      { ElementType::TEXTLINE_SEGMENT,     "TextLineSegment",      QT_TRANSLATE_NOOP("elementName", "Text Line Segment") },
      { ElementType::VOLTA_SEGMENT,        "VoltaSegment",         QT_TRANSLATE_NOOP("elementName", "Volta Segment") },
      { ElementType::PEDAL_SEGMENT,        "PedalSegment",         QT_TRANSLATE_NOOP("elementName", "Pedal Segment") },
      { ElementType::LYRICSLINE_SEGMENT,   "LyricsLineSegment",    QT_TRANSLATE_NOOP("elementName", "Melisma Line Segment") },
      { ElementType::GLISSANDO_SEGMENT,    "GlissandoSegment",     QT_TRANSLATE_NOOP("elementName", "Glissando Segment") },
      { ElementType::LAYOUT_BREAK,         "LayoutBreak",          QT_TRANSLATE_NOOP("elementName", "Layout Break") },
      { ElementType::SPACER,               "Spacer",               QT_TRANSLATE_NOOP("elementName", "Spacer") },
      { ElementType::STAFF_STATE,          "StaffState",           QT_TRANSLATE_NOOP("elementName", "Staff State") },
      { ElementType::NOTEHEAD,             "NoteHead",             QT_TRANSLATE_NOOP("elementName", "Notehead") },
      { ElementType::NOTEDOT,              "NoteDot",              QT_TRANSLATE_NOOP("elementName", "Note Dot") },
      { ElementType::TREMOLO,              "Tremolo",              QT_TRANSLATE_NOOP("elementName", "Tremolo") },
      { ElementType::IMAGE,                "Image",                QT_TRANSLATE_NOOP("elementName", "Image") },
      { ElementType::MEASURE,              "Measure",              QT_TRANSLATE_NOOP("elementName", "Measure") },
      { ElementType::SELECTION,            "Selection",            QT_TRANSLATE_NOOP("elementName", "Selection") },
      { ElementType::LASSO,                "Lasso",                QT_TRANSLATE_NOOP("elementName", "Lasso") },
      { ElementType::SHADOW_NOTE,          "ShadowNote",           QT_TRANSLATE_NOOP("elementName", "Shadow Note") },
      { ElementType::TAB_DURATION_SYMBOL,  "TabDurationSymbol",    QT_TRANSLATE_NOOP("elementName", "Tab Duration Symbol") },
      { ElementType::FSYMBOL,              "FSymbol",              QT_TRANSLATE_NOOP("elementName", "Font Symbol") },
      { ElementType::PAGE,                 "Page",                 QT_TRANSLATE_NOOP("elementName", "Page") },
      { ElementType::HAIRPIN,              "HairPin",              QT_TRANSLATE_NOOP("elementName", "Hairpin") },
      { ElementType::OTTAVA,               "Ottava",               QT_TRANSLATE_NOOP("elementName", "Ottava") },
      { ElementType::PEDAL,                "Pedal",                QT_TRANSLATE_NOOP("elementName", "Pedal") },
      { ElementType::TRILL,                "Trill",                QT_TRANSLATE_NOOP("elementName", "Trill") },
      { ElementType::LET_RING,             "LetRing",              QT_TRANSLATE_NOOP("elementName", "Let Ring") },
      { ElementType::VIBRATO,              "Vibrato",              QT_TRANSLATE_NOOP("elementName", "Vibrato") },
      { ElementType::PALM_MUTE,            "PalmMute",             QT_TRANSLATE_NOOP("elementName", "Palm Mute") },
      { ElementType::TEXTLINE,             "TextLine",             QT_TRANSLATE_NOOP("elementName", "Text Line") },
      { ElementType::TEXTLINE_BASE,        "TextLineBase",         QT_TRANSLATE_NOOP("elementName", "Text Line Base") },  // remove
      { ElementType::NOTELINE,             "NoteLine",             QT_TRANSLATE_NOOP("elementName", "Note Line") },
      { ElementType::LYRICSLINE,           "LyricsLine",           QT_TRANSLATE_NOOP("elementName", "Melisma Line") },
      { ElementType::GLISSANDO,            "Glissando",            QT_TRANSLATE_NOOP("elementName", "Glissando") },
      { ElementType::BRACKET,              "Bracket",              QT_TRANSLATE_NOOP("elementName", "Bracket") },
      { ElementType::SEGMENT,              "Segment",              QT_TRANSLATE_NOOP("elementName", "Segment") },
      { ElementType::SYSTEM,               "System",               QT_TRANSLATE_NOOP("elementName", "System") },
      { ElementType::COMPOUND,             "Compound",             QT_TRANSLATE_NOOP("elementName", "Compound") },
      { ElementType::CHORD,                "Chord",                QT_TRANSLATE_NOOP("elementName", "Chord") },
      { ElementType::SLUR,                 "Slur",                 QT_TRANSLATE_NOOP("elementName", "Slur") },
      { ElementType::ELEMENT,              "Element",              QT_TRANSLATE_NOOP("elementName", "Element") },
      { ElementType::ELEMENT_LIST,         "ElementList",          QT_TRANSLATE_NOOP("elementName", "Element List") },
      { ElementType::STAFF_LIST,           "StaffList",            QT_TRANSLATE_NOOP("elementName", "Staff List") },
      { ElementType::MEASURE_LIST,         "MeasureList",          QT_TRANSLATE_NOOP("elementName", "Measure List") },
      { ElementType::HBOX,                 "HBox",                 QT_TRANSLATE_NOOP("elementName", "Horizontal Frame") },
      { ElementType::VBOX,                 "VBox",                 QT_TRANSLATE_NOOP("elementName", "Vertical Frame") },
      { ElementType::TBOX,                 "TBox",                 QT_TRANSLATE_NOOP("elementName", "Text Frame") },
      { ElementType::FBOX,                 "FBox",                 QT_TRANSLATE_NOOP("elementName", "Fretboard Diagram Frame") },
      { ElementType::ICON,                 "Icon",                 QT_TRANSLATE_NOOP("elementName", "Icon") },
      { ElementType::OSSIA,                "Ossia",                QT_TRANSLATE_NOOP("elementName", "Ossia") },
      { ElementType::BAGPIPE_EMBELLISHMENT,"BagpipeEmbellishment", QT_TRANSLATE_NOOP("elementName", "Bagpipe Embellishment") }
      };

//---------------------------------------------------------
//   ScoreElement
//---------------------------------------------------------

ScoreElement::ScoreElement(const ScoreElement& se)
      {
      _score        = se._score;
      _elementStyle = se._elementStyle;
      if (_elementStyle) {
            size_t n = _elementStyle->size();
            _propertyFlagsList = new PropertyFlags[n];
            for (size_t i = 0; i < n; ++i)
                  _propertyFlagsList[i] = se._propertyFlagsList[i];
            }
      _links = 0;
      }

//---------------------------------------------------------
//   ~Element
//---------------------------------------------------------

ScoreElement::~ScoreElement()
      {
      if (_links) {
            _links->removeOne(this);
            if (_links->empty()) {
                  delete _links;
                  _links = 0;
                  }
            }
      delete[] _propertyFlagsList;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant ScoreElement::propertyDefault(Pid pid, Tid tid) const
      {
      for (const StyledProperty& spp : *textStyle(tid)) {
            if (spp.pid == pid)
                  return score()->styleValue(pid, spp.sid);
            }
      return QVariant();
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant ScoreElement::propertyDefault(Pid pid) const
      {
      Sid sid = getPropertyStyle(pid);
      if (sid != Sid::NOSTYLE)
            return score()->styleValue(pid, sid);
//      qDebug("<%s>(%d) not found in <%s>", propertyQmlName(pid), int(pid), name());
      return QVariant();
      }

//---------------------------------------------------------
//   initElementStyle
//---------------------------------------------------------

void ScoreElement::initElementStyle(const ElementStyle* ss)
      {
      _elementStyle = ss;
      size_t n      = _elementStyle->size();
      delete[] _propertyFlagsList;
      _propertyFlagsList = new PropertyFlags[n];
      for (size_t i = 0; i < n; ++i)
            _propertyFlagsList[i] = PropertyFlags::STYLED;
      for (const StyledProperty& spp : *_elementStyle)
            setProperty(spp.pid, score()->styleValue(spp.pid, spp.sid));
      }

//---------------------------------------------------------
//   resetProperty
//---------------------------------------------------------

void ScoreElement::resetProperty(Pid pid)
      {
      QVariant v = propertyDefault(pid);
      if (v.isValid()) {
            setProperty(pid, v);
            PropertyFlags p = propertyFlags(pid);
            if (p != PropertyFlags::NOSTYLE)
                  setPropertyFlags(pid, PropertyFlags::STYLED);
            }
      }

//---------------------------------------------------------
//   undoResetProperty
//---------------------------------------------------------

void ScoreElement::undoResetProperty(Pid id)
      {
      PropertyFlags f = propertyFlags(id);
      if (f == PropertyFlags::UNSTYLED)
            undoChangeProperty(id, propertyDefault(id), PropertyFlags::STYLED);
      else
            undoChangeProperty(id, propertyDefault(id), f);
      }

//---------------------------------------------------------
//   isStyled
//---------------------------------------------------------

bool ScoreElement::isStyled(Pid pid) const
      {
      PropertyFlags f = propertyFlags(pid);
      return f == PropertyFlags::STYLED;
      }

//---------------------------------------------------------
//   changeProperty
//---------------------------------------------------------

static void changeProperty(ScoreElement* e, Pid t, const QVariant& st, PropertyFlags ps)
      {
      if (e->getProperty(t) != st || e->propertyFlags(t) != ps) {
            if (e->isBracketItem()) {
                  BracketItem* bi = toBracketItem(e);
                  e->score()->undo(new ChangeBracketProperty(bi->staff(), bi->column(), t, st, ps));
                  }
            else
                  e->score()->undo(new ChangeProperty(e, t, st, ps));
            }
      }

//---------------------------------------------------------
//   changeProperties
//---------------------------------------------------------

static void changeProperties(ScoreElement* e, Pid t, const QVariant& st, PropertyFlags ps)
      {
      if (propertyLink(t)) {
            for (ScoreElement* ee : e->linkList())
                  changeProperty(ee, t, st, ps);
            }
      else
            changeProperty(e, t, st, ps);
      }

//---------------------------------------------------------
//   undoChangeProperty
//---------------------------------------------------------

void ScoreElement::undoChangeProperty(Pid id, const QVariant& v)
      {
      undoChangeProperty(id, v, propertyFlags(id));
      }

void ScoreElement::undoChangeProperty(Pid id, const QVariant& v, PropertyFlags ps)
      {
      bool doUpdateInspector = false;
      if (isBracket()) {
            // brackets do not survive layout() and therefore cannot be on
            // the undo stack; delegate to BracketItem:

            BracketItem* bi = toBracket(this)->bracketItem();
            bi->undoChangeProperty(id, v, ps);
            return;
            }
      if (id == Pid::PLACEMENT) {
            // first set placment, then set offset for above/below if styled
            changeProperties(this, id, v, ps);
            if (isStyled(Pid::OFFSET))
                  ScoreElement::undoChangeProperty(Pid::OFFSET, score()->styleV(getPropertyStyle(Pid::OFFSET)).toPointF() * score()->spatium());
            doUpdateInspector = true;
            }
      if (id == Pid::SUB_STYLE) {
            //
            // change a list of properties
            //
            auto l = textStyle(Tid(v.toInt()));
            // Change to ElementStyle defaults
            for (const StyledProperty& p : *l) {
                  if (p.sid == Sid::NOSTYLE)
                        break;
                  changeProperties(this, p.pid, score()->styleV(p.sid), PropertyFlags::STYLED);
                  }
            }
      changeProperties(this, id, v, ps);
      if (id != Pid::GENERATED)
            changeProperties(this, Pid::GENERATED, QVariant(false), PropertyFlags::NOSTYLE);
      if (doUpdateInspector)
            MuseScoreCore::mscoreCore->updateInspector();
      }

//---------------------------------------------------------
//   undoPushProperty
//---------------------------------------------------------

void ScoreElement::undoPushProperty(Pid id)
      {
      QVariant val = getProperty(id);
      score()->undoStack()->push1(new ChangeProperty(this, id, val));
      }

//---------------------------------------------------------
//   readProperty
//---------------------------------------------------------

bool ScoreElement::readProperty(const QStringRef& s, XmlReader& e, Pid id)
      {
      if (s == propertyName(id)) {
            QVariant v = Ms::getProperty(id, e);
            switch (propertyType(id)) {
                  case P_TYPE::SP_REAL:
                        v = v.toReal() * score()->spatium();
                        break;
                  case P_TYPE::POINT_SP_MM:
                        if (sizeIsSpatiumDependent())
                              v = v.toPointF() * score()->spatium();
                        else
                              v = v.toPointF() * DPMM;
                        break;
                  default:
                        break;
                  }
            setProperty(id, v);
            if (isStyled(id))
                  setPropertyFlags(id, PropertyFlags::UNSTYLED);
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   writeProperty
//---------------------------------------------------------

void ScoreElement::writeProperty(XmlWriter& xml, Pid id) const
      {
      if (isStyled(id))
            return;
      if (propertyType(id) == P_TYPE::SP_REAL) {
            qreal _spatium = score()->spatium();
            qreal f1       = getProperty(id).toReal();
            qreal f2       = propertyDefault(id).toReal();

            if (qAbs(f1 - f2) < 0.0001)
                  return;
            QVariant val          = QVariant(f1/_spatium);
            QVariant defaultValue = QVariant(f2/_spatium);
            xml.tag(id, val, defaultValue);
            }
      else if (propertyType(id) == P_TYPE::POINT_SP) {
            QPointF p1 = getProperty(id).toPointF();
            QPointF p2 = propertyDefault(id).toPointF();
            if ( (qAbs(p1.x() - p2.x()) < 0.0001) && (qAbs(p1.y() - p2.y()) < 0.0001))
                  return;
            qreal _spatium = score()->spatium();
            QVariant val          = QVariant(p1/_spatium);
            QVariant defaultValue = QVariant(p2/_spatium);
            xml.tag(id, val, defaultValue);
            }
      else if (propertyType(id) == P_TYPE::POINT_SP_MM) {
            QPointF p1 = getProperty(id).toPointF();
            QPointF p2 = propertyDefault(id).toPointF();
            if ((qAbs(p1.x() - p2.x()) < 0.0001) && (qAbs(p1.y() - p2.y()) < 0.0001))
                  return;
            if (sizeIsSpatiumDependent()) {
                  qreal _spatium        = score()->spatium();
                  QVariant val          = QVariant(p1/_spatium);
                  QVariant defaultValue = QVariant(p2/_spatium);
                  xml.tag(id, val, defaultValue);
                  }
            else {
                  QVariant val          = QVariant(p1/DPMM);
                  QVariant defaultValue = QVariant(p2/DPMM);
                  xml.tag(id, val, defaultValue);
                  }
            }
      else {
            if (getProperty(id).isValid())
                  xml.tag(id, getProperty(id), propertyDefault(id));
            else
                  qDebug("%s invalid property <%s><%s>", name(), propertyName(id), propertyQmlName(id));
            }
      }

//---------------------------------------------------------
//   readStyledProperty
//---------------------------------------------------------

bool ScoreElement::readStyledProperty(XmlReader& e, const QStringRef& tag)
      {
      for (const StyledProperty& spp : *styledProperties()) {
            if (readProperty(tag, e, spp.pid))
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   writeStyledProperties
//---------------------------------------------------------

void ScoreElement::writeStyledProperties(XmlWriter& xml) const
      {
      for (const StyledProperty& spp : *styledProperties())
            writeProperty(xml, spp.pid);
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void ScoreElement::reset()
      {
      for (const StyledProperty& spp : *styledProperties())
            undoResetProperty(spp.pid);
      }

//---------------------------------------------------------
//   readAddConnector
//---------------------------------------------------------

void ScoreElement::readAddConnector(ConnectorInfoReader* info, bool pasteMode)
      {
      Q_UNUSED(pasteMode);
      qDebug("Cannot add connector %s to %s", info->connector()->name(), name());
      }

//---------------------------------------------------------
//   linkTo
//    link this to element
//---------------------------------------------------------

void ScoreElement::linkTo(ScoreElement* element)
      {
      Q_ASSERT(element != this);
      Q_ASSERT(!_links);

      if (element->links()) {
            _links = element->_links;
            Q_ASSERT(_links->contains(element));
            }
      else {
            if (isStaff())
                  _links = new LinkedElements(score(), -1); // dont use lid
            else
                  _links = new LinkedElements(score());
            _links->append(element);
            element->_links = _links;
            }
      Q_ASSERT(!_links->contains(this));
      _links->append(this);
      }

//---------------------------------------------------------
//   unlink
//---------------------------------------------------------

void ScoreElement::unlink()
      {
      Q_ASSERT(_links);
      Q_ASSERT(_links->contains(this));
      _links->removeOne(this);

      // if link list is empty, remove list
      if (_links->size() <= 1) {
            if (!_links->empty())
                  _links->front()->_links = 0;
            delete _links;
            }
      _links = 0; // this element is not linked anymore
      }

//---------------------------------------------------------
//   isLinked
///  return true if se is different and
///  linked to this element
//---------------------------------------------------------

bool ScoreElement::isLinked(ScoreElement* se)
      {
      return se != this && _links && _links->contains(se);
      }

//---------------------------------------------------------
//   undoUnlink
//---------------------------------------------------------

void ScoreElement::undoUnlink()
      {
      if (_links)
            _score->undo(new Unlink(this));
      }

//---------------------------------------------------------
//   linkList
//---------------------------------------------------------

QList<ScoreElement*> ScoreElement::linkList() const
      {
      QList<ScoreElement*> el;
      if (_links)
            el = *_links;
      else
            el.append(const_cast<ScoreElement*>(this));
      return el;
      }

//---------------------------------------------------------
//   LinkedElements
//---------------------------------------------------------

LinkedElements::LinkedElements(Score* score)
      {
      _lid = score->linkId(); // create new unique id
      }

LinkedElements::LinkedElements(Score* score, int id)
      {
      _lid = id;
      if (_lid != -1)
            score->linkId(id);      // remember used id
      }

//---------------------------------------------------------
//   setLid
//---------------------------------------------------------

void LinkedElements::setLid(Score* score, int id)
      {
      _lid = id;
      score->linkId(id);
      }

//---------------------------------------------------------
//   mainElement
//---------------------------------------------------------

ScoreElement* LinkedElements::mainElement()
      {
      if (isEmpty())
            return nullptr;
      MasterScore* ms = at(0)->masterScore();
      for (ScoreElement* se : *this) {
            if (se->score() == ms)
                  return se;
            }
      return front();
      }

//---------------------------------------------------------
//   masterScore
//---------------------------------------------------------

MasterScore* ScoreElement::masterScore() const
      {
      return _score->masterScore();
      }

//---------------------------------------------------------
//   getPropertyFlagsIdx
//---------------------------------------------------------

int ScoreElement::getPropertyFlagsIdx(Pid id) const
      {
      int i = 0;
      for (const StyledProperty& p : *_elementStyle) {
            if (p.pid == id)
                  return i;
            ++i;
            }
      return -1;
      }

//---------------------------------------------------------
//   propertyFlags
//---------------------------------------------------------

PropertyFlags ScoreElement::propertyFlags(Pid id) const
      {
      static PropertyFlags f = PropertyFlags::NOSTYLE;

      int i = getPropertyFlagsIdx(id);
      if (i == -1)
            return f;
      return _propertyFlagsList[i];
      }

//---------------------------------------------------------
//   setPropertyFlags
//---------------------------------------------------------

void ScoreElement::setPropertyFlags(Pid id, PropertyFlags f)
      {
      int i = getPropertyFlagsIdx(id);
      if (i == -1)
            return;
      _propertyFlagsList[i] = f;
      }

//---------------------------------------------------------
//   getPropertyStyle
//---------------------------------------------------------

Sid ScoreElement::getPropertyStyle(Pid id) const
      {
      for (const StyledProperty& p : *_elementStyle) {
            if (p.pid == id)
                  return p.sid;
            }
      return Sid::NOSTYLE;
      }

//---------------------------------------------------------
//   styleChanged
//---------------------------------------------------------

void ScoreElement::styleChanged()
      {
      for (const StyledProperty& spp : *_elementStyle) {
            PropertyFlags f = propertyFlags(spp.pid);
            if (f == PropertyFlags::STYLED)
                  setProperty(spp.pid, score()->styleValue(spp.pid, spp.sid));
            }
      }

//---------------------------------------------------------
//   name
//---------------------------------------------------------

const char* ScoreElement::name() const
      {
      return name(type());
      }

//---------------------------------------------------------
//   name
//---------------------------------------------------------

const char* ScoreElement::name(ElementType type)
      {
      return elementNames[int(type)].name;
      }

//---------------------------------------------------------
//   userName
//---------------------------------------------------------

QString ScoreElement::userName() const
      {
      return qApp->translate("elementName", elementNames[int(type())].userName);
      }

//---------------------------------------------------------
//   name2type
//---------------------------------------------------------

ElementType ScoreElement::name2type(const QStringRef& s)
      {
      for (int i = 0; i < int(ElementType::MAXTYPE); ++i) {
            if (s == elementNames[i].name)
                  return ElementType(i);
            }
      qDebug("unknown type <%s>", qPrintable(s.toString()));
      return ElementType::INVALID;
      }

//---------------------------------------------------------
//   isSLineSegment
//---------------------------------------------------------

bool ScoreElement::isSLineSegment() const
      {
      return isHairpinSegment() || isOttavaSegment() || isPedalSegment()
         || isTrillSegment() || isVoltaSegment() || isTextLineSegment()
         || isGlissandoSegment() || isLetRingSegment() || isVibratoSegment() || isPalmMuteSegment();
      }

//---------------------------------------------------------
//   isText
//---------------------------------------------------------

bool ScoreElement::isTextBase() const
      {
      return type()  == ElementType::TEXT
         || type() == ElementType::LYRICS
         || type() == ElementType::DYNAMIC
         || type() == ElementType::FINGERING
         || type() == ElementType::HARMONY
         || type() == ElementType::MARKER
         || type() == ElementType::JUMP
         || type() == ElementType::STAFF_TEXT
         || type() == ElementType::SYSTEM_TEXT
         || type() == ElementType::REHEARSAL_MARK
         || type() == ElementType::INSTRUMENT_CHANGE
         || type() == ElementType::FIGURED_BASS
         || type() == ElementType::TEMPO_TEXT
         || type() == ElementType::INSTRUMENT_NAME
         || type() == ElementType::MEASURE_NUMBER
         ;
      }
}

