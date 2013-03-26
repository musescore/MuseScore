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

#ifndef __SPARM_H__
#define __SPARM_H__

class Xml;
class XmlReader;
class Synth;
class SyntiParameterData;

//---------------------------------------------------------
//   SParmId
//---------------------------------------------------------

struct SParmId {
      union {
            struct {
                  unsigned syntiId:2;
                  unsigned subsystemId:4;
                  unsigned paramId:16;
                  };
            unsigned val;
            };
      SParmId(int v) : val(v) {}
      SParmId(int a, int b, int c) {
            val         = 0;
            syntiId     = a;
            subsystemId = b;
            paramId     = c;
            }
      };

enum {
      FLUID_ID  = 0,
      AEOLUS_ID = 1,
      ZERBERUS_ID = 2,
      };

//---------------------------------------------------------
//   SyntiParameterType
//---------------------------------------------------------

enum SyntiParameterType {
      SP_FLOAT, SP_STRING
      };

//---------------------------------------------------------
//   SyntiParameter
//---------------------------------------------------------

class SyntiParameter {
   protected:
      int _id;
      QString _name;
      SyntiParameterType _type;
      float  _fval, _min, _max;
      QString _sval;

   public:
      SyntiParameter();
      SyntiParameter(const SyntiParameter&);
      SyntiParameter(const QString& name, float val);
      SyntiParameter(int id, const QString& name, float);
      SyntiParameter(int id, const QString& name, const QString& val);
      SyntiParameter(const QString& name, const QString& val);

      SyntiParameterType type() const { return _type; }

      void write(Xml&) const;

      const QString& name() const    { return _name; }
      void setName(const QString& s) { _name = s; }

      int id() const       { return _id; }
      void setId(int v)    { _id = v; }

      QString sval() const { return _sval; }
      float fval() const   { return _fval; }
      void set(const QString& s) { _sval = s; }
      void set(float v)          { _fval = v; }
      void set(const QString& s, float, float, float);
      float min() const    { return _min; }
      float max() const    { return _max; }
      void setRange(float a, float b) { _min = a; _max = b; }

      bool operator==(const SyntiParameter&) const;
      void print() const;
      };

//---------------------------------------------------------
//   SyntiState
///   List of SyntiParameter describing the state of
///   the msynth software synthesizer.
//---------------------------------------------------------

class SyntiState : public QList<SyntiParameter> {

   public:
      SyntiState() {}
      void write(Xml&) const;
      void read(XmlReader&);
      bool operator==(const SyntiState&) const;
      };

#endif

