//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: harmony.cpp 5568 2012-04-22 10:08:43Z wschweer $
//
//  Copyright (C) 2008-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "harmony.h"
#include "pitchspelling.h"
#include "score.h"
#include "system.h"
#include "measure.h"
#include "segment.h"
#include "chordlist.h"
#include "mscore.h"
#include "fret.h"

//---------------------------------------------------------
//   harmonyName
//---------------------------------------------------------

QString Harmony::harmonyName() const
      {
      bool germanNames = score()->styleB(ST_useGermanNoteNames);

      HChord hc = descr() ? descr()->chord : HChord();
      QString s;

      if (!_degreeList.isEmpty()) {
            hc.add(_degreeList);
            // try to find the chord in chordList
            const ChordDescription* newExtension = 0;
            ChordList* cl = score()->style()->chordList();
            foreach(const ChordDescription* cd, *cl) {
                  if (cd->chord == hc && !cd->names.isEmpty()) {
                        newExtension = cd;
                        break;
                        }
                  }
            // now determine the chord name
            if (newExtension)
                  s = tpc2name(_rootTpc, germanNames) + newExtension->names.front();
            else
                  // not in table, fallback to using HChord.name()
                  s = hc.name(_rootTpc);
            //s += " ";
            } // end if (degreeList ...
      else {
            s = tpc2name(_rootTpc, germanNames);
            if (descr()) {
                  //s += " ";
                  s += descr()->names.front();
                  }
            }
      if (_baseTpc != INVALID_TPC) {
            s += "/";
            s += tpc2name(_baseTpc, germanNames);
            }
      return s;
      }

//---------------------------------------------------------
//   resolveDegreeList
//    try to detect chord number and to eliminate degree
//    list
//---------------------------------------------------------

void Harmony::resolveDegreeList()
      {
      if (_degreeList.isEmpty())
            return;

      HChord hc = descr() ? descr()->chord : HChord();

      hc.add(_degreeList);

// qDebug("resolveDegreeList: <%s> <%s-%s>: ", _descr->name, _descr->xmlKind, _descr->xmlDegrees);
// hc.print();
// _descr->chord.print();

      // try to find the chord in chordList
      ChordList* cl = score()->style()->chordList();
      foreach(const ChordDescription* cd, *cl) {
            if ((cd->chord == hc) && !cd->names.isEmpty()) {
qDebug("ResolveDegreeList: found in table as %s", qPrintable(cd->names.front()));
                  _id = cd->id;
                  _degreeList.clear();
                  return;
                  }
            }
qDebug("ResolveDegreeList: not found in table");
      }

//---------------------------------------------------------
//   Harmony
//---------------------------------------------------------

Harmony::Harmony(Score* s)
   : Text(s)
      {
      setTextStyle(s->textStyle(TEXT_STYLE_HARMONY));
      setUnstyled();

      _rootTpc   = INVALID_TPC;
      _baseTpc   = INVALID_TPC;
      _id        = -1;
      }

Harmony::Harmony(const Harmony& h)
   : Text(h)
      {
      _rootTpc    = h._rootTpc;
      _baseTpc    = h._baseTpc;
      _id         = h._id;
      _degreeList = h._degreeList;
      }

//---------------------------------------------------------
//   ~Harmony
//---------------------------------------------------------

