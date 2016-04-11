//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: mscore.cpp 4220 2011-04-22 10:31:26Z wschweer $
//
//  Copyright (C) 2011 Werner Schweer and others
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

#ifndef __ALBUM_H__
#define __ALBUM_H__

namespace Ms {

class Score;
class Xml;
class XmlReader;

//---------------------------------------------------------
//   AlbumItem
//---------------------------------------------------------

struct AlbumItem {
      QString name;
      QString path;
      Score* score { 0 };

      AlbumItem() {}
      AlbumItem(const QString& p) : path(p) {}
      ~AlbumItem();
      };

//---------------------------------------------------------
//   Album
//---------------------------------------------------------

class Album {
      bool _dirty;
      QString _path;
      QString _name;
      QList<AlbumItem*> _scores;

      void load(XmlReader&);
      void save(Xml&);
      void loadScores();

   public:
      Album();
      ~Album();

      void print();
      bool createScore(const QString& fn, bool addPageBreak = false, bool addSectionBreak = true);
      bool read(const QString& path);
      void write(Xml& xml);
      bool dirty() const             { return _dirty; }
      QString name() const           { return _name;  }
      QString path() const           { return _path;  }
      void setName(const QString&);
      void setPath(const QString&);
      QList<AlbumItem*>& scores()    { return _scores; }
      void append(AlbumItem* item);
      void remove(int);
      void swap(int, int);
      AlbumItem* item(int idx)       { return _scores[idx]; }
      void setDirty(bool val)        { _dirty = val;        }
      };
}
#endif

