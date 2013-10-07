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
#include "musescore.h"

namespace Ms {

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
//   Binding
//---------------------------------------------------------

struct Binding {
      const char* name;
      int key1, key2;
      const char* slot;
      };

//---------------------------------------------------------
//   QmlEdit
//---------------------------------------------------------

QmlEdit::QmlEdit(QWidget* parent)
   : QPlainTextEdit(parent)
      {
      setBackgroundVisible(true);
      setLineWrapMode(QPlainTextEdit::NoWrap);
      QFont font("FreeMono", 12);
      font.setStyleHint(QFont::TypeWriter);
      font.setFixedPitch(true);
      setFont(font);
      document()->setDefaultFont(font);

      QTextCursor c = textCursor();
      QTextCharFormat cf = c.charFormat();
      cf.setFont(font);
      c.setCharFormat(cf);
      setTextCursor(c);

      static const Binding bindings[] = {
#if 0
            { "start",       Qt::CTRL+Qt::Key_Q, Qt::CTRL+Qt::Key_E, SLOT(start()) },
            { "end",         Qt::CTRL+Qt::Key_Q, Qt::CTRL+Qt::Key_X, SLOT(end()) },
            { "startOfLine", Qt::CTRL+Qt::Key_Q, Qt::CTRL+Qt::Key_S, SLOT(startOfLine()) },
            { "endOfLine",   Qt::CTRL+Qt::Key_Q, Qt::CTRL+Qt::Key_D, SLOT(endOfLine())   },
            { "up",          Qt::CTRL+Qt::Key_E, 0, SLOT(upLine())     },
            { "down",        Qt::CTRL+Qt::Key_X, 0, SLOT(downLine())   },
            { "right",       Qt::CTRL+Qt::Key_D, 0, SLOT(right())      },
            { "left",        Qt::CTRL+Qt::Key_S, 0, SLOT(left())       },
            { "rightWord",   Qt::CTRL+Qt::Key_F, 0, SLOT(rightWord())  },
            { "leftWord",    Qt::CTRL+Qt::Key_A, 0, SLOT(leftWord())   },
            { "pick",        Qt::Key_F8,         0, SLOT(pick())       },
            { "put",         Qt::Key_F9,         0, SLOT(put())        },
            { "delLine",     Qt::CTRL+Qt::Key_Y, 0, SLOT(delLine())    },
            { "delWord",     Qt::CTRL+Qt::Key_T, 0, SLOT(delWord())    }
#endif
            };
      setTabChangesFocus(false);
      setBackgroundVisible(false);
      setCursorWidth(3);

      QPalette p = palette();
      p.setColor(QPalette::Text, Qt::black);
      p.setColor(QPalette::Base, QColor(0xe0, 0xe0, 0xe0));
      setPalette(p);
      hl = new JSHighlighter(document());
      lineNumberArea = new LineNumberArea(this);

      for (unsigned int i = 0; i < sizeof(bindings)/sizeof(*bindings); ++i) {
            const Binding& b = bindings[i];
            QAction* a = new QAction(b.name, this);
            a->setShortcut(QKeySequence(b.key1, b.key2));
            a->setShortcutContext(Qt::WidgetShortcut);
            a->setPriority(QAction::HighPriority);
            addAction(a);
            connect(a, SIGNAL(triggered()), b.slot);
            }

      connect(this, SIGNAL(blockCountChanged(int)),   SLOT(updateLineNumberAreaWidth(int)));
      connect(this, SIGNAL(updateRequest(QRect,int)), SLOT(updateLineNumberArea(QRect,int)));
      connect(this, SIGNAL(cursorPositionChanged()),  SLOT(highlightCurrentLine()));

      updateLineNumberAreaWidth(0);
      highlightCurrentLine();
      }

//---------------------------------------------------------
//   focusInEvent
//---------------------------------------------------------

void QmlEdit::focusInEvent(QFocusEvent* event)
      {
      mscoreState = mscore->state();
      mscore->changeState(STATE_DISABLED);
      QPlainTextEdit::focusInEvent(event);
      }

//---------------------------------------------------------
//   focusOutEvent
//---------------------------------------------------------

void QmlEdit::focusOutEvent(QFocusEvent* event)
      {
      mscore->changeState(mscoreState);
      QPlainTextEdit::focusOutEvent(event);
      }

//---------------------------------------------------------
//   move
//---------------------------------------------------------

void QmlEdit::move(QTextCursor::MoveOperation op)
      {
      QTextCursor tc(textCursor());
      tc.movePosition(op);
      setTextCursor(tc);
      update();
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

      QFont font("FreeMono", 12);
      font.setStyleHint(QFont::TypeWriter);
      font.setFixedPitch(true);
      painter.setFont(font);

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

//---------------------------------------------------------
//   pick
//---------------------------------------------------------

void QmlEdit::pick()
      {
      pickBuffer = textCursor().block().text();
      }

//---------------------------------------------------------
//   put
//---------------------------------------------------------

void QmlEdit::put()
      {
      QTextCursor c = textCursor();
      int column = c.columnNumber();
      c.movePosition(QTextCursor::StartOfBlock);
      c.insertText(pickBuffer + "\n");
      c.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, column);
      c.movePosition(QTextCursor::Up);
      setTextCursor(c);
      }

//---------------------------------------------------------
//   delLine
//---------------------------------------------------------

void QmlEdit::delLine()
      {
      QTextCursor c = textCursor();
      c.select(QTextCursor::BlockUnderCursor);
      pickBuffer = c.selectedText().mid(1);
      c.removeSelectedText();
      c.movePosition(QTextCursor::Down);
      setTextCursor(c);
      }

//---------------------------------------------------------
//   delWord
//---------------------------------------------------------

void QmlEdit::delWord()
      {
      QTextCursor c = textCursor();
      int i = c.position();
      if (document()->characterAt(i) == QChar(' ')) {
            while(document()->characterAt(i) == QChar(' '))
                  c.deleteChar();
            }
      else {
            for (;;) {
                  QChar ch = document()->characterAt(i);
                  if (ch == QChar(' ') || ch == QChar('\n'))
                        break;
                  c.deleteChar();
                  }
            while(document()->characterAt(i) == QChar(' '))
                  c.deleteChar();
            }
      }

//---------------------------------------------------------
//   downLine
//---------------------------------------------------------

void QmlEdit::downLine()
      {
      printf("down line\n");
      move(QTextCursor::Down);
      }

//---------------------------------------------------------
//   leftWord
//---------------------------------------------------------

void QmlEdit::leftWord()
      {
      QTextCursor c = textCursor();

      if (c.positionInBlock() == 0)
            return;
      c.movePosition(QTextCursor::Left);

      bool inSpace = true;
      for (;c.positionInBlock();) {
            int i = c.position();
            if (document()->characterAt(i) == QChar(' ')) {
                  if (!inSpace) {
                        c.movePosition(QTextCursor::Right);
                        break;
                        }
                  }
            else {
                  if (inSpace)
                        inSpace = false;
                  }
            c.movePosition(QTextCursor::Left);
            }
      setTextCursor(c);
      }

//---------------------------------------------------------
//   keyPressEvent
//---------------------------------------------------------

void QmlEdit::keyPressEvent(QKeyEvent* event)
      {

      if (event->modifiers() != Qt::ControlModifier && event->key() == Qt::Key_Tab) {
            tab();
            event->accept();
            return;
            }
      QPlainTextEdit::keyPressEvent(event);
      }

//---------------------------------------------------------
//   tab
//---------------------------------------------------------

void QmlEdit::tab()
      {
      QTextCursor c = textCursor();
      c.insertText(" ");
      while (c.positionInBlock() % 6)
            c.insertText(" ");
      setTextCursor(c);
      }
}

