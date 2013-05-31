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
      minor << "mi" << "min" << "minor" << "-";
      diminished << "dim" << "o";
      augmented << "aug" << "+";
      lower << "b" << "-";
      raise << "#" << "+";
      mod1 << "sus" << "alt";
      mod2 << "sus" << "add" << "no" << "omit";
      symbols << "t" << "^" << "-" << "+" << "o" << "0";
      }

//---------------------------------------------------------
//  parse
//    return true if chord was parseable
//---------------------------------------------------------

bool ParsedChord::parse(const QString& s, const ChordList* cl, bool syntaxOnly)
      {
      QString tok1, tok1L, tok2, tok2L;
      QString extensionDigits = "123456789";
      QString special = "(), ";
      QString leading = "( ";
      QString trailing = "), ";
      QString initial;
      bool take6 = false, take7 = false, take9 = false, take11 = false, take13 = false;
      int firstLeadingToken, lastLeadingToken;
      int len = s.size();
      int i, j;

      configure(cl);
      _parseable = true;
      i = 0;

      // eat leading parens
      firstLeadingToken = _tokenList.size();
      while (i < len && leading.contains(s[i]))
           addToken(QString(s[i++]),QUALITY);
      lastLeadingToken = _tokenList.size();
      // get quality
      for (j = 0, tok1 = "", tok1L = "", initial = ""; i < len; ++i, ++j) {
            // up to first (non-zero) digit, paren, or comma
            if (extensionDigits.contains(s[i]) || special.contains(s[i]))
                  break;
            tok1[j] = s[i];
            tok1L[j] = s[i].toLower();
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
            }
      else if (tok1 == "m" || minor.contains(tok1L)) {
            _quality = "minor";
            take6 = true; take7 = true; take9 = true; take11 = true; take13 = true;
            }
      else if (diminished.contains(tok1L)) {
            _quality = "diminished";
            take7 = true;
            }
      else if (augmented.contains(tok1L)) {
            _quality = "augmented";
            take7 = true;
            }
      else if (tok1L == "0") {
            _quality = "half-diminished";
            }
      else if (tok1L == "") {
            // empty quality - this will turn out to be major or dominant
            _quality = "";
            }
      else {
            // anything else is not a quality after all, but a modifier
            // reset to read again as modifier
            _quality = "";
            tok1 = "";
            tok1L = "";
            i = lastLeadingToken;
            }
      if (tok1 != "")
            addToken(tok1,QUALITY);
      else {
            // leading tokens were not really QUALITY
            for (int t = firstLeadingToken; t < lastLeadingToken; ++t)
                  _tokenList[t].tokenClass = EXTENSION;
            }
      _xmlKind = _quality;
      _xmlText = tok1;
      if (symbols.contains(tok1))
            _xmlSymbols = "yes";
      else
            _xmlSymbols = "no";
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
      _extension = tok1;
      if (_quality == "") {
            if (_extension == "7" || _extension == "9" || _extension == "11" || _extension == "13") {
                  _quality = "dominant";
                  _xmlKind = "dominant";
                  take7 = true; take9 = true; take11 = true; take13 = true;
                  }
            else {
                  _quality = "major";
                  _xmlKind = "major";
                  take6 = true; take7 = true; take9 = true; take11 = true; take13 = true;
                  }
            }
      if (tok1 != "")
            addToken(tok1,EXTENSION);
      else {
            // leading tokens were not really EXTENSION
            for (int t = firstLeadingToken; t < lastLeadingToken; ++t)
                  _tokenList[t].tokenClass = MODIFIER;
            }
      if (!syntaxOnly) {
            QStringList extl;
            if (tok1 == "5")
                  _xmlKind = "power";
            else if (tok1 == "6") {
                  if (take6)
                        _xmlKind += "-sixth";
                  else
                        extl << "6";
                  }
            else if (tok1 == "7") {
                  if (take7)
                        _xmlKind += "-seventh";
                  else
                        extl << "7";
                  }
            else if (tok1 == "9") {
                  if (take9)
                        _xmlKind += "-ninth";
                  else if (take7) {
                        _xmlKind += "-seventh";
                        extl << "9";
                        }
                  else
                        extl << "7" << "9";
                  }
            else if (tok1 == "11") {
                  if (take11)
                        _xmlKind += "-eleventh";
                  else if (take7) {
                        _xmlKind += "-seventh";
                        extl << "9" << "11";
                        }
                  else
                        extl << "7" << "9" << "11";
                  }
            else if (tok1 == "13") {
                  if (take13)
                        _xmlKind += "-thirteenth";
                  else if (take7) {
                        _xmlKind += "-seventh";
                        extl << "9" << "11" << "13";
                        }
                  else
                        extl << "7" << "9" << "11" << "13";
                  }
            else if (tok1 == "69") {
                  if (take6) {
                        _xmlKind += "-sixth";
                        extl << "9";
                        }
                  else
                        extl << "6" << "9";
            }
            foreach (QString e, extl) {
                  QString d = "add" + e;
                  _xmlDegrees += d;
                  }
            if (_xmlKind == "dominant-seventh")
                  _xmlKind = "dominant";
            _xmlText += tok1;
            }
      // eat trailing parens and commas
      while (i < len && trailing.contains(s[i]))
           addToken(QString(s[i++]),EXTENSION);

      // get modifiers
      bool addPending = false;
      _xmlParens = "no";
      _modifierList.clear();
      while (i < len) {
            // eat leading parens
            while (i < len && leading.contains(s[i])) {
                  addToken(QString(s[i++]),MODIFIER);
                  _xmlParens = "yes";
                  }
            // get first token - up to first digit, paren, or comma
            for (j = 0, tok1 = "", tok1L = "", initial = ""; i < len; ++i, ++j) {
                  if (s[i].isDigit() || special.contains(s[i]))
                        break;
                  tok1[j] = s[i];
                  tok1L[j] = s[i].toLower();
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
                  addToken(tok1,MODIFIER);
                  addPending = true;
                  continue;
                  }
            // eat spaces
            while (i < len && s[i] == ' ')
                  ++i;
            // get second token - up to first non-digit
            for (j = 0, tok2 = ""; i < len; ++i) {
                  if (!s[i].isDigit())
                        break;
                  tok2[j++] = s[i];
                  }
            tok2L = tok2.toLower();
            // standardize spelling
            if (tok1 == "M" || major.contains(tok1L))
                  tok1L = "major";
            else if (tok1L == "omit")
                  tok1L = "no";
            else if (tok1L == "sus" && tok2L == "")
                  tok2L = "4";
            else if (augmented.contains(tok1L) && tok2L == "") {
                  _xmlDegrees += "alt#5";
                  tok1L = "";
            }
            QString m = tok1L + tok2L;
            if (m != "")
                  _modifierList += m;
            if (tok1 != "")
                  addToken(tok1,MODIFIER);
            if (tok2 != "")
                  addToken(tok2,MODIFIER);
            if (!syntaxOnly) {
                  QString degree;
                  bool alter = false;
                  if (tok1L == "add")
                        degree = "add" + tok2L;
                  else if (tok1L == "no")
                        degree = "sub" + tok2L;
                  else if (tok1L == "sus") {
                        QString seventh;
                        if (tok2L == "4")
                              _xmlKind = "suspended-fourth";
                        else if (tok2L == "2")
                              _xmlKind = "suspended-second";
                        if (_extension == "7" || _extension == "9" || _extension == "11" || _extension == "13")
                              degree = (_quality == "major") ? "add#7" : "add7";
                        else if (_extension != "")
                              degree = "add" + _extension;
                        if (_extension == "13") {
                              _xmlDegrees += "add9";
                              _xmlDegrees += "add11";
                              _xmlDegrees += "add13";
                        }
                        if (_extension == "11") {
                              _xmlDegrees += "add9";
                              _xmlDegrees += "add11";
                              }
                        if (_extension == "9")
                              _xmlDegrees += "add9";
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
                              }
                        }
                  else if (tok1L == "alt") {
                        _xmlDegrees += "altb5";
                        _xmlDegrees += "add#5";
                        _xmlDegrees += "addb9";
                        _xmlDegrees += "add#9";
                        }
                  else if (tok1L == "blues") {
                        // this isn't really well-defined, but it might as well mean something
                        if (_extension == "11" || _extension == "13")
                              _xmlDegrees += "alt#9";
                        else
                              _xmlDegrees += "add#9";
                        }
                  else if (tok1L == "lyd") {
                        if (_extension == "13")
                              _xmlDegrees += "alt#11";
                        else
                              _xmlDegrees += "add#11";
                        }
                  else if (tok1L == "phryg") {
                        if (!_xmlKind.startsWith("minor"))
                              _xmlKind = "minor-seventh";
                        if (_extension == "11" || _extension == "13")
                              _xmlDegrees += "altb9";
                        else
                              _xmlDegrees += "addb9";
                        }
                  else if (tok1L == "tristan") {
                        _xmlKind = "Tristan";
                        }
                  else if (addPending)
                        degree = "add" + tok1L + tok2L;
                  else if (tok1L == "")
                        degree = "add" + tok2L;
                  else if (lower.contains(tok1L)) {
                        tok1L = "b";
                        alter = true;
                        }
                  else if (raise.contains(tok1L)) {
                        tok1L = "#";
                        alter = true;
                        }
                  else
                        _understandable = false;
                  if (alter) {
                        if (tok2 == "5")
                              degree = "alt";
                        else if (tok2 == "9" && (_extension == "11" || _extension == "13"))
                              degree = "alt";
                        else if (tok2 == "11" && _extension == "13")
                              degree = "alt";
                        else
                              degree = "add";
                        degree += tok1L + tok2L;
                        }
                  if (degree != "")
                        _xmlDegrees += degree;
                  }
            // eat trailing parens and commas
            while (i < len && trailing.contains(s[i]))
                  addToken(QString(s[i++]),MODIFIER);
            addPending = false;
            }
      if (!_modifierList.isEmpty()) {
            _modifierList.sort();
            _modifiers = "<" + _modifierList.join("><") + ">";
            }

      _handle = "<" + _quality + "><" + _extension + ">" + _modifiers;
      qDebug("parse: source = <%s>, handle = %s",qPrintable(s),qPrintable(_handle));
      qDebug("parse: xmlKind = <%s>, text = <%s>",qPrintable(_xmlKind),qPrintable(_xmlText));
      qDebug("parse: xmlSymbols = %s, xmlParens = %s",qPrintable(_xmlSymbols),qPrintable(_xmlParens));
      qDebug("parse: xmlDegrees = <%s>",qPrintable(_xmlDegrees.join(",")));
      return _parseable;
      }

