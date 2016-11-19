//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __SYNTHESIZERSTATE_H__
#define __SYNTHESIZERSTATE_H__

#include <list>

namespace Ms {

class XmlWriter;
class XmlReader;


//---------------------------------------------------------
//   IdValue
//---------------------------------------------------------

struct IdValue {
      int id;
      QString data;

      IdValue() {}
      IdValue(int _id, const QString& _data) : id(_id), data(_data) {}
      };

//---------------------------------------------------------
//   SynthesizerGroup
//---------------------------------------------------------

class SynthesizerGroup : public std::list<IdValue> {
      QString _name;

   public:
      const QString& name() const { return _name; }
      void setName(const QString& s) { _name = s; }

      SynthesizerGroup() : std::list<IdValue>() {}
      SynthesizerGroup(const char* n, std::list<IdValue> l) : std::list<IdValue>(l), _name(n) {}
      };

//---------------------------------------------------------
//   SynthesizerState
//---------------------------------------------------------

class SynthesizerState : public std::list<SynthesizerGroup> {

   public:
      SynthesizerState(std::initializer_list<SynthesizerGroup> l) {
            insert(end(), l.begin(), l.end());
            }
      SynthesizerState() : std::list<SynthesizerGroup>() {}

      void write(XmlWriter&) const;
      void read(XmlReader&);
      };


}     // namespace Ms
#endif

