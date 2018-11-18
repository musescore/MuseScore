//=============================================================================
//  MusE Score
//  Linux Music Score Editor
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
#include "sparm_p.h"

using namespace Ms;

//---------------------------------------------------------
//   SyntiParameterData
//---------------------------------------------------------

SyntiParameterData::SyntiParameterData()
      {
      _id   = -1;
      _type = SP_FLOAT;
      _fval = 0.0;
      }

SyntiParameterData::SyntiParameterData(const QString& name, float val)
      {
      _id = -1;
      _name = name;
      _type = SP_FLOAT;
      _fval = val;
      }

SyntiParameterData::SyntiParameterData(int i, const QString& name, float val)
      {
      _id   = i;
      _name = name;
      _type =  SP_FLOAT;
      _fval = val;
      }

SyntiParameterData::SyntiParameterData(const QString& name, const QString& val)
      {
      _id   = -1;
      _name = name;
      _type =  SP_STRING;
      _sval = val;
      }

SyntiParameterData::SyntiParameterData(int i, const QString& name, const QString& val)
      {
      _id   = i;
      _name = name;
      _type =  SP_STRING;
      _sval = val;
      }

SyntiParameterData::~SyntiParameterData()
{
}

SyntiParameterData::SyntiParameterData(const SyntiParameterData& pd)
   : QSharedData(pd)
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

SyntiParameter& SyntiParameter::operator=(const SyntiParameter& sp)
      {
      d = sp.d;
      return *this;
      }

bool SyntiParameterData::operator==(const SyntiParameterData& sp) const
      {
      if (_id == -1 ? (_name != sp._name) : (_id != sp._id))
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

void SyntiParameterData::write(Xml& xml) const
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

void SyntiParameterData::print() const
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
//   SyntiParameter
//---------------------------------------------------------

SyntiParameter::SyntiParameter()
      {
      d = new SyntiParameterData;
      }

SyntiParameter::SyntiParameter(const SyntiParameter& sp)
   : d(sp.d)
      {
      }

SyntiParameter::SyntiParameter(const QString& name, float val)
      {
      d = new SyntiParameterData(name, val);
      }

SyntiParameter::SyntiParameter(int id, const QString& name, float val)
      {
      d = new SyntiParameterData(id, name, val);
      }

SyntiParameter::SyntiParameter(const QString& name, const QString& val)
      {
      d = new SyntiParameterData(name, val);
      }

SyntiParameter::SyntiParameter(int id, const QString& name, const QString& val)
      {
      d = new SyntiParameterData(id, name, val);
      }

SyntiParameter::~SyntiParameter()
      {
      }

//---------------------------------------------------------
//   type
//---------------------------------------------------------

SyntiParameterType SyntiParameter::type() const
      {
      return d->_type;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void SyntiParameter::write(Xml& xml) const
      {
      d->write(xml);
      }

//---------------------------------------------------------
//   name
//---------------------------------------------------------

const QString& SyntiParameter::name() const
      {
      return d->_name;
      }

void SyntiParameter::setName(const QString& s)
      {
      d->_name = s;
      }

//---------------------------------------------------------
//   id
//---------------------------------------------------------

int SyntiParameter::id() const
      {
      return d->_id;
      }

//---------------------------------------------------------
//   setId
//---------------------------------------------------------

void SyntiParameter::setId(int v)
      {
      d->_id = v;
      }

//---------------------------------------------------------
//   sval
//---------------------------------------------------------

QString SyntiParameter::sval() const
      {
      return d->_sval;
      }

//---------------------------------------------------------
//   set
//---------------------------------------------------------

void SyntiParameter::set(const QString& s)
      {
      d->_sval = s;
      }

void SyntiParameter::set(float v)
      {
      d->_fval = v;
      }

void SyntiParameter::set(const QString& s, float v, float min, float max)
      {
      d->_name = s;
      d->_fval = v;
      d->_min  = min;
      d->_max  = max;
      }

//---------------------------------------------------------
//   fval
//---------------------------------------------------------

float SyntiParameter::fval() const
      {
      return d->_fval;
      }

//---------------------------------------------------------
//   min
//---------------------------------------------------------

float SyntiParameter::min() const
      {
      return d->_min;
      }

//---------------------------------------------------------
//   max
//---------------------------------------------------------

float SyntiParameter::max() const
      {
      return d->_max;
      }

//---------------------------------------------------------
//   setRange
//---------------------------------------------------------

void SyntiParameter::setRange(float a, float b)
      {
      d->_min = a;
      d->_max = b;
      }

//---------------------------------------------------------
//   operator==
//---------------------------------------------------------

bool SyntiParameter::operator==(const SyntiParameter& sp) const
      {
      return d->operator==(*sp.d);
      }

//---------------------------------------------------------
//   print
//---------------------------------------------------------

void SyntiParameter::print() const
      {
      d->print();
      }

