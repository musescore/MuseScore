//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2010 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef __TEXTEDITOR_H__
#define __TEXTEDITOR_H__

namespace Ms {

//---------------------------------------------------------
//   TextEditor
//---------------------------------------------------------

class TextEditor : public QDialog {
      Q_OBJECT
      QTextEdit* edit;
      QToolButton* typefaceBold;
      QToolButton* typefaceItalic;
      QToolButton* typefaceUnderline;
      QToolButton* leftAlign;
      QToolButton* centerAlign;
      QToolButton* rightAlign;
      QToolButton* typefaceSubscript;
      QToolButton* typefaceSuperscript;
      QDoubleSpinBox* typefaceSize;
      QFontComboBox* typefaceFamily;
      QDialogButtonBox* dialogButtons;

   private slots:
      void toggleBold(bool);
      void toggleUnderline(bool);
      void toggleItalic(bool);
      void toggleLeftAlign(bool);
      void toggleCenterAlign(bool);
      void toggleRightAlign(bool);
      void toggleTypefaceSubscript(bool);
      void toggleTypefaceSuperscript(bool);
      void charFormatChanged(const QTextCharFormat& f);
      void cursorPositionChanged();
      void sizeChanged(double);
      void fontChanged(const QFont&);
      void okClicked();
      void cancelClicked();
      void cancelEditing();

   private:
      bool editAsHtml;
      QString originalText;

   public:
      TextEditor(QWidget* parent = 0);
      QString toHtml() const;
      void setHtml(const QString&);
      QString toPlainText() const;
      void setPlainText(const QString&);
      };

extern QString editPlainText(const QString& s, const QString& title);
extern QString editHtml(const QString& s, const QString& title);
}

#endif
