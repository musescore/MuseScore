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

#ifndef __HARMONY_H__
#define __HARMONY_H__

#include "text.h"

namespace Ms {

struct ChordDescription;

//---------------------------------------------------------
//   TextSegment
//---------------------------------------------------------

struct TextSegment {
      QFont font;
      QString text;
      qreal x, y;
      bool select;

      qreal width() const;
      QRectF boundingRect() const;
      QRectF tightBoundingRect() const;

      TextSegment()                { select = false; x = y = 0.0; }
      TextSegment(const QFont& f, qreal _x, qreal _y) : font(f), x(_x), y(_y), select(false) {}
      TextSegment(const QString&, const QFont&, qreal x, qreal y);
      void set(const QString&, const QFont&, qreal x, qreal y);
      void setText(const QString& t)      { text = t; }
      };

//---------------------------------------------------------
//   @@ Harmony
//   @P id int          harmony identifier
//   @P rootTpc int     root note as "tonal pitch class"
//   @P baseTpc int     bass note as "tonal pitch class"
//
///    root note and bass note are notatated as
///    "tonal pitch class":
///
///           bb   b   -   #  ##
///            0,  7, 14, 21, 28,  // C
///            2,  9, 16, 23, 30,  // D
///            4, 11, 18, 25, 32,  // E
///           -1,  6, 13, 20, 27,  // F
///            1,  8, 15, 22, 29,  // G
///            3, 10, 17, 24, 31,  // A
///            5, 12, 19, 26, 33,  // B
//---------------------------------------------------------

struct RenderAction;
class HDegree;

class Harmony : public Text {
      Q_OBJECT
      Q_PROPERTY(int id  READ id  WRITE setId)
      Q_PROPERTY(int rootTpc  READ rootTpc  WRITE setRootTpc)
      Q_PROPERTY(int baseTpc  READ baseTpc  WRITE setBaseTpc)

      int _rootTpc;                       // root note for chord
      int _baseTpc;                       // bass note or chord base; used for "slash" chords
                                          // or notation of base note in chord
      int _id;
      QString _userName;

      QList<HDegree> _degreeList;
      QList<QFont> fontList;              // temp values used in render()
      QList<TextSegment*> textList;       // rendered chord

      mutable QRectF _tbbox;

      virtual void draw(QPainter*) const;
      void render(const QList<RenderAction>& renderList, qreal&, qreal&, int tpc);

   public:
      Harmony(Score* = 0);
      Harmony(const Harmony&);
      ~Harmony();
      virtual Harmony* clone() const           { return new Harmony(*this); }
      virtual ElementType type() const         { return HARMONY; }

      void setId(int d)                        { _id = d; }
      int id() const                           { return _id;           }

      const ChordDescription* descr() const;

      virtual void layout();

      const QRectF& bboxtight() const          { return _tbbox;        }
      QRectF& bboxtight()                      { return _tbbox;        }
      void setbboxtight(const QRectF& r) const { _tbbox = r;           }

      virtual bool isEditable() const { return true; }
      virtual void startEdit(MuseScoreView*, const QPointF&);
      virtual bool edit(MuseScoreView*, int grip, int key, Qt::KeyboardModifiers, const QString& s);
      virtual void endEdit();

      QString hUserName() const                { return _userName;     }
      int baseTpc() const                      { return _baseTpc;      }
      void setBaseTpc(int val)                 { _baseTpc = val;       }
      int rootTpc() const                      { return _rootTpc;      }
      void setRootTpc(int val)                 { _rootTpc = val;       }
      void addDegree(const HDegree& d);
      int numberOfDegrees() const;
      HDegree degree(int i) const;
      void clearDegrees();
      const QList<HDegree>& degreeList() const;

      virtual void write(Xml& xml) const;
      virtual void read(XmlReader&);
      QString harmonyName() const;
      void render(const TextStyle* ts = 0);

      bool parseHarmony(const QString& s, int* root, int* base);

      // extension name is used by MusicXml export as <kind text="name">xmlKind</>

      QString extensionName() const;
      QString xmlKind() const;
      QStringList xmlDegrees() const;

      void resolveDegreeList();

      virtual bool isEmpty() const;
      virtual qreal baseLine() const;

      const ChordDescription* fromXml(const QString& s,  const QList<HDegree>&);
      const ChordDescription* fromXml(const QString& s);
      virtual void spatiumChanged(qreal oldValue, qreal newValue);
      virtual QLineF dragAnchor() const;
      void setHarmony(const QString& s);
      virtual QPainterPath shape() const;
      };


}     // namespace Ms
#endif

