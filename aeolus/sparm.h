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

#ifndef __SPARM_H__
#define __SPARM_H__

namespace Ms {
      class Xml;
      class Synth;
      }

class SyntiParameterData;

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
      QSharedDataPointer<SyntiParameterData> d;

   public:
      SyntiParameter();
      ~SyntiParameter();
      SyntiParameter(const SyntiParameter&);
      SyntiParameter& operator=(const SyntiParameter&);
      SyntiParameter(const QString& name, float val);
      SyntiParameter(int id, const QString& name, float);
      SyntiParameter(int id, const QString& name, const QString& val);
      SyntiParameter(const QString& name, const QString& val);

      SyntiParameterType type() const;

      void write(Ms::Xml&) const;

      const QString& name() const;
      void setName(const QString& s);

      int id() const;
      void setId(int v);

      QString sval() const;
      float fval() const;
      void set(const QString& s);
      void set(float v);
      void set(const QString& s, float, float, float);
      float min() const;
      float max() const;
      void setRange(float a, float b);

      bool operator==(const SyntiParameter&) const;
      void print() const;
      };

#endif