//---------------------------------------------------------
//   renderList
//---------------------------------------------------------

const QList<RenderAction>& ParsedChord::renderList(const ChordList* cl)
      {
      if (_renderList.isEmpty()) {
            foreach (ChordToken tok, _tokenList) {
                  QString n = tok.names.first();
                  QList<RenderAction> rl;
                  QList<ChordToken> definedTokens;
                  bool found = false;
                  // potential definitions for token
                  foreach (ChordToken ct, cl->chordTokenList) {
                        foreach (QString ctn, ct.names) {
                              if (ctn == n)
                                    definedTokens += ct;
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

ChordDescription::ChordDescription(int i, ChordList* cl)
      {
      if (!i)
            i = --(cl->privateID);
      id = i;
      generated = false;
      renderListGenerated = false;
      }

//---------------------------------------------------------
//   ChordDescription
//    this form is used when generating from name
//    a private id is always assigned
//---------------------------------------------------------

ChordDescription::ChordDescription(const QString& name, ChordList* cl)
      {
      id = --(cl->privateID);
      generated = true;
      names.append(name);
      renderListGenerated = false;
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
            pc->parse(names.front(),cl);
            }
      parsedChords.append(*pc);
      if (renderList.isEmpty() || renderListGenerated) {
            renderList = pc->renderList(cl);
            renderListGenerated = true;
            }
      if (xmlKind == "") {
            xmlKind = pc->xmlKind();
            xmlText = pc->xmlText();
            xmlSymbols = pc->xmlSymbols();
            xmlParens = pc->xmlParens();
            xmlDegrees = pc->xmlDegrees();
            }
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

void ChordDescription::write(Xml& xml)
      {
      if (generated)
            return;
      if (id > 0)
            xml.stag(QString("chord id=\"%1\"").arg(id));
      else
            xml.stag(QString("chord"));
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
//   ChordList
//---------------------------------------------------------

int ChordList::privateID = -1000;

ChordList::ChordList()
      {
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
                  ChordDescription* cd = 0;
                  if (id)
                        cd = take(id);
                  if (!cd)
                        cd = new ChordDescription(id,this);
                  // record updated id
                  id = cd->id;
                  // read rest of description
                  cd->read(e);
                  // restore updated id
                  cd->id = id;
                  // throw away previously parsed chords
                  cd->parsedChords.clear();
                  // generate any missing info (including new parsed chords)
                  cd->complete(0,this);
                  // add to list
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

