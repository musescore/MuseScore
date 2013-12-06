//=============================================================================
//  MuseScore
//  Music Composition & Notation
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
#include "mscore.h"

namespace Ms {

class MuseScoreView;
class TextProp;

struct SymCode;

//---------------------------------------------------------
//   @@ MText
//   @P text QString
//---------------------------------------------------------

class Text : public SimpleText {
      Q_OBJECT
      Q_PROPERTY(QString text READ text WRITE undoSetText)

      QTextDocument* _doc;
      int _styleIndex;        // set to TEXT_STYLE_UNSTYLED if not styled
      static QTextCursor* _cursor;

      void createDoc();
      void setUnstyledText(const QString& s);
      void layoutEdit();
      bool isSimpleText() const;

   protected:
      bool setCursor(const QPointF& p, QTextCursor::MoveMode mm = QTextCursor::MoveAnchor);

   public:
      Text(Score* = 0);
      Text(const Text&);
      ~Text();

      virtual void draw(QPainter*) const;

      Text &operator=(const Text&);
      virtual Text* clone() const         { return new Text(*this); }
      virtual ElementType type() const    { return TEXT; }

      void setText(const QString&);
      void undoSetText(const QString&);
      void setText(const QTextDocumentFragment&);
      void setHtml(const QString& s);

      QString text() const;
      QString getHtml() const;
      QTextDocumentFragment getFragment() const;

      bool sizeIsSpatiumDependent() const;
      void setSizeIsSpatiumDependent(int v);
      void setFrameWidth(Spatium val);
      void setPaddingWidth(Spatium val);
      void setFrameColor(const QColor& val);
      void setFrameRound(int val);
      void setCircle(bool val);
      void setHasFrame(bool);
      qreal xoff() const;
      qreal yoff() const;
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

      virtual void startEdit(MuseScoreView*, const QPointF&);
      virtual bool edit(MuseScoreView*, int grip, int key, Qt::KeyboardModifiers, const QString&);
      QTextCursor* startCursorEdit();
      void endCursorEdit();
      virtual void endEdit();
      void addChar(int code);
      void setBlockFormat(const QTextBlockFormat&);
      virtual void write(Xml& xml) const;
      virtual void read(XmlReader&);
      void writeProperties(Xml& xml, bool writeText = true) const;
      bool readProperties(XmlReader& node);
      virtual void layout();
      void layout1();
      virtual QPainterPath shape() const;
      virtual bool mousePress(const QPointF&, QMouseEvent* ev);
      qreal lineSpacing() const;
      qreal lineHeight() const;
      void moveCursorToStart();
      void moveCursorToEnd();

      virtual QLineF dragAnchor() const;

      void setAbove(bool val);
      virtual qreal baseLine() const;
      virtual void paste();

      void replaceSpecialChars();
      virtual void spatiumChanged(qreal oldValue, qreal newValue);

      void dragTo(const QPointF&p);

      bool styled() const                 { return _styleIndex != TEXT_STYLE_UNSTYLED; }
      int textStyleType() const           { return _styleIndex; }
      void setTextStyleType(int);
      void setUnstyled();

      bool isEmpty() const;
      void setModified(bool v);
      void clear();
      QRectF pageRectangle() const;
      virtual void textStyleChanged();
      virtual void setScore(Score* s);
      friend class TextProperties;

      void spellCheckUnderline(bool);
      void insertText(const QString& s);
      void undo();
      void redo();
      QString selection() const;
      QFont curFont() const;
      bool curItalic() const;
      bool curBold() const;
      bool curUnderline() const;
      bool curSubscript() const;
      bool curSuperscript() const;

      void setCurFontPointSize(double value);
      void setCurFontFamily(const QString& s);
      void setCurUnderline(bool val);
      void setCurItalic(bool val);
      void setCurBold(bool val);
      void setCurSuperscript(bool val);
      void setCurSubscript(bool val);
      void setCurHalign(int val);
      void orderedList();
      void unorderedList();
      void indentLess();
      void indentMore();

      virtual bool systemFlag() const;

      virtual void textChanged()          {}

      QVariant getProperty(P_ID propertyId) const;
      bool setProperty(P_ID propertyId, const QVariant& v);
      };


}     // namespace Ms
#endif

