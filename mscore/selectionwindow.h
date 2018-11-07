//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2002-2016 Werner Schweer and others
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

#ifndef SELECTIONWINDOW_H
#define SELECTIONWINDOW_H

namespace Ms {
class Score;

class SelectionListWidget : public QListWidget {
      Q_OBJECT
      virtual void focusInEvent(QFocusEvent*) override;

   public:
      SelectionListWidget(QWidget* parent = 0);
      void retranslate();
      };


class SelectionWindow : public QDockWidget {
      Q_OBJECT

      Score* _score;
      SelectionListWidget* _listWidget;

      virtual void closeEvent(QCloseEvent*);
      virtual void hideEvent (QHideEvent* event);
      void updateFilteredElements();

   private slots:
      void changeCheckbox(QListWidgetItem*);

   protected:
      virtual void changeEvent(QEvent *event);
      void retranslate();

   signals:
      void closed(bool);

   public:
      SelectionWindow(QWidget *parent = 0, Score* score = 0);
      ~SelectionWindow();
      virtual QSize sizeHint() const;
      void setScore(Score*);
      };
} // namespace Ms
#endif // SELECTIONWINDOW_H
