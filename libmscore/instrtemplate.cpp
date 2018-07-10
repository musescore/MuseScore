//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "instrtemplate.h"
#include "bracket.h"
#include "drumset.h"
#include "stafftype.h"
#include "style.h"
#include "sym.h"
#include "stringdata.h"
#include "utils.h"
#include "xml.h"

namespace Ms {

QList<InstrumentGroup*> instrumentGroups;
QList<MidiArticulation> articulation;                // global articulations
QList<InstrumentGenre*> instrumentGenres;

//---------------------------------------------------------
//   searchGenre
//---------------------------------------------------------

static InstrumentGenre * searchInstrumentGenre(const QString& genre)
      {
      foreach(InstrumentGenre* ig, instrumentGenres) {
            if (ig->id == genre)
                  return ig;
            }
      return nullptr;
      }

//---------------------------------------------------------
//   searchInstrumentGroup
//---------------------------------------------------------

static InstrumentGroup* searchInstrumentGroup(const QString& name)
      {
      foreach(InstrumentGroup* g, instrumentGroups) {
            if (g->id == name)
                  return g;
            }
      return nullptr;
      }

//---------------------------------------------------------
//   searchArticulation
//---------------------------------------------------------

static MidiArticulation searchArticulation(const QString& name)
      {
      foreach(MidiArticulation a, articulation) {
            if (a.name == name)
                  return a;
            }
      return MidiArticulation();
      }

//---------------------------------------------------------
//   readStaffIdx
//---------------------------------------------------------

static int readStaffIdx(XmlReader& e)
      {
      int idx = e.intAttribute("staff", 1) - 1;
      if (idx >= MAX_STAVES)
            idx = MAX_STAVES-1;
      if (idx < 0)
            idx = 0;
      return idx;
      }

//---------------------------------------------------------
//   read InstrumentGroup
//---------------------------------------------------------

void InstrumentGroup::read(XmlReader& e)
      {
      id       = e.attribute("id");
      name     = qApp->translate("InstrumentsXML", e.attribute("name").toUtf8().data());
      extended = e.intAttribute("extended", 0);

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "instrument" || tag == "Instrument") {
                  QString id = e.attribute("id");
                  InstrumentTemplate* t = searchTemplate(id);
                  if (t == 0) {
                        t = new InstrumentTemplate;
                        t->articulation.append(articulation);     // init with global articulation
                        instrumentTemplates.append(t);
                        }
                  t->read(e);
                  }
            else if (tag == "ref") {
                  InstrumentTemplate* ttt = searchTemplate(e.readElementText());
                  if (ttt) {
                        InstrumentTemplate* t = new InstrumentTemplate(*ttt);
                        instrumentTemplates.append(t);
                        }
                  else
                        qDebug("instrument reference not found <%s>", e.text().toUtf8().data());
                  }
            else if (tag == "name")
                  name = qApp->translate("InstrumentsXML", e.readElementText().toUtf8().data());
            else if (tag == "extended")
                  extended = e.readInt();
            else
                  e.unknown();
            }
      if (id.isEmpty())
            id = name.toLower().replace(" ", "-");
      }

//---------------------------------------------------------
//   clear InstrumentGroup
//---------------------------------------------------------

void InstrumentGroup::clear()
      {
      qDeleteAll(instrumentTemplates);
      instrumentTemplates.clear();
      }

//---------------------------------------------------------
//   InstrumentTemplate
//---------------------------------------------------------

InstrumentTemplate::InstrumentTemplate()
      {
      staves             = 1;
      minPitchA          = 0;
      maxPitchA          = 127;
      minPitchP          = 0;
      maxPitchP          = 127;
      staffGroup         = StaffGroup::STANDARD;
      staffTypePreset    = 0;
      useDrumset         = false;
      drumset            = 0;
      extended           = false;

      for (int i = 0; i < MAX_STAVES; ++i) {
            clefTypes[i]._concertClef = ClefType::G;
            clefTypes[i]._transposingClef = ClefType::G;
            staffLines[i]  = 5;
            smallStaff[i]  = false;
            bracket[i]     = BracketType::NO_BRACKET;
            bracketSpan[i] = 0;
            barlineSpan[i] = false;
            }
      transpose.diatonic   = 0;
      transpose.chromatic  = 0;
      }

