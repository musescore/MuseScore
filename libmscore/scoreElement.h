//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2015 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __SCORE_ELEMENT_H__
#define __SCORE_ELEMENT_H__

namespace Ms {

class Score;
class MasterScore;
class Xml;
class ScoreElement;

enum class P_ID : int;
enum class PropertyStyle : char;
enum class StyleIdx : int;

//---------------------------------------------------------
//   LinkedElements
//---------------------------------------------------------

class LinkedElements : public QList<ScoreElement*> {
      int _lid;         // unique id for every linked list

   public:
      LinkedElements(Score*);
      LinkedElements(Score*, int id);

      void setLid(Score*, int val);
      int lid() const   { return _lid;    }
      };

//---------------------------------------------------------
//   ScoreElement
//---------------------------------------------------------

class ScoreElement {
      Score* _score;

   protected:
      LinkedElements* _links { 0 };

   public:
      ScoreElement(Score* s) : _score(s)   {}
      ScoreElement(const ScoreElement& se);
      virtual ~ScoreElement();

      Score* score() const                 { return _score;      }
      MasterScore* masterScore() const;
      virtual void setScore(Score* s)      { _score = s;         }
      virtual const char* name() const = 0;

      virtual QVariant getProperty(P_ID) const = 0;
      virtual bool setProperty(P_ID, const QVariant&) = 0;
      virtual QVariant propertyDefault(P_ID) const { return QVariant(); }
      virtual void resetProperty(P_ID id);
      virtual PropertyStyle propertyStyle(P_ID) const;
      virtual StyleIdx getPropertyStyle(P_ID) const;

      void undoChangeProperty(P_ID, const QVariant&);
      void undoPushProperty(P_ID);
      void writeProperty(Xml& xml, P_ID id) const;

      QList<ScoreElement*> linkList() const;

      void linkTo(ScoreElement*);
      void unlink();
      virtual void undoUnlink();
      int lid() const                         { return _links ? _links->lid() : 0; }
      const LinkedElements* links() const     { return _links;      }
      void setLinks(LinkedElements* le)       { _links = le;        }
      };
}

#endif

