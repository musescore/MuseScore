//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __TEXTTOOLS_H__
#define __TEXTTOOLS_H__

namespace Ms {

class TextBase;
class TextCursor;
class EditData;
class ScoreView;

//---------------------------------------------------------
//   TextTools
//---------------------------------------------------------

class TextTools : public QDockWidget {
      Q_OBJECT

      TextBase* text;
      TextCursor* cursor;

      QDoubleSpinBox* typefaceSize;
      QFontComboBox* typefaceFamily;
      QAction* typefaceBold;
      QAction* typefaceItalic;
      QAction* typefaceUnderline;
      QAction* typefaceSubscript;
      QAction* typefaceSuperscript;
      QAction* showKeyboard;

      void blockAllSignals(bool val);
      void updateText();
      void layoutText();

   private slots:
      void sizeChanged(double value);
      void fontChanged(const QFont&);
      void boldClicked(bool);
      void italicClicked(bool);
      void underlineClicked(bool);
      void subscriptClicked(bool);
      void superscriptClicked(bool);
      void showKeyboardClicked(bool);

   public:
      TextTools(QWidget* parent = 0);
      void updateTools(EditData&);
      QAction* kbAction() const      { return showKeyboard; }
      void toggleBold();
      void toggleItalic();
      void toggleUnderline();
      TextBase* textElement();
      };
}

#endif

