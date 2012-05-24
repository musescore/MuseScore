//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: text.h 5500 2012-03-28 16:28:26Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __TEXT_H__
#define __TEXT_H__

#include "elementlayout.h"
#include "simpletext.h"

class MuseScoreView;
class TextProp;

struct SymCode;

//---------------------------------------------------------
//   Text
//---------------------------------------------------------

class Text : public SimpleText {
      QTextDocument* _doc;
      int _styleIndex;

      void createDoc();
      void setUnstyledText(const QString& s);
      void layoutEdit();

      void* pStyleIndex() { return &_styleIndex; }

   protected:
      bool _editMode;
      QTextCursor* _cursor;
      bool setCursor(const QPointF& p, QTextCursor::MoveMode mm = QTextCursor::MoveAnchor);

   public:
      Text(Score*);
      Text(const Text&);
      ~Text();

      Text &operator=(const Text&);
      virtual Text* clone() const         { return new Text(*this); }
      virtual ElementType type() const    { return TEXT; }

      void setText(const QString& s);
      void setText(const QTextDocumentFragment&);
      void setHtml(const QString& s);

      QString getText() const;
      QString getHtml() const;
      QTextDocumentFragment getFragment() const;

      bool sizeIsSpatiumDependent() const;
      void setSizeIsSpatiumDependent(int v);
      void setFrameWidth(qreal val);
      void setPaddingWidth(qreal val);
      void setFrameColor(const QColor& val);
      void setFrameRound(int val);
      void setCircle(bool val);
      void setHasFrame(bool);
      qreal xoff() const;
      qreal yoff() const;
      Align align() const;
      OffsetType offsetType() const;
      QPointF reloff() const;
      void setAlign(Align val);
      void setXoff(qreal val);
      void setYoff(qreal val);
      void setOffsetType(OffsetType val);
      void setRxoff(qreal v);
      void setRyoff(qreal v);
      void setReloff(const QPointF&);
      QFont font() const;
      void setFont(const QFont&);
      void setItalic(bool);
      void setBold(bool);
      void setSize(qreal);

      virtual void draw(QPainter*) const;

      virtual void startEdit(MuseScoreView*, const QPointF&);
      virtual bool edit(MuseScoreView*, int grip, int key, Qt::KeyboardModifiers, const QString&);
      QTextCursor* startCursorEdit();
      void endCursorEdit();
      virtual void endEdit();
      void addSymbol(const SymCode&, QTextCursor* c = 0);
      void addChar(int code, QTextCursor* cur = 0);
      void setBlockFormat(const QTextBlockFormat&);
      virtual void write(Xml& xml) const;
      virtual void read(const QDomElement&);
      void writeProperties(Xml& xml, bool writeText = true) const;
      bool readProperties(const QDomElement& node);
      virtual void layout();
      virtual QPainterPath shape() const;
      virtual bool mousePress(const QPointF&, QMouseEvent* ev);
      qreal lineSpacing() const;
      qreal lineHeight() const;
      void moveCursorToEnd();
      void moveCursor(int val);

      virtual QLineF dragAnchor() const;

      void setAbove(bool val);
      virtual qreal baseLine() const;
      virtual void paste();

      bool replaceSpecialChars();
      virtual void spatiumChanged(qreal oldValue, qreal newValue);

      void dragTo(const QPointF&p);
      bool editMode() const               { return _editMode; }

      bool styled() const                 { return _styleIndex != -1; }
      int textStyleType() const           { return _styleIndex; }
      void setTextStyleType(int);
      void setUnstyled();

      bool isEmpty() const;
      void setModified(bool v);
      void clear();
      QRectF pageRectangle() const;
      virtual void styleChanged();
      virtual void setScore(Score* s);
      friend class TextProperties;

      virtual void textChanged()          {}

      QTextCursor* cursor()               { return _cursor; }
      QTextDocument* doc() const          { return _doc;    }

      virtual bool systemFlag() const;

      static Property<Text> propertyList[];

      Property<Text>* property(P_ID id) const;
      QVariant getProperty(P_ID propertyId) const;
      bool setProperty(P_ID propertyId, const QVariant& v);
      };

#endif
