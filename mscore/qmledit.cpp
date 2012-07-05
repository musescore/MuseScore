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
//
//  Syntax highlighter based on example code from Ariya Hidayat
//  (git://gitorious.org/ofi-labs/x2.git BSD licensed).
//=============================================================================

#include "qmledit.h"

//---------------------------------------------------------
//   JSHighlighter
//---------------------------------------------------------

JSHighlighter::JSHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
    , m_markCaseSensitivity(Qt::CaseInsensitive)
      {
      // default color scheme
      m_colors[QmlEdit::Normal]     = QColor("#000000");
      m_colors[QmlEdit::Comment]    = QColor("#808080");
      m_colors[QmlEdit::Number]     = QColor("#008000");
      m_colors[QmlEdit::String]     = QColor("#800000");
      m_colors[QmlEdit::Operator]   = QColor("#808000");
      m_colors[QmlEdit::Identifier] = QColor("#000020");
      m_colors[QmlEdit::Keyword]    = QColor("#000080");
      m_colors[QmlEdit::BuiltIn]    = QColor("#008080");
      m_colors[QmlEdit::Marker]     = QColor("#ffff00");

      // https://developer.mozilla.org/en/JavaScript/Reference/Reserved_Words

      static const char* data1[] = { "break", "case", "catch", "continue",
         "default", "delete", "do", "else", "finally", "for", "function",
         "if", "in", "instanceof", "new", "return", "switch", "this", "throw",
         "try", "typeof", "var", "void", "while", "with", "true", "false",
         "null" };

      for (unsigned int i = 0; i < sizeof(data1)/sizeof(*data1); ++i)
            m_keywords.insert(data1[i]);

      // built-in and other popular objects + properties

      static const char* data2[] = { "Object", "prototype", "create",
         "defineProperty", "defineProperties", "getOwnPropertyDescriptor",
         "keys", "getOwnPropertyNames", "constructor", "__parent__", "__proto__",
         "__defineGetter__", "__defineSetter__", "eval", "hasOwnProperty",
         "isPrototypeOf", "__lookupGetter__", "__lookupSetter__", "__noSuchMethod__",
         "propertyIsEnumerable", "toSource", "toLocaleString", "toString",
         "unwatch", "valueOf", "watch", "Function", "arguments", "arity", "caller",
         "constructor", "length", "name", "apply", "bind", "call", "String",
         "fromCharCode", "length", "charAt", "charCodeAt", "concat", "indexOf",
         "lastIndexOf", "localCompare", "match", "quote", "replace", "search",
         "slice", "split", "substr", "substring", "toLocaleLowerCase",
         "toLocaleUpperCase", "toLowerCase", "toUpperCase", "trim", "trimLeft",
         "trimRight", "Array", "isArray", "index", "input", "pop", "push",
         "reverse", "shift", "sort", "splice", "unshift", "concat", "join",
         "filter", "forEach", "every", "map", "some", "reduce", "reduceRight",
         "RegExp", "global", "ignoreCase", "lastIndex", "multiline", "source",
         "exec", "test", "JSON", "parse", "stringify", "decodeURI",
         "decodeURIComponent", "encodeURI", "encodeURIComponent", "eval",
         "isFinite", "isNaN", "parseFloat", "parseInt", "Infinity", "NaN",
         "undefined", "Math", "E", "LN2", "LN10", "LOG2E", "LOG10E", "PI",
         "SQRT1_2", "SQRT2", "abs", "acos", "asin", "atan", "atan2", "ceil",
         "cos", "exp", "floor", "log", "max", "min", "pow", "random", "round",
         "sin", "sqrt", "tan", "document", "window", "navigator", "userAgent"
         };

      for (unsigned int i = 0; i < sizeof(data2)/sizeof(*data2); ++i)
            m_knownIds.insert(data2[i]);
      }

//---------------------------------------------------------
//   setColor
//---------------------------------------------------------

void JSHighlighter::setColor(QmlEdit::ColorComponent component, const QColor& color)
      {
      m_colors[component] = color;
      rehighlight();
      }

//---------------------------------------------------------
//   highlightBlock
//---------------------------------------------------------

