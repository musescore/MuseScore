//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2010-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __REVISIONS_H__
#define __REVISIONS_H__

namespace Ms {

class XmlWriter;
class XmlReader;

//---------------------------------------------------------
//   Revision
//---------------------------------------------------------

class Revision {
      QString _id;
      QString _diff;          // diff to parent
      QDateTime _dateTime;
      Revision* _parent;
      QList<Revision*> _branches;

   public:
      Revision();
      void read(XmlReader&);
      void write(XmlWriter&) const;
      void setParent(Revision* r)              { _parent = r; }
      Revision* parent() const                 { return _parent; }
      const QList<Revision*>& branches() const { return _branches; }
      void setId(const QString& s)             { _id = s; }
      void setDiff(const QString& s)           { _diff = s; }
      };

//---------------------------------------------------------
//   Revisions
//    id:  2.3.1
//         | | +-- revision of branch
//         | +---- branch number
//         +------ revision
//---------------------------------------------------------

class Revisions {
      Revision* _trunk;

      void write(XmlWriter&, const Revision*) const;

   public:
      Revisions();
      void add(Revision*);
      QString getRevision(QString id);
      Revision* trunk() { return _trunk; }
      void write(XmlWriter&) const;
      };


}     // namespace Ms
#endif

