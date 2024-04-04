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
      for (const QString& s : qAsConst(sl)) {
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

QString HChord::name(int tpc) const
      {
      static const HChord C0(0,3,6,9);
      static const HChord C1(0,3);

      QString buf = tpc2name(tpc, NoteSpellingType::STANDARD, NoteCaseType::AUTO, false);
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
//   voicing
//---------------------------------------------------------

QString HChord::voicing() const
      {
      QString s = "C";
      const char* names[] = { "C", " Db", " D", " Eb", " E", " F", " Gb", " G", " Ab", " A", " Bb", " B" };

      for (int i = 1; i < 12; i++) {
            if (contains(i))
                  s += names[i];
            }
      return s;
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
      for (const HDegree& d : degreeList) {
            int dv  = degreeTable[(d.value() - 1) % 7] + d.alter();
            int dv1 = degreeTable[(d.value() - 1) % 7];

            if (d.value() == 7 && d.alter() == 0) {
                  // DEBUG: seventh degree is Bb, not B
                  //        except Maj   (TODO)
                  dv -= 1;
                  }

            if (d.type() == HDegreeType::ADD)
                  *this += dv;
            else if (d.type() == HDegreeType::ALTER) {
                  if (contains(dv1)) {
                        *this -= dv1;
                        *this += dv;
                        }
                  else {
//                        qDebug("HDegreeType::ALTER: chord does not contain degree %d(%d):",
//                           d.value(), d.alter());
//                        print();
                        *this += dv;      // DEBUG: default to add
                        }
                  }
            else if (d.type() == HDegreeType::SUBTRACT) {
                  if (contains(dv1))
                        *this -= dv1;
                  else {
                        qDebug("SUB: chord does not contain degree %d(%d):",
                           d.value(), d.alter());
                        }
                  }
            else
                  qDebug("degree type %d not supported", static_cast<int>(d.type()));

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
      for (const QString& s : qAsConst(sl)) {
            if (s.startsWith("m:")) {
                  QStringList ssl = s.split(":", QString::SkipEmptyParts);
                  if (ssl.size() == 3) {
                        // m:x:y
                        RenderAction a;
                        a.type = RenderAction::RenderActionType::MOVE;
                        a.movex = ssl[1].toDouble();
                        a.movey = ssl[2].toDouble();
                        renderList.append(a);
                        }
#if 0
                  else if (ssl.size() == 2) {
                        // m:keyword
                        RenderAction a;
                        a.type = RenderAction::RenderActionType::MOVE;
                        // TODO: derive offset from keyword
                        }
#endif
                  }
            else if (s == ":push")
                  renderList.append(RenderAction(RenderAction::RenderActionType::PUSH));
            else if (s == ":pop")
                  renderList.append(RenderAction(RenderAction::RenderActionType::POP));
            else if (s == ":n")
                  renderList.append(RenderAction(RenderAction::RenderActionType::NOTE));
            else if (s == ":a")
                  renderList.append(RenderAction(RenderAction::RenderActionType::ACCIDENTAL));
            else {
                  RenderAction a(RenderAction::RenderActionType::SET);
                  a.text = s;
                  renderList.append(a);
                  }
            }
      }

//---------------------------------------------------------
//   writeRenderList
//---------------------------------------------------------

static void writeRenderList(XmlWriter& xml, const QList<RenderAction>* al, const QString& name)
      {
      QString s;

      int n = al->size();
      for (int i = 0; i < n; ++i) {
            if (!s.isEmpty())
                  s += " ";
            const RenderAction& a = (*al)[i];
            switch(a.type) {
                  case RenderAction::RenderActionType::SET:
                        s += a.text;
                        break;
                  case RenderAction::RenderActionType::MOVE:
                        if (!qFuzzyIsNull(a.movex) || !qFuzzyIsNull(a.movey))
                              s += QString("m:%1:%2").arg(a.movex).arg(a.movey);
                        break;
                  case RenderAction::RenderActionType::PUSH:
                        s += ":push";
                        break;
                  case RenderAction::RenderActionType::POP:
                        s += ":pop";
                        break;
                  case RenderAction::RenderActionType::NOTE:
                        s += ":n";
                        break;
                  case RenderAction::RenderActionType::ACCIDENTAL:
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
            tokenClass = ChordTokenClass::QUALITY;
      else if (c == "extension")
            tokenClass = ChordTokenClass::EXTENSION;
      else if (c == "modifier")
            tokenClass = ChordTokenClass::MODIFIER;
      else
            tokenClass = ChordTokenClass::ALL;
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

void ChordToken::write(XmlWriter& xml) const
      {
      QString t = "token";
      switch (tokenClass) {
            case ChordTokenClass::QUALITY:
                  t += " class=\"quality\"";
                  break;
            case ChordTokenClass::EXTENSION:
                  t += " class=\"extension\"";
                  break;
            case ChordTokenClass::MODIFIER:
                  t += " class=\"modifier\"";
                  break;
            default:
                  break;
      }
      xml.stag(t);
      for (const QString& s : names)
            xml.tag("name", s);
      writeRenderList(xml, &renderList, "render");
      xml.etag();
      }

//---------------------------------------------------------
//  ParsedChord
//---------------------------------------------------------

ParsedChord::ParsedChord()
      {
      _parseable = false;
      _understandable = false;
      }

//---------------------------------------------------------
//  configure
//---------------------------------------------------------

void ParsedChord::configure(const ChordList* cl)
      {
      if (!cl)
            return;
      // TODO: allow this to be parameterized via chord list
      major << "ma" << "maj" << "major" << "t" << "^";
      minor << "mi" << "min" << "minor" << "-" << "=";
      diminished << "dim" << "o";
      augmented << "aug" << "+";
      lower << "b" << "-" << "dim";
      raise << "#" << "+" << "aug";
      mod1 << "sus" << "alt";
      mod2 << "sus" << "add" << "no" << "omit" << "^";
      symbols << "t" << "^" << "-" << "+" << "o" << "0";
      }

//---------------------------------------------------------
//  correctXmlText
//    remove digits from _xmlText, optionally replace with s
//    needed for m(Maj9) et al
//---------------------------------------------------------

void ParsedChord::correctXmlText(const QString& s)
      {
      _xmlText.remove(QRegExp("[0-9]"));
      if (s != "") {
            int pos = _xmlText.lastIndexOf(')');
            if (pos == -1)
                  pos = _xmlText.size();
            _xmlText.insert(pos,s);
            }
      }

//---------------------------------------------------------
//  parse
//    returns true if chord was parseable
//---------------------------------------------------------

bool ParsedChord::parse(const QString& s, const ChordList* cl, bool syntaxOnly, bool preferMinor)
      {
      QString tok1, tok1L, tok2, tok2L;
      QString extensionDigits = "123456789";
      QString special = "()[],/\\ ";
      QString leading = "([ ";
      QString trailing = ")],/\\ ";
      QString initial;
      bool take6 = false, take7 = false, take9 = false, take11 = false, take13 = false;
#if 0
// enable this to allow quality or extension to be parenthesized
      int firstLeadingToken;
#endif
      int lastLeadingToken;
      int len = s.size();
      int i;
      int thirdKey = 0, seventhKey = 0;
      bool susChord = false;
      QList<HDegree> hdl;
      int key[] = { 0, 0, 2, 4, 5, 7, 9, 11, 0, 2, 4, 5, 7, 9, 11 };

      configure(cl);
      _name = s;
      _parseable = true;
      _understandable = true;
      i = 0;

#if 0
// enable this code to allow quality to be parenthesized
// otherwise, parentheses will automatically cause contents to be interpreted as extension or modifier
      // eat leading parens
      firstLeadingToken = _tokenList.size();
      while (i < len && leading.contains(s[i]))
           addToken(QString(s[i++]),ChordTokenClass::QUALITY);
#endif
      lastLeadingToken = _tokenList.size();
      // get quality
      for (tok1 = "", tok1L = "", initial = ""; i < len; ++i) {
            // up to first (non-zero) digit, paren, or comma
            if (extensionDigits.contains(s[i]) || special.contains(s[i]))
                  break;
            tok1.push_back(s[i]);
            tok1L.push_back(s[i].toLower());
            if (tok1L == "m" || major.contains(tok1L) || minor.contains(tok1L) || diminished.contains(tok1L) || augmented.contains(tok1L))
                  initial = tok1;
            }
      // special case for "madd", which needs to parse as m,add rather than ma,dd
      if (tok1L.startsWith("madd"))
            initial = tok1[0];
      // quality and first modifier ran together with no separation - eg, mima7, augadd
      // keep quality portion, reset index to read modifier portion later
      if (initial != "" && initial != tok1 && tok1L != "tristan" && tok1L != "omit") {
            i -= (tok1.length() - initial.length());
            tok1 = initial;
            tok1L = initial.toLower();
            }
      // determine quality
      if (tok1 == "M" || major.contains(tok1L)) {
            _quality = "major";
            take6 = true; take7 = true; take9 = true; take11 = true; take13 = true;
            if (!syntaxOnly)
                  chord = HChord("C E G");
            }
      else if (tok1 == "m" || minor.contains(tok1L)) {
            _quality = "minor";
            take6 = true; take7 = true; take9 = true; take11 = true; take13 = true;
            if (!syntaxOnly)
                  chord = HChord("C Eb G");
            }
      else if (diminished.contains(tok1L)) {
            _quality = "diminished";
            take7 = true;
            if (!syntaxOnly)
                  chord = HChord("C Eb Gb");
            }
      else if (augmented.contains(tok1L)) {
            _quality = "augmented";
            take7 = true;
            if (!syntaxOnly)
                  chord = HChord("C E G#");
            }
      else if (tok1L == "0") {
            _quality = "half-diminished";
            if (!syntaxOnly)
                  chord = HChord("C Eb Gb Bb");
            }
      else if (tok1L == "") {
            // empty quality - this will turn out to be major or dominant (or minor if preferMinor)
            _quality = "";
            if (!syntaxOnly)
                  chord = preferMinor ? HChord("C Eb G") : HChord("C E G");
            if (preferMinor)
                  _name = "=" + _name;
            }
      else {
            // anything else is not a quality after all, but a modifier
            // reset to read again as modifier
            _quality = "";
            tok1 = "";
            tok1L = "";
            i = lastLeadingToken;
            if (!syntaxOnly)
                  chord = HChord("C E G");
            }
      if (tok1L == "=") {
            tok1 = "";
            tok1L = "";
            }
      if (tok1 != "")
            addToken(tok1,ChordTokenClass::QUALITY);
#if 0
// enable this code to allow quality to be parenthesized
// otherwise, parentheses will automatically cause contents to be interpreted as extension or modifier
      else {
            // leading tokens were not really ChordTokenClass::QUALITY
            for (int t = firstLeadingToken; t < lastLeadingToken; ++t)
                  _tokenList[t].tokenClass = ChordTokenClass::EXTENSION;
            }
#endif
      if (!syntaxOnly) {
            _xmlKind = _quality;
            _xmlParens = "no";
            if (symbols.contains(tok1)) {
                  _xmlSymbols = "yes";
                  _xmlText = "";
                  }
            else {
                  _xmlSymbols = "no";
                  _xmlText = tok1;
                  }
            }
      // eat trailing parens and commas
      while (i < len && trailing.contains(s[i]))
           addToken(QString(s[i++]),ChordTokenClass::QUALITY);

#if 0
// enable this code to allow extensions to be parenthesized
// otherwise, parentheses will automatically cause contents to be interpreted as modifier
      // eat leading parens
      firstLeadingToken = _tokenList.size();
      while (i < len && leading.contains(s[i]))
            addToken(QString(s[i++]),ChordTokenClass::EXTENSION);
#endif
      lastLeadingToken = _tokenList.size();
      // get extension - up to first non-digit other than comma or slash
      for (tok1 = ""; i < len; ++i) {
            if (!s[i].isDigit() && s[i] != ',' && s[i] != '/')
                  break;
            tok1.push_back(s[i]);
            }
      _extension = tok1;
      if (_quality == "") {
            if (_extension == "7" || _extension == "9" || _extension == "11" || _extension == "13") {
                  _quality = preferMinor ? "minor" : "dominant";
                  if (!syntaxOnly)
                        _xmlKind = preferMinor ? "minor" : "dominant";
                  take7 = true; take9 = true; take11 = true; take13 = true;
                  }
            else {
                  _quality = preferMinor ? "minor" : "major";
                  if (!syntaxOnly)
                        _xmlKind = preferMinor ? "minor" : "major";
                  take6 = true; take7 = true; take9 = true; take11 = true; take13 = true;
                  }
            }
      if (tok1 != "")
            addToken(tok1,ChordTokenClass::EXTENSION);
#if 0
// enable this code to allow extensions to be parenthesized
// otherwise, parentheses will automatically cause contents to be interpreted as modifier
      else {
            // leading tokens were not really ChordTokenClass::EXTENSION
            for (int t = firstLeadingToken; t < lastLeadingToken; ++t) {
                  _tokenList[t].tokenClass = ChordTokenClass::MODIFIER;
                  if (!syntaxOnly)
                        _xmlParens = "yes";
                  }
            }
#endif
      if (!syntaxOnly) {
            if (_quality == "minor")
                  thirdKey = 3;
            else
                  thirdKey = 4;
            if (_quality == "major")
                  seventhKey = 11;
            else if (_quality == "diminished")
                  seventhKey = 9;
            else
                  seventhKey = 10;
            _xmlText += _extension;
            QStringList extl;
            if (tok1 == "2") {
                  QString d = "add" + tok1;
                  _xmlDegrees += d;
                  _xmlText.remove(tok1);
                  chord += 2;
                  }
            else if (tok1 == "4") {
                  QString d = "add" + tok1;
                  _xmlDegrees += d;
                  _xmlText.remove(tok1);
                  chord += 5;
                  }
            else if (tok1 == "5") {
                  _xmlKind = "power";
                  chord -= thirdKey;
                  }
            else if (tok1 == "6") {
                  if (take6)
                        _xmlKind += "-sixth";
                  else
                        extl << "6";
                  chord += 9;
                  }
            else if (tok1 == "7") {
                  if (take7)
                        _xmlKind += "-seventh";
                  else if (_xmlKind != "half-diminished")
                        extl << "7";
                  chord += seventhKey;
                  }
            else if (tok1 == "9") {
                  if (take9)
                        _xmlKind += "-ninth";
                  else if (take7) {
                              _xmlKind += "-seventh";
                              extl << "9";
                              correctXmlText("7");
                              }
                  else if (_xmlKind == "half-diminished") {
                        extl << "9";
                        correctXmlText();
                        }
                  else {
                        extl << "7" << "9";
                        correctXmlText();
                        }
                  chord += seventhKey;
                  chord += 2;
                  }
            else if (tok1 == "11") {
                  if (take11)
                        _xmlKind += "-11th";
                  else if (take7) {
                        _xmlKind += "-seventh";
                        extl << "9" << "11";
                        correctXmlText("7");
                        }
                  else if (_xmlKind == "half-diminished") {
                        extl << "9" << "11";
                        correctXmlText();
                        }
                  else {
                        extl << "7" << "9" << "11";
                        correctXmlText();
                        }
                  chord += seventhKey;
                  chord += 2;
                  chord += 5;
                  }
            else if (tok1 == "13") {
                  if (take13)
                        _xmlKind += "-13th";
                  else if (take7) {
                        _xmlKind += "-seventh";
                        extl << "9" << "11" << "13";
                        correctXmlText("7");
                        }
                  else if (_xmlKind == "half-diminished") {
                        extl << "9" << "11" << "13";
                        correctXmlText();
                        }
                  else {
                        extl << "7" << "9" << "11" << "13";
                        correctXmlText();
                        }
                  chord += seventhKey;
                  chord += 2;
                  chord += 5;
                  chord += 9;
                  }
            else if (tok1 == "69" || tok1 == "6,9" || tok1 == "6/9") {
                  if (take6) {
                        _xmlKind += "-sixth";
                        extl << "9";
                        correctXmlText("6");
                        }
                  else {
                        extl << "6" << "9";
                        correctXmlText();
                        }
                  _extension = "69";
                  chord += 9;
                  chord += 2;
                  }
            for (const QString &e : qAsConst(extl)) {
                  QString d = "add" + e;
                  _xmlDegrees += d;
                  }
            if (_xmlKind == "dominant-seventh")
                  _xmlKind = "dominant";
            }
      // eat trailing parens and commas
      while (i < len && trailing.contains(s[i]))
           addToken(QString(s[i++]),ChordTokenClass::EXTENSION);

      // get modifiers
      bool addPending = false;
      _modifierList.clear();
      while (i < len) {
            // eat leading parens
            while (i < len && leading.contains(s[i])) {
                  addToken(QString(s[i++]),ChordTokenClass::MODIFIER);
                  _xmlParens = "yes";
                  }
            // get first token - up to first digit, paren, or comma
            for (tok1 = "", tok1L = "", initial = ""; i < len; ++i) {
                  if (s[i].isDigit() || special.contains(s[i]))
                        break;
                  tok1.push_back(s[i]);
                  tok1L.push_back(s[i].toLower());
                  if (mod2.contains(tok1L))
                        initial = tok1;
                  }
            // if we reached the end of the string and never got a token,
            // then nothing to do, and no sense in looking for a second token
            if (i == len && tok1 == "")
                  break;
            if (initial != "" && initial != tok1) {
                  // two modifiers ran together with no separation - eg, susb9
                  // keep first, reset index to read second later
                  i -= (tok1.length() - initial.length());
                  tok1 = initial;
                  tok1L = initial.toLower();
                  }
            // for "add", just add the token and then read argument as a separate modifier
            // this allows the argument to itself be a two-part string
            // thus allowing addb9 -> add;b,9
            if (tok1L == "add") {
                  addToken(tok1,ChordTokenClass::MODIFIER);
                  addPending = true;
                  continue;
                  }
            // eat spaces
            while (i < len && s[i] == ' ')
                  ++i;
            // get second token - a number <= 13
            for (tok2 = ""; i < len; ++i) {
                  if (!s[i].isDigit())
                        break;
                  if (tok2.size() == 1 && (tok2[0] != '1' || s[i] > '3'))
                        break;
                  tok2.push_back(s[i]);
                  }
            tok2L = tok2.toLower();
            // re-attach "add"
            if (addPending) {
                  if (raise.contains(tok1L))
                        tok1L = "#";
                  else if (lower.contains(tok1L))
                        tok1L = "b";
                  else if (tok1 == "M" || major.contains(tok1L))
                        tok1L = "major";
                  tok2L = tok1L + tok2L;
                  tok1L = "add";
                  }
            // standardize spelling
            if (tok1 == "M" || major.contains(tok1L))
                  tok1L = "major";
            else if (tok1L == "omit")
                  tok1L = "no";
            else if (tok1L == "sus" && tok2L == "")
                  tok2L = "4";
            else if (augmented.contains(tok1L) && tok2L == "") {
                  if (_quality == "dominant" && _extension == "7") {
                        _quality = "augmented";
                        if (!syntaxOnly) {
                              _xmlKind = "augmented-seventh";
                              _xmlText = _extension + tok1;
                              chord -= 7;
                              chord += 8;
                              }
                        tok1L = "";
                        }
                  else {
                        tok1L = "#";
                        tok2L = "5";
                        }
                  }
            else if (diminished.contains(tok1)) {
                  _quality = "diminished";
                  if (!syntaxOnly) {
                        _xmlKind = "diminished";
                        _xmlText = _extension + tok1;
                        chord -= 4;
                        chord += 3;
                        chord -= 7;
                        chord += 6;
                        }
                  tok1L = "";
                  }
            else if ((lower.contains(tok1L) || raise.contains(tok1L)) && tok2L == "") {
                  // trailing alteration - treat as applying to extension (and convert to modifier)
                  // this handles C5b, C9#, etc
                  tok2L = _extension;
                  if (!syntaxOnly) {
                        _xmlKind = (_quality == "dominant") ? "major" : _quality;
                        _xmlText.remove(_extension);
                        if (_extension == "5")
                              chord += thirdKey;
                        else
                              chord -= seventhKey;
                        }
                  if (_quality == "dominant")
                        _quality = "major";
                  _extension = "";
                  if (lower.contains(tok1L))
                        tok1L = "b";
                  else
                        tok1L = "#";
                  }
            else if (lower.contains(tok1L))
                  tok1L = "b";
            else if (raise.contains(tok1L))
                  tok1L = "#";
            QString m = tok1L + tok2L;
            if (m != "")
                  _modifierList += m;
            if (tok1 != "")
                  addToken(tok1,ChordTokenClass::MODIFIER);
            if (tok2 != "")
                  addToken(tok2,ChordTokenClass::MODIFIER);
            if (!syntaxOnly) {
                  int d;
                  if (tok2L == "")
                        d = 0;
                  else
                        d = tok2L.toInt();
                  if (d > 13)
                        d = 13;
                  QString degree;
                  bool alter = false;
                  if (tok1L == "add") {
                        if (d)
                              hdl += HDegree(d, 0, HDegreeType::ADD);
                        else if (tok2L != "") {
                              // this was result of addPending
                              // alteration; tok1 = alter, tok2 = value
                              d = tok2.toInt();
                              if (raise.contains(tok1) || tok1 == "M" || major.contains(tok1.toLower())) {
                                    if (d == 7) {
                                          chord += 11;
                                          tok2L = "#7";
                                          }
                                    else if (raise.contains(tok1))
                                          hdl += HDegree(d, 1, HDegreeType::ADD);
                                    }
                              else if (lower.contains(tok1)) {
                                    if (d == 7)
                                          chord += 10;
                                    else
                                          hdl += HDegree(d, -1, HDegreeType::ADD);
                                    }
                              else if (d)
                                    hdl += HDegree(d, 0, HDegreeType::ADD);
                              }
                        degree = "add" + tok2L;
                        }
                  else if (tok1L == "no") {
                        degree = "sub" + tok2L;
                        if (d)
                              hdl += HDegree(d, 0, HDegreeType::SUBTRACT);
                        }
                  else if (tok1L == "sus") {
#if 0
// enable this code to export 7sus4 as dominant,sub3,add4 rather than suspended-fourth,add7 (similar for 9sus4, 13sus4)
// should basically work, but not fully tested
// probably needs corresponding special casing at end of loop
                        if (_extension == "") {
                              if (tok2L == "4")
                                    _xmlKind = "suspended-fourth";
                              else if (tok2L == "2")
                                    _xmlKind = "suspended-second";
                              _xmlText = tok1 + tok2;
                              }
                        else {
                              _xmlDegrees += "sub3";
                              tok1L = "add";
                              }
#else
// enable this code to export 7sus4 as suspended-fourth,add7 rather than dominant,sub3,add4 (similar for 9sus4, 13sus4)
                        // convert chords with sus into suspended "kind"
                        // extension then becomes a series of degree adds
                        if (tok2L == "4")
                              _xmlKind = "suspended-fourth";
                        else if (tok2L == "2")
                              _xmlKind = "suspended-second";
                        _xmlText = tok1 + tok2;
                        if (_extension == "7" || _extension == "9" || _extension == "11" || _extension == "13") {
                              _xmlDegrees += (_quality == "major") ? "add#7" : "add7";
                              // hack for programs that cannot assemble names well
                              // even though the kind is suspended, set text to also include the extension
                              // in export, we will set the degree text to null
                              _xmlText = _extension + _xmlText;
                              degree = "";
                              }
                        else if (_extension != "")
                              degree = "add" + _extension;
                        if (_extension == "13") {
                              _xmlDegrees += "add9";
                              _xmlDegrees += "add11";
                              _xmlDegrees += "add13";
                        }
                        else if (_extension == "11") {
                              _xmlDegrees += "add9";
                              _xmlDegrees += "add11";
                              }
                        else if (_extension == "9")
                              _xmlDegrees += "add9";
#endif
                        susChord = true;
                        chord -= thirdKey;
                        if (d)
                              chord += key[d];
                        }
                  else if (tok1L == "major") {
                        if (_xmlKind.startsWith("minor")) {
                              _xmlKind = "major-minor";
                              if (_extension == "9" || tok2L == "9")
                                    _xmlDegrees += "add9";
                              if (_extension == "11" || tok2L == "11") {
                                    _xmlDegrees += "add9";
                                    _xmlDegrees += "add11";
                                    }
                              if (_extension == "13" || tok2L == "13") {
                                    _xmlDegrees += "add9";
                                    _xmlDegrees += "add11";
                                    _xmlDegrees += "add13";
                                    }
                              _xmlText += tok1 + tok2;
                              correctXmlText("7");
                              }
                        else
                              tok1L = "add";
                        chord -= 10;
                        chord += 11;
                        if (d && d != 7)
                              hdl += HDegree(d, 0, HDegreeType::ADD);
                        }
                  else if (tok1L == "alt") {
                        _xmlDegrees += "altb5";
                        _xmlDegrees += "add#5";
                        _xmlDegrees += "addb9";
                        _xmlDegrees += "add#9";
                        chord -= 7;
                        chord += 6;
                        chord += 8;
                        chord += 1;
                        chord += 3;
                        }
                  else if (tok1L == "blues") {
                        // this isn't really well-defined, but it might as well mean something
                        if (_extension == "11" || _extension == "13")
                              _xmlDegrees += "alt#9";
                        else
                              _xmlDegrees += "add#9";
                        chord += 3;
                        }
                  else if (tok1L == "lyd") {
                        if (_extension == "13")
                              _xmlDegrees += "alt#11";
                        else
                              _xmlDegrees += "add#11";
                        chord += 6;
                        }
                  else if (tok1L == "phryg") {
                        if (!_xmlKind.startsWith("minor"))
                              _xmlKind = "minor-seventh";
                        if (_extension == "11" || _extension == "13")
                              _xmlDegrees += "altb9";
                        else
                              _xmlDegrees += "addb9";
                        _xmlText += tok1;
                        chord = HChord("C Db Eb G Bb");
                        }
                  else if (tok1L == "tristan") {
                        _xmlKind = "Tristan";
                        _xmlText = tok1;
                        chord = HChord("C F# A# D#");
                        }
                  else if (addPending) {
                        degree = "add" + tok1L + tok2L;
                        if (raise.contains(tok1L))
                              hdl += HDegree(d, 1, HDegreeType::ADD);
                        else if (lower.contains(tok1L))
                              hdl += HDegree(d, -1, HDegreeType::ADD);
                        else
                              hdl += HDegree(d, 0, HDegreeType::ADD);
                        }
                  else if (tok1L == "" && tok2L != "") {
                        degree = "add" + tok2L;
                        hdl += HDegree(d, 0, HDegreeType::ADD);
                        }
                  else if (lower.contains(tok1L)) {
                        tok1L = "b";
                        alter = true;
                        }
                  else if (raise.contains(tok1L)) {
                        tok1L = "#";
                        alter = true;
                        }
                  else if (tok1L == "") {
                        // token was already handled fully
                        }
                  else {
                        _understandable = false;
                        if (s.startsWith(tok1)) {
                              // unrecognized token right from very beginning
                              _xmlKind = "other";
                              _xmlText = tok1;
                              }
                        }
                  if (alter) {
                        if (tok2L == "4" && _xmlKind == "suspended-fourth")
                              degree = "alt";
                        else if (tok2L == "5")
                              degree = "alt";
                        else if (tok2L == "9" && (_extension == "11" || _extension == "13"))
                              degree = "alt";
                        else if (tok2L == "11" && _extension == "13")
                              degree = "alt";
                        else
                              degree = "add";
                        degree += tok1L + tok2L;
                        if (chord.contains(key[d]) && !(susChord && (d == 11)))
                              hdl += HDegree(d, 0, HDegreeType::SUBTRACT);
                        if (tok1L == "#")
                              hdl += HDegree(d, 1, HDegreeType::ADD);
                        else if (tok1L == "b")
                              hdl += HDegree(d, -1, HDegreeType::ADD);
                        }
                  if (degree != "")
                        _xmlDegrees += degree;
                  }
            // eat trailing parens and commas
            while (i < len && trailing.contains(s[i]))
                  addToken(QString(s[i++]),ChordTokenClass::MODIFIER);
            addPending = false;
            }
      if (!syntaxOnly) {
            chord.add(hdl);
            // fix "add" / "alt" conflicts
            // so add9,altb9 -> addb9
            QStringList altList = _xmlDegrees.filter("alt");
            for (const QString& d : qAsConst(altList)) {
                  QString unalt(d);
                  unalt.replace(QRegExp("alt[b#]"),"add");
                  if (_xmlDegrees.removeAll(unalt) > 0) {
                        QString alt(d);
                        alt.replace("alt","add");
                        int i1 = _xmlDegrees.indexOf(d);
                        _xmlDegrees.replace(i1, alt);
                        }
                  }
            }

      // construct handle
      if (!_modifierList.empty()) {
            _modifierList.sort();
            _modifiers = "<" + _modifierList.join("><") + ">";
            }
      _handle = "<" + _quality + "><" + _extension + ">" + _modifiers;

      // force <minor><7><b5> to export as half-diminished
      if (!syntaxOnly && _handle == "<minor><7><b5>") {
                  _xmlKind = "half-diminished";
                  _xmlText = s;
                  _xmlDegrees.clear();
            }
      if (MScore::debugMode) {
            qDebug("parse: source = <%s>, handle = %s", qPrintable(s), qPrintable(_handle));
            if (!syntaxOnly) {
                  qDebug("parse: HChord = <%s> (%d)", qPrintable(chord.voicing()), chord.getKeys());
                  qDebug("parse: xmlKind = <%s>, text = <%s>", qPrintable(_xmlKind), qPrintable(_xmlText));
                  qDebug("parse: xmlSymbols = %s, xmlParens = %s", qPrintable(_xmlSymbols), qPrintable(_xmlParens));
                  qDebug("parse: xmlDegrees = <%s>", qPrintable(_xmlDegrees.join(",")));
                  }
            }
      return _parseable;
      }

//---------------------------------------------------------
//   fromXml
//---------------------------------------------------------

QString ParsedChord::fromXml(const QString& rawKind, const QString& rawKindText, const QString& useSymbols, const QString& useParens, const QList<HDegree>& dl, const ChordList* cl)
      {
      QString kind = rawKind;
      QString kindText = rawKindText;
      bool syms = (useSymbols == "yes");
      bool parens = (useParens == "yes");
      bool implied = false;
      bool extend = false;
      int extension = 0;
      _parseable = true;
      _understandable = true;

      // get quality info from kind
      if (kind == "major-minor") {
            _quality = "minor";
            _modifierList += "major7";
            extend = true;
            }
      else if (kind.contains("major")) {
            _quality = "major";
            if (kind == "major" || kind == "major-sixth")
                  implied = true;
            }
      else if (kind.contains("minor"))
            _quality = "minor";
      else if (kind.contains("dominant")) {
            _quality = "dominant";
            implied = true;
            extension = 7;
            }
      else if (kind == "augmented-seventh") {
            _quality = "augmented";
            extension = 7;
            extend = true;
            }
      else if (kind == "augmented")
            _quality = "augmented";
      else if (kind == "half-diminished") {
            _quality = "half-diminished";
            if (syms) {
                  extension = 7;
                  extend = true;
                  }
            }
      else if (kind == "diminished-seventh") {
            _quality = "diminished";
            extension = 7;
            }
      else if (kind == "diminished")
            _quality = "diminished";
      else if (kind == "suspended-fourth") {
            _quality = "major";
            implied = true;
            _modifierList += "sus4";
            }
      else if (kind == "suspended-second") {
            _quality = "major";
            implied = true;
            _modifierList += "sus2";
            }
      else if (kind == "power") {
            _quality = "major";
            implied = true;
            extension = 5;
            }
      else
            _quality = kind;

      // get extension info from kind
      if (kind.contains("seventh"))
            extension = 7;
      else if (kind.contains("ninth"))
            extension = 9;
      else if (kind.contains("11th"))
            extension = 11;
      else if (kind.contains("13th"))
            extension = 13;
      else if (kind.contains("sixth"))
            extension = 6;

      // get modifier info from degree list
      for (const HDegree& d : dl) {
            QString mod;
            int v = d.value();
            switch (d.type()) {
                  case HDegreeType::ADD:
                  case HDegreeType::ALTER:
                        switch (d.alter()) {
                              case -1:    mod = "b"; break;
                              case 1:     mod = "#"; break;
                              case 0:     mod = "add"; break;
                              }
                        break;
                  case HDegreeType::SUBTRACT:
                        mod = "no";
                        break;
                  case HDegreeType::UNDEF:
                  default:
                        break;
                  }
            mod += QString("%1").arg(v);
            if (mod == "add7" && kind.contains("suspended")) {
                  _quality = "dominant";
                  implied = true;
                  extension = 7;
                  extend = true;
                  mod = "";
                  }
            else if (mod == "add#7" && kind.contains("suspended")) {
                  _quality = "major";
                  implied = false;
                  extension = 7;
                  extend = true;
                  mod = "";
                  }
            else if (mod == "add9" && extend) {
                  if (extension < 9)
                        extension = 9;
                  mod = "";
                  }
            else if (mod == "add11" && extend) {
                  if (extension < 11)
                        extension = 11;
                  mod = "";
                  }
            else if (mod == "add13" && extend) {
                  extension = 13;
                  mod = "";
                  }
            else if (mod == "add9" && kind.contains("sixth")) {
                  extension = 69;
                  mod = "";
                  }
            if (mod != "")
                  _modifierList += mod;
            }
      // convert no3,add[42] into sus[42]
      int no3 = _modifierList.indexOf("no3");
      if (no3 >= 0) {
            int addn = _modifierList.indexOf("add4");
            if (addn == -1)
                  addn = _modifierList.indexOf("add2");
            if (addn != -1) {
                  QString& s = _modifierList[addn];
                  s.replace("add","sus");
                  _modifierList.removeAt(no3);
                  }
            }
      // convert kind=minor-seventh, degree=altb5 to kind=half-diminished (suppression of degree=altb comes later)
      if (kind == "minor-seventh" && _modifierList.size() == 1 && _modifierList.front() == "b5")
            kind = "half-diminished";
      // force parens where necessary)
      if (!parens && extension == 0 && !_modifierList.empty()) {
            QString firstMod = _modifierList.front();
            if (firstMod != "" && (firstMod.startsWith('#') || firstMod.startsWith('b')))
                  parens = true;
            }

      // record extension
      if (extension)
            _extension = QString("%1").arg(extension);

      // validate kindText
      if (kindText != "" && kind != "none" && kind != "other") {
            ParsedChord validate;
            validate.parse(kindText, cl, false);
            // kindText should parse to produce same kind, no degrees
            if (validate._xmlKind != kind || !validate._xmlDegrees.empty())
                  kindText = "";
            }

      // construct name & handle
      _name = "";
      if (kindText != "") {
            if (_extension != "" && kind.contains("suspended"))
                  _name += _extension;
            _name += kindText;
            if (extension == 69)
                  _name += "9";
            }
      else if (implied)
            _name = _extension;
      else {
            if (_quality == "major")
                  _name = syms ? "^" : "maj";
            else if (_quality == "minor")
                  _name = syms ? "-" : "m";
            else if (_quality == "augmented")
                  _name = syms ? "+" : "aug";
            else if (_quality == "diminished")
                  _name = syms ? "o" : "dim";
            else if (_quality == "half-diminished")
                  _name = syms ? "0" : "m7b5";
            else
                  _name = _quality;
            _name += _extension;
            }
      if (parens)
            _name += "(";
      for (QString mod : qAsConst(_modifierList)) {
            mod.replace("major","maj");
            if (kindText != "" && kind.contains("suspended") && mod.startsWith("sus"))
                  continue;
            else if (kindText != "" && kind == "major-minor" && mod.startsWith("maj"))
                  continue;
            _name += mod;
            }
      if (parens)
            _name += ")";

      // parse name to construct handle & tokenList
      parse(_name, cl, true);

      // record original MusicXML
      _xmlKind = kind;
      _xmlText = kindText;
      _xmlSymbols = useSymbols;
      _xmlParens = useParens;
      for (const HDegree& d : dl) {
            if (kind == "half-diminished" && d.type() == HDegreeType::ALTER && d.alter() == -1 && d.value() == 5)
                  continue;
            _xmlDegrees += d.text();
            }

      return _name;
      }


//---------------------------------------------------------
//   position
//---------------------------------------------------------

qreal ChordList::position(const QStringList& names, ChordTokenClass ctc) const
      {
      QString name = names.empty() ? "" : names.first();
      switch (ctc) {
            case ChordTokenClass::EXTENSION:
                  return _eadjust;
            case ChordTokenClass::MODIFIER: {
                  QChar c = name.isEmpty() ? name.at(0) : '0';
                  if (c.isDigit() || c.isPunct())
                        return _madjust;
                  else
                        return 0.0;
                  }
            default:
                  if (name == "o" || name == "0")
                        return _eadjust;
                  else
                        return 0.0;
            }
      }

//---------------------------------------------------------
//   renderList
//---------------------------------------------------------

const QList<RenderAction>& ParsedChord::renderList(const ChordList* cl)
      {
      // generate anew on each call,
      // in case chord list has changed since last time
      if (!_renderList.empty())
            _renderList.clear();
      bool adjust = cl ? cl->autoAdjust() : false;
      for (const ChordToken &tok : qAsConst(_tokenList)) {
            QString n = tok.names.first();
            QList<RenderAction> rl;
            QList<ChordToken> definedTokens;
            bool found = false;
            // potential definitions for token
            if (cl) {
                  for (const ChordToken &ct : cl->chordTokenList) {
                        for (const QString &ctn : qAsConst(ct.names)) {
                              if (ctn == n)
                                    definedTokens += ct;
                              }
                        }
                  }
            // find matching class, fallback on ChordTokenClass::ALL
            ChordTokenClass ctc = ChordTokenClass::ALL;
            for (const ChordToken &matchingTok : qAsConst(definedTokens)) {
                  if (tok.tokenClass == matchingTok.tokenClass) {
                        rl = matchingTok.renderList;
                        ctc = tok.tokenClass;
                        found = true;
                        break;
                        }
                  else if (matchingTok.tokenClass == ChordTokenClass::ALL) {
                        rl = matchingTok.renderList;
                        found = true;
                        }
                  }
            // check for adjustments
            // stop adjusting when first non-adjusted modifier found
            qreal p = adjust ? cl->position(tok.names, ctc) : 0.0;
            if (tok.tokenClass == ChordTokenClass::MODIFIER && qFuzzyIsNull(p))
                  adjust = false;
            // build render list
            if (!qFuzzyIsNull(p)) {
                  RenderAction m1 = RenderAction(RenderAction::RenderActionType::MOVE);
                  m1.movex = 0.0;
                  m1.movey = p;
                  _renderList.append(m1);
                  }
            if (found) {
                  _renderList.append(rl);
                  }
            else {
                  // no definition for token, so render as literal
                  RenderAction a(RenderAction::RenderActionType::SET);
                  a.text = tok.names.first();
                  _renderList.append(a);
                  }
            if (!qFuzzyIsNull(p)) {
                  RenderAction m2 = RenderAction(RenderAction::RenderActionType::MOVE);
                  m2.movex = 0.0;
                  m2.movey = -p;
                  _renderList.append(m2);
                  }
            }
      return _renderList;
      }

//---------------------------------------------------------
//   addToken
//---------------------------------------------------------

void ParsedChord::addToken(QString s, ChordTokenClass tc)
      {
      if (s == "")
            return;
      ChordToken tok;
      tok.names += s;
      tok.tokenClass = tc;
      _tokenList += tok;
      }

//---------------------------------------------------------
//   ChordDescription
//    this form is used when reading from file
//    a private id is assigned for id = 0
//---------------------------------------------------------

ChordDescription::ChordDescription(int i)
      {
      if (!i)
//            i = --(cl->privateID);
            i = -- ChordList::privateID;
      id = i;
      generated = false;
      renderListGenerated = false;
      exportOk = true;
      }

//---------------------------------------------------------
//   ChordDescription
//    this form is used when generating from name
//    a private id is always assigned
//---------------------------------------------------------

ChordDescription::ChordDescription(const QString& name)
      {
      id = -- ChordList::privateID;
      generated = true;
      names.append(name);
      renderListGenerated = false;
      exportOk = false;
      }

//---------------------------------------------------------
//   complete
//    generate missing renderList and semantic (Xml) info
//---------------------------------------------------------

void ChordDescription::complete(ParsedChord* pc, const ChordList* cl)
      {
      ParsedChord tempPc;
      if (!pc) {
            // generate parsed chord for its rendering & semantic (xml) info
            pc = &tempPc;
            QString n;
            if (!names.empty())
                  n = names.front();
            pc->parse(n, cl);
            }
      parsedChords.append(*pc);
      if (renderList.empty() || renderListGenerated) {
            renderList = pc->renderList(cl);
            renderListGenerated = true;
            }
      if (xmlKind == "") {
            xmlKind = pc->xmlKind();
            xmlDegrees = pc->xmlDegrees();
            }
      // these fields are not read from chord description files
      // so get them from the parsed representation in all cases
      xmlText = pc->xmlText();
      xmlSymbols = pc->xmlSymbols();
      xmlParens = pc->xmlParens();
      if (chord.getKeys() == 0) {
            chord = HChord(pc->keys());
            }
      _quality = pc->quality();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void ChordDescription::read(XmlReader& e)
      {
      int ni = 0;
      id = e.attribute("id").toInt();
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "name") {
                  QString n = e.readElementText();
                  // stack names for this file on top of the list
                  names.insert(ni++,n);
                  }
            else if (tag == "xml")
                  xmlKind = e.readElementText();
            else if (tag == "degree")
                  xmlDegrees.append(e.readElementText());
            else if (tag == "voicing")
                  chord = HChord(e.readElementText());
            else if (tag == "render") {
                  readRenderList(e.readElementText(), renderList);
                  renderListGenerated = false;
                  }
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void ChordDescription::write(XmlWriter& xml) const
      {
      if (generated && !exportOk)
            return;
      if (id > 0)
            xml.stag(QString("chord id=\"%1\"").arg(id));
      else
            xml.stag(QString("chord"));
      for (const QString& s : names)
            xml.tag("name", s);
      xml.tag("xml", xmlKind);
      xml.tag("voicing", chord.voicing());
      for (const QString& s : xmlDegrees)
            xml.tag("degree", s);
      writeRenderList(xml, &renderList, "render");
      xml.etag();
      }


//---------------------------------------------------------
//   ChordList
//---------------------------------------------------------

int ChordList::privateID = -1000;

//---------------------------------------------------------
//   configureAutoAdjust
//---------------------------------------------------------

void ChordList::configureAutoAdjust(qreal emag, qreal eadjust, qreal mmag, qreal madjust)
      {
      _emag = emag;
      _eadjust = eadjust;
      _mmag = mmag;
      _madjust = madjust;
#if 0
      // TODO: regenerate all chord descriptions
      // currently we always reload the entire chordlist
      if (_autoAdjust) {
            for (ChordFont cf : fonts) {
                  if (cf.fontClass == "extension")
                        cf.mag = _emag;
                  else if (cf.fontClass == "modifier")
                        cf.mag = _mmag;
                  }
            }
#endif
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void ChordList::read(XmlReader& e)
      {
      int fontIdx = fonts.size();
      _autoAdjust = false;
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "font") {
                  ChordFont f;
                  f.family = e.attribute("family", "default");
                  if (f.family == "MuseJazz")
                        f.family = "MuseJazz Text";
                  f.mag    = 1.0;
                  f.fontClass = e.attribute("class");
                  while (e.readNextStartElement()) {
                        if (e.name() == "sym") {
                              ChordSymbol cs;
                              QString code;
                              QString symClass;
                              cs.fontIdx = fontIdx;
                              cs.name    = e.attribute("name");
                              cs.value   = e.attribute("value");
                              code       = e.attribute("code");
                              symClass   = e.attribute("class");
                              if (code != "") {
                                    bool ok = true;
                                    int val = code.toInt(&ok, 0);
                                    if (!ok) {
                                          cs.code = 0;
                                          cs.value = code;
                                          }
                                    else if (val & 0xffff0000) {
                                          cs.code = 0;
                                          QChar high = QChar(QChar::highSurrogate(val));
                                          QChar low = QChar(QChar::lowSurrogate(val));
                                          cs.value = QString("%1%2").arg(high).arg(low);
                                          }
                                    else {
                                          cs.code = val;
                                          cs.value = QString(cs.code);
                                          }
                                    }
                              else
                                    cs.code = 0;
                              if (cs.value == "")
                                    cs.value = cs.name;
                              cs.name = symClass + cs.name;
                              symbols.insert(cs.name, cs);
                              e.readNext();
                              }
                        else if (e.name() == "mag")
                              f.mag = e.readDouble();
                        else
                              e.unknown();
                        }
                  if (_autoAdjust) {
                        if (f.fontClass == "extension")
                              f.mag *= _emag;
                        else if (f.fontClass == "modifier")
                              f.mag *= _mmag;
                        }
                  fonts.append(f);
                  ++fontIdx;
                  }
            else if (tag == "autoAdjust") {
                  QString nmag = e.attribute("mag");
                  _nmag = nmag.toDouble();
                  QString nadjust = e.attribute("adjust");
                  _nadjust = nadjust.toDouble();
                  _autoAdjust = e.readBool();
                  }
            else if (tag == "token") {
                  ChordToken t;
                  t.read(e);
                  chordTokenList.append(t);
                  }
            else if (tag == "chord") {
                  int id = e.intAttribute("id");
                  // if no id attribute (id == 0), then assign it a private id
                  // user chords that match these ChordDescriptions will be treated as normal recognized chords
                  // except that the id will not be written to the score file
                  ChordDescription cd = (id && contains(id)) ? take(id) : ChordDescription(id);

                  // record updated id
                  id = cd.id;
                  // read rest of description
                  cd.read(e);
                  // restore updated id
                  cd.id = id;
                  // throw away previously parsed chords
                  cd.parsedChords.clear();
                  // generate any missing info (including new parsed chords)
                  cd.complete(0,this);
                  // add to list
                  insert(id, cd);
                  }
            else if (tag == "renderRoot")
                  readRenderList(e.readElementText(), renderListRoot);
            else if (tag == "renderFunction")
                  readRenderList(e.readElementText(), renderListFunction);
            else if (tag == "renderBase")
                  readRenderList(e.readElementText(), renderListBase);
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void ChordList::write(XmlWriter& xml) const
      {
      int fontIdx = 0;
      for (const ChordFont &f : fonts) {
            xml.stag(QString("font id=\"%1\" family=\"%2\"").arg(fontIdx).arg(f.family));
            xml.tag("mag", f.mag);
            for (const ChordSymbol &s : symbols) {
                  if (s.fontIdx == fontIdx) {
                        if (s.code.isNull())
                              xml.tagE(QString("sym name=\"%1\" value=\"%2\"").arg(s.name, s.value));
                        else
                              xml.tagE(QString("sym name=\"%1\" code=\"0x%2\"").arg(s.name).arg(s.code.unicode(), 0, 16));
                        }
                  }
            xml.etag();
            ++fontIdx;
            }
      if (_autoAdjust)
            xml.tagE(QString("autoAdjust mag=\"%1\" adjust=\"%2\"").arg(_nmag).arg(_nadjust));
      for (const ChordToken &t : chordTokenList)
            t.write(xml);
      if (!renderListRoot.empty())
            writeRenderList(xml, &renderListRoot, "renderRoot");
      if (!renderListFunction.empty())
            writeRenderList(xml, &renderListFunction, "renderFunction");
      if (!renderListBase.empty())
            writeRenderList(xml, &renderListBase, "renderBase");
      for (const ChordDescription& cd : *this)
            cd.write(xml);
      }

//---------------------------------------------------------
//   read
//    read Chord List, return false on error
//---------------------------------------------------------

bool ChordList::read(const QString& name)
      {
//      qDebug("ChordList::read <%s>", qPrintable(name));
      QString path;
      QFileInfo ftest(name);
      if (ftest.isAbsolute())
            path = name;
      else {
#if defined(Q_OS_IOS)
            path = QString("%1/%2").arg(MScore::globalShare()).arg(name);
#elif defined(Q_OS_ANDROID)
            path = QString(":/styles/%1").arg(name);
#else
            path = QString("%1styles/%2").arg(MScore::globalShare(), name);
#endif
            }
      // default to chords_std.xml
      QFileInfo fi(path);
      if (!fi.exists())
#if defined(Q_OS_IOS)
            path = QString("%1/%2").arg(MScore::globalShare()).arg("chords_std.xml");
#elif defined(Q_OS_ANDROID)
            path = QString(":/styles/chords_std.xml");
#else
            path = QString("%1styles/%2").arg(MScore::globalShare(), "chords_std.xml");
#endif

      if (name.isEmpty())
            return false;
      QFile f(path);
      if (!f.open(QIODevice::ReadOnly)) {
            MScore::lastError = QObject::tr("Cannot open chord description:\n%1\n%2").arg(f.fileName(), f.errorString());
            qDebug("ChordList::read failed: <%s>", qPrintable(path));
            return false;
            }
      XmlReader e(&f);

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

bool ChordList::write(const QString& name) const
      {
      QFileInfo info(name);

      if (info.suffix().isEmpty()) {
            QString path = info.filePath();
            path += QString(".xml");
            info.setFile(path);
            }

      QFile f(info.filePath());

      if (!f.open(QIODevice::WriteOnly)) {
            MScore::lastError = QObject::tr("Open chord description\n%1\nfailed: %2").arg(f.fileName(), f.errorString());
            return false;
            }

      XmlWriter xml(0, &f);
      xml.header();
      xml.stag("museScore version=\"" MSC_VERSION "\"");

      write(xml);
      xml.etag();
      if (f.error() != QFile::NoError) {
            MScore::lastError = QObject::tr("Write chord description failed: %1").arg(f.errorString());
            }
      return true;
      }

//---------------------------------------------------------
//   loaded
//---------------------------------------------------------

bool ChordList::loaded() const
      {
      // track whether a description file has been loaded
      // since chords.xml really doesn't load enough to stand alone,
      // we need a way to track when a "real" chord list has been loaded
      // for lack of anything better, key off renderListRoot
      return !renderListRoot.empty();
      }

//---------------------------------------------------------
//   unload
//---------------------------------------------------------

void ChordList::unload()
      {
      clear();
      symbols.clear();
      fonts.clear();
      renderListRoot.clear();
      renderListBase.clear();
      chordTokenList.clear();
      _autoAdjust = false;
      }

//---------------------------------------------------------
//   print
//    only for debugging
//---------------------------------------------------------

void RenderAction::print() const
      {
      static const char* names[] = {
            "SET", "MOVE", "PUSH", "POP",
            "NOTE", "ACCIDENTAL"
            };
      qDebug("%10s <%s> %f %f", names[int(type)], qPrintable(text), movex, movey);
      }


}
