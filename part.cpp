//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: part.cpp 5335 2012-02-17 13:51:54Z lasconic $
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "part.h"
#include "staff.h"
#include "xml.h"
#include "score.h"
#include "style.h"
#include "note.h"
#include "drumset.h"
#include "instrtemplate.h"
#include "text.h"
#include "measure.h"
#include "tablature.h"
#include "stafftype.h"

//---------------------------------------------------------
//   Part
//---------------------------------------------------------

Part::Part(Score* s)
      {
      _score = s;
      _show  = true;
      setInstrument(Instrument(), 0);     // default instrument
      }

//---------------------------------------------------------
//   initFromInstrTemplate
//---------------------------------------------------------

void Part::initFromInstrTemplate(const InstrumentTemplate* t)
      {
      _partName = t->trackName;
      Instrument instr = Instrument::fromTemplate(t);
      setInstrument(instr, 0);
      }

//---------------------------------------------------------
//   staff
//---------------------------------------------------------

Staff* Part::staff(int idx) const
      {
      return _staves[idx];
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Part::read(const QDomElement& de)
      {
      int rstaff = 0;
      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            const QString& val(e.text());
            if (tag == "Staff") {
                  Staff* staff = new Staff(_score, this, rstaff);
                  _score->staves().push_back(staff);
                  _staves.push_back(staff);
                  staff->read(e);
                  ++rstaff;
                  }
            else if (tag == "Instrument") {
                  instr(0)->read(e);
                  }
            else if (tag == "name") {
                  if (_score->mscVersion() <= 101)
                        instr(0)->setLongName(QTextDocumentFragment::fromHtml(val));
                  else if (_score->mscVersion() <= 110)
                        instr(0)->setLongName(QTextDocumentFragment::fromHtml(Xml::htmlToString(e.firstChildElement())));
                  else if (_score->mscVersion() < 120) {
                        Text* t = new Text(score());
                        t->read(e);
                        instr(0)->setLongName(t->getFragment());
                        delete t;
                        }
                  else {
                        instr(0)->setLongName(QTextDocumentFragment::fromHtml(val));
                        }
                  }
            else if (tag == "shortName") {
                  if (_score->mscVersion() <= 101)
                        instr(0)->setShortName(QTextDocumentFragment::fromHtml(val));
                  else if (_score->mscVersion() <= 110)
                        instr(0)->setShortName(QTextDocumentFragment::fromHtml(Xml::htmlToString(e.firstChildElement())));
                  else if (_score->mscVersion() < 120) {
                        Text* t = new Text(score());
                        t->read(e);
                        instr(0)->setShortName(t->getFragment());
                        delete t;
                        }
                  else {
                        instr(0)->setShortName(QTextDocumentFragment::fromHtml(val));
                        }
                  }
            else if (tag == "trackName")
                  _partName = val;
            else if (tag == "show")
                  _show = val.toInt();
            else
                  domError(e);
            }
      if (_partName.isEmpty())
            _partName = instr(0)->trackName();
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Part::write(Xml& xml) const
      {
      xml.stag("Part");
      foreach(const Staff* staff, _staves)
            staff->write(xml);
      if (!_show)
            xml.tag("show", _show);
      xml.tag("trackName", _partName);
      instr(0)->write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   parseInstrName
//---------------------------------------------------------

#if 0
static QTextDocumentFragment parseInstrName(const QString& name)
      {
      if (name.isEmpty())
            return QTextDocumentFragment();
      QTextDocument doc;
      QTextCursor cursor(&doc);
      QTextCharFormat f = cursor.charFormat();
      QTextCharFormat sf(f);

      QFont font("MScore1-test");
      sf.setFont(font);

      QDomDocument dom;
      int line, column;
      QString err;
      if (!dom.setContent(name, false, &err, &line, &column)) {
            QString col, ln;
            col.setNum(column);
            ln.setNum(line);
            QString error = err + "\n at line " + ln + " column " + col;
            qDebug("parse instrument name: %s\n", qPrintable(error));
            qDebug("   data:<%s>\n", qPrintable(name));
            return QTextDocumentFragment();
            }

      for (QDomNode e = dom.documentElement(); !e.isNull(); e = e.nextSibling()) {
            for (QDomNode ee = e.firstChild(); !ee.isNull(); ee = ee.nextSibling()) {
                  QDomElement de1 = ee.toElement();
                  QString tag(de1.tagName());
                  if (tag == "symbol") {
                        QString name = de1.attribute(QString("name"));
                        if (name == "flat")
                              cursor.insertText(QString(0xe10d), sf);
                        else if (name == "sharp")
                              cursor.insertText(QString(0xe10c), sf);
                        }
                  QDomText t = ee.toText();
                  if (!t.isNull())
                        cursor.insertText(t.data(), f);
                  }
            }
      return QTextDocumentFragment(&doc);
      }
#endif

//---------------------------------------------------------
//   setLongNames
//---------------------------------------------------------

void Part::setLongNames(QList<StaffNameDoc>& name, int tick)
      {
      instr(tick)->longNames() = name;
      }

void Part::setShortNames(QList<StaffNameDoc>& name, int tick)
      {
      instr(tick)->shortNames() = name;
      }

//---------------------------------------------------------
//   setStaves
//---------------------------------------------------------

void Part::setStaves(int n)
      {
      int ns = _staves.size();
      if (n < ns) {
            qDebug("Part::setStaves(): remove staves not implemented!\n");
            return;
            }
      int staffIdx = _score->staffIdx(this) + ns;
      for (int i = ns; i < n; ++i) {
            Staff* staff = new Staff(_score, this, i);
            _staves.push_back(staff);
            _score->staves().insert(staffIdx, staff);
            for (MeasureBase* mb = _score->first(); mb; mb = mb->next()) {
                  if (mb->type() != MEASURE)
                        continue;
                  Measure* m = static_cast<Measure*>(mb);
                  m->insertStaff(staff, staffIdx);
                  }
            ++staffIdx;
            }
      }

//---------------------------------------------------------
//   insertStaff
//---------------------------------------------------------

void Part::insertStaff(Staff* staff)
      {
      int idx = staff->rstaff();
      if (idx > _staves.size())
            idx = _staves.size();
      _staves.insert(idx, staff);
      staff->setShow(_show);
      staff->setPart(this);
      idx = 0;
      foreach(Staff* staff, _staves)
            staff->setRstaff(idx++);
      }

//---------------------------------------------------------
//   removeStaff
//---------------------------------------------------------

void Part::removeStaff(Staff* staff)
      {
      if (!_staves.removeOne(staff)) {
            qDebug("Part::removeStaff: not found %p\n", staff);
            return;
            }
      int idx = 0;
      foreach(Staff* staff, _staves)
            staff->setRstaff(idx++);
      }

//---------------------------------------------------------
//   setShow
//---------------------------------------------------------

void Part::setShow(bool val)
      {
      _show = val;
      foreach(Staff* staff, _staves)
            staff->setShow(_show);
      }

//---------------------------------------------------------
//   setMidiProgram
//    TODO
//---------------------------------------------------------

void Part::setMidiProgram(int program, int bank)
      {
      Channel c = instr(0)->channel(0);
      c.program = program;
      c.bank    = bank;
      c.updateInitList();
      instr(0)->setChannel(0, c);
      }

int Part::volume() const
      {
      return instr(0)->channel(0).volume;
      }
      
void Part::setVolume(int volume)
      { 
      instr(0)->channel(0).volume = volume;
      }

int Part::reverb() const
      {
      return instr(0)->channel(0).reverb;
      }

int Part::chorus() const
      {
      return instr(0)->channel(0).chorus;
      }

int Part::pan() const
      {
      return instr(0)->channel(0).pan;
      }

void Part::setPan(int pan)
      { 
      instr(0)->channel(0).pan = pan;
      }
      
int Part::midiProgram() const
      {
      return instr(0)->channel(0).program;
      }

//---------------------------------------------------------
//   midiChannel
//---------------------------------------------------------

int Part::midiChannel() const
      {
      return score()->midiChannel(instr(0)->channel(0).channel);
      }

//---------------------------------------------------------
//   setMidiChannel
//    called from importmusicxml
//---------------------------------------------------------

void Part::setMidiChannel(int) const
      {
      }

//---------------------------------------------------------
//   setInstrument
//---------------------------------------------------------

void Part::setInstrument(const Instrument& i, int tick)
      {
      _instrList.setInstrument(i, tick);

      //CHECK: ??

      if (!_score->styleB(ST_concertPitch) && i.transpose().chromatic) {
            foreach(Staff* staff, _staves) {
                  if (staff->staffType()->group() != PERCUSSION_STAFF)
                        _score->cmdTransposeStaff(staff->idx(), i.transpose(), false);
                  }
            }
      if (!_score->styleB(ST_concertPitch) && i.transpose().chromatic) {
            foreach(Staff* staff, _staves) {
                  Interval iv(i.transpose());
                  iv.flip();
                  if (staff->staffType()->group() != PERCUSSION_STAFF)
                        _score->cmdTransposeStaff(staff->idx(), iv, false);
                  }
            }
      }

//---------------------------------------------------------
//   removeInstrument
//---------------------------------------------------------

void Part::removeInstrument(int tick)
      {
      _instrList.erase(tick);
      }

//---------------------------------------------------------
//   instr
//---------------------------------------------------------

Instrument* Part::instr(int tick)
      {
      return &_instrList.instrument(tick);
      }

//---------------------------------------------------------
//   instr
//---------------------------------------------------------

const Instrument* Part::instr(int tick) const
      {
      return &_instrList.instrument(tick);
      }

//---------------------------------------------------------
//   longName
//---------------------------------------------------------

QTextDocumentFragment Part::longName(int tick) const
      {
      const QList<StaffNameDoc>& nl = longNames(tick);
      return nl.isEmpty() ? QTextDocumentFragment() : nl[0].name;
      }

//---------------------------------------------------------
//   instrumentName
//---------------------------------------------------------

QString Part::instrumentName(int tick) const
      {
      return instr(tick)->trackName();
      }

//---------------------------------------------------------
//   shortName
//---------------------------------------------------------

QTextDocumentFragment Part::shortName(int tick) const
      {
      const QList<StaffNameDoc>& nl = shortNames(tick);
      return nl.isEmpty() ? QTextDocumentFragment() : nl[0].name;
      }

//---------------------------------------------------------
//   setLongName
//---------------------------------------------------------

void Part::setLongName(const QString& s)
      {
      instr(0)->setLongName(QTextDocumentFragment::fromPlainText(s));
      }

//---------------------------------------------------------
//   setShortName
//---------------------------------------------------------

void Part::setShortName(const QString& s)
      {
      instr(0)->setShortName(QTextDocumentFragment::fromPlainText(s));
      }