Harmony::~Harmony()
      {
      foreach(const TextSegment* ts, textList)
            delete ts;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Harmony::write(Xml& xml) const
      {
      xml.stag("Harmony");
      if (_rootTpc != INVALID_TPC) {
            xml.tag("root", _rootTpc);
            if (_id != -1)
                  xml.tag("extension", _id);
            if (_baseTpc != INVALID_TPC)
                  xml.tag("base", _baseTpc);
            foreach(const HDegree& hd, _degreeList) {
                  int tp = hd.type();
                  if (tp == ADD || tp == ALTER || tp == SUBTRACT) {
                        xml.stag("degree");
                        xml.tag("degree-value", hd.value());
                        xml.tag("degree-alter", hd.alter());
                        switch (tp) {
                              case ADD:
                                    xml.tag("degree-type", "add");
                                    break;
                              case ALTER:
                                    xml.tag("degree-type", "alter");
                                    break;
                              case SUBTRACT:
                                    xml.tag("degree-type", "subtract");
                                    break;
                              default:
                                    break;
                              }
                        xml.etag();
                        }
                  }
            Element::writeProperties(xml);
            }
      else
            Text::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Harmony::read(XmlReader& e)
      {
      // convert table to tpc values
      static const int table[] = {
            14, 9, 16, 11, 18, 13, 8, 15, 10, 17, 12, 19
            };

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "base") {
                  if (score()->mscVersion() >= 106)
                        setBaseTpc(e.readInt());
                  else
                        setBaseTpc(table[e.readInt()-1]);    // obsolete
                  }
            else if (tag == "extension")
                  setId(e.readInt());
            else if (tag == "root") {
                  if (score()->mscVersion() >= 106)
                        setRootTpc(e.readInt());
                  else
                        setRootTpc(table[e.readInt()-1]);    // obsolete
                  }
            else if (tag == "degree") {
                  int degreeValue = 0;
                  int degreeAlter = 0;
                  QString degreeType = "";
                  while (e.readNextStartElement()) {
                        const QStringRef& tag(e.name());
                        if (tag == "degree-value")
                              degreeValue = e.readInt();
                        else if (tag == "degree-alter")
                              degreeAlter = e.readInt();
                        else if (tag == "degree-type")
                              degreeType = e.readElementText();
                        else
                              e.unknown();
                        }
                  if (degreeValue <= 0 || degreeValue > 13
                      || degreeAlter < -2 || degreeAlter > 2
                      || (degreeType != "add" && degreeType != "alter" && degreeType != "subtract")) {
                        qDebug("incorrect degree: degreeValue=%d degreeAlter=%d degreeType=%s",
                               degreeValue, degreeAlter, qPrintable(degreeType));
                        }
                  else {
                        if (degreeType == "add")
                              addDegree(HDegree(degreeValue, degreeAlter, ADD));
                        else if (degreeType == "alter")
                              addDegree(HDegree(degreeValue, degreeAlter, ALTER));
                        else if (degreeType == "subtract")
                              addDegree(HDegree(degreeValue, degreeAlter, SUBTRACT));
                        }
                  }
            else if (!Text::readProperties(e))
                  e.unknown();
            }
      render();
      }

//---------------------------------------------------------
//   convertRoot
//    convert something like "C#" into tpc 21
//---------------------------------------------------------

static int convertRoot(const QString& s, bool germanNames)
      {
      int n = s.size();
      if (n < 1)
            return INVALID_TPC;
      int alter = 0;
      if (n > 1) {
            if (s[1].toLower().toLatin1() == 'b')
                  alter = -1;
            else if (s[1] == '#')
                  alter = 1;
            }
      int r;
      if (germanNames) {
            switch(s[0].toLower().toLatin1()) {
                  case 'c':   r = 0; break;
                  case 'd':   r = 1; break;
                  case 'e':   r = 2; break;
                  case 'f':   r = 3; break;
                  case 'g':   r = 4; break;
                  case 'a':   r = 5; break;
                  case 'h':   r = 6; break;
                  case 'b':
                        if (alter)
                              return INVALID_TPC;
                        r = 6;
                        alter = -1;
                        break;
                  default:
                        return INVALID_TPC;
                  }
            static const int spellings[] = {
               // bb  b   -   #  ##
                  0,  7, 14, 21, 28,  // C
                  2,  9, 16, 23, 30,  // D
                  4, 11, 18, 25, 32,  // E
                 -1,  6, 13, 20, 27,  // F
                  1,  8, 15, 22, 29,  // G
                  3, 10, 17, 24, 31,  // A
                  5, 12, 19, 26, 33,  // B
                  };
            r = spellings[r * 5 + alter + 2];
            }
      else {
            switch(s[0].toLower().toLatin1()) {
                  case 'c':   r = 0; break;
                  case 'd':   r = 1; break;
                  case 'e':   r = 2; break;
                  case 'f':   r = 3; break;
                  case 'g':   r = 4; break;
                  case 'a':   r = 5; break;
                  case 'b':   r = 6; break;
                  default:    return INVALID_TPC;
                  }
            static const int spellings[] = {
               // bb  b   -   #  ##
                  0,  7, 14, 21, 28,  // C
                  2,  9, 16, 23, 30,  // D
                  4, 11, 18, 25, 32,  // E
                 -1,  6, 13, 20, 27,  // F
                  1,  8, 15, 22, 29,  // G
                  3, 10, 17, 24, 31,  // A
                  5, 12, 19, 26, 33,  // B
                  };
            r = spellings[r * 5 + alter + 2];
            }
      return r;
      }

