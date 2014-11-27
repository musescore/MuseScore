//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2008-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __EXCERPT_H__
#define __EXCERPT_H__

namespace Ms {

class Score;
class Part;
class Xml;
class Staff;
class XmlReader;

//---------------------------------------------------------
//   Excerpt
//---------------------------------------------------------

class Excerpt {
      Score* _oscore;               // main score
      Score* _partScore  { 0 };
      QString _title;
      QList<Part*> _parts;

   public:
      Excerpt(Score* s)                    { _oscore = s;       }

      QList<Part*>& parts()                { return _parts;     }
      void setParts(const QList<Part*>& p) { _parts = p;        }
      Score* oscore() const                { return _oscore;    }
      void setPartScore(Score* s)          { _partScore = s;    }
      Score* partScore() const             { return _partScore; }

      void read(XmlReader&);

      bool operator!=(const Excerpt&) const;
      QString title() const           { return _title; }
      void setTitle(const QString& s) { _title = s;    }
      };

extern void createExcerpt(Excerpt*);
extern void cloneStaves(Score* oscore, Score* score, const QList<int>& map);
extern void cloneStaff(Staff* ostaff, Staff* nstaff);
extern void cloneStaff2(Staff* ostaff, Staff* nstaff, int stick, int etick);


}     // namespace Ms
#endif

