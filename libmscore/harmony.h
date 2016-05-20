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
#include "pitchspelling.h"

namespace Ms {

struct ChordDescription;
class ParsedChord;

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
///    root note and bass note are notated as "tonal pitch class":
///   <table>
///         <tr><td>&nbsp;</td><td>bb</td><td> b</td><td> -</td><td> #</td><td>##</td></tr>
///         <tr><td>C</td>     <td> 0</td><td> 7</td><td>14</td><td>21</td><td>28</td></tr>
///         <tr><td>D</td>     <td> 2</td><td> 9</td><td>16</td><td>23</td><td>30</td></tr>
///         <tr><td>E</td>     <td> 4</td><td>11</td><td>18</td><td>25</td><td>32</td></tr>
///         <tr><td>F</td>     <td>-1</td><td> 6</td><td>13</td><td>20</td><td>27</td></tr>
///         <tr><td>G</td>     <td> 1</td><td> 8</td><td>15</td><td>22</td><td>29</td></tr>
///         <tr><td>A</td>     <td> 3</td><td>10</td><td>17</td><td>24</td><td>31</td></tr>
///         <tr><td>B</td>     <td> 5</td><td>12</td><td>19</td><td>26</td><td>33</td></tr></table>
//
//   @P baseTpc   int   bass note as "tonal pitch class"
//   @P id        int   harmony identifier
//   @P rootTpc   int   root note as "tonal pitch class"
//---------------------------------------------------------

struct RenderAction;
class HDegree;

class Harmony : public Text {
      Q_OBJECT
      Q_PROPERTY(int baseTpc  READ baseTpc  WRITE setBaseTpc)
      Q_PROPERTY(int id  READ id  WRITE setId)
      Q_PROPERTY(int rootTpc  READ rootTpc  WRITE setRootTpc)

      int _rootTpc;                       // root note for chord
      int _baseTpc;                       // bass note or chord base; used for "slash" chords
                                          // or notation of base note in chord
      int _id;                            // >0 = id of matched chord from chord list, if applicable
                                          // -1 = invalid chord
                                          // <-10000 = private id of generated chord or matched chord with no id
      QString _userName;                  // name as typed by user if applicable
      QString _textName;                  // name recognized from chord list, read from score file, or constructed from imported source
      ParsedChord* _parsedForm;           // parsed form of chord

      QList<HDegree> _degreeList;
      QList<QFont> fontList;              // temp values used in render()
      QList<TextSegment*> textList;       // rendered chord

      bool _leftParen, _rightParen;       // include opening and/or closing parenthesis

      mutable QRectF _tbbox;

      NoteSpellingType _rootSpelling, _baseSpelling;
      NoteCaseType _rootCase, _baseCase;              // case as typed
      NoteCaseType _rootRenderCase, _baseRenderCase;  // case to render

      void determineRootBaseSpelling();
      virtual void draw(QPainter*) const override;
      void render(const QString&, qreal&, qreal&);
      void render(const QList<RenderAction>& renderList, qreal&, qreal&, int tpc, NoteSpellingType noteSpelling = NoteSpellingType::STANDARD, NoteCaseType noteCase = NoteCaseType::AUTO);
      virtual void styleChanged() override     { render(); }
      virtual void setTextStyle(const TextStyle& st) override;

   public:
      Harmony(Score* = 0);
      Harmony(const Harmony&);
      ~Harmony();
      virtual Harmony* clone() const override     { return new Harmony(*this); }
      virtual Element::Type type() const override { return Element::Type::HARMONY; }
      virtual bool systemFlag() const override    { return false;  }

      void setId(int d)                        { _id = d; }
      int id() const                           { return _id;           }

      bool leftParen() const                   { return _leftParen;    }
      bool rightParen() const                  { return _rightParen;   }
      void setLeftParen(bool leftParen)        { _leftParen = leftParen; }
      void setRightParen(bool rightParen)      { _rightParen = rightParen; }

      const ChordDescription* descr() const;
      const ChordDescription* descr(const QString&, const ParsedChord* pc = 0) const;
      const ChordDescription* getDescription();
      const ChordDescription* getDescription(const QString&, const ParsedChord* pc = 0);
      const ChordDescription* generateDescription();

      void determineRootBaseSpelling(NoteSpellingType& rootSpelling, NoteCaseType& rootCase,
         NoteSpellingType& baseSpelling, NoteCaseType& baseCase);

      virtual void textChanged() override;
      virtual void layout() override;

      const QRectF& bboxtight() const          { return _tbbox;        }
      QRectF& bboxtight()                      { return _tbbox;        }
      void setbboxtight(const QRectF& r) const { _tbbox = r;           }

      virtual bool isEditable() const override { return true; }
      virtual void startEdit(MuseScoreView*, const QPointF&) override;
      virtual bool edit(MuseScoreView*, Grip, int key, Qt::KeyboardModifiers, const QString& s) override;
      virtual void endEdit() override;

      QString hUserName() const                { return _userName;     }
      QString hTextName() const                { return _textName;     }
      int baseTpc() const                      { return _baseTpc;      }
      void setBaseTpc(int val)                 { _baseTpc = val;       }
      int rootTpc() const                      { return _rootTpc;      }
      void setRootTpc(int val)                 { _rootTpc = val;       }
      void setTextName(const QString& s)       { _textName = s;        }
      QString rootName();
      QString baseName();
      void addDegree(const HDegree& d);
      int numberOfDegrees() const;
      HDegree degree(int i) const;
      void clearDegrees();
      const QList<HDegree>& degreeList() const;
      const ParsedChord* parsedForm();

      virtual void write(Xml& xml) const override;
      virtual void read(XmlReader&) override;
      QString harmonyName() const;
      void render(const TextStyle* ts = 0);

      const ChordDescription* parseHarmony(const QString& s, int* root, int* base, bool syntaxOnly = false);

      const QString& extensionName() const;

      QString xmlKind() const;
      QString xmlText() const;
      QString xmlSymbols() const;
      QString xmlParens() const;
      QStringList xmlDegrees() const;

      void resolveDegreeList();

      virtual qreal baseLine() const override;

      const ChordDescription* fromXml(const QString&, const QString&, const QString&, const QString&, const QList<HDegree>&);
      const ChordDescription* fromXml(const QString& s, const QList<HDegree>&);
      const ChordDescription* fromXml(const QString& s);
      virtual void spatiumChanged(qreal oldValue, qreal newValue) override;
      virtual void localSpatiumChanged(qreal oldValue, qreal newValue) override;
      void setHarmony(const QString& s);
      virtual QPainterPath outline() const override;
      void calculateBoundingRect();

      virtual QString accessibleInfo() const override;
      virtual QString screenReaderInfo() const override;
      };


}     // namespace Ms
#endif