void JSHighlighter::highlightBlock(const QString& text)
      {
      // parsing state
      enum {
            Start = 0,
            Number = 1,
            Identifier = 2,
            String = 3,
            Comment = 4,
            Regex = 5
            };

      QList<int> bracketPositions;
      int blockState = previousBlockState();
      int bracketLevel = blockState >> 4;
      int state = blockState & 15;
      if (blockState < 0) {
            bracketLevel = 0;
            state = Start;
            }
      int start = 0;
      int i = 0;
      while (i <= text.length()) {
            QChar ch = (i < text.length()) ? text.at(i) : QChar();
            QChar next = (i < text.length() - 1) ? text.at(i + 1) : QChar();
            switch (state) {
                  case Start:
                        start = i;
                        if (ch.isSpace()) {
                              ++i;
                              }
                        else if (ch.isDigit()) {
                              ++i;
                              state = Number;
                              }
                        else if (ch.isLetter() || ch == '_') {
                              ++i;
                              state = Identifier;
                              }
                        else if (ch == '\'' || ch == '\"') {
                              ++i;
                              state = String;
                              }
                        else if (ch == '/' && next == '*') {
                              ++i;
                              ++i;
                              state = Comment;
                              }
                        else if (ch == '/' && next == '/') {
                              i = text.length();
                              setFormat(start, text.length(), m_colors[QmlEdit::Comment]);
                              }
                        else if (ch == '/' && next != '*') {
                              ++i;
                              state = Regex;
                              }
                        else {
                              if (!QString("(){}[]").contains(ch))
                                    setFormat(start, 1, m_colors[QmlEdit::Operator]);
                              if (ch =='{' || ch == '}') {
                                    bracketPositions += i;
                                    if (ch == '{')
                                          bracketLevel++;
                                    else
                                          bracketLevel--;
                                    }
                              ++i;
                              state = Start;
                              }
                        break;
                  case Number:
                        if (ch.isSpace() || !ch.isDigit()) {
                              setFormat(start, i - start, m_colors[QmlEdit::Number]);
                              state = Start;
                              }
                        else {
                              ++i;
                              }
                        break;
                  case Identifier:
                        if (ch.isSpace() || !(ch.isDigit() || ch.isLetter() || ch == '_')) {
                              QString token = text.mid(start, i - start).trimmed();
                              if (m_keywords.contains(token))
                                    setFormat(start, i - start, m_colors[QmlEdit::Keyword]);
                              else if (m_knownIds.contains(token))
                                    setFormat(start, i - start, m_colors[QmlEdit::BuiltIn]);
                              state = Start;
                              }
                        else {
                              ++i;
                              }
                        break;
                  case String:
                        if (ch == text.at(start)) {
                              QChar prev = (i > 0) ? text.at(i - 1) : QChar();
                              if (prev != '\\') {
                                    ++i;
                                    setFormat(start, i - start, m_colors[QmlEdit::String]);
                                    state = Start;
                                    }
                              else {
                                    ++i;
                                    }
                              }
                        else {
                              ++i;
                              }
                        break;
                  case Comment:
                        if (ch == '*' && next == '/') {
                              ++i;
                              ++i;
                              setFormat(start, i - start, m_colors[QmlEdit::Comment]);
                              state = Start;
                              }
                        else {
                              ++i;
                              }
                        break;
                  case Regex:
                        if (ch == '/') {
                              QChar prev = (i > 0) ? text.at(i - 1) : QChar();
                              if (prev != '\\') {
                                    ++i;
                                    setFormat(start, i - start, m_colors[QmlEdit::String]);
                                    state = Start;
                                    }
                              else {
                                    ++i;
                                    }
                              }
                        else {
                              ++i;
                              }
                        break;
                  default:
                        state = Start;
                        break;
                  }
            }

      if (state == Comment)
            setFormat(start, text.length(), m_colors[QmlEdit::Comment]);
      else
            state = Start;

      if (!m_markString.isEmpty()) {
            int pos = 0;
            int len = m_markString.length();
            QTextCharFormat markerFormat;
            markerFormat.setBackground(m_colors[QmlEdit::Marker]);
            markerFormat.setForeground(m_colors[QmlEdit::Normal]);
            for (;;) {
                  pos = text.indexOf(m_markString, pos, m_markCaseSensitivity);
                  if (pos < 0)
                        break;
                  setFormat(pos, len, markerFormat);
                        ++pos;
                  }
            }
      if (!bracketPositions.isEmpty()) {
            JSBlockData *blockData = reinterpret_cast<JSBlockData*>(currentBlock().userData());
            if (!blockData) {
                  blockData = new JSBlockData;
                  currentBlock().setUserData(blockData);
                  }
            blockData->bracketPositions = bracketPositions;
            }
      blockState = (state & 15) | (bracketLevel << 4);
      setCurrentBlockState(blockState);
      }

//---------------------------------------------------------
//   mark
//---------------------------------------------------------

void JSHighlighter::mark(const QString &str, Qt::CaseSensitivity caseSensitivity)
      {
      m_markString = str;
      m_markCaseSensitivity = caseSensitivity;
      rehighlight();
      }

