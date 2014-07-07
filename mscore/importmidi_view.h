#ifndef IMPORTMIDI_VIEW_H
#define IMPORTMIDI_VIEW_H

#include <QTableView>


class SeparatorDelegate;

class TracksView : public QTableView
      {
      Q_OBJECT

   public:
      TracksView(QWidget *parent);
      ~TracksView();

      void setModel(QAbstractItemModel *model);

      void setFrozenRowCount(int count);
      void setFrozenColCount(int count);

   protected:
      void resizeEvent(QResizeEvent *event);
      bool viewportEvent(QEvent *event);
      void wheelEvent(QWheelEvent *event);

   private slots:
      void currentChanged(const QModelIndex &, const QModelIndex &);

      void updateMainViewSectionWidth(int,int,int);
      void updateMainViewSectionHeight(int,int,int);
      void updateFrozenSectionWidth(int,int,int);
      void updateFrozenSectionHeight(int,int,int);
      void onHSectionMove(int,int,int);
      void onVSectionMove(int,int,int);

   private:
      void initHorizontalView();
      void initVerticalView();
      void initCornerView();
      void initMainView();
      void initConnections();
      void setupEditTriggers();
      void updateFrozenTableGeometry();

      void keepVisible(const QModelIndex &previous, const QModelIndex &current);

      int frozenRowHeight();
      int frozenColWidth();

      int frozenVTableWidth();
      int frozenHTableHeight();

      void updateFocus(int currentRow, int currentColumn);

      QTableView *_frozenHTableView;
      QTableView *_frozenVTableView;
      QTableView *_frozenCornerTableView;
      SeparatorDelegate *_delegate;
      int _frozenRowCount;
      int _frozenColCount;
      };


#endif // IMPORTMIDI_VIEW_H
