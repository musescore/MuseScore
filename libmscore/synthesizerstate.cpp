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

void SynthesizerState::write(XmlWriter& xml) const
      {
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
            push_back(group);
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

}

