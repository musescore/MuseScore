//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __WAVEVIEW_H__
#define __WAVEVIEW_H__

#include "libmscore/pos.h"

namespace Ms {

class Audio;
class Score;

//---------------------------------------------------------
//   WaveView
//---------------------------------------------------------

class WaveView : public QWidget
      {
      Q_OBJECT
      Pos _cursor;
      Pos* _locator;
      Score* _score;
      QByteArray waves;

      TType _timeType;
      int magStep;
      double _xmag;
      int _xpos;

      int pos2pix(const Pos& p) const;
      Pos pix2pos(int x) const;
      virtual void paintEvent(QPaintEvent*);
      virtual QSize sizeHint() const { return QSize(50, 50); }
      int pegel(int frame1, int frame2);

   public slots:
      void setMag(double,double);
      void moveLocator(int);

   public:
      WaveView(QWidget* parent = 0);
      void setAudio(Audio*);
      void setXpos(int);
      void setScore(Score* s, Pos* lc);
      };
}

#endif