//---------------------------------------------------------
//   parseHarmony
//    return ChordDescription
//---------------------------------------------------------

bool Harmony::parseHarmony(const QString& ss, int* root, int* base)
      {
      _id = -1;
      QString s = ss.simplified();
      _userName = s;
      int n = s.size();
      if (n < 1)
            return false;
      bool germanNames = score()->styleB(ST_useGermanNoteNames);
      int r = convertRoot(s, germanNames);
      if (r == INVALID_TPC) {
            qDebug("1:parseHarmony failed <%s>", qPrintable(ss));
            return false;
            }
      *root = r;
      int idx = ((n > 1) && ((s[1] == 'b') || (s[1] == '#'))) ? 2 : 1;
      *base = INVALID_TPC;
      int slash = s.indexOf('/');
      if (slash != -1) {
            QString bs = s.mid(slash+1);
            s     = s.mid(idx, slash - idx);
            *base = convertRoot(bs, germanNames);
            }
      else
            s = s.mid(idx).simplified();
      s = s.toLower();
      foreach(const ChordDescription* cd, *score()->style()->chordList()) {
            foreach(QString ss, cd->names) {
                  if (s == ss) {
                        _id = cd->id;
                        return true;
                        }
                  }
            }
      _userName = s;
      qDebug("2:parseHarmony failed <%s><%s>", qPrintable(ss), qPrintable(s));
      return false;
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void Harmony::startEdit(MuseScoreView* view, const QPointF& p)
      {
      if (!textList.isEmpty()) {
            QString s(harmonyName());
            setText(s);
            }
      Text::startEdit(view, p);
      }

//---------------------------------------------------------
//   edit
//---------------------------------------------------------

bool Harmony::edit(MuseScoreView* view, int grip, int key, Qt::KeyboardModifiers mod, const QString& s)
      {
      bool rv = Text::edit(view, grip, key, mod, s);
      QString str = text();
      int root, base;
      bool badSpell = !str.isEmpty() && !parseHarmony(str, &root, &base);
      spellCheckUnderline(badSpell);
      return rv;
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Harmony::endEdit()
      {
      Text::endEdit();
      setHarmony(text());
      layout();
      score()->setLayoutAll(true);
      }

//---------------------------------------------------------
//   setHarmony
//---------------------------------------------------------

void Harmony::setHarmony(const QString& s)
      {
      int r, b;
      parseHarmony(s, &r, &b);
      if (_id != -1) {
            setRootTpc(r);
            setBaseTpc(b);
            render();
            }
      else {
            // syntax error, leave text as is
            foreach(const TextSegment* s, textList)
                  delete s;
            textList.clear();

            setRootTpc(INVALID_TPC);
            setBaseTpc(INVALID_TPC);
            _id = -1;
            }
      }

//---------------------------------------------------------
//   baseLine
//---------------------------------------------------------

qreal Harmony::baseLine() const
      {
      return (editMode() || textList.isEmpty()) ? Text::baseLine() : 0.0;
      }

//---------------------------------------------------------
//   text
//---------------------------------------------------------

QString HDegree::text() const
      {
      if (_type == UNDEF)
            return QString();
      const char* d = 0;
      switch(_type) {
            case UNDEF: break;
            case ADD:         d= "add"; break;
            case ALTER:       d= "alt"; break;
            case SUBTRACT:    d= "sub"; break;
            }
      QString degree(d);
      switch(_alter) {
            case -1:          degree += "b"; break;
            case 1:           degree += "#"; break;
            default:          break;
            }
/*      QString s = QString("%1").arg(_value);
      QString ss; //  = degree + s;
      return ss;
      */
      return QString();
      }

//---------------------------------------------------------
//   fromXml
//    lookup harmony in harmony data base
//    using musicXml "kind" string and degree list
//---------------------------------------------------------

const ChordDescription* Harmony::fromXml(const QString& kind,  const QList<HDegree>& dl)
      {
      QStringList degrees;

      foreach(const HDegree& d, dl)
            degrees.append(d.text());

      QString lowerCaseKind = kind.toLower();
      ChordList* cl = score()->style()->chordList();
      foreach(const ChordDescription* cd, *cl) {
            QString k     = cd->xmlKind;
            QString lowerCaseK = k.toLower(); // required for xmlKind Tristan
            QStringList d = cd->xmlDegrees;
            if ((lowerCaseKind == lowerCaseK) && (d == degrees)) {
//                  qDebug("harmony found in db: %s %s -> %d", qPrintable(kind), qPrintable(degrees), cd->id);
                  return cd;
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   fromXml
//    lookup harmony in harmony data base
//    using musicXml "kind" string and degree list
//---------------------------------------------------------

const ChordDescription* Harmony::fromXml(const QString& kind)
      {
      QString lowerCaseKind = kind.toLower();
      ChordList* cl = score()->style()->chordList();
      foreach(const ChordDescription* cd, *cl) {
            if (lowerCaseKind == cd->xmlKind)
                  return cd;
            }
      return 0;
      }

//---------------------------------------------------------
//   descr
//---------------------------------------------------------

const ChordDescription* Harmony::descr() const
      {
      return score()->style()->chordDescription(_id);
      }

//---------------------------------------------------------
//   isEmpty
//---------------------------------------------------------

bool Harmony::isEmpty() const
      {
      return textList.isEmpty() && Text::isEmpty();
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Harmony::layout()
      {
      if (editMode() || textList.isEmpty())
            Text::layout1();
      else {
            // textStyle().layout(this);
            QRectF bb;
            foreach(const TextSegment* ts, textList)
                  bb |= ts->boundingRect().translated(ts->x, ts->y);
            setbbox(bb);
            }
      if (!parent()) {          // for use in palette
            setPos(QPointF());
            return;
            }

      qreal yy = 0.0;
      if (parent()->type() == SEGMENT) {
            Measure* m = static_cast<Measure*>(parent()->parent());
            yy = track() < 0 ? 0.0 : m->system()->staff(staffIdx())->y();
            yy += score()->styleP(ST_harmonyY);
            }
      else if (parent()->type() == FRET_DIAGRAM) {
            yy = score()->styleP(ST_harmonyFretDist);
            }
      setPos(QPointF(0.0, yy));

      if (!readPos().isNull()) {
            // version 114 is measure based
            // rebase to segment
            if (score()->mscVersion() == 114) {
                  setReadPos(readPos() - parent()->pos());
                  }
            setUserOff(readPos() - ipos());
            setReadPos(QPointF());
            }
      if (parent()->type() == FRET_DIAGRAM && parent()->parent()->type() == SEGMENT) {
            MStaff* mstaff = static_cast<Segment*>(parent()->parent())->measure()->mstaff(staffIdx());
            qreal dist = -(bbox().top());
            mstaff->distanceUp = qMax(mstaff->distanceUp, dist + spatium());
            }
      }

//---------------------------------------------------------
//   shape
//---------------------------------------------------------

QPainterPath Harmony::shape() const
      {
      QPainterPath pp;
      pp.addRect(bbox());
      return pp;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Harmony::draw(QPainter* painter) const
      {
      painter->setPen(curColor());
      if (editMode() || textList.isEmpty()) {
            Text::draw(painter);
            return;
            }
      foreach(const TextSegment* ts, textList) {
            painter->setFont(ts->font);
            painter->drawText(QPointF(ts->x, ts->y), ts->text);
            }
      }

//---------------------------------------------------------
//   TextSegment
//---------------------------------------------------------

TextSegment::TextSegment(const QString& s, const QFont& f, qreal x, qreal y)
      {
      set(s, f, x, y);
      select = false;
      }

//---------------------------------------------------------
//   width
//---------------------------------------------------------

qreal TextSegment::width() const
      {
      QFontMetricsF fm(font);
      qreal w = 0.0;
      foreach(QChar c, text) {
            w += fm.width(c);
            }
      return w;
      }

//---------------------------------------------------------
//   boundingRect
//---------------------------------------------------------

QRectF TextSegment::boundingRect() const
      {
      QFontMetricsF fm(font);
      return fm.boundingRect(text);
      }

//---------------------------------------------------------
//   set
//---------------------------------------------------------

void TextSegment::set(const QString& s, const QFont& f, qreal _x, qreal _y)
      {
      font = f;
      x    = _x;
      y    = _y;
      setText(s);
      }

//---------------------------------------------------------
//   render
//---------------------------------------------------------

void Harmony::render(const QList<RenderAction>& renderList, qreal& x, qreal& y, int tpc)
      {
      ChordList* chordList = score()->style()->chordList();
      QStack<QPointF> stack;
      int fontIdx = 0;
      qreal _spatium = spatium();
      qreal mag = (MScore::DPI / PPI) * (_spatium / (SPATIUM20 * MScore::DPI));

      foreach(const RenderAction& a, renderList) {
            if (a.type == RenderAction::RENDER_SET) {
                  TextSegment* ts = new TextSegment(fontList[fontIdx], x, y);
                  ChordSymbol cs = chordList->symbol(a.text);
                  if (cs.isValid()) {
                        ts->font = fontList[cs.fontIdx];
                        ts->setText(QString(cs.code));
                        }
                  else
                        ts->setText(a.text);
                  textList.append(ts);
                  x += ts->width();
                  }
            else if (a.type == RenderAction::RENDER_MOVE) {
                  x += a.movex * mag;
                  y += a.movey * mag;
                  }
            else if (a.type == RenderAction::RENDER_PUSH)
                  stack.push(QPointF(x,y));
            else if (a.type == RenderAction::RENDER_POP) {
                  if (!stack.isEmpty()) {
                        QPointF pt = stack.pop();
                        x = pt.x();
                        y = pt.y();
                        }
                  else
                        qDebug("RenderAction::RENDER_POP: stack empty");
                  }
            else if (a.type == RenderAction::RENDER_NOTE) {
                  bool germanNames = score()->styleB(ST_useGermanNoteNames);
                  QChar c;
                  int acc;
                  tpc2name(tpc, germanNames, &c, &acc);
                  TextSegment* ts = new TextSegment(fontList[fontIdx], x, y);
                  ChordSymbol cs = chordList->symbol(QString(c));
                  if (cs.isValid()) {
                        ts->font = fontList[cs.fontIdx];
                        ts->setText(QString(cs.code));
                        }
                  else
                        ts->setText(QString(c));
                  textList.append(ts);
                  x += ts->width();
                  }
            else if (a.type == RenderAction::RENDER_ACCIDENTAL) {
                  QChar c;
                  int acc;
                  tpc2name(tpc, false, &c, &acc);
                  if (acc) {
                        TextSegment* ts = new TextSegment(fontList[fontIdx], x, y);
                        QString s;
                        if (acc == -1)
                              s = "b";
                        else if (acc == 1)
                              s = "#";
                        ChordSymbol cs = chordList->symbol(s);
                        if (cs.isValid()) {
                              ts->font = fontList[cs.fontIdx];
                              ts->setText(QString(cs.code));
                              }
                        else
                              ts->setText(s);
                        textList.append(ts);
                        x += ts->width();
                        }
                  }
            else
                  qDebug("========unknown render action %d", a.type);
            }
      }

//---------------------------------------------------------
//   render
//    construct Chord Symbol
//---------------------------------------------------------

void Harmony::render(const TextStyle* st)
      {
      if (_rootTpc == INVALID_TPC)
            return;

      if (st == 0)
            st = &textStyle();
      ChordList* chordList = score()->style()->chordList();

      fontList.clear();
      foreach(ChordFont cf, chordList->fonts) {
            if (cf.family.isEmpty() || cf.family == "default")
                  fontList.append(st->fontPx(spatium() * cf.mag));
            else {
                  QFont ff(st->fontPx(spatium() * cf.mag));
                  ff.setFamily(cf.family);
                  fontList.append(ff);
                  }
            }
      if (fontList.isEmpty())
            fontList.append(st->fontPx(spatium()));

      foreach(const TextSegment* s, textList)
            delete s;
      textList.clear();

      qreal x = 0.0, y = 0.0;
      render(chordList->renderListRoot, x, y, _rootTpc);
      ChordDescription* cd = chordList->value(_id);
      if (cd) {
            render(cd->renderList, x, y, 0);
            }
      if (_baseTpc != INVALID_TPC)
            render(chordList->renderListBase, x, y, _baseTpc);
      }

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void Harmony::spatiumChanged(qreal oldValue, qreal newValue)
      {
      Text::spatiumChanged(oldValue, newValue);
      render();
      }

//---------------------------------------------------------
//   dragAnchor
//---------------------------------------------------------

QLineF Harmony::dragAnchor() const
      {
      qreal xp = 0.0;
      for (Element* e = parent(); e; e = e->parent())
            xp += e->x();
      qreal yp;
      if (parent()->type() == SEGMENT)
            yp = static_cast<Segment*>(parent())->measure()->system()->staffY(staffIdx());
      else
            yp = parent()->canvasPos().y();
      QPointF p(xp, yp);
      return QLineF(p, canvasPos());
      }

//---------------------------------------------------------
//   extensionName
//---------------------------------------------------------

QString Harmony::extensionName() const
      {
      return _id != -1 ? descr()->names.front() : _userName;
      }

//---------------------------------------------------------
//   xmlKind
//---------------------------------------------------------

QString Harmony::xmlKind() const
      {
      return _id != -1 ? descr()->xmlKind : QString();
      }

//---------------------------------------------------------
//   xmlDegrees
//---------------------------------------------------------

QStringList Harmony::xmlDegrees() const
      {
      return _id != -1 ? descr()->xmlDegrees : QStringList();
      }

//---------------------------------------------------------
//   degree
//---------------------------------------------------------

HDegree Harmony::degree(int i) const
      {
      return _degreeList.value(i);
      }

//---------------------------------------------------------
//   addDegree
//---------------------------------------------------------

void Harmony::addDegree(const HDegree& d)
      {
      _degreeList << d;
      }

//---------------------------------------------------------
//   numberOfDegrees
//---------------------------------------------------------

int Harmony::numberOfDegrees() const
      {
      return _degreeList.size();
      }

//---------------------------------------------------------
//   clearDegrees
//---------------------------------------------------------

void Harmony::clearDegrees()
      {
      _degreeList.clear();
      }

//---------------------------------------------------------
//   degreeList
//---------------------------------------------------------

const QList<HDegree>& Harmony::degreeList() const
      {
      return _degreeList;
      }

