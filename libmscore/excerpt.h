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

class MasterScore;
class Score;
class Part;
class Measure;
class XmlWriter;
class Staff;
class XmlReader;

//---------------------------------------------------------
//   @@ Excerpt
//   @P partScore  Score      the score object for this part
//   @P title      string     the title of this part
//---------------------------------------------------------

#include <QMultiMap>

class Excerpt : public QObject {
      MasterScore* _oscore;

      Score* _partScore           { 0 };
      QString _title;
      QList<Part*> _parts;
      QMultiMap<int, int> _tracks;

   public:
      Excerpt(MasterScore* s = 0)          { _oscore = s;       }
      ~Excerpt();

      QList<Part*>& parts()                { return _parts;     }
      void setParts(const QList<Part*>& p) { _parts = p;        }


      QMultiMap<int, int>& tracks()                  { return _tracks;    }
      void setTracks(const QMultiMap<int, int>& t)   { _tracks = t;       }

      MasterScore* oscore() const          { return _oscore;    }
      Score* partScore() const             { return _partScore; }
      void setPartScore(Score* s);

      void read(XmlReader&);

      bool operator!=(const Excerpt&) const;
      bool operator==(const Excerpt&) const;

      QString title() const           { return _title; }
      void setTitle(const QString& s) { _title = s;    }

      static QList<Excerpt*> createAllExcerpt(MasterScore* score);
      static QString createName(const QString& partName, QList<Excerpt*>&);
      static void createExcerpt(Excerpt*);
      static void cloneStaves(Score* oscore, Score* score, const QList<int>& map, QMultiMap<int, int>& allTracks);
      static void cloneStaff(Staff* ostaff, Staff* nstaff);
      static void cloneStaff2(Staff* ostaff, Staff* nstaff, int stick, int etick);
      };

}     // namespace Ms
#endif

