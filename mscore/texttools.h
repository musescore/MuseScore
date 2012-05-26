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

class Text;

//---------------------------------------------------------
//   TextTools
//---------------------------------------------------------

class TextTools : public QDockWidget {
      Q_OBJECT

      Text* _textElement;

      QDoubleSpinBox* typefaceSize;
      QFontComboBox* typefaceFamily;
      QAction* typefaceBold;
      QAction* typefaceItalic;
      QAction* typefaceUnderline;
      QAction* leftAlign;
      QAction* centerAlign;
      QAction* topAlign;
      QAction* bottomAlign;
      QAction* vcenterAlign;
      QAction* rightAlign;
      QAction* typefaceSubscript;
      QAction* typefaceSuperscript;
      QAction* showKeyboard;
      QAction* unorderedList;
      QAction* orderedList;
      QAction* indentMore;
      QAction* indentLess;
      QComboBox* textStyles;

      void blockAllSignals(bool val);
      void updateText();
      void layoutText();
      QTextCursor* cursor();

   private slots:
      void sizeChanged(double value);
      void fontChanged(const QFont&);
      void boldClicked(bool);
      void italicClicked(bool);
      void underlineClicked(bool);
      void subscriptClicked(bool);
      void superscriptClicked(bool);
      void setLeftAlign();
      void setRightAlign();
      void setHCenterAlign();
      void setTopAlign();
      void setBottomAlign();
      void setVCenterAlign();
      void showKeyboardClicked(bool);
      void styleChanged(int);
      void unorderedListClicked();
      void orderedListClicked();
      void indentMoreClicked();
      void indentLessClicked();

   public:
      TextTools(QWidget* parent = 0);
      void setText(Text* te);
      void updateTools();
      QAction* kbAction() const { return showKeyboard; }
      };

#endif

