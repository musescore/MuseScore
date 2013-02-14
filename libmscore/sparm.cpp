//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2010 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "libmscore/xml.h"
#include "sparm.h"

//---------------------------------------------------------
//   SyntiParameter
//---------------------------------------------------------

SyntiParameter::SyntiParameter()
      {
      _id   = -1;
      _type = SP_FLOAT;
      _fval = 0.0;
      }

SyntiParameter::SyntiParameter(const QString& name, float val)
      {
      _id = -1;
      _name = name;
      _type = SP_FLOAT;
      _fval = val;
      }

SyntiParameter::SyntiParameter(int i, const QString& name, float val)
      {
      _id   = i;
      _name = name;
      _type =  SP_FLOAT;
      _fval = val;
      }

SyntiParameter::SyntiParameter(const QString& name, const QString& val)
      {
      _id   = -1;
      _name = name;
      _type =  SP_STRING;
      _sval = val;
      }

SyntiParameter::SyntiParameter(int i, const QString& name, const QString& val)
      {
      _id   = i;
      _name = name;
      _type =  SP_STRING;
      _sval = val;
      }

SyntiParameter::SyntiParameter(const SyntiParameter& pd)
      {
      _id   = pd._id;
      _name = pd._name;
      _type = pd._type;
      switch(_type) {
            case SP_FLOAT:
                  _fval = pd._fval;
                  _min  = pd._min;
                  _max  = pd._max;
                  break;
            case SP_STRING:
                  _sval = pd._sval;
                  break;
            }
      }

bool SyntiParameter::operator==(const SyntiParameter& sp) const
      {
      if (_name != sp._name)
            return false;
      switch(_type) {
            case SP_FLOAT:
                  return qAbs(_fval - sp._fval) < 0.000001;
            case SP_STRING:
                  return _sval == sp._sval;
            }
      return false;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void SyntiParameter::write(Xml& xml) const
      {
      if (_type == SP_FLOAT)
            xml.tagE(QString("f name=\"%1\" val=\"%3\"").arg(_name).arg(_fval));
      else if (_type == SP_STRING)
            xml.tagE(QString("s name=\"%1\" val=\"%3\"").arg(_name).arg(Xml::xmlString(_sval)));
      }

//---------------------------------------------------------
//   print
//    for debugging
//---------------------------------------------------------

void SyntiParameter::print() const
      {
      SParmId spid(_id);
      if (_type == SP_FLOAT) {
            printf("<id=(%d,%d,%d) name=%s val=%f>",
               spid.syntiId, spid.subsystemId, spid.paramId,
               qPrintable(_name), _fval);
            }
      else if (_type == SP_STRING) {
            printf("<id=(%d,%d,%d) name=%s val=%s>",
               spid.syntiId, spid.subsystemId, spid.paramId,
               qPrintable(_name), qPrintable(_sval));
            }
      }

//---------------------------------------------------------
//   SyntiState::write
//---------------------------------------------------------

void SyntiState::write(Xml& xml) const
      {
      xml.stag("SyntiSettings");
      foreach(const SyntiParameter& sp, *this)
            sp.write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   SyntiSettings::read
//---------------------------------------------------------

void SyntiState::read(XmlReader& e)
      {
      while (e.readNextStartElement()) {
            const QStringRef& tag(e.name());
            QString name(e.attribute("name"));
            if (tag == "f") {
                  double val = e.doubleAttribute("val");
                  append(SyntiParameter(name, val));
                  e.skipCurrentElement();
                  }
            else if (tag == "s") {
                  append(SyntiParameter(name, e.attribute("val")));
                  e.skipCurrentElement();
                  }
            else if (tag == "Synth") {
                  // obsolete
                  while (e.readNextStartElement()) {
                        const QStringRef& tag(e.name());
                        if (tag == "f") {
                              double val = e.doubleAttribute("val");
                              append(SyntiParameter(name, val));
                              }
                        else if (tag == "s")
                              append(SyntiParameter(name, e.attribute("val")));
                        else
                              e.unknown();
                        }
                  }
            else
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   operator==
//---------------------------------------------------------

bool SyntiState::operator==(const SyntiState& st) const
      {
      int n = size();
      if (n != st.size())
            return false;
      for (int i = 0; i < n; ++i) {
            if (!(at(i) == st.at(i)))
                  return false;
            }
      return true;
      }

//---------------------------------------------------------
//   set
//---------------------------------------------------------

void SyntiParameter::set(const QString& s, float val, float min, float max)
      {
      _sval = s;
      _fval = val;
      _min = min;
      _max = max;
      }

