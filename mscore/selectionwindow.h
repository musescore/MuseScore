#ifndef SELECTIONWINDOW_H
#define SELECTIONWINDOW_H

namespace Ms {
class Score;

class SelectionListWidget : public QListWidget {
      Q_OBJECT
      virtual void focusInEvent(QFocusEvent*) override;
public:
      SelectionListWidget(QWidget* parent = 0);
      };


class SelectionWindow : public QDockWidget {
      Q_OBJECT

      Score* _score;
      QListWidget* _listWidget;

      virtual void closeEvent(QCloseEvent*);
      virtual void hideEvent (QHideEvent* event);
      void updateFilteredElements();

private slots:
   void changeCheckbox(QListWidgetItem*);

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
