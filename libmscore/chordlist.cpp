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
#include "score.h"
#include "xml.h"
#include "pitchspelling.h"
#include "mscore.h"

namespace Ms {

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
      renderList.clear();
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
//  read
//---------------------------------------------------------

void ChordToken::read(XmlReader& e)
      {
      QString c = e.attribute("class");
      if (c == "quality")
            tokenClass = QUALITY;
      else if (c == "extension")
            tokenClass = EXTENSION;
      else if (c == "modifier")
            tokenClass = MODIFIER;
      else
            tokenClass = ALL;
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "name")
                  names += e.readElementText();
            else if (tag == "render")
                  readRenderList(e.readElementText(), renderList);
            }
      }

//---------------------------------------------------------
//  write
//---------------------------------------------------------

void ChordToken::write(Xml& xml)
      {
      QString t = "token";
      switch (tokenClass) {
            case QUALITY:
                  t += " class=\"quality\"";
                  break;
            case EXTENSION:
                  t += " class=\"extension\"";
                  break;
            case MODIFIER:
                  t += " class=\"modifier\"";
                  break;
            default:
                  break;
      }
      xml.stag(t);
      foreach(const QString& s, names)
            xml.tag("name", s);
      writeRenderList(xml, &renderList, "render");
      xml.etag();
      }

//---------------------------------------------------------
//  parse
//    return true if chord was parseable
//---------------------------------------------------------

bool ParsedChord::parse(QString s, bool syntaxOnly)
      {
      QString tok1, tok1L, tok2, tok2L, modifiers;
      QString chordDigits = "123456789";
      QString special = "(),";
      QString leading = "(";
      QString trailing = "),";
      int firstLeadingToken, lastLeadingToken;
      int len = s.size();
      int i, j;

      _parseable = true;
      i = 0;

      // eat leading parens
      firstLeadingToken = _tokenList.size();
      while (i < len && leading.contains(s[i]))
           addToken(QString(s[i++]),QUALITY);
      lastLeadingToken = _tokenList.size();
      // get quality
      for (tok1 = ""; i < len; ++i) {
            // up to first (non-zero) digit, paren, or comma
            if (chordDigits.contains(s[i]) || special.contains(s[i]))
                  break;
            tok1[i] = s[i];
            }
      // TODO: special cases for mMaj, augadd, etc.
      if (tok1 != "")
            addToken(tok1,QUALITY);
      else {
            // leading tokens were not really QUALITY
            for (int t = firstLeadingToken; t < lastLeadingToken; ++t)
                  _tokenList[t].tokenClass = EXTENSION;
            }
      tok1L = tok1.toLower();
      if (tok1 == "M" || tok1L == "ma" || tok1L == "maj" || tok1L == "t" || tok1L == "^")
            quality = "major";
      else if (tok1L == "m" || tok1L == "mi" || tok1L == "min" || tok1L == "-")
            quality = "minor";
      else if (tok1L == "aug" || tok1L == "+")
            quality = "augmented";
      else if (tok1L == "dim" || tok1L == "o")
            quality = "diminished";
      else if (tok1L == "0")
            quality = "half-diminished";
      else
            quality = tok1L;
      // eat trailing parens and commas
      while (i < len && trailing.contains(s[i]))
           addToken(QString(s[i++]),QUALITY);

      // eat leading parens
      firstLeadingToken = _tokenList.size();
      while (i < len && leading.contains(s[i])) {
            addToken(QString(s[i++]),EXTENSION);
            }
      lastLeadingToken = _tokenList.size();
      // get extension - up to first non-digit
      for (j = 0, tok1 = ""; i < len; ++i, ++j) {
            if (!s[i].isDigit())
                  break;
            tok1[j] = s[i];
            }
      if (tok1 != "")
            addToken(tok1,EXTENSION);
      else {
            // leading tokens were not really EXTENSION
            for (int t = firstLeadingToken; t < lastLeadingToken; ++t)
                  _tokenList[t].tokenClass = MODIFIER;
            }
      extension = tok1;
      // eat trailing parens and commas
      while (i < len && trailing.contains(s[i]))
           addToken(QString(s[i++]),EXTENSION);

      // get modifiers
      while (i < len) {
            // eat leading parens
            while (i < len && leading.contains(s[i]))
                  addToken(QString(s[i++]),MODIFIER);
            // get first token - up to first digit, paren, or comma
            for (j = 0, tok1 = ""; i < len; ++i) {
                  if (s[i].isDigit() || special.contains(s[i]))
                        break;
                  tok1[j++] = s[i];
                  }
            tok1L = tok1.toLower();
            // if we reached the end of the string and never got a token,
            // then nothing to do, and no sense in looking for a second token
            if (i == len && tok1 == "")
                  break;
            // get second token - up to first non-digit
            for (j = 0, tok2 = ""; i < len; ++i) {
                  if (!s[i].isDigit())
                        break;
                  tok2[j++] = s[i];
                  }
            tok2L = tok2.toLower();
            // special cases
            if (tok1L == "susb" || tok1L == "sus#") {
                  modifierList += "sus4";
                  addToken("sus",MODIFIER);
                  tok1 = tok1[3];
                  tok1L = tok1L[3];
                  }
            else if (tok1 == "M" || tok1L == "ma" || tok1L == "maj" || tok1L == "t" || tok1L == "^")
                  tok1L = "major";
            else if (tok1L == "omit")
                  tok1L = "no";
            else if (tok1L == "sus" && tok2L == "")
                  tok2L = "4";
            if (tok1 != "")
                  addToken(tok1,MODIFIER);
            if (tok2 != "")
                  addToken(tok2,MODIFIER);
            QString mod = tok1L + tok2L;
            if (mod != "")
                  modifierList += mod;
            // eat trailing parens and commas
            while (i < len && trailing.contains(s[i]))
                  addToken(QString(s[i++]),MODIFIER);
            }
      if (!modifierList.isEmpty()) {
            modifierList.sort();
            modifiers = "<" + modifierList.join("><") + ">";
            }

      // special cases
      if (quality == "") {
            if (extension == "7" || extension == "9" || extension == "11" || extension == "13")
                  quality = "dominant";
            else
                  quality = "major";
            }
      // more special cases TODO: mMaj, madd, augadd, ...?

      handle = "<" + quality + "><" + extension + ">" + modifiers;
//      qDebug("parse: %s -> %s", qPrintable(s), qPrintable(handle);
      _understandable = false;
      return _parseable;
      }

