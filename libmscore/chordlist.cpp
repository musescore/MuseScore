//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "config.h"
#include "chordlist.h"
#include "xml.h"
#include "pitchspelling.h"
#include "mscore.h"

//---------------------------------------------------------
//   HChord
//---------------------------------------------------------

HChord::HChord(const QString& str)
      {
      static const char* const scaleNames[2][12] = {
            { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" },
            { "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B" }
            };
      keys = 0;
      QStringList sl = str.split(" ", QString::SkipEmptyParts);
      foreach(const QString& s, sl) {
            for (int i = 0; i < 12; ++i) {
                  if (s == scaleNames[0][i] || s == scaleNames[1][i]) {
                        operator+=(i);
                        break;
                        }
                  }
            }
      }

//---------------------------------------------------------
//   HChord
//---------------------------------------------------------

HChord::HChord(int a, int b, int c, int d, int e, int f, int g, int h, int i, int k, int l)
      {
      keys = 0;
      if (a >= 0)
            operator+=(a);
      if (b >= 0)
            operator+=(b);
      if (c >= 0)
            operator+=(c);
      if (d >= 0)
            operator+=(d);
      if (e >= 0)
            operator+=(e);
      if (f >= 0)
            operator+=(f);
      if (g >= 0)
            operator+=(g);
      if (h >= 0)
            operator+=(h);
      if (i >= 0)
            operator+=(i);
      if (k >= 0)
            operator+=(k);
      if (l >= 0)
            operator+=(l);
      }

//---------------------------------------------------------
//   rotate
//    rotate 12 Bits
//---------------------------------------------------------

void HChord::rotate(int semiTones)
      {
      while (semiTones > 0) {
            if (keys & 0x800)
                  keys = ((keys & ~0x800) << 1) + 1;
            else
                  keys <<= 1;
            --semiTones;
            }
      while (semiTones < 0) {
            if (keys & 1)
                  keys = (keys >> 1) | 0x800;
            else
                  keys >>= 1;
            ++semiTones;
            }
      }

//---------------------------------------------------------
//   name
//---------------------------------------------------------

QString HChord::name(int tpc)
      {
      static const HChord C0(0,3,6,9);
      static const HChord C1(0,3);

      QString buf = tpc2name(tpc, false);
      HChord c(*this);

      int key = tpc2pitch(tpc);

      c.rotate(-key);        // transpose to C

      // special cases
      if (c == C0) {
            buf += "dim";
            return buf;
            }
      if (c == C1) {
            buf += "no5";
            return buf;
            }

      bool seven   = false;
      bool sharp9  = false;
      bool nat11   = false;
      bool sharp11 = false;
      bool nat13   = false;
      bool flat13  = false;

      // minor?
      if (c.contains(3)) {
            if (!c.contains(4))
                  buf += "m";
            else
                  sharp9 = true;
            }

      // 7
      if (c.contains(11)) {
            buf += "Maj7";
            seven = true;
            }
      else if (c.contains(10)) {
            buf += "7";
            seven = true;
            }

      // 4
      if (c.contains(5)) {
            if (!c.contains(4)) {
                  buf += "sus4";
                  }
            else
                  nat11 = true;
            }

      // 5
      if (c.contains(7)) {
            if (c.contains(6))
                  sharp11 = true;
            if (c.contains(8))
                  flat13 = true;
            }
      else {
            if (c.contains(6))
                  buf += "b5";
            if (c.contains(8))
                  buf += "#5";
            }

      // 6
      if (c.contains(9)) {
            if (!seven)
                  buf += "6";
            else
                  nat13 = true;
            }

      // 9
      if (c.contains(1))
            buf += "b9";
      if (c.contains(2))
            buf += "9";
      if (sharp9)
            buf += "#9";

      // 11
      if (nat11)
            buf += "11 ";
      if (sharp11)
            buf += "#11";

      // 13
      if (flat13)
            buf += "b13";
      if (nat13) {
            if (c.contains(1) || c.contains(2) || sharp9 || nat11 || sharp11)
                  buf += "13";
            else
                  buf += "add13";
            }
      return buf;
      }

//---------------------------------------------------------
//   print
//---------------------------------------------------------

void HChord::print() const
      {
      const char* names[] = { "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B" };

      for (int i = 0; i < 12; i++) {
            if (contains(i))
                  qDebug(" %s", names[i]);
            }
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void HChord::add(const QList<HDegree>& degreeList)
      {
// qDebug("HChord::add   ");print();
      // convert degrees to semitones
      static const int degreeTable[] = {
            // 1  2  3  4  5  6   7
            // C  D  E  F  G  A   B
               0, 2, 4, 5, 7, 9, 11
            };
      // factor in the degrees
      foreach(const HDegree& d, degreeList) {
            int dv  = degreeTable[(d.value() - 1) % 7] + d.alter();
            int dv1 = degreeTable[(d.value() - 1) % 7];

            if (d.value() == 7 && d.alter() == 0) {
                  // DEBUG: seventh degree is Bb, not B
                  //        except Maj   (TODO)
                  dv -= 1;
                  }

            if (d.type() == ADD)
                  *this += dv;
            else if (d.type() == ALTER) {
                  if (contains(dv1)) {
                        *this -= dv1;
                        *this += dv;
                        }
                  else {
//                        qDebug("ALTER: chord does not contain degree %d(%d):",
//                           d.value(), d.alter());
//                        print();
                        *this += dv;      // DEBUG: default to add
                        }
                  }
            else if (d.type() == SUBTRACT) {
                  if (contains(dv1))
                        *this -= dv1;
                  else {
                        qDebug("SUB: chord does not contain degree %d(%d):",
                           d.value(), d.alter());
                        print();
                        }
                  }
            else
                  qDebug("degree type %d not supported", d.type());

// qDebug("  HCHord::added  "); print();
            }
      }

//---------------------------------------------------------
//   readRenderList
//---------------------------------------------------------

static void readRenderList(QString val, QList<RenderAction>& renderList)
      {
      QStringList sl = val.split(" ", QString::SkipEmptyParts);
      foreach(const QString& s, sl) {
            if (s.startsWith("m:")) {
                  QStringList ssl = s.split(":", QString::SkipEmptyParts);
                  if (ssl.size() == 3) {
                        RenderAction a;
                        a.type = RenderAction::RENDER_MOVE;
                        a.movex = ssl[1].toDouble();
                        a.movey = ssl[2].toDouble();
                        renderList.append(a);
                        }
                  }
            else if (s == ":push")
                  renderList.append(RenderAction(RenderAction::RENDER_PUSH));
            else if (s == ":pop")
                  renderList.append(RenderAction(RenderAction::RENDER_POP));
            else if (s == ":n")
                  renderList.append(RenderAction(RenderAction::RENDER_NOTE));
            else if (s == ":a")
                  renderList.append(RenderAction(RenderAction::RENDER_ACCIDENTAL));
            else {
                  RenderAction a(RenderAction::RENDER_SET);
                  a.text = s;
                  renderList.append(a);
                  }
            }
      }

//---------------------------------------------------------
//   writeRenderList
//---------------------------------------------------------

static void writeRenderList(Xml& xml, const QList<RenderAction>* al, const QString& name)
      {
      QString s;

      int n = al->size();
      for (int i = 0; i < n; ++i) {
            if (!s.isEmpty())
                  s += " ";
            const RenderAction& a = (*al)[i];
            switch(a.type) {
                  case RenderAction::RENDER_SET:
                        s += a.text;
                        break;
                  case RenderAction::RENDER_MOVE:
                        if (a.movex != 0.0 || a.movey != 0.0)
                              s += QString("m:%1:%2").arg(a.movex).arg(a.movey);
                        break;
                  case RenderAction::RENDER_PUSH:
                        s += ":push";
                        break;
                  case RenderAction::RENDER_POP:
                        s += ":pop";
                        break;
                  case RenderAction::RENDER_NOTE:
                        s += ":n";
                        break;
                  case RenderAction::RENDER_ACCIDENTAL:
                        s += ":a";
                        break;
                  }
            }
      xml.tag(name, s);
      }

//---------------------------------------------------------
//  parse
//---------------------------------------------------------

bool ParsedChord::parse (QString s)
      {
      QString sp, elem, elemLower, quality, extension, modifiers;
      QStringList modifierList;
      int len = s.size();
      int i, j;

//      qDebug("flexibleChordParse: s = %s", qPrintable(s));

      // get quality
      for (i = 0; i < len && !s[i].isDigit(); ++i) {
            if (s[i] == '(')
                  break;
            elem[i] = s[i];
            }
      elemLower = elem.toLower();
      if (elem == "M" || elemLower == "ma" || elemLower == "maj")
            quality = "<major>";
      else if (elem == "m" || elemLower == "mi" || elemLower == "min" || elemLower == "-")
            quality = "<minor>";
      else if (elemLower == "aug" || elemLower == "+")
            quality = "<augmented>";
      else if (elemLower == "dim" || elemLower == "o")
            quality = "<diminished>";
      else
            quality = "<" + elemLower + ">";
//      qDebug("flexibleChordParse: quality = %s, i = %d", qPrintable(quality), i);

      // get extension
      for (j = 0; i < len && s[i].isDigit(); ++i, ++j)
            extension[j] = s[i];
      extension = "<" + extension + ">";
//      qDebug("flexibleChordParse: extension = %s, i = %d", qPrintable(extension), i);

      // get modifiers
      while (i < len) {
            QString tok1, tok2;
            // get first token - up to first digit
            // ignore leading open paren
            // break on comma or close paren and skip past
            for (j = 0, tok1 = ""; i < len; ++i) {
                  if (s[i] == '(')
                        continue;
                  else if (s[i] == ',' || s[i] == ')') {
                        ++i;
                        break;
                        }
                  else if (s[i].isDigit())
                        break;
                  else
                        tok1[j++] = s[i];
                  }
            tok1 = tok1.toLower();
//            qDebug("flexibleChordParse: tok1 = <%s>, i = %d", qPrintable(tok1), i);
            // get second token - up to first non-digit
            // again skip past comma or close paren
            for (j = 0, tok2 = ""; i < len; ++i) {
                  if (s[i] == ',' || s[i] == ')') {
                        ++i;
                        break;
                        }
                  else if (!s[i].isDigit())
                        break;
                  else
                        tok2[j++] = s[i];
                  }
            tok2 = tok2.toLower();
//            qDebug("flexibleChordParse: tok2 = <%s>, i = %d", qPrintable(tok2), i);
            if (tok1 == "m" || tok1 == "ma" || tok1 == "maj")
                  tok1 = "major";
            elem = "<" + tok1 + tok2 + ">";
            modifierList += elem;
            }
      if (!modifierList.isEmpty()) {
            modifierList.sort();
            modifiers = modifierList.join("");
            }

      // special cases
      if (quality == "<>") {
            if (extension != "<>")
                  quality = "<dominant>";
            else
                  quality = "<major>";
            }
      // more special cases TODO: mMaj, madd, augadd, no/omit, ...?

      // TODO - make attempt to "understand" the chord
      handle = quality + extension + modifiers;
      return true;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void ChordDescription::read(XmlReader& e)
      {
      id = e.attribute("id").toInt();
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "name") {
                  QString n = e.readElementText();
                  names.append(n);
                  ParsedChord pc;
                  pc.parse(n);
                  if (parsedChords.indexOf(pc) < 0)
                        parsedChords.append(pc);
                  }
            else if (tag == "xml")
                  xmlKind = e.readElementText();
            else if (tag == "degree")
                  xmlDegrees.append(e.readElementText());
            else if (tag == "voicing")
                  chord = HChord(e.readElementText());
            else if (tag == "render")
                  readRenderList(e.readElementText(), renderList);
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void ChordDescription::write(Xml& xml)
      {
      xml.stag(QString("chord id=\"%1\"").arg(id));
      foreach(const QString& s, names)
            xml.tag("name", s);
      xml.tag("xml", xmlKind);
      xml.tag("voicing", chord.getKeys());
      foreach(const QString& s, xmlDegrees)
            xml.tag("degree", s);
      writeRenderList(xml, &renderList, "render");
      xml.etag();
      }

//---------------------------------------------------------
//   ~ChordList
//---------------------------------------------------------

ChordList::~ChordList()
      {
      if (isDetached()) {
            QMapIterator<int, ChordDescription*> i(*this);
            while(i.hasNext()) {
                  i.next();
                  delete i.value();
                  }
            }
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void ChordList::read(XmlReader& e)
      {
      int fontIdx = 0;
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "font") {
                  ChordFont f;
                  f.family = e.attribute("family", "default");
                  f.mag    = 1.0;
                  while (e.readNextStartElement()) {
                        if (e.name() == "sym") {
                              ChordSymbol cs;
                              cs.fontIdx = fontIdx;
                              cs.name    = e.attribute("name");
                              cs.code    = e.attribute("code").toInt(0, 0);
                              symbols.insert(cs.name, cs);
                              e.readNext();
                              }
                        else if (e.name() == "mag")
                              f.mag = e.readDouble();
                        else
                              e.unknown();
                        }
                  fonts.append(f);
                  ++fontIdx;
                  }
            else if (tag == "chord") {
                  int id = e.intAttribute("id");
                  ChordDescription* cd = take(id);
                  if (cd == 0)
                        cd = new ChordDescription();
                  cd->read(e);
                  insert(id, cd);
                  }
            else if (tag == "renderRoot")
                  readRenderList(e.readElementText(), renderListRoot);
            else if (tag == "renderBase")
                  readRenderList(e.readElementText(), renderListBase);
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void ChordList::write(Xml& xml)
      {
      int fontIdx = 0;
      foreach (ChordFont f, fonts) {
            xml.stag(QString("font id=\"%1\" family=\"%2\"").arg(fontIdx).arg(f.family));
            xml.tag("mag", f.mag);
            foreach(ChordSymbol s, symbols) {
                  if (s.fontIdx == fontIdx) {
                        xml.tagE(QString("sym name=\"%1\" code=\"%2\"").arg(s.name).arg(s.code.unicode()));
                        }
                  }
            xml.etag();
            ++fontIdx;
            }
      if (!renderListRoot.isEmpty())
            writeRenderList(xml, &renderListRoot, "renderRoot");
      if (!renderListBase.isEmpty())
            writeRenderList(xml, &renderListBase, "renderBase");
      foreach(ChordDescription* d, *this)
            d->write(xml);
      }

//---------------------------------------------------------
//   read
//    read Chord List, return false on error
//---------------------------------------------------------

bool ChordList::read(const QString& name)
      {
      qDebug("ChordList::read <%s>", qPrintable(name));
      QString path;
      QFileInfo ftest(name);
      if (ftest.isAbsolute())
            path = name;
      else {
#if defined(Q_WS_IOS)
            path = QString("%1/%2").arg(MScore::globalShare()).arg(name);
#elif defined(Q_OS_ANDROID)
            path = QString(":/styles/%1").arg(name);
#else
            path = QString("%1styles/%2").arg(MScore::globalShare()).arg(name);
#endif
            }
      //default to stdchords.xml
      QFileInfo fi(path);
      if (!fi.exists())
#if defined(Q_WS_IOS)
            path = QString("%1/%2").arg(MScore::globalShare()).arg("stdchords.xml");
#elif defined(Q_OS_ANDROID)
            path = QString(":/styles/stdchords.xml");
#else
            path = QString("%1styles/%2").arg(MScore::globalShare()).arg("stdchords.xml");
#endif

      if (name.isEmpty())
            return false;
      QFile f(path);
      if (!f.open(QIODevice::ReadOnly)) {
            QString s = QT_TRANSLATE_NOOP("file", "cannot open chord description:\n%1\n%2");
            MScore::lastError = s.arg(f.fileName()).arg(f.errorString());
            qDebug("ChordList::read failed: <%s>", qPrintable(path));
            return false;
            }
      XmlReader e(&f);
      docName = f.fileName();

      while (e.readNextStartElement()) {
            if (e.name() == "museScore") {
                  // QString version = e.attribute(QString("version"));
                  // QStringList sl = version.split('.');
                  // int _mscVersion = sl[0].toInt() * 100 + sl[1].toInt();
                  read(e);
                  return true;
                  }
            }
      return false;
      }

//---------------------------------------------------------
//   writeChordList
//---------------------------------------------------------

bool ChordList::write(const QString& name)
      {
      QFileInfo info(name);

      if (info.suffix().isEmpty()) {
            QString path = info.filePath();
            path += QString(".xml");
            info.setFile(path);
            }

      QFile f(info.filePath());

      if (!f.open(QIODevice::WriteOnly)) {
            QString s = QT_TRANSLATE_NOOP("file", "Open Chord Description\n%1\nfailed: %2");
            MScore::lastError = s.arg(f.fileName()).arg(f.errorString());
            return false;
            }

      Xml xml(&f);
      xml.header();
      xml.stag("museScore version=\"" MSC_VERSION "\"");

      write(xml);
      xml.etag();
      if (f.error() != QFile::NoError) {
            QString s = QT_TRANSLATE_NOOP("file", "Write Chord Description failed: %1");
            MScore::lastError = s.arg(f.errorString());
            }
      return true;
      }