//---------------------------------------------------------
//   keywords
//---------------------------------------------------------

QStringList JSHighlighter::keywords() const
      {
      return m_keywords.toList();
      }

//---------------------------------------------------------
//   setKeywords
//---------------------------------------------------------

void JSHighlighter::setKeywords(const QStringList &keywords)
      {
      m_keywords = QSet<QString>::fromList(keywords);
      rehighlight();
      }

//---------------------------------------------------------
//   QmlEdit
//---------------------------------------------------------

QmlEdit::QmlEdit(QWidget* parent)
   : QPlainTextEdit(parent)
      {
      setTabStopWidth(6);

      QPalette p = palette();
      p.setColor(QPalette::Text, Qt::black);
      p.setColor(QPalette::Base, QColor(0xe0, 0xe0, 0xe0));
      setPalette(p);
      hl = new JSHighlighter(document());
      lineNumberArea = new LineNumberArea(this);

      QAction* a = new QAction("gotoBeginLine", this);
      a->addShortcut(QShortcut(Qt::CTRL + Qt::Key_Q));
      addAction(a);

      connect(this, SIGNAL(blockCountChanged(int)),   SLOT(updateLineNumberAreaWidth(int)));
      connect(this, SIGNAL(updateRequest(QRect,int)), SLOT(updateLineNumberArea(QRect,int)));
      connect(this, SIGNAL(cursorPositionChanged()),  SLOT(highlightCurrentLine()));

      updateLineNumberAreaWidth(0);
      highlightCurrentLine();
      }

//---------------------------------------------------------
//   QmlEdit
//---------------------------------------------------------

QmlEdit::~QmlEdit()
      {
      delete hl;
      }

//---------------------------------------------------------
//   lineNumberAreaWidth
//---------------------------------------------------------

int QmlEdit::lineNumberAreaWidth()
      {
      int digits = 1;
      int max = qMax(1, blockCount());
      while (max >= 10) {
            max /= 10;
            ++digits;
            }
      int space = 6 + fontMetrics().width(QLatin1Char('9')) * digits;
      return space;
      }

//---------------------------------------------------------
//   updateLineNumberAreaWidth
//---------------------------------------------------------

void QmlEdit::updateLineNumberAreaWidth(int /* newBlockCount */)
      {
      setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
      }

//---------------------------------------------------------
//   updateLineNumberArea
//---------------------------------------------------------

void QmlEdit::updateLineNumberArea(const QRect& rect, int dy)
      {
      if (dy)
            lineNumberArea->scroll(0, dy);
      else
            lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

      if (rect.contains(viewport()->rect()))
            updateLineNumberAreaWidth(0);
      }

//---------------------------------------------------------
//   resizeEvent
//---------------------------------------------------------

void QmlEdit::resizeEvent(QResizeEvent *e)
      {
      QPlainTextEdit::resizeEvent(e);

      QRect cr = contentsRect();
      lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
      }

//---------------------------------------------------------
//   highlightCurrentLine
//---------------------------------------------------------

void QmlEdit::highlightCurrentLine()
      {
      QList<QTextEdit::ExtraSelection> extraSelections;

      if (!isReadOnly()) {
            QTextEdit::ExtraSelection selection;

            QColor lineColor = QColor(Qt::white);

            selection.format.setBackground(lineColor);
            selection.format.setProperty(QTextFormat::FullWidthSelection, true);
            selection.cursor = textCursor();
            selection.cursor.clearSelection();
            extraSelections.append(selection);
            }

      setExtraSelections(extraSelections);
      }

//---------------------------------------------------------
//   lineNumberAreaPaintEvent
//---------------------------------------------------------

void QmlEdit::lineNumberAreaPaintEvent(QPaintEvent *event)
      {
      QPainter painter(lineNumberArea);
      painter.fillRect(event->rect(), Qt::lightGray);

      QTextBlock block = firstVisibleBlock();
      int blockNumber = block.blockNumber();
      int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
      int bottom = top + (int) blockBoundingRect(block).height();

      int w = lineNumberArea->width();
      int h = fontMetrics().height();
      while (block.isValid() && top <= event->rect().bottom()) {
            if (block.isVisible() && bottom >= event->rect().top()) {
                  QString number = QString::number(blockNumber + 1);
                  painter.setPen(Qt::black);
                  painter.drawText(0, top, w-3, h, Qt::AlignRight, number);
                  }

            block = block.next();
            top   = bottom;
            bottom = top + (int) blockBoundingRect(block).height();
            ++blockNumber;
            }
       }
