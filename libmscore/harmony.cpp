//=============================================================================
//  MuseScore
//  Music Composition & Notation
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
#include "staff.h"
#include "part.h"
#include "utils.h"
#include "sym.h"
#include "xml.h"

namespace Ms {

//---------------------------------------------------------
//   harmonyName
//---------------------------------------------------------

QString Harmony::harmonyName() const
      {
      // Hack:
      const_cast<Harmony*>(this)->determineRootBaseSpelling();

      HChord hc = descr() ? descr()->chord : HChord();
      QString s, r, e, b;

      if (_leftParen)
            s = "(";

      if (_rootTpc != Tpc::TPC_INVALID)
            r = tpc2name(_rootTpc, _rootSpelling, _rootCase);

      if (_textName != "") {
            e = _textName;
            e.remove('=');
            }
      else if (!_degreeList.empty()) {
            hc.add(_degreeList);
            // try to find the chord in chordList
            const ChordDescription* newExtension = 0;
            const ChordList* cl = score()->style().chordList();
            for (const ChordDescription& cd : *cl) {
                  if (cd.chord == hc && !cd.names.empty()) {
                        newExtension = &cd;
                        break;
                        }
                  }
            // now determine the chord name
            if (newExtension)
                  e = newExtension->names.front();
            else {
                  // not in table, fallback to using HChord.name()
                  r = hc.name(_rootTpc);
                  e = "";
                  }
            }

      if (_baseTpc != Tpc::TPC_INVALID)
            b = "/" + tpc2name(_baseTpc, _baseSpelling, _baseCase);

      s += r + e + b;

      if (_rightParen)
            s += ")";

      return s;
      }

//---------------------------------------------------------
//   rootName
//---------------------------------------------------------

QString Harmony::rootName()
      {
      determineRootBaseSpelling();
      return tpc2name(_rootTpc, _rootSpelling, _rootCase);
      }

//---------------------------------------------------------
//   baseName
//---------------------------------------------------------

QString Harmony::baseName()
      {
      determineRootBaseSpelling();
      return tpc2name(_baseTpc, _baseSpelling, _baseCase);
      }

//---------------------------------------------------------
//   resolveDegreeList
//    try to detect chord number and to eliminate degree
//    list
//---------------------------------------------------------

void Harmony::resolveDegreeList()
      {
      if (_degreeList.empty())
            return;

      HChord hc = descr() ? descr()->chord : HChord();

      hc.add(_degreeList);

// qDebug("resolveDegreeList: <%s> <%s-%s>: ", _descr->name, _descr->xmlKind, _descr->xmlDegrees);
// hc.print();
// _descr->chord.print();

      // try to find the chord in chordList
      const ChordList* cl = score()->style().chordList();
      foreach(const ChordDescription& cd, *cl) {
            if ((cd.chord == hc) && !cd.names.empty()) {
qDebug("ResolveDegreeList: found in table as %s", qPrintable(cd.names.front()));
                  _id = cd.id;
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
   : TextBase(s)
      {
      initSubStyle(SubStyleId::HARMONY);
      _rootTpc    = Tpc::TPC_INVALID;
      _baseTpc    = Tpc::TPC_INVALID;
      _rootCase   = NoteCaseType::CAPITAL;
      _baseCase   = NoteCaseType::CAPITAL;
      _id         = -1;
      _parsedForm = 0;
      _leftParen  = false;
      _rightParen = false;
      setFlags(ElementFlag::MOVABLE | ElementFlag::SELECTABLE | ElementFlag::ON_STAFF);
      }

Harmony::Harmony(const Harmony& h)
   : TextBase(h)
      {
      _rootTpc    = h._rootTpc;
      _baseTpc    = h._baseTpc;
      _rootCase   = h._rootCase;
      _baseCase   = h._baseCase;
      _id         = h._id;
      _leftParen  = h._leftParen;
      _rightParen = h._rightParen;
      _degreeList = h._degreeList;
      _parsedForm = h._parsedForm ? new ParsedChord(*h._parsedForm) : 0;
      _textName   = h._textName;
      _userName   = h._userName;
      foreach(const TextSegment* s, h.textList) {
            TextSegment* ns = new TextSegment();
            ns->set(s->text, s->font, s->x, s->y);
            textList.append(ns);
            }
      }

//---------------------------------------------------------
//   ~Harmony
//---------------------------------------------------------

Harmony::~Harmony()
      {
      foreach(const TextSegment* ts, textList)
            delete ts;
      if (_parsedForm)
            delete _parsedForm;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Harmony::write(XmlWriter& xml) const
      {
      if (!xml.canWrite(this))
            return;
      xml.stag("Harmony");
      if (_leftParen)
            xml.tagE("leftParen");
      if (_rootTpc != Tpc::TPC_INVALID || _baseTpc != Tpc::TPC_INVALID) {
            int rRootTpc = _rootTpc;
            int rBaseTpc = _baseTpc;
            if (staff()) {
                  Segment* segment = toSegment(parent());
                  int tick = segment ? segment->tick() : -1;
                  const Interval& interval = part()->instrument(tick)->transpose();
                  if (xml.clipboardmode() && !score()->styleB(Sid::concertPitch) && interval.chromatic) {
                        rRootTpc = transposeTpc(_rootTpc, interval, true);
                        rBaseTpc = transposeTpc(_baseTpc, interval, true);
                        }
                  }
            if (rRootTpc != Tpc::TPC_INVALID) {
                  xml.tag("root", rRootTpc);
                  if (_rootCase != NoteCaseType::CAPITAL)
                        xml.tag("rootCase", static_cast<int>(_rootCase));
                  }
            if (_id > 0)
                  xml.tag("extension", _id);
            // parser uses leading "=" as a hidden specifier for minor
            // this may or may not currently be incorporated into _textName
            QString writeName = _textName;
            if (_parsedForm && _parsedForm->name().startsWith("=") && !writeName.startsWith("="))
                  writeName = "=" + writeName;
            if (!writeName.isEmpty())
                  xml.tag("name", writeName);

            if (rBaseTpc != Tpc::TPC_INVALID) {
                  xml.tag("base", rBaseTpc);
                  if (_baseCase != NoteCaseType::CAPITAL)
                        xml.tag("baseCase", static_cast<int>(_baseCase));
                  }
            for (const HDegree& hd : _degreeList) {
                  HDegreeType tp = hd.type();
                  if (tp == HDegreeType::ADD || tp == HDegreeType::ALTER || tp == HDegreeType::SUBTRACT) {
                        xml.stag("degree");
                        xml.tag("degree-value", hd.value());
                        xml.tag("degree-alter", hd.alter());
                        switch (tp) {
                              case HDegreeType::ADD:
                                    xml.tag("degree-type", "add");
                                    break;
                              case HDegreeType::ALTER:
                                    xml.tag("degree-type", "alter");
                                    break;
                              case HDegreeType::SUBTRACT:
                                    xml.tag("degree-type", "subtract");
                                    break;
                              default:
                                    break;
                              }
                        xml.etag();
                        }
                  }
            }
      else
            xml.tag("name", _textName);
      TextBase::writeProperties(xml, false, true);
      if (_rightParen)
            xml.tagE("rightParen");
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
            else if (tag == "baseCase")
                  _baseCase = static_cast<NoteCaseType>(e.readInt());
            else if (tag == "extension")
                  setId(e.readInt());
            else if (tag == "name")
                  _textName = e.readElementText();
            else if (tag == "root") {
                  if (score()->mscVersion() >= 106)
                        setRootTpc(e.readInt());
                  else
                        setRootTpc(table[e.readInt()-1]);    // obsolete
                  }
            else if (tag == "rootCase")
                  _rootCase = static_cast<NoteCaseType>(e.readInt());
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
                              addDegree(HDegree(degreeValue, degreeAlter, HDegreeType::ADD));
                        else if (degreeType == "alter")
                              addDegree(HDegree(degreeValue, degreeAlter, HDegreeType::ALTER));
                        else if (degreeType == "subtract")
                              addDegree(HDegree(degreeValue, degreeAlter, HDegreeType::SUBTRACT));
                        }
                  }
            else if (tag == "leftParen") {
                  _leftParen = true;
                  e.readNext();
                  }
            else if (tag == "rightParen") {
                  _rightParen = true;
                  e.readNext();
                  }
            else if (!TextBase::readProperties(e))
                  e.unknown();
            }

      // TODO: now that we can render arbitrary chords,
      // we could try to construct a full representation from a degree list.
      // These will typically only exist for chords imported from MusicXML prior to MuseScore 2.0
      // or constructed in the Chord Symbol Properties dialog.

      if (_rootTpc != Tpc::TPC_INVALID) {
            if (_id > 0) {
                  // positive id will happen only for scores that were created with explicit chord lists
                  // lookup id in chord list and generate new description if necessary
                  getDescription();
                  }
            else
                  {
                  // default case: look up by name
                  // description will be found for any chord already read in this score
                  // and we will generate a new one if necessary
                  getDescription(_textName);
                  }
            }
      else if (_textName == "") {
            // unrecognized chords prior to 2.0 were stored as text with markup
            // we need to strip away the markup
            // this removes any user-applied formatting,
            // but we no longer support user-applied formatting for chord symbols anyhow
            // with any luck, the resulting text will be parseable now, so give it a shot
            createLayout();
            QString s = plainText();
            if (!s.isEmpty()) {
                  setHarmony(s);
                  return;
                  }
            // empty text could also indicate a root-less slash chord ("/E")
            // we'll fall through and render it normally
            }

      // render chord from description (or _textName)
      render();
      }

//---------------------------------------------------------
//   determineRootBaseSpelling
//---------------------------------------------------------

void Harmony::determineRootBaseSpelling(NoteSpellingType& rootSpelling, NoteCaseType& rootCase,
   NoteSpellingType& baseSpelling, NoteCaseType& baseCase)
      {
      // spelling
      if (score()->styleB(Sid::useStandardNoteNames))
            rootSpelling = NoteSpellingType::STANDARD;
      else if (score()->styleB(Sid::useGermanNoteNames))
            rootSpelling = NoteSpellingType::GERMAN;
      else if (score()->styleB(Sid::useFullGermanNoteNames))
            rootSpelling = NoteSpellingType::GERMAN_PURE;
      else if (score()->styleB(Sid::useSolfeggioNoteNames))
            rootSpelling = NoteSpellingType::SOLFEGGIO;
      else if (score()->styleB(Sid::useFrenchNoteNames))
            rootSpelling = NoteSpellingType::FRENCH;
      baseSpelling = rootSpelling;

      // case

      // always use case as typed if automatic capitalization is off
      if (!score()->styleB(Sid::automaticCapitalization)) {
            rootCase = _rootCase;
            baseCase = _baseCase;
            return;
            }

      // set default
      if (score()->styleB(Sid::allCapsNoteNames)) {
            rootCase = NoteCaseType::UPPER;
            baseCase = NoteCaseType::UPPER;
            }
      else {
            rootCase = NoteCaseType::CAPITAL;
            baseCase = NoteCaseType::CAPITAL;
            }

      // override for bass note
      if (score()->styleB(Sid::lowerCaseBassNotes))
            baseCase = NoteCaseType::LOWER;

      // override for minor chords
      if (score()->styleB(Sid::lowerCaseMinorChords)) {
            const ChordDescription* cd = descr();
            QString quality;
            if (cd) {
                  // use chord description if possible
                  // this is the usual case
                  quality = cd->quality();
                  }
            else if (_parsedForm) {
                  // this happens on load of new chord list
                  // for chord symbols that were added/edited since the score was loaded
                  // or read aloud with screenreader
                  // parsed form is usable even if out of date with respect to chord list
                  quality = _parsedForm->quality();
                  }
            else {
                  // this happens on load of new chord list
                  // for chord symbols that have not been edited since the score was loaded
                  // we need to parse this chord for now to determine quality
                  // but don't keep the parsed form around as we're not ready for it yet
                  quality = parsedForm()->quality();
                  delete _parsedForm;
                  _parsedForm = 0;
                  }
            if (quality == "minor" || quality == "diminished" || quality == "half-diminished")
                  rootCase = NoteCaseType::LOWER;
            }
      }

//---------------------------------------------------------
//   determineRootBaseSpelling
//---------------------------------------------------------

void Harmony::determineRootBaseSpelling()
      {
      determineRootBaseSpelling(_rootSpelling, _rootRenderCase,
        _baseSpelling, _baseRenderCase);
      }

//---------------------------------------------------------
//   convertNote
//    convert something like "C#" into tpc 21
//---------------------------------------------------------

static int convertNote(const QString& s, NoteSpellingType noteSpelling, NoteCaseType& noteCase, int& idx)
      {
      bool useGerman = false;
      bool useSolfeggio = false;
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
      if (s == "")
            return Tpc::TPC_INVALID;
      noteCase = s[0].isLower() ? NoteCaseType::LOWER : NoteCaseType::CAPITAL;
      int acci;
      switch (noteSpelling) {
            case NoteSpellingType::SOLFEGGIO:
            case NoteSpellingType::FRENCH:
                  useSolfeggio = true;
                  if (s.toLower().startsWith("sol"))
                        acci = 3;
                  else
                        acci = 2;
                  break;
            case NoteSpellingType::GERMAN:
            case NoteSpellingType::GERMAN_PURE:
                  useGerman = true;
                  // fall through
            default:
                  acci = 1;
            }
      idx = acci;
      int alter = 0;
      int n = s.size();
      QString acc = s.right(n-acci);
      if (acc != "") {
            if (acc.startsWith("bb")) {
                  alter = -2;
                  idx += 2;
                  }
            else if (acc.startsWith("b")) {
                  alter = -1;
                  idx += 1;
                  }
            else if (useGerman && acc.startsWith("eses")) {
                  alter = -2;
                  idx += 4;
                  }
            else if (useGerman && (acc.startsWith("ses") || acc.startsWith("sas"))) {
                  alter = -2;
                  idx += 3;
                  }
            else if (useGerman && acc.startsWith("es")) {
                  alter = -1;
                  idx += 2;
                  }
            else if (useGerman && acc.startsWith("s") && !acc.startsWith("su")) {
                  alter = -1;
                  idx += 1;
                  }
            else if (acc.startsWith("##")) {
                  alter = 2;
                  idx += 2;
                  }
            else if (acc.startsWith("x")) {
                  alter = 2;
                  idx += 1;
                  }
            else if (acc.startsWith("#")) {
                  alter = 1;
                  idx += 1;
                  }
            else if (useGerman && acc.startsWith("isis")) {
                  alter = 2;
                  idx += 4;
                  }
            else if (useGerman && acc.startsWith("is")) {
                  alter = 1;
                  idx += 2;
                  }
            }
      int r;
      if (useGerman) {
            switch(s[0].toLower().toLatin1()) {
                  case 'c':   r = 0; break;
                  case 'd':   r = 1; break;
                  case 'e':   r = 2; break;
                  case 'f':   r = 3; break;
                  case 'g':   r = 4; break;
                  case 'a':   r = 5; break;
                  case 'h':   r = 6; break;
                  case 'b':
                        if (alter && alter != -1)
                              return Tpc::TPC_INVALID;
                        r = 6;
                        alter = -1;
                        break;
                  default:
                        return Tpc::TPC_INVALID;
                  }
            }
      else if (useSolfeggio) {
            if (s.length() < 2)
                  return Tpc::TPC_INVALID;
            if (s[1].isUpper())
                  noteCase = NoteCaseType::UPPER;
            QString ss = s.toLower().left(2);
            if (ss == "do")
                  r = 0;
            else if (ss == "re" || ss == "ré")
                  r = 1;
            else if (ss == "mi")
                  r = 2;
            else if (ss == "fa")
                  r = 3;
            else if (ss == "so")    // sol, but only check first 2 characters
                  r = 4;
            else if (ss == "la")
                  r = 5;
            else if (ss == "si")
                  r = 6;
            else
                  return Tpc::TPC_INVALID;
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
                  default:    return Tpc::TPC_INVALID;
                  }
            }
      r = spellings[r * 5 + alter + 2];
      return r;
      }

//---------------------------------------------------------
//   parseHarmony
//    determine root and bass tpc & case
//    compare body of chordname against chord list
//    return true if chord is recognized
//---------------------------------------------------------

const ChordDescription* Harmony::parseHarmony(const QString& ss, int* root, int* base, bool syntaxOnly)
      {
      _id = -1;
      if (_parsedForm) {
            delete _parsedForm;
            _parsedForm = 0;
            }
      _textName.clear();
      bool useLiteral = false;
      if (ss.endsWith(' '))
            useLiteral = true;
      QString s = ss.simplified();

      if ((_leftParen = s.startsWith('(')))
            s.remove(0,1);

      if ((_rightParen = (s.endsWith(')') && s.count('(') < s.count(')'))))
            s.remove(s.size()-1,1);

      if (_leftParen || _rightParen)
            s = s.simplified();     // in case of spaces inside parentheses

      int n = s.size();
      if (n < 1)
            return 0;
      determineRootBaseSpelling();
      int idx;
      int r = convertNote(s, _rootSpelling, _rootCase, idx);
      if (r == Tpc::TPC_INVALID) {
            if (s[0] == '/')
                  idx = 0;
            else {
                  qDebug("failed <%s>", qPrintable(ss));
                  _userName = s;
                  _textName = s;
                  return 0;
                  }
            }
      *root = r;
      bool preferMinor;
      if (score()->styleB(Sid::lowerCaseMinorChords) && s[0].isLower())
            preferMinor = true;
      else
            preferMinor = false;
      *base = Tpc::TPC_INVALID;
      int slash = s.lastIndexOf('/');
      if (slash != -1) {
            QString bs = s.mid(slash + 1).simplified();
            s = s.mid(idx, slash - idx).simplified();
            int idx2;
            *base = convertNote(bs, _baseSpelling, _baseCase, idx2);
            if (idx2 != bs.size())
                  *base = Tpc::TPC_INVALID;
            if (*base == Tpc::TPC_INVALID) {
                  // if what follows after slash is not (just) a TPC
                  // then reassemble chord and try to parse with the slash
                  s = s + "/" + bs;
                  }
            }
      else
            s = s.mid(idx);   // don't simplify; keep leading space before extension if present
      _userName = s;
      const ChordList* cl = score()->style().chordList();
      const ChordDescription* cd = 0;
      if (useLiteral)
            cd = descr(s);
      else {
            _parsedForm = new ParsedChord();
            _parsedForm->parse(s, cl, syntaxOnly, preferMinor);
            // parser prepends "=" to name of implied minor chords
            // use this here as well
            if (preferMinor)
                  s = _parsedForm->name();
            // look up to see if we already have a descriptor (chord has been used before)
            cd = descr(s, _parsedForm);
            }
      if (cd) {
            // descriptor found; use its information
            _id = cd->id;
            if (!cd->names.empty())
                  _textName = cd->names.front();
            }
      else {
            // no descriptor yet; just set textname
            // we will generate descriptor later if necessary (when we are done editing this chord)
            _textName = s;
            }
      return cd;
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void Harmony::startEdit(EditData& ed)
      {
      if (!textList.empty())
            setXmlText(harmonyName());
      TextBase::startEdit(ed);
      layout();
      }

//---------------------------------------------------------
//   edit
//---------------------------------------------------------

bool Harmony::edit(EditData& ed)
      {
      if (ed.key == Qt::Key_Return)
            return true; // Harmony only single line
      bool rv = TextBase::edit(ed);
      setHarmony(plainText());
      int root, base;
      QString str = xmlText();
      bool badSpell = !str.isEmpty() && !parseHarmony(str, &root, &base, true);
      spellCheckUnderline(badSpell);
      return rv;
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Harmony::endEdit(EditData& ed)
      {
      TextBase::endEdit(ed);
      layout();
      if (links()) {
            for (ScoreElement* e : *links()) {
                  if (e == this)
                        continue;
                  Harmony* h = toHarmony(e);
                  // transpose if necessary
                  // at this point chord will already have been rendered in same key as original
                  // (as a result of TextBase::endEdit() calling setText() for linked elements)
                  // we may now need to change the TPC's and the text, and re-render
                  if (score()->styleB(Sid::concertPitch) != h->score()->styleB(Sid::concertPitch)) {
                        Part* partDest = h->part();
                        Segment* segment = toSegment(parent());
                        int tick = segment ? segment->tick() : -1;
                        Interval interval = partDest->instrument(tick)->transpose();
                        if (!interval.isZero()) {
                              if (!h->score()->styleB(Sid::concertPitch))
                                    interval.flip();
                              int rootTpc = transposeTpc(h->rootTpc(), interval, true);
                              int baseTpc = transposeTpc(h->baseTpc(), interval, true);
                              //score()->undoTransposeHarmony(h, rootTpc, baseTpc);
                              h->setRootTpc(rootTpc);
                              h->setBaseTpc(baseTpc);
                              h->setXmlText(h->harmonyName());
                              h->setHarmony(h->plainText());
                              }
                        }
                  }
            }
      triggerLayout();
      }

//---------------------------------------------------------
//   setHarmony
//---------------------------------------------------------

void Harmony::setHarmony(const QString& s)
      {
      int r, b;
      const ChordDescription* cd = parseHarmony(s, &r, &b);
      if (!cd && _parsedForm && _parsedForm->parseable()) {
            // our first time encountering this chord
            // generate a descriptor and use it
            cd = generateDescription();
            _id = cd->id;
            }
      if (cd) {
            setRootTpc(r);
            setBaseTpc(b);
            render();
            }
      else {
            // unparseable chord, render as plain text
            foreach (const TextSegment* s, textList)
                  delete s;
            textList.clear();
            setRootTpc(Tpc::TPC_INVALID);
            setBaseTpc(Tpc::TPC_INVALID);
            _id = -1;
            render();
            }
      }

//---------------------------------------------------------
//   baseLine
//---------------------------------------------------------

qreal Harmony::baseLine() const
      {
      return (textList.empty()) ? TextBase::baseLine() : 0.0;
      }

//---------------------------------------------------------
//   text
//---------------------------------------------------------

QString HDegree::text() const
      {
      if (_type == HDegreeType::UNDEF)
            return QString();
      const char* d = 0;
      switch(_type) {
            case HDegreeType::UNDEF: break;
            case HDegreeType::ADD:         d= "add"; break;
            case HDegreeType::ALTER:       d= "alt"; break;
            case HDegreeType::SUBTRACT:    d= "sub"; break;
            }
      QString degree(d);
      switch(_alter) {
            case -1:          degree += "b"; break;
            case 1:           degree += "#"; break;
            default:          break;
            }
      QString s = QString("%1").arg(_value);
      QString ss = degree + s;
      return ss;
      }

//---------------------------------------------------------
//   fromXml
//    lookup harmony in harmony data base
//    using musicXml "kind" string and degree list
//---------------------------------------------------------

const ChordDescription* Harmony::fromXml(const QString& kind, const QList<HDegree>& dl)
      {
      QStringList degrees;

      foreach(const HDegree& d, dl)
            degrees.append(d.text());

      QString lowerCaseKind = kind.toLower();
      const ChordList* cl = score()->style().chordList();
      foreach(const ChordDescription& cd, *cl) {
            QString k     = cd.xmlKind;
            QString lowerCaseK = k.toLower(); // required for xmlKind Tristan
            QStringList d = cd.xmlDegrees;
            if ((lowerCaseKind == lowerCaseK) && (d == degrees)) {
//                  qDebug("harmony found in db: %s %s -> %d", qPrintable(kind), qPrintable(degrees), cd->id);
                  return &cd;
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   fromXml
//    lookup harmony in harmony data base
//    using musicXml "kind" string only
//---------------------------------------------------------

const ChordDescription* Harmony::fromXml(const QString& kind)
      {
      QString lowerCaseKind = kind.toLower();
      const ChordList* cl = score()->style().chordList();
      foreach(const ChordDescription& cd, *cl) {
            if (lowerCaseKind == cd.xmlKind)
                  return &cd;
            }
      return 0;
      }

//---------------------------------------------------------
//   fromXml
//    construct harmony directly from XML
//    build name first
//    then generate chord description from that
//---------------------------------------------------------

const ChordDescription* Harmony::fromXml(const QString& kind, const QString& kindText, const QString& symbols, const QString& parens, const QList<HDegree>& dl)
      {
      ParsedChord* pc = new ParsedChord;
      _textName = pc->fromXml(kind, kindText, symbols, parens, dl, score()->style().chordList());
      _parsedForm = pc;
      const ChordDescription* cd = getDescription(_textName,pc);
      return cd;
      }

//---------------------------------------------------------
//   descr
//    look up id in chord list
//    return chord description if found, or null
//---------------------------------------------------------

const ChordDescription* Harmony::descr() const
      {
      return score()->style().chordDescription(_id);
      }

//---------------------------------------------------------
//   descr
//    look up name in chord list
//    optionally look up by parsed chord as fallback
//    return chord description if found, or null
//---------------------------------------------------------

const ChordDescription* Harmony::descr(const QString& name, const ParsedChord* pc) const
      {
      const ChordList* cl = score()->style().chordList();
      const ChordDescription* match = 0;
      if (cl) {
            foreach (const ChordDescription& cd, *cl) {
                  foreach (const QString& s, cd.names) {
                        if (s == name)
                              return &cd;
                        else if (pc) {
                              foreach (const ParsedChord& sParsed, cd.parsedChords) {
                                    if (sParsed == *pc)
                                          match = &cd;
                                    }
                              }
                        }
                  }
            }
      // exact match failed, so fall back on parsed match if one was found
      return match;
      }

//---------------------------------------------------------
//   getDescription
//    look up id in chord list
//    return chord description if found
//    if not found, and chord is parseable,
//    generate a new chord description
//    and add to chord list
//---------------------------------------------------------

const ChordDescription* Harmony::getDescription()
      {
      const ChordDescription* cd = descr();
      if (cd && !cd->names.empty())
            _textName = cd->names.front();
      else if (_textName != "") {
            cd = generateDescription();
            _id = cd->id;
            }
      return cd;
      }

//---------------------------------------------------------
//   getDescription
//    same but lookup by name and optionally parsed chord
//---------------------------------------------------------

const ChordDescription* Harmony::getDescription(const QString& name, const ParsedChord* pc)
      {
      const ChordDescription* cd = descr(name, pc);
      if (cd)
            _id = cd->id;
      else {
            cd = generateDescription();
            _id = cd->id;
            }
      return cd;
      }

//---------------------------------------------------------
//   generateDescription
//    generate new chord description from _textName
//    add to chord list using private id
//---------------------------------------------------------

const ChordDescription* Harmony::generateDescription()
      {
      ChordList* cl = score()->style().chordList();
      ChordDescription cd(_textName);
      cd.complete(_parsedForm, cl);
      // remove parsed chord from description
      // so we will only match it literally in the future
      cd.parsedChords.clear();
      return &*cl->insert(cd.id, cd);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Harmony::layout()
      {
      calculateBoundingRect();    // for normal symbols this is called in layout: computeMinWidth()

      if (!parent()) {
            setPos(0.0, 0.0);
            return;
            }

      qreal yy = 0.0;

      if (parent()->isSegment()) {
            Segment* s = toSegment(parent());
            // look for fret diagram
            bool fretsFound = false;
            for (Element* e : s->annotations()) {
                  if (e->isFretDiagram() && e->track() == track()) {
                        yy -= score()->styleP(Sid::fretY);
                        e->layout();
                        yy -= e->height();
                        yy -= score()->styleP(Sid::harmonyFretDist);
                        fretsFound = true;
                        break;
                        }
                  }
            if (!fretsFound)
                  yy -= score()->styleP(Sid::harmonyY);
            }
      else if (parent()->isFretDiagram()) {
            qDebug("Harmony %s with fret diagram as parent", qPrintable(_textName)); // not possible?
            yy = -score()->styleP(Sid::harmonyFretDist);
            }
      yy += offset().y();           //      yy += offset(_spatium).y();

      qreal hb = lineHeight() - TextBase::baseLine();
      if (align() & Align::BOTTOM)
            yy -= hb;
      else if (align() & Align::VCENTER) {
            yy -= hb;
            yy += (height() * .5);
            }
      else if (align() & Align::BASELINE) {
            }
      else { // Align::TOP
            yy -= hb;
            yy += height();
            }

      qreal xx = 0.0; // offset(_spatium).x();

      qreal cw = symWidth(SymId::noteheadBlack);
      if (align() & Align::RIGHT) {
            xx += cw;
            xx -= width();
            }
      else if (align() & Align::HCENTER) {
            xx += (cw * .5);
            xx -= (width() * .5);
            }

      setPos(xx, yy);

      if (!readPos().isNull()) {
            // version 114 is measure based
            // rebase to segment
            if (score()->mscVersion() == 114) {
                  setReadPos(readPos() - parent()->pos());
                  }
            setUserOff(readPos() - ipos());
            setReadPos(QPointF());
            }

      if (parent()->isFretDiagram() && parent()->parent()->isSegment()) {
            qDebug("Harmony %s with fret diagram as parent and segment as grandparent", qPrintable(_textName));
//            MStaff* mstaff = toSegment(parent()->parent())->measure()->mstaff(staffIdx());
//WS            qreal dist = -(bbox().top());
//            mstaff->distanceUp = qMax(mstaff->distanceUp, dist + _spatium);
            }

      if (hasFrame()) {
            QRectF saveBbox = bbox();
            setbbox(bboxtight());
            layoutFrame();
            setbbox(saveBbox);
            }
      }

//---------------------------------------------------------
//   calculateBoundingRect
//---------------------------------------------------------

void Harmony::calculateBoundingRect()
      {
      if (textList.empty()) {
            TextBase::layout1();
            setbboxtight(bbox());
            }
      else {
            // textStyle().layout(this);
            QRectF bb, tbb;
            foreach(const TextSegment* ts, textList) {
                  bb |= ts->boundingRect().translated(ts->x, ts->y);
                  tbb |= ts->tightBoundingRect().translated(ts->x, ts->y);
                  }
            setbbox(bb);
            setbboxtight(tbb);
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Harmony::draw(QPainter* painter) const
      {
      // painter->setPen(curColor());
      if (textList.empty()) {
            TextBase::draw(painter);
            return;
            }
      if (hasFrame()) {
            if (frameWidth().val() != 0.0) {
                  QColor color = frameColor();
                  QPen pen(color, frameWidth().val() * spatium(), Qt::SolidLine,
                     Qt::SquareCap, Qt::MiterJoin);
                  painter->setPen(pen);
                  }
            else
                  painter->setPen(Qt::NoPen);
            QColor bg(bgColor());
            painter->setBrush(bg.alpha() ? QBrush(bg) : Qt::NoBrush);
            if (circle())
                  painter->drawArc(frame, 0, 5760);
            else {
                  int r2 = frameRound();
                  if (r2 > 99)
                        r2 = 99;
                  painter->drawRoundedRect(frame, frameRound(), r2);
                  }
            }
      painter->setBrush(Qt::NoBrush);
      QColor color = textColor();
      painter->setPen(color);
      for (const TextSegment* ts : textList) {
            QFont f(ts->font);
            f.setPointSizeF(f.pointSizeF() * MScore::pixelRatio);
            painter->setFont(f);
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
      QFontMetricsF fm(font, MScore::paintDevice());
#if 1
      return fm.width(text);
#else
      qreal w = 0.0;
      foreach(QChar c, text) {
            // if we calculate width by character, at least skip high surrogates
            if (c.isHighSurrogate())
                  continue;
            w += fm.width(c);
            }
      return w;
#endif
      }

//---------------------------------------------------------
//   boundingRect
//---------------------------------------------------------

QRectF TextSegment::boundingRect() const
      {
      QFontMetricsF fm(font, MScore::paintDevice());
      return fm.boundingRect(text);
      }

//---------------------------------------------------------
//   tightBoundingRect
//---------------------------------------------------------

QRectF TextSegment::tightBoundingRect() const
      {
      QFontMetricsF fm(font, MScore::paintDevice());
      return fm.tightBoundingRect(text);
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

void Harmony::render(const QString& s, qreal& x, qreal& y)
      {
      int fontIdx = 0;
      if(!s.isEmpty()) {
            TextSegment* ts = new TextSegment(s, fontList[fontIdx], x, y);
            textList.append(ts);
            x += ts->width();
            }
      }

//---------------------------------------------------------
//   render
//---------------------------------------------------------

void Harmony::render(const QList<RenderAction>& renderList, qreal& x, qreal& y, int tpc, NoteSpellingType noteSpelling, NoteCaseType noteCase)
      {
      ChordList* chordList = score()->style().chordList();
      QStack<QPointF> stack;
      int fontIdx    = 0;
      qreal _spatium = spatium();
      qreal mag      = _spatium / SPATIUM20;

      for (const RenderAction& a : renderList) {
            if (a.type == RenderAction::RenderActionType::SET) {
                  TextSegment* ts = new TextSegment(fontList[fontIdx], x, y);
                  ChordSymbol cs = chordList->symbol(a.text);
                  if (cs.isValid()) {
                        ts->font = fontList[cs.fontIdx];
                        ts->setText(cs.value);
                        }
                  else
                        ts->setText(a.text);
                  textList.append(ts);
                  x += ts->width();
                  }
            else if (a.type == RenderAction::RenderActionType::MOVE) {
                  x += a.movex * mag;
                  y += a.movey * mag;
                  }
            else if (a.type == RenderAction::RenderActionType::PUSH)
                  stack.push(QPointF(x,y));
            else if (a.type == RenderAction::RenderActionType::POP) {
                  if (!stack.empty()) {
                        QPointF pt = stack.pop();
                        x = pt.x();
                        y = pt.y();
                        }
                  else
                        qDebug("RenderAction::RenderActionType::POP: stack empty");
                  }
            else if (a.type == RenderAction::RenderActionType::NOTE) {
                  QString c;
                  int acc;
                  tpc2name(tpc, noteSpelling, noteCase, c, acc);
                  TextSegment* ts = new TextSegment(fontList[fontIdx], x, y);
                  QString lookup = "note" + c;
                  ChordSymbol cs = chordList->symbol(lookup);
                  if (!cs.isValid())
                        cs = chordList->symbol(c);
                  if (cs.isValid()) {
                        ts->font = fontList[cs.fontIdx];
                        ts->setText(cs.value);
                        }
                  else
                        ts->setText(c);
                  textList.append(ts);
                  x += ts->width();
                  }
            else if (a.type == RenderAction::RenderActionType::ACCIDENTAL) {
                  QString c;
                  QString acc;
                  QString context = "accidental";
                  tpc2name(tpc, noteSpelling, noteCase, c, acc);
                  // German spelling - use special symbol for accidental in TPC_B_B
                  // to allow it to be rendered as either Bb or B
                  if (tpc == Tpc::TPC_B_B && noteSpelling == NoteSpellingType::GERMAN)
                        context = "german_B";
                  if (acc != "") {
                        TextSegment* ts = new TextSegment(fontList[fontIdx], x, y);
                        QString lookup = context + acc;
                        ChordSymbol cs = chordList->symbol(lookup);
                        if (!cs.isValid())
                              cs = chordList->symbol(acc);
                        if (cs.isValid()) {
                              ts->font = fontList[cs.fontIdx];
                              ts->setText(cs.value);
                              }
                        else
                              ts->setText(acc);
                        textList.append(ts);
                        x += ts->width();
                        }
                  }
            else
                  qDebug("Harmony::render(): unknown render action %d", static_cast<int>(a.type));
            }
      }

//---------------------------------------------------------
//   render
//    construct Chord Symbol
//---------------------------------------------------------

void Harmony::render()
      {
      int capo = score()->styleI(Sid::capoPosition);

      ChordList* chordList = score()->style().chordList();

      fontList.clear();
      for (const ChordFont& cf : chordList->fonts) {
            QFont ff(font());
            ff.setPointSizeF(ff.pointSizeF() * cf.mag);
            if (!(cf.family.isEmpty() || cf.family == "default"))
                  ff.setFamily(cf.family);
            fontList.append(ff);
            }
      if (fontList.empty())
            fontList.append(font());

      for (const TextSegment* s : textList)
            delete s;
      textList.clear();
      qreal x = 0.0, y = 0.0;

      determineRootBaseSpelling();

      if (_leftParen)
            render("( ", x, y);

      if (_rootTpc != Tpc::TPC_INVALID) {
            // render root
            render(chordList->renderListRoot, x, y, _rootTpc, _rootSpelling, _rootRenderCase);
            // render extension
            const ChordDescription* cd = getDescription();
            if (cd)
                  render(cd->renderList, x, y, 0);
            }
      else
            render(_textName, x, y);

      // render bass
      if (_baseTpc != Tpc::TPC_INVALID)
            render(chordList->renderListBase, x, y, _baseTpc, _baseSpelling, _baseRenderCase);

      if (_rootTpc != Tpc::TPC_INVALID && capo > 0 && capo < 12) {
            int tpcOffset[] = { 0, 5, -2, 3, -4, 1, 6, -1, 4, -3, 2, -5 };
            int capoRootTpc = _rootTpc + tpcOffset[capo];
            int capoBassTpc = _baseTpc;

            if (capoBassTpc != Tpc::TPC_INVALID)
                  capoBassTpc += tpcOffset[capo];

            /*
             * For guitarists, avoid x and bb in Root or Bass,
             * and also avoid E#, B#, Cb and Fb in Root.
             */
            if (capoRootTpc < 8 || (capoBassTpc != Tpc::TPC_INVALID && capoBassTpc < 6)) {
                  capoRootTpc += 12;
                  if (capoBassTpc != Tpc::TPC_INVALID)
                        capoBassTpc += 12;
                  }
            else if (capoRootTpc > 24 || (capoBassTpc != Tpc::TPC_INVALID && capoBassTpc > 26)) {
                  capoRootTpc -= 12;
                  if (capoBassTpc != Tpc::TPC_INVALID)
                        capoBassTpc -= 12;
                  }

            render("(", x, y);
            render(chordList->renderListRoot, x, y, capoRootTpc, _rootSpelling, _rootRenderCase);

            // render extension
            const ChordDescription* cd = getDescription();
            if (cd)
                  render(cd->renderList, x, y, 0);

            if (capoBassTpc != Tpc::TPC_INVALID)
                  render(chordList->renderListBase, x, y, capoBassTpc, _baseSpelling, _baseRenderCase);
            render(")", x, y);
            }

      if (_rightParen)
            render(" )", x, y);
      }

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void Harmony::spatiumChanged(qreal oldValue, qreal newValue)
      {
      TextBase::spatiumChanged(oldValue, newValue);
      render();
      }

//---------------------------------------------------------
//   localSpatiumChanged
//---------------------------------------------------------

void Harmony::localSpatiumChanged(qreal oldValue, qreal newValue)
      {
      TextBase::localSpatiumChanged(oldValue, newValue);
      render();
      }

//---------------------------------------------------------
//   extensionName
//---------------------------------------------------------

const QString& Harmony::extensionName() const
      {
      return _textName;
      }

//---------------------------------------------------------
//   xmlKind
//---------------------------------------------------------

QString Harmony::xmlKind() const
      {
      const ChordDescription* cd = descr();
      return cd ? cd->xmlKind : QString();
      }

//---------------------------------------------------------
//   musicXmlText
//---------------------------------------------------------

QString Harmony::musicXmlText() const
      {
      const ChordDescription* cd = descr();
      return cd ? cd->xmlText : QString();
      }

//---------------------------------------------------------
//   xmlSymbols
//---------------------------------------------------------

QString Harmony::xmlSymbols() const
      {
      const ChordDescription* cd = descr();
      return cd ? cd->xmlSymbols : QString();
      }

//---------------------------------------------------------
//   xmlParens
//---------------------------------------------------------

QString Harmony::xmlParens() const
      {
      const ChordDescription* cd = descr();
      return cd ? cd->xmlParens : QString();
      }

//---------------------------------------------------------
//   xmlDegrees
//---------------------------------------------------------

QStringList Harmony::xmlDegrees() const
      {
      const ChordDescription* cd = descr();
      return cd ? cd->xmlDegrees : QStringList();
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

//---------------------------------------------------------
//   parsedForm
//---------------------------------------------------------

const ParsedChord* Harmony::parsedForm()
      {
      if (!_parsedForm) {
            ChordList* cl = score()->style().chordList();
            _parsedForm = new ParsedChord();
            _parsedForm->parse(_textName, cl, false);
            }
      return _parsedForm;
      }

//---------------------------------------------------------
//   accessibleInfo
//---------------------------------------------------------

QString Harmony::accessibleInfo() const
      {
      return QString("%1: %2").arg(Element::accessibleInfo()).arg(harmonyName());
      }

//---------------------------------------------------------
//   screenReaderInfo
//---------------------------------------------------------

QString Harmony::screenReaderInfo() const
      {
      QString rez = Element::accessibleInfo();
      if (_rootTpc != Tpc::TPC_INVALID)
            rez = QString("%1 %2").arg(rez).arg(tpc2name(_rootTpc, NoteSpellingType::STANDARD, NoteCaseType::AUTO, true));

      if (const_cast<Harmony*>(this)->parsedForm() && !hTextName().isEmpty()) {
            QString aux = const_cast<Harmony*>(this)->parsedForm()->handle();
            aux = aux.replace("#", QObject::tr("sharp")).replace("<", "");
            QString extension = "";

            foreach (QString s, aux.split(">", QString::SkipEmptyParts)) {
                  if(!s.contains("blues"))
                        s.replace("b", QObject::tr("flat"));
                  extension += s + " ";
                  }
            rez = QString("%1 %2").arg(rez).arg(extension);
            }
      else {
            rez = QString("%1 %2").arg(rez).arg(hTextName());
            }

      if (_baseTpc != Tpc::TPC_INVALID)
            rez = QString("%1 / %2").arg(rez).arg(tpc2name(_baseTpc, NoteSpellingType::STANDARD, NoteCaseType::AUTO, true));

      return rez;
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Harmony::acceptDrop(EditData& data) const
      {
      return data.element->type() == ElementType::FRET_DIAGRAM;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Harmony::drop(EditData& data)
      {
      Element* e = data.element;
      if (e->type() == ElementType::FRET_DIAGRAM) {
            FretDiagram* fd = toFretDiagram(e);
            fd->setParent(parent());
            fd->setTrack(track());
            score()->undoAddElement(fd);
            }
      else {
            qWarning("Harmony: cannot drop <%s>\n", e->name());
            delete e;
            e = 0;
            }
      return e;
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Harmony::propertyDefault(Pid id) const
      {
      QVariant v;
      switch (id) {
            case Pid::SUB_STYLE:
                  v = int(SubStyleId::HARMONY);
                  break;
            default:
                  v = styledPropertyDefault(id);
                  if (!v.isValid()) {
                        for (const StyledProperty& p : defaultStyle) {
                              if (p.pid == id) {
                                    v = score()->styleV(p.sid);
                                    break;
                                    }
                              }
                        }
                  if (!v.isValid())
                        v = Element::propertyDefault(id);
                  break;
            }
      return v;
      }

}
