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
      Score* _score;
      QString _title;
      QList<Part*> _parts;

   public:
      Excerpt(Score* s)               { _score = s; }
      QList<Part*>& parts()           { return _parts; }
      Score* score() const            { return _score;  }
      void setScore(Score* s)         { _score = s; }

      void read(XmlReader&);

      bool operator!=(const Excerpt&) const;
      QString title() const           { return _title; }
      void setTitle(const QString& s) { _title = s; }
      };

extern void createExcerpt(Score*, const QList<Part*>&);
extern void cloneStaves(Score* oscore, Score* score, const QList<int>& map);
extern void cloneStaff(Staff* ostaff, Staff* nstaff);
extern void cloneStaff2(Staff* ostaff, Staff* nstaff, int stick, int etick);


}     // namespace Ms
#endif

