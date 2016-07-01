//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "soundbank.h"
#include "score.h"
#include "instrtemplate.h"
#include "instrument.h"

namespace Ms {

std::vector<SoundBank *> soundbanks;

SoundBank::SoundBank(QString name)
      {
      _name = name;
      }

void SoundBankMatch::read(XmlReader &e) {
      type = e.name().toString();
      value = e.attribute("id");
      }

void SoundBankMatch::write(Xml &xml) const {
      if (type == "Genre")
            xml.tagE(QString("Genre id=\"%1\"").arg(value));
      else if (type == "InstrumentGroup")
            xml.tagE(QString("InstrumentGroup id=\"%1\"").arg(value));
      else if (type == "Instrument")
            xml.tagE(QString("Instrument id=\"%1\"").arg(value));
      }

void SoundBankSpannerDefault::read(XmlReader &e) {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            if (tag == "ccValue") {
                  value = e.attribute("val").toInt();
                  e.readNext();
                  }
            else if (tag == "ccRange") {
                  from = e.attribute("from").toInt();
                  to = e.attribute("to").toInt();
                  e.readNext();
                  }
            else if (tag == "ccType") {
                  QString name = e.attribute("name");
                  if (name == "CONTINUOUS")
                        type = ccType::CONTINUOUS;
                  else if (name == "SWITCH")
                        type = ccType::SWITCH;
                  else if (name == "CON_EVERY_CHORD")
                        type = ccType::CON_EVERY_CHORD;
                  else if (name == "ON_CHORD")
                        type = ccType::ON_CHORD;
                  else
                        type = ccType::NONE;
                  e.readNext();
                  }
            else if (tag == "ccBind") {
                  QString readbind = e.attribute("type");
                  if (readbind == "ARTICULATION")
                        bind = ccBind::ARTICULATION;
                  else if (readbind == "TEMPO")
                        bind = ccBind::TEMPO;
                  else if (readbind == "VELOCITY")
                        bind = ccBind::VELOCITY;
                  else if (readbind == "PITCH")
                        bind = ccBind::PITCH;
                  else
                        bind = ccBind::NONE;
                  e.readNext();
                  }
            else
                  e.unknown();

            }
      }

void SoundBankSpannerDefault::write(Xml &xml) const
      {
      if (spannerType == Element::Type::SLUR)
            xml.stag(QString("slur name=\"%1\"").arg(name));
      else if (spannerType == Element::Type::HAIRPIN)
            xml.stag(QString("hairpin name=\"%1\"").arg(name));
      else if (spannerType == Element::Type::PEDAL)
            xml.stag(QString("pedal name=\"%1\"").arg(name));
      else
            return;

      xml.tagE(QString("ccValue val=\"%1\"").arg(value));
      xml.tagE(QString("ccRange from=\"%1\" to=\"%2\"").arg(from).arg(to));

      if (type == ccType::CONTINUOUS)
            xml.tagE("ccType name=\"CONTINUOUS\"");
      else if (type == ccType::SWITCH)
            xml.tagE("ccType name=\"SWITCH\"");
      else if (type == ccType::CON_EVERY_CHORD)
            xml.tagE("ccType name=\"CON_EVERY_CHORD\"");
      else if (type == ccType::ON_CHORD)
            xml.tagE("ccType name=\"ON_CHORD\"");

      if (bind == ccBind::ARTICULATION)
            xml.tagE("ccBind name=\"ARTICULATION\"");
      else if (bind == ccBind::TEMPO)
            xml.tagE("ccBind name=\"TEMPO\"");
      else if (bind == ccBind::VELOCITY)
            xml.tagE("ccBind name=\"VELOCITY\"");
      else if (bind == ccBind::PITCH)
            xml.tagE("ccBind name=\"PITCH\"");

      if (isDefault)
            xml.tagE("default");

      xml.etag();
      }

void SoundBank::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "match") {
                  readMatches(e);
                  }
            else if(tag == "spanners") {
                  _spannerDefaults.read(e);
                  }
            else if(tag == "Channel") {
                  Channel* newChannel = new Channel();
                  newChannel->read(e, nullptr);
                  _channel.push_back(newChannel);
                  }
            else if(tag == "options") {
                  _options.read(e);
                  }
            else if(tag == "MidiAction") {
                  NamedEventList a;
                  a.read(e);
                  _midiActions.push_back(a);
                  }
            else {
                  e.unknown();
                  }
            }
      }

void SoundBank::write(Xml& xml) const
      {
      xml.stag(QString("SoundBank name=\"%1\"").arg(_name));

      if (_matches.size() > 0) {
            xml.stag("match");
            for (SoundBankMatch match : _matches)
                        match.write(xml);
            xml.etag();
            }

      _spannerDefaults.write(xml);

      for (Channel* c : _channel)
            c->write(xml, nullptr);

      _options.write(xml);

      for (const NamedEventList& a : _midiActions)
            a.write(xml, "MidiAction");

      xml.etag();
      }

void SoundBank::readMatches(XmlReader & e) {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "InstrumentGroup" || tag == "Instrument" || tag == "Genre") {
                  SoundBankMatch newMatch;
                  newMatch.read(e);
                  _matches.push_back(newMatch);
                  e.readNext();
                  }
            else {
                  e.unknown();
                  }
            }
      }

