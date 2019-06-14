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
class SynthesizerState;

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
      bool _isDefault        { true };

   public:
      SynthesizerState(std::initializer_list<SynthesizerGroup> l) {
            insert(end(), l.begin(), l.end());
            }
      SynthesizerState() : std::list<SynthesizerGroup>() {}

      void write(XmlWriter&, bool force = false) const;
      void read(XmlReader&);
      SynthesizerGroup group(const QString& name) const;
      bool isDefaultSynthSoundfont();
      int ccToUse() const;
      int method() const;
      bool isDefault() const        { return _isDefault; }
      void setIsDefault(bool val)   { _isDefault = val; }
      };

//---------------------------------------------------------
//   default buildin SynthesizerState
//    used if synthesizer.xml does not exist or is not
//    readable
//---------------------------------------------------------

static SynthesizerState defaultState = {
      { "master", {
            { 0, "Zita1" },
            { 2, "0.1"   },
            { 3, "440"   },
            { 4, "1"     },
            { 5, "1"     }
            },
            },
      { "Fluid", {
            { 0, "MuseScore_General.sf3" },
            },
            },
//      { "Zerberus", {
//            { 0, "SalamanderGrandPiano.sfz" },
//            },
//            },
      };

}     // namespace Ms
#endif

