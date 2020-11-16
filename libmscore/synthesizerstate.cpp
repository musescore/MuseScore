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

#include "synthesizerstate.h"
#include "xml.h"

namespace Ms {

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void SynthesizerState::write(XmlWriter& xml, bool force /* = false */) const
      {
      if (isDefault() && !force)
            return;

      xml.stag("Synthesizer");
      for (const SynthesizerGroup& g : *this) {
            if (!g.name().isEmpty()) {
                  xml.stag(g.name());
                  for (const IdValue& v : g)
                        xml.tag(QString("val id=\"%1\"").arg(v.id), v.data);
                  xml.etag();
                  }
            }
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void SynthesizerState::read(XmlReader& e)
      {
      std::list<SynthesizerGroup> tempGroups;
      while (e.readNextStartElement()) {
            SynthesizerGroup group;
            group.setName(e.name().toString());

            while (e.readNextStartElement()) {
                  if (e.name() == "val") {
                        int id = e.intAttribute("id");
                        group.push_back(IdValue(id, e.readElementText()));
                        }
                  else
                        e.unknown();
                  }
            tempGroups.push_back(group);
            }

      if (!tempGroups.empty()) {
            // Replace any previously set state if we have read a new state
            swap(tempGroups);
            setIsDefault(false);
            }
      }

//---------------------------------------------------------
//   group
///  Get SynthesizerGroup by name
//---------------------------------------------------------

SynthesizerGroup SynthesizerState::group(const QString& name) const
      {
      for (const SynthesizerGroup& g : *this) {
            if (g.name() == name)
                  return g;
            }
      SynthesizerGroup sg;
      return sg;
      }

//---------------------------------------------------------
//   isDefaultSynthSoundfont
///  check if synthesizer state uses default synth and
///  default font only
//---------------------------------------------------------

bool SynthesizerState::isDefaultSynthSoundfont()
      {
      SynthesizerGroup fluid = group("Fluid");
      SynthesizerGroup zerberus = group("Zerberus");
      if (zerberus.size() == 0 && fluid.size() == 1) {
            if (fluid.front().data == "MuseScore_General.sf3")
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   ccToUse
//---------------------------------------------------------

int SynthesizerState::ccToUse() const
      {
      SynthesizerGroup g = group("master");

      int method = 1;
      int cc = -1;

      for (const IdValue &idVal : g) {
            if (idVal.id == 4)
                  method = idVal.data.toInt();
            else if (idVal.id == 5) {
                  switch (idVal.data.toInt()) {
                        case 0:
                              cc = 1;
                              break;
                        case 1:
                              cc = 2;
                              break;
                        case 2:
                              cc = 4;
                              break;
                        case 3:
                              cc = 11;
                              break;
                        default:
                              qWarning("Unrecognised CCToUse index from synthesizer: %d", idVal.data.toInt());
                        }
                  }
            }

      if (method == 0)        // velocity only
            return -1;

      return cc;
      }

//---------------------------------------------------------
//   method
//---------------------------------------------------------

int SynthesizerState::method() const
      {
      SynthesizerGroup g = group("master");

      int method = -1;

      for (const IdValue &idVal : g) {
            if (idVal.id == 4) {
                  method = idVal.data.toInt();
                  break;
                  }
            }

      return method;
      }

}