//---------------------------------------------------------
//   renderList
//---------------------------------------------------------

const QList<RenderAction>& ParsedChord::renderList(const QList<ChordToken>& tokenDefinitionList, bool regenerate)
      {
      if (regenerate)
            _renderList.clear();
      if (_renderList.isEmpty()) {
            foreach (ChordToken tok, _tokenList) {
                  QString n = tok.names.first();
                  QList<RenderAction> rl;
                  QList<ChordToken> definedTokens;
                  bool found = false;
                  // potential definitions for token
                  foreach (ChordToken dt, tokenDefinitionList) {
                        foreach (QString dtn, dt.names) {
                              if (dtn == n)
                                    definedTokens += dt;
                              }
                        }
                  // find matching class, fallback on ALL
                  foreach (ChordToken matchingTok, definedTokens) {
                        if (tok.tokenClass == matchingTok.tokenClass) {
                              rl = matchingTok.renderList;
                              found = true;
                              break;
                              }
                        else if (matchingTok.tokenClass == ALL) {
                              rl = matchingTok.renderList;
                              found = true;
                              }
                        }
                  if (found)
                        _renderList.append(rl);
                  else {
                        // no definition for token, so render as literal
                        RenderAction a(RenderAction::RENDER_SET);
                        a.text = tok.names.first();
                        _renderList.append(a);
                        }
                  }
            }
      return _renderList;
      }

//---------------------------------------------------------
//   addToken
//---------------------------------------------------------

 void ParsedChord::addToken(QString s, ChordTokenClass tc)
      {
      ChordToken tok;
      tok.names += s;
      tok.tokenClass = tc;
      _tokenList += tok;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void ChordDescription::read(XmlReader& e, const QList<ChordToken>& tokenList)
      {
      int ni = 0, pci = 0;
      bool renderFound = false;
      id = e.attribute("id").toInt();
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "name") {
                  QString n = e.readElementText();
                  names.insert(ni++,n); // stack name for this file on top of the list
                  ParsedChord pc;
                  pc.parse(n);
                  if (parsedChords.indexOf(pc) < 0)
                        parsedChords.insert(pci++,pc);
                  }
            else if (tag == "xml")
                  xmlKind = e.readElementText();
            else if (tag == "degree")
                  xmlDegrees.append(e.readElementText());
            else if (tag == "voicing")
                  chord = HChord(e.readElementText());
            else if (tag == "render") {
                  readRenderList(e.readElementText(), renderList);
                  renderFound = true;
                  }
            else
                  e.unknown();
            }
      if (!renderFound) {
            ParsedChord pc = parsedChords.first();
            if (pc.renderable())
                  renderList = pc.renderList(tokenList);
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
      static int privateID = 10000;
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
            else if (tag == "token") {
                  ChordToken t;
                  t.read(e);
                  chordTokenList.append(t);
                  }
            else if (tag == "chord") {
                  int id = e.intAttribute("id");
                  // if no id attribute (id == 0), then assign it a private id for just QMap purposes
                  // user chords that match these ChordDescriptions will be treated as normal recognized chords
                  // except that the id will not be written to the score file
                  if (id == 0)
                        id = privateID++;
                  ChordDescription* cd = take(id);
                  if (cd == 0)
                        cd = new ChordDescription();
                  cd->read(e,chordTokenList);
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
      foreach (ChordToken t, chordTokenList)
            t.write(xml);
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


}