InstrumentTemplate::InstrumentTemplate(const InstrumentTemplate& t)
      {
      init(t);
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void InstrumentTemplate::init(const InstrumentTemplate& t)
      {
      longNames  = t.longNames;
      shortNames = t.shortNames;
      musicXMLid = t.musicXMLid;
      staves     = t.staves;
      extended   = t.extended;

      for (int i = 0; i < MAX_STAVES; ++i) {
            clefTypes[i]   = t.clefTypes[i];
            staffLines[i]  = t.staffLines[i];
            smallStaff[i]  = t.smallStaff[i];
            bracket[i]     = t.bracket[i];
            bracketSpan[i] = t.bracketSpan[i];
            barlineSpan[i] = t.barlineSpan[i];
            }
      minPitchA  = t.minPitchA;
      maxPitchA  = t.maxPitchA;
      minPitchP  = t.minPitchP;
      maxPitchP  = t.maxPitchP;
      transpose  = t.transpose;
      staffGroup = t.staffGroup;
      staffTypePreset   = t.staffTypePreset;
      useDrumset = t.useDrumset;
      if (t.drumset)
            drumset = new Drumset(*t.drumset);
      else
            drumset = 0;
      stringData  = t.stringData;
      midiActions = t.midiActions;
      channel     = t.channel;
      }

InstrumentTemplate::~InstrumentTemplate()
      {
      delete drumset;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void InstrumentTemplate::write(XmlWriter& xml) const
      {
      xml.stag(QString("Instrument id=\"%1\"").arg(id));
      longNames.write(xml, "longName");
      shortNames.write(xml, "shortName");

      if (longNames.size() > 1)
            xml.tag("trackName", trackName);
      xml.tag("description", description);
      xml.tag("musicXMLid", musicXMLid);
      if (extended)
            xml.tag("extended", extended);
      stringData.write(xml);
      if (staves > 1)
            xml.tag("staves", staves);
      for (int i = 0; i < staves; ++i) {
            if (clefTypes[i]._concertClef == clefTypes[i]._transposingClef) {
                  QString tag = ClefInfo::tag(clefTypes[i]._concertClef);
                  if (i)
                        xml.tag(QString("clef staff=\"%1\"").arg(i+1), tag);
                  else
                        xml.tag("clef", tag);
                  }
            else {
                  QString tag1 = ClefInfo::tag(clefTypes[i]._concertClef);
                  QString tag2 = ClefInfo::tag(clefTypes[i]._transposingClef);
                  if (i) {
                        xml.tag(QString("concertClef staff=\"%1\"").arg(i+1), tag1);
                        xml.tag(QString("transposingClef staff=\"%1\"").arg(i+1), tag2);
                        }
                  else {
                        xml.tag("concertClef", tag1);
                        xml.tag("transposingClef", tag2);
                        }
                  }
            if (staffLines[i] != 5) {
                  if (i)
                        xml.tag(QString("stafflines staff=\"%1\"").arg(i+1), staffLines[i]);
                  else
                        xml.tag("stafflines", staffLines[i]);
                  }
            if (smallStaff[i]) {
                  if (i)
                        xml.tag(QString("smallStaff staff=\"%1\"").arg(i+1), smallStaff[i]);
                  else
                        xml.tag("smallStaff", smallStaff[i]);
                  }

            if (bracket[i] != BracketType::NO_BRACKET) {
                  if (i)
                        xml.tag(QString("bracket staff=\"%1\"").arg(i+1), int(bracket[i]));
                  else
                        xml.tag("bracket", int(bracket[i]));
                  }
            if (bracketSpan[i] != 0) {
                  if (i)
                        xml.tag(QString("bracketSpan staff=\"%1\"").arg(i+1), bracketSpan[i]);
                  else
                        xml.tag("bracketSpan", bracketSpan[i]);
                  }
            if (barlineSpan[i]) {
                  if (i)
                        xml.tag(QString("barlineSpan staff=\"%1\"").arg(i+1), barlineSpan[i]);
                  else
                        xml.tag("barlineSpan", barlineSpan[i]);
                  }
            }
      if (minPitchA != 0 || maxPitchA != 127)
            xml.tag("aPitchRange", QString("%1-%2").arg(int(minPitchA)).arg(int(maxPitchA)));
      if (minPitchP != 0 || maxPitchP != 127)
            xml.tag("pPitchRange", QString("%1-%2").arg(int(minPitchP)).arg(int(maxPitchP)));
      if (transpose.diatonic)
            xml.tag("transposeDiatonic", transpose.diatonic);
      if (transpose.chromatic)
            xml.tag("transposeChromatic", transpose.chromatic);
      if (useDrumset)
            xml.tag("drumset", int(useDrumset));
      if (drumset)
            drumset->save(xml);
      foreach(const NamedEventList& a, midiActions)
            a.write(xml, "MidiAction");
      foreach(const Channel& a, channel)
            a.write(xml, nullptr);
      foreach(const MidiArticulation& ma, articulation) {
            bool isGlobal = false;
            foreach(const MidiArticulation& ga, Ms::articulation) {
                  if (ma == ga) {
                        isGlobal = true;
                        break;
                        }
                  }
            if (!isGlobal)
                  ma.write(xml);
            }
      xml.etag();
      }

//---------------------------------------------------------
//   write1
//    output only translatable names
//---------------------------------------------------------

void InstrumentTemplate::write1(XmlWriter& xml) const
      {
      xml.stag(QString("Instrument id=\"%1\"").arg(id));
      longNames.write(xml, "longName");
      shortNames.write(xml, "shortName");
      if (longNames.size() > 1)
            xml.tag("trackName", trackName);
      xml.tag("description", description);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void InstrumentTemplate::read(XmlReader& e)
      {
      id = e.attribute("id");

      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            if (tag == "longName" || tag == "name") {               // "name" is obsolete
                  int pos = e.intAttribute("pos", 0);
                  for (QList<StaffName>::iterator i = longNames.begin(); i != longNames.end(); ++i) {
                        if ((*i).pos() == pos)
                              longNames.erase(i);
                        break;
                        }
                  longNames.append(StaffName(qApp->translate("InstrumentsXML", e.readElementText().toUtf8().data()), pos));
                  }
            else if (tag == "shortName" || tag == "short-name") {   // "short-name" is obsolete
                  int pos = e.intAttribute("pos", 0);
                  for (QList<StaffName>::iterator i = shortNames.begin(); i != shortNames.end(); ++i) {
                        if ((*i).pos() == pos)
                              shortNames.erase(i);
                        break;
                        }
                  shortNames.append(StaffName(qApp->translate("InstrumentsXML", e.readElementText().toUtf8().data()), pos));
                  }
            else if (tag == "trackName")
                  trackName = qApp->translate("InstrumentsXML", e.readElementText().toUtf8().data());
            else if (tag == "description")
                  description = e.readElementText();
            else if (tag == "extended")
                  extended = e.readInt();
            else if (tag == "staves") {
                  staves = e.readInt();
                  bracketSpan[0] = staves;
//                  for (int i = 0; i < staves-1; ++i)
//                        barlineSpan[i] = true;
                  }
            else if (tag == "clef") {           // sets both transposing and concert clef
                  int idx = readStaffIdx(e);
                  QString val(e.readElementText());
                  bool ok;
                  int i = val.toInt(&ok);
                  ClefType ct = ok ? ClefType(i) : Clef::clefType(val);
                  clefTypes[idx]._concertClef = ct;
                  clefTypes[idx]._transposingClef = ct;
                  }
            else if (tag == "concertClef") {
                  int idx = readStaffIdx(e);
                  QString val(e.readElementText());
                  bool ok;
                  int i = val.toInt(&ok);
                  clefTypes[idx]._concertClef = ok ? ClefType(i) : Clef::clefType(val);
                  }
            else if (tag == "transposingClef") {
                  int idx = readStaffIdx(e);
                  QString val(e.readElementText());
                  bool ok;
                  int i = val.toInt(&ok);
                  clefTypes[idx]._transposingClef = ok ? ClefType(i) : Clef::clefType(val);
                  }
            else if (tag == "stafflines") {
                  int idx = readStaffIdx(e);
                  staffLines[idx] = e.readInt();
                  }
            else if (tag == "smallStaff") {
                  int idx = readStaffIdx(e);
                  smallStaff[idx] = e.readInt();
                  }
            else if (tag == "bracket") {
                  int idx = readStaffIdx(e);
                  bracket[idx] = BracketType(e.readInt());
                  }
            else if (tag == "bracketSpan") {
                  int idx = readStaffIdx(e);
                  bracketSpan[idx] = e.readInt();
                  }
            else if (tag == "barlineSpan") {
                  int idx = readStaffIdx(e);
                  int span = e.readInt();
                  for (int i = 0; i < span-1; ++i)
                        barlineSpan[idx+i] = true;
                  }
            else if (tag == "aPitchRange")
                  setPitchRange(e.readElementText(), &minPitchA, &maxPitchA);
            else if (tag == "pPitchRange")
                  setPitchRange(e.readElementText(), &minPitchP, &maxPitchP);
            else if (tag == "transposition") {    // obsolete
                  int i = e.readInt();
                  transpose.chromatic = i;
                  transpose.diatonic = chromatic2diatonic(i);
                  }
            else if (tag == "transposeChromatic")
                  transpose.chromatic = e.readInt();
            else if (tag == "transposeDiatonic")
                  transpose.diatonic = e.readInt();
            else if (tag == "StringData")
                  stringData.read(e);
            else if (tag == "drumset")
                  useDrumset = e.readInt();
            else if (tag == "Drum") {
                  // if we see one of this tags, a custom drumset will
                  // be created
                  if (drumset == 0) {
                        drumset = new Drumset(*smDrumset);
                        drumset->clear();
                        }
                  drumset->load(e);
                  }
            else if (tag == "MidiAction") {
                  NamedEventList a;
                  a.read(e);
                  midiActions.append(a);
                  }
            else if (tag == "Channel" || tag == "channel") {
                  Channel a;
                  a.read(e, nullptr);
                  channel.append(a);
                  }
            else if (tag == "Articulation") {
                  MidiArticulation a;
                  a.read(e);
                  int n = articulation.size();
                  int i;
                  for(i = 0; i < n; ++i) {
                        if (articulation[i].name == a.name) {
                              articulation[i] = a;
                              break;
                              }
                        }
                  if (i == n)
                        articulation.append(a);
                  }
            else if (tag == "stafftype") {
                  int staffIdx = readStaffIdx(e);
                  QString xmlPresetName = e.attribute("staffTypePreset", "");
                  QString stfGroup = e.readElementText();
                  if (stfGroup == "percussion")
                        staffGroup = StaffGroup::PERCUSSION;
                  else if (stfGroup == "tablature")
                        staffGroup = StaffGroup::TAB;
                  else
                        staffGroup = StaffGroup::STANDARD;
                  staffTypePreset = 0;
                  if (!xmlPresetName.isEmpty())
                        staffTypePreset = StaffType::presetFromXmlName(xmlPresetName);
                  if (!staffTypePreset || staffTypePreset->group() != staffGroup)
                        staffTypePreset = StaffType::getDefaultPreset(staffGroup);
                  if (staffTypePreset)
                        staffLines[staffIdx] = staffTypePreset->lines();
                  }
            else if (tag == "init") {
                  QString val(e.readElementText());
                  InstrumentTemplate* ttt = searchTemplate(val);
                  if (ttt)
                        init(*ttt);
                  else
                        qDebug("InstrumentTemplate:: init instrument <%s> not found", qPrintable(val));
                  }
            else if (tag == "musicXMLid") {
                  musicXMLid = e.readElementText();
                  }
            else if (tag == "genre") {
                  QString val(e.readElementText());
                  linkGenre(val);
                  }
            else
                  e.unknown();
            }
      if (channel.empty()) {
            Channel a;
            a.chorus       = 0;
            a.reverb       = 0;
            a.name         = "normal";
            a.program      = 0;
            a.bank         = 0;
            a.volume       = 100;
            a.pan          = 64; // actually 63.5 for center
            channel.append(a);
            }
      if (useDrumset) {
            if (channel[0].bank == 0)
                  channel[0].bank = 128;
            channel[0].updateInitList();
            }
      if (trackName.isEmpty() && !longNames.isEmpty())
            trackName = longNames[0].name();
      if (description.isEmpty() && !longNames.isEmpty())
            description = longNames[0].name();
      if (id.isEmpty())
            id = trackName.toLower().replace(" ", "-");

      if (staves == 0)
            qDebug(" 2Instrument: staves == 0 <%s>", qPrintable(id));
      }


//---------------------------------------------------------
//   setPitchRange
//---------------------------------------------------------

void InstrumentTemplate::setPitchRange(const QString& s, char* a, char* b) const
      {
      QStringList sl = s.split("-");
      if (sl.size() != 2) {
            *a = 0;
            *b = 127;
            return;
            }
      *a = sl[0].toInt();
      *b = sl[1].toInt();
      }

//---------------------------------------------------------
//   saveInstrumentTemplates
//---------------------------------------------------------

bool saveInstrumentTemplates(const QString& instrTemplates)
      {
      QFile qf(instrTemplates);
      if (!qf.open(QIODevice::WriteOnly)) {
            qDebug("cannot save instrument templates at <%s>", qPrintable(instrTemplates));
            return false;
            }
      XmlWriter xml(0, &qf);
      xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
      xml.stag("museScore");
      foreach(const InstrumentGenre* genre, instrumentGenres)
            genre->write(xml);
      xml << "\n";
      foreach(const MidiArticulation& a, articulation)
            a.write(xml);
      xml << "\n";
      foreach(InstrumentGroup* group, instrumentGroups) {
            xml.stag(QString("InstrumentGroup id=\"%1\"").arg(group->id));
            xml.tag("name", group->name);
            if (group->extended)
                  xml.tag("extended", group->extended);
            foreach(InstrumentTemplate* it, group->instrumentTemplates) {
                  it->write(xml);
                  xml << "\n";
                  }
            xml.etag();
            xml << "\n";
            }
      xml.etag();
      qf.close();
      return true;
      }

//---------------------------------------------------------
//   saveInstrumentTemplates1
//---------------------------------------------------------

bool saveInstrumentTemplates1(const QString& instrTemplates)
      {
      QFile qf(instrTemplates);
      if (!qf.open(QIODevice::WriteOnly)) {
            qDebug("cannot save instrument templates at <%s>", qPrintable(instrTemplates));
            return false;
            }
      XmlWriter xml(0, &qf);
      xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
      xml.stag("museScore");
      foreach(const InstrumentGenre* genre, instrumentGenres)
            genre->write1(xml);
      foreach(InstrumentGroup* group, instrumentGroups) {
            xml.stag(QString("InstrumentGroup id=\"%1\"").arg(group->id));
            xml.tag("name", group->name);
            foreach(InstrumentTemplate* it, group->instrumentTemplates) {
                  it->write1(xml);
                  xml << "\n";
                  }
            xml.etag();
            xml << "\n";
            }
      xml.etag();
      qf.close();
      return true;
      }

//---------------------------------------------------------
//   clearInstrumentTemplates
//---------------------------------------------------------

void clearInstrumentTemplates()
      {
      for (InstrumentGroup* g : instrumentGroups)
            g->clear();
      qDeleteAll(instrumentGroups);
      instrumentGroups.clear();
      qDeleteAll(instrumentGenres);
      instrumentGenres.clear();
      articulation.clear();
      }

//---------------------------------------------------------
//   loadInstrumentTemplates
//---------------------------------------------------------

bool loadInstrumentTemplates(const QString& instrTemplates)
      {
      QFile qf(instrTemplates);
      if (!qf.open(QIODevice::Text | QIODevice::ReadOnly)) {
            qDebug("cannot load instrument templates at <%s>", qPrintable(instrTemplates));
            return false;
            }

      XmlReader e(&qf);
      while (e.readNextStartElement()) {
            if (e.name() == "museScore") {
                  while (e.readNextStartElement()) {
                        const QStringRef& tag(e.name());
                        if (tag == "instrument-group" || tag == "InstrumentGroup") {
                              QString idGroup(e.attribute("id"));
                              InstrumentGroup* group = searchInstrumentGroup(idGroup);
                              if (group == 0) {
                                    group = new InstrumentGroup;
                                    instrumentGroups.append(group);
                                    }
                              group->read(e);
                              }
                        else if (tag == "Articulation") {
                              // read global articulation
                              QString name(e.attribute("name"));
                              MidiArticulation a = searchArticulation(name);
                              a.read(e);
                              articulation.append(a);
                              }
                        else if (tag == "Genre") {
                              QString idGenre(e.attribute("id"));
                              InstrumentGenre* genre = searchInstrumentGenre(idGenre);
                              if (!genre) {
                                    genre = new InstrumentGenre;
                                    instrumentGenres.append(genre);
                                    }
                              genre->read(e);
                              }
                        else
                              e.unknown();
                        }
                  }
            }
      // saveInstrumentTemplates1("/home/ws/mops.xml");
      return true;
      }

//---------------------------------------------------------
//   searchTemplate
//---------------------------------------------------------

InstrumentTemplate* searchTemplate(const QString& name)
      {
      foreach (InstrumentGroup* g, instrumentGroups) {
            foreach(InstrumentTemplate* it, g->instrumentTemplates) {
                  if (it->id == name)
                        return it;
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   searchTemplateForMusicXMLid
//---------------------------------------------------------

InstrumentTemplate* searchTemplateForMusicXmlId(const QString& mxmlId)
      {
      foreach(InstrumentGroup* g, instrumentGroups) {
            foreach(InstrumentTemplate* it, g->instrumentTemplates) {
                  if (it->musicXMLid == mxmlId)
                        return it;
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   linkGenre
//      link the current instrument template to the genre list specified by "genre"
//      Each genre is a list of pointers to instrument templates
//      The list of genres is at application level
//---------------------------------------------------------

void InstrumentTemplate::linkGenre(const QString& genre)
      {
      InstrumentGenre *ig = searchInstrumentGenre(genre);
      if (ig)
            genres.append(ig);
      }

//---------------------------------------------------------
//   genreMember
//      is this instrument template a member of the supplied genre
//---------------------------------------------------------

bool InstrumentTemplate::genreMember(const QString& name)
      {
            bool rVal=false;
            foreach(InstrumentGenre *instrumentGenre, genres ) {
                if(instrumentGenre->id == name) {
                      rVal = true;
                      break;
                }
            }
            return rVal;
      }

void InstrumentGenre::write(XmlWriter& xml) const
      {
      xml.stag(QString("Genre id=\"%1\"").arg(id));
      xml.tag("name", name);
      xml.etag();
      }

void InstrumentGenre::write1(XmlWriter& xml) const
      {
      write(xml);
      }

void InstrumentGenre::read(XmlReader& e)
      {
      id = e.attribute("id");
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "name") {
                  name = qApp->translate("InstrumentsXML", e.readElementText().toUtf8().data());
            }
            else
                  e.unknown();
            }
     }

//---------------------------------------------------------
//   clefType
//---------------------------------------------------------

ClefTypeList InstrumentTemplate::clefType(int staffIdx) const
      {
      if (staffIdx < staves)
            return clefTypes[staffIdx];
      return clefTypes[0];
      }

//---------------------------------------------------------
//   defaultClef
//    traverse the instrument list for first instrument
//    with midi patch 'program'. Return the default clef
//    for this instrument.
//---------------------------------------------------------

ClefType defaultClef(int program)
      {
      if (program >= 25 && program < 32)              // this are guitars
            return ClefType::G8_VB;
      else if (program >= 33 && program < 41)         // this is bass
            return ClefType::F8_VB;

      for (InstrumentGroup* g : instrumentGroups) {
            for (InstrumentTemplate* it : g->instrumentTemplates) {
                  if (it->channel[0].bank == 0 && it->channel[0].program == program){
                        return (it->clefTypes[0]._concertClef);
                        }
                  }
            }
      return ClefType::G;
      }

}


