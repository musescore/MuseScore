//=============================================================================
//  MuseSynth
//  Music Software Synthesizer
//
//  Copyright (C) 2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __SYNTHESIZERGUI_H__
#define __SYNTHESIZERGUI_H__

namespace Ms {

class Synthesizer;

//---------------------------------------------------------
//   SynthesizerGui
//---------------------------------------------------------

class SynthesizerGui : public QWidget {
      Q_OBJECT
      Synthesizer* _synthesizer;

   signals:
      void sfChanged();
      void valueChanged();

   public slots:
      virtual void synthesizerChanged() {}

   public:
      SynthesizerGui(Synthesizer*);
      Synthesizer* synthesizer() { return _synthesizer; }
      };

}
#endif