void SoundBankSpannerDefaults::read(XmlReader &e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());

            if (tag == "slur") {
                  SoundBankSpannerDefault* newSlurDefault = new SoundBankSpannerDefault(Element::Type::SLUR);
                  newSlurDefault->name = e.attribute("name");
                  newSlurDefault->read(e);
                  _spannerDefaults.push_back(newSlurDefault);
                  }
            else if (tag == "hairpin") {
                  SoundBankSpannerDefault* newHairpinDefault = new SoundBankSpannerDefault(Element::Type::HAIRPIN);
                  newHairpinDefault->name = e.attribute("name");
                  newHairpinDefault->read(e);
                  _spannerDefaults.push_back(newHairpinDefault);
                  }
            else if (tag == "pedal") {
                  SoundBankSpannerDefault* newPedalDefault = new SoundBankSpannerDefault(Element::Type::PEDAL);
                  newPedalDefault->name = e.attribute("name");
                  newPedalDefault->read(e);
                  _spannerDefaults.push_back(newPedalDefault);
                  }
            else {
                  e.unknown();
                  }
            }
      }

void SoundBankSpannerDefaults::write(Xml &xml) const
      {
      if (_spannerDefaults.size() == 0)
            return;
      xml.stag("spanners");
      for (SoundBankSpannerDefault *sd : _spannerDefaults)
            sd->write(xml);
      xml.etag();
      }

void SoundBankOptions::read(XmlReader &e) {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "fixedVelocity") {
                  fixedVelocity = e.attribute("value").toInt();
                  e.readNext();
                  }
            else if (tag == "dynamic") {
                  if (e.attribute("type") == "expression")
                        useExpression = true;
                  e.readNext();
                  }
            else {
                  e.unknown();
                  }
            }
      }

void SoundBankOptions::write(Xml &xml) const {
      if (!useExpression && fixedVelocity == 0)
            return;
      xml.stag("options");
      xml.tagE(QString("fixedVelocity value=\"%1\"").arg(fixedVelocity));
      if (useExpression)
            xml.tagE("dynamic type=\"expression\"");
      xml.etag();
      }

void addToSoundBanks(QString filename)
      {
      QFile file(filename);
      if (!file.open(QIODevice::ReadOnly)) {
            qDebug("Could not open file " + filename.toLatin1());
            return;
            }
      XmlReader e(&file);
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            if (tag == "SoundBank") {
                  QString sbName = e.attribute("name");
                  SoundBank* newSb = new SoundBank(sbName);
                  newSb->read(e);
                  soundbanks.push_back(newSb);
                  }
            else
                  e.unknown();
            }
      }

void removeSoundBank(QString Bankname)
      {
      for (std::vector<SoundBank*>::iterator it = soundbanks.begin(); it != soundbanks.end(); it++) {
            if ((*it)->getName() == Bankname) {
                  soundbanks.erase(it);
                  delete (*it);
                  break;
                  }
            }
      }

void soundbankMatchInstrumentTemplate(InstrumentTemplate *itemp)
      {
      if (!itemp)
            return;
      itemp->matchedSoundbanks.clear();
      bool matches = true;
      for (SoundBank *sb : soundbanks) {
            for (SoundBankMatch m : sb->getMatches()) {
                  if (m.type == "Instrument") {
                        matches = itemp->id == m.value;
                        }
                  else if(m.type == "InstrumentGroup") {
                        matches = itemp->groupMember(m.value);
                        }
                  else if(m.type == "Genre") {
                        matches = itemp->genreMember(m.value);
                        }
                  if (!matches)
                        break;
                  }
            if (matches)
                  itemp->matchedSoundbanks.push_back(sb);
            }
      }

void soundbankMatchInstrumentTemplates()
      {
      for (InstrumentGroup* g : instrumentGroups) {
            for (InstrumentTemplate* it : g->instrumentTemplates)
                  soundbankMatchInstrumentTemplate(it);
            }
      }

std::vector<SoundBankSpannerDefault*> SoundBankSpannerDefaults::getSpannerDefaults(Element::Type type)
      {
      std::vector<SoundBankSpannerDefault*> defaults;
      for (SoundBankSpannerDefault* d : _spannerDefaults) {
            if (d->spannerType == type)
                  defaults.push_back(d);
            }
      return defaults;
      }

SoundBankSpannerDefault* SoundBankSpannerDefaults::getSpannerDefault(Element::Type type) {
      std::vector<SoundBankSpannerDefault*> defaults = getSpannerDefaults(type);
      for (SoundBankSpannerDefault* d : defaults) {
            if (d->isDefault)
                  return d;
            }
      if (defaults.size() > 0)
            return defaults.front();
      else
            return nullptr;
      }

void SoundBankSpannerDefaults::setSelected(unsigned int v, Element::Type type)
      {
      std::vector<SoundBankSpannerDefault*> defaults = getSpannerDefaults(type);
      unsigned int i = 0;
      for (SoundBankSpannerDefault* d : defaults) {
            if (i == v)
                  d->isDefault = true;
            else
                  d->isDefault = false;
            i++;
            }
      }

void SoundBankSpannerDefaults::operator=(const SoundBankSpannerDefaults &sbsd)
      {
     for (const SoundBankSpannerDefault *sd : sbsd._spannerDefaults)
           {
           SoundBankSpannerDefault* newSpannerDefault = new SoundBankSpannerDefault(sd->spannerType);
           *newSpannerDefault = *sd;
           _spannerDefaults.push_back(newSpannerDefault);
           }
      }

SoundBankSpannerDefaults::~SoundBankSpannerDefaults()
      {
      qDeleteAll(_spannerDefaults);
      _spannerDefaults.clear();
      }

}
