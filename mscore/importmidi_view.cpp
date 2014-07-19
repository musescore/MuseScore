#include "importmidi_view.h"


class SizedHHeaderView : public QHeaderView
      {
   public:
      explicit SizedHHeaderView(QHeaderView *mainView, QWidget *parent = 0)
            : QHeaderView(Qt::Horizontal, parent)
            , _mainView(mainView)
            {
            }

      QSize sizeHint() const
            {
            QSize size = QHeaderView::sizeHint();
            size.setHeight(_mainView->sizeHint().height());
            return size;
            }

   private:
      QHeaderView *_mainView;
      };

TracksView::TracksView(QWidget *parent)
      : QTableView(parent)
      , _frozenRowCount(0)
      , _frozenColCount(0)
      {
      _frozenHTableView = new QTableView(this);
      _frozenVTableView = new QTableView(this);
      _frozenCornerTableView = new QTableView(this);
      _delegate = new SeparatorDelegate(this);

      _delegate->setFrozenRowIndex(_frozenRowCount - 1);
      _delegate->setFrozenColIndex(_frozenColCount - 1);

      initHorizontalView();
      initVerticalView();
      initCornerView();
      initMainView();
      initConnections();

      setupEditTriggers();
      }

TracksView::~TracksView()
      {
      }

void TracksView::setupEditTriggers()
      {
      const EditTriggers triggers = QAbstractItemView::DoubleClicked
                                  | QAbstractItemView::EditKeyPressed
                                  | QAbstractItemView::SelectedClicked;
      setEditTriggers(triggers);
      _frozenHTableView->setEditTriggers(triggers);
      _frozenVTableView->setEditTriggers(triggers);
      _frozenCornerTableView->setEditTriggers(triggers);

                  // QAbstractItemView::CurrentChanged trigger doesn't work as expected
                  // so we emulate this behaviour
      connect(this, SIGNAL(clicked(const QModelIndex &)),
              this, SLOT(edit(const QModelIndex &)));
      connect(_frozenHTableView, SIGNAL(clicked(const QModelIndex &)),
              _frozenHTableView, SLOT(edit(const QModelIndex &)));
      connect(_frozenVTableView, SIGNAL(clicked(const QModelIndex &)),
              _frozenVTableView, SLOT(edit(const QModelIndex &)));
      connect(_frozenCornerTableView, SIGNAL(clicked(const QModelIndex &)),
              _frozenCornerTableView, SLOT(edit(const QModelIndex &)));
      }

void TracksView::initHorizontalView()
      {
      _frozenHTableView->setHorizontalHeader(new SizedHHeaderView(horizontalHeader()));
      _frozenHTableView->horizontalHeader()->hide();
      _frozenHTableView->setSelectionBehavior(SelectItems);
      _frozenHTableView->setSelectionMode(SingleSelection);
      _frozenHTableView->setAutoScroll(false);
      _frozenHTableView->setFocusPolicy(Qt::StrongFocus);

      _frozenHTableView->setStyleSheet("QTableView { border: none; }");
      _frozenHTableView->setItemDelegate(_delegate);

      _frozenHTableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      _frozenHTableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      _frozenHTableView->show();

      _frozenHTableView->setHorizontalScrollMode(ScrollPerPixel);
      }

void TracksView::initVerticalView()
      {
      _frozenVTableView->setHorizontalHeader(new SizedHHeaderView(horizontalHeader()));
      _frozenHTableView->horizontalHeader()->setSectionsMovable(false);
      _frozenVTableView->verticalHeader()->hide();
      _frozenVTableView->setSelectionBehavior(SelectItems);
      _frozenVTableView->setSelectionMode(SingleSelection);
      _frozenVTableView->setAutoScroll(false);
      _frozenVTableView->setFocusPolicy(Qt::StrongFocus);

      _frozenVTableView->setStyleSheet("QTableView { border: none; }");
      _frozenVTableView->setItemDelegate(_delegate);

      _frozenVTableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      _frozenVTableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      _frozenVTableView->show();

      _frozenVTableView->setVerticalScrollMode(ScrollPerPixel);

      _frozenHTableView->stackUnder(_frozenVTableView);
      }

void TracksView::initCornerView()
      {
      _frozenCornerTableView->setHorizontalHeader(new SizedHHeaderView(horizontalHeader()));
      _frozenCornerTableView->horizontalHeader()->hide();
      _frozenCornerTableView->verticalHeader()->hide();
      _frozenCornerTableView->setAutoScroll(false);
      _frozenCornerTableView->setSelectionBehavior(SelectItems);
      _frozenCornerTableView->setSelectionMode(SingleSelection);
      _frozenCornerTableView->setFocusPolicy(Qt::StrongFocus);

      _frozenCornerTableView->setStyleSheet("QTableView { border: none; }");
      _frozenCornerTableView->setItemDelegate(_delegate);

      _frozenCornerTableView->horizontalScrollBar()->setDisabled(true);
      _frozenCornerTableView->verticalScrollBar()->setDisabled(true);
      _frozenCornerTableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      _frozenCornerTableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      _frozenCornerTableView->show();

      _frozenVTableView->stackUnder(_frozenCornerTableView);
      }

void TracksView::initMainView()
      {
      horizontalHeader()->setSectionsMovable(false);
      verticalHeader()->setSectionsMovable(true);

      setHorizontalScrollMode(ScrollPerPixel);
      setVerticalScrollMode(ScrollPerPixel);
      setSelectionBehavior(SelectItems);
      setSelectionMode(SingleSelection);
      setAutoScroll(false);
      setFocusPolicy(Qt::StrongFocus);
      horizontalScrollBar()->setFocusPolicy(Qt::NoFocus);
      verticalScrollBar()->setFocusPolicy(Qt::NoFocus);

      viewport()->stackUnder(_frozenHTableView);
      }

void TracksView::initConnections()
      {
      connect(horizontalHeader(), SIGNAL(sectionResized(int,int,int)),
              this, SLOT(updateMainViewSectionWidth(int,int,int)));
      connect(verticalHeader(), SIGNAL(sectionResized(int,int,int)),
              this, SLOT(updateMainViewSectionHeight(int,int,int)));

      connect(verticalHeader(), SIGNAL(sectionMoved(int,int,int)),
              SLOT(onVSectionMove(int,int,int)));
      connect(horizontalHeader(), SIGNAL(sectionMoved(int,int,int)),
              SLOT(onHSectionMove(int,int,int)));

      connect(horizontalScrollBar(), SIGNAL(valueChanged(int)),
              _frozenHTableView->horizontalScrollBar(), SLOT(setValue(int)));
      connect(_frozenHTableView->horizontalScrollBar(), SIGNAL(valueChanged(int)),
              horizontalScrollBar(), SLOT(setValue(int)));

      connect(verticalScrollBar(), SIGNAL(valueChanged(int)),
              _frozenVTableView->verticalScrollBar(), SLOT(setValue(int)));
      connect(_frozenVTableView->verticalScrollBar(), SIGNAL(valueChanged(int)),
              verticalScrollBar(), SLOT(setValue(int)));
      }

void TracksView::setModel(QAbstractItemModel *model)
      {
      QTableView::setModel(model);

      _frozenHTableView->setModel(model);
      _frozenVTableView->setModel(model);
      _frozenCornerTableView->setModel(model);

      _frozenHTableView->setSelectionModel(selectionModel());
      _frozenVTableView->setSelectionModel(selectionModel());
      _frozenCornerTableView->setSelectionModel(selectionModel());

      connect(_frozenVTableView->selectionModel(),
              SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
              SLOT(currentChanged(const QModelIndex &, const QModelIndex &)),
              Qt::UniqueConnection);
      connect(_frozenHTableView->selectionModel(),
              SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
              SLOT(currentChanged(const QModelIndex &, const QModelIndex &)),
              Qt::UniqueConnection);
      connect(_frozenCornerTableView->selectionModel(),
              SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
              SLOT(currentChanged(const QModelIndex &, const QModelIndex &)),
              Qt::UniqueConnection);

      setFrozenRowCount(_frozenRowCount);
      setFrozenColCount(_frozenColCount);

      connect(_frozenVTableView->horizontalHeader(),SIGNAL(sectionResized(int,int,int)),
              this, SLOT(updateFrozenSectionWidth(int,int,int)), Qt::UniqueConnection);
      connect(_frozenHTableView->verticalHeader(),SIGNAL(sectionResized(int,int,int)),
              this, SLOT(updateFrozenSectionHeight(int,int,int)), Qt::UniqueConnection);
      }

void TracksView::setFrozenRowCount(int count)
      {
      _frozenRowCount = count;
      _delegate->setFrozenRowIndex(_frozenRowCount - 1);

      if (model()->rowCount() == 0)
            return;

      Q_ASSERT_X(count >= 0 && count < model()->rowCount(),
                 "TracksView::setFrozenRowCount", "Invalid frozen row count");

      for (int row = 0; row != count; ++row) {
            _frozenHTableView->setRowHidden(row, false);
            _frozenHTableView->setRowHeight(row, rowHeight(row));
            _frozenCornerTableView->setRowHidden(row, false);
            }

      for (int row = count; row < model()->rowCount(); ++row) {
            _frozenHTableView->setRowHidden(row, true);
            _frozenCornerTableView->setRowHidden(row, true);
            }

      updateFrozenTableGeometry();
      }

void TracksView::setFrozenColCount(int count)
      {
      _frozenColCount = count;
      _delegate->setFrozenColIndex(_frozenColCount - 1);

      if (model()->columnCount() == 0)
            return;

      Q_ASSERT_X(count >= 0 && count < model()->columnCount(),
                 "TracksView::setFrozenColCount", "Invalid frozen column count");

      for (int col = 0; col != count; ++col) {
            _frozenVTableView->setColumnHidden(col, false);
            _frozenVTableView->setColumnWidth(col, columnWidth(col));
            _frozenCornerTableView->setColumnHidden(col, false);
            }

      for (int col = count; col < model()->columnCount(); ++col) {
            _frozenVTableView->setColumnHidden(col, true);
            _frozenCornerTableView->setColumnHidden(col, true);
            }

      updateFrozenTableGeometry();
      }

void TracksView::setItemDelegate(SeparatorDelegate *delegate)
      {
      delete _delegate;
      _delegate = delegate;
      QTableView::setItemDelegate(delegate);
      _frozenHTableView->setItemDelegate(delegate);
      _frozenVTableView->setItemDelegate(delegate);
      _frozenCornerTableView->setItemDelegate(delegate);
      }

void TracksView::restoreHHeaderState(const QByteArray &data)
      {
      horizontalHeader()->restoreState(data);
      _frozenHTableView->horizontalHeader()->restoreState(data);
      _frozenVTableView->horizontalHeader()->restoreState(data);
      _frozenCornerTableView->horizontalHeader()->restoreState(data);
      }

void TracksView::restoreVHeaderState(const QByteArray &data)
      {
      verticalHeader()->restoreState(data);
      _frozenHTableView->verticalHeader()->restoreState(data);
      _frozenVTableView->verticalHeader()->restoreState(data);
      _frozenCornerTableView->verticalHeader()->restoreState(data);
      }

void TracksView::setHHeaderResizeMode(QHeaderView::ResizeMode mode)
      {
      horizontalHeader()->setResizeMode(mode);
      _frozenHTableView->horizontalHeader()->setResizeMode(mode);
      _frozenVTableView->horizontalHeader()->setResizeMode(mode);
      _frozenCornerTableView->horizontalHeader()->setResizeMode(mode);
      }

void TracksView::setVHeaderDefaultSectionSize(int size)
      {
      verticalHeader()->setDefaultSectionSize(size);
      _frozenHTableView->verticalHeader()->setDefaultSectionSize(size);
      _frozenVTableView->verticalHeader()->setDefaultSectionSize(size);
      _frozenCornerTableView->verticalHeader()->setDefaultSectionSize(size);
      }

void TracksView::resetHHeader()
      {
      horizontalHeader()->reset();
      _frozenHTableView->horizontalHeader()->reset();
      _frozenVTableView->horizontalHeader()->reset();
      _frozenCornerTableView->horizontalHeader()->reset();
      }

void TracksView::resetVHeader()
      {
      verticalHeader()->reset();
      _frozenHTableView->verticalHeader()->reset();
      _frozenVTableView->verticalHeader()->reset();
      _frozenCornerTableView->verticalHeader()->reset();
      }

void TracksView::updateMainViewSectionWidth(int logicalIndex, int /*oldSize*/, int newSize)
      {
      if (logicalIndex < _frozenColCount) {
            _frozenVTableView->setColumnWidth(logicalIndex, newSize);
            _frozenCornerTableView->setColumnWidth(logicalIndex, newSize);
            }
      _frozenHTableView->setColumnWidth(logicalIndex, newSize);
      updateFrozenTableGeometry();
      }

void TracksView::updateMainViewSectionHeight(int logicalIndex, int /*oldSize*/, int newSize)
      {
      if (logicalIndex < _frozenRowCount) {
            _frozenHTableView->setRowHeight(logicalIndex, newSize);
            _frozenCornerTableView->setRowHeight(logicalIndex, newSize);
            }
      _frozenVTableView->setRowHeight(logicalIndex, newSize);
      updateFrozenTableGeometry();
      }

void TracksView::updateFrozenSectionWidth(int logicalIndex, int /*oldSize*/, int newSize)
      {
      if (logicalIndex < _frozenColCount) {
            setColumnWidth(logicalIndex, newSize);
            _frozenCornerTableView->setColumnWidth(logicalIndex, newSize);
            _frozenHTableView->setColumnWidth(logicalIndex, newSize);
            updateFrozenTableGeometry();
            }
      }

void TracksView::updateFrozenSectionHeight(int logicalIndex, int /*oldSize*/, int newSize)
      {
      if (logicalIndex < _frozenRowCount) {
            setRowHeight(logicalIndex, newSize);
            _frozenCornerTableView->setRowHeight(logicalIndex, newSize);
            _frozenVTableView->setRowHeight(logicalIndex, newSize);
            updateFrozenTableGeometry();
            }
      }

void TracksView::resizeEvent(QResizeEvent *event)
      {
      QTableView::resizeEvent(event);
      updateFrozenTableGeometry();
      }

// show tooltip if the text is wider than the table cell

bool TracksView::viewportEvent(QEvent *event)
      {
      if (event->type() == QEvent::ToolTip) {
            QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
            QModelIndex index = indexAt(helpEvent->pos());
            if (index.isValid()) {
                  QSize sizeHint = itemDelegate(index)->sizeHint(viewOptions(), index);
                  QRect rItem(0, 0, sizeHint.width(), sizeHint.height());
                  QRect rVisual = visualRect(index);
                  if (rItem.width() <= rVisual.width()) {
                        QToolTip::hideText();
                        return false;
                        }
                  }
            }

      return QTableView::viewportEvent(event);
}

void TracksView::wheelEvent(QWheelEvent *event)
      {
      const int degrees = event->delta() / 8;
      const int steps = degrees / 15;

      if ((event->modifiers() & Qt::ShiftModifier) || (event->modifiers() & Qt::ControlModifier)) {
            const int pixelsToScroll = steps * 30;
            horizontalScrollBar()->setValue(horizontalScrollBar()->value() - pixelsToScroll);
            }
      else {
            const int pixelsToScroll = steps * 15;
            verticalScrollBar()->setValue(verticalScrollBar()->value() - pixelsToScroll);
            }
      }

void TracksView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
      {
      updateFocus(current.row(), current.column());
      keepVisible(previous, current);
      }

void TracksView::onHSectionMove(int /*logicalIndex*/, int oldVisualIndex, int newVisualIndex)
      {
      if (newVisualIndex < _frozenColCount) {		// disallow move
            horizontalHeader()->moveSection(newVisualIndex, oldVisualIndex);	// move back
            }
      else if (oldVisualIndex >= _frozenColCount) {
            // move only if this slot was not called after previous move back
            _frozenHTableView->horizontalHeader()->moveSection(oldVisualIndex, newVisualIndex);
            _frozenVTableView->horizontalHeader()->moveSection(oldVisualIndex, newVisualIndex);
            _frozenCornerTableView->horizontalHeader()->moveSection(oldVisualIndex, newVisualIndex);
            }
      }

void TracksView::onVSectionMove(int /*logicalIndex*/, int oldVisualIndex, int newVisualIndex)
      {
      if (newVisualIndex < _frozenRowCount) {		// disallow move
            verticalHeader()->moveSection(newVisualIndex, oldVisualIndex);		// move back
            }
      else if (oldVisualIndex >= _frozenRowCount) {
            // move only if this slot was not called after previous move back
            _frozenHTableView->verticalHeader()->moveSection(oldVisualIndex, newVisualIndex);
            _frozenVTableView->verticalHeader()->moveSection(oldVisualIndex, newVisualIndex);
            _frozenCornerTableView->verticalHeader()->moveSection(oldVisualIndex, newVisualIndex);
            }
      }

void TracksView::updateFocus(int currentRow, int currentColumn)
      {
      if (currentRow < _frozenRowCount) {
            if (currentColumn < _frozenColCount)
                  _frozenCornerTableView->setFocus();
            else
                  _frozenHTableView->setFocus();
            }
      else {
            if (currentColumn < _frozenColCount)
                  _frozenVTableView->setFocus();
            else
                  setFocus();
            }
      }

void TracksView::keepVisible(const QModelIndex &previous, const QModelIndex &current)
      {
      // update scroll bars to show the cell hidden by the frozen table view if it gets focus
      const int hInvisibleGap = visualRect(current).topLeft().x() - frozenVTableWidth();
      const int vInvisibleGap = visualRect(current).topLeft().y() - frozenHTableHeight();

      if (current.column() != previous.column() && current.column() >= _frozenColCount
                  && hInvisibleGap < 0) {
            const int newValue = horizontalScrollBar()->value() + hInvisibleGap;
            horizontalScrollBar()->setValue(newValue);
            }
      if (current.row() != previous.row() && current.row() >= _frozenRowCount
                  && vInvisibleGap < 0) {
            const int newValue = verticalScrollBar()->value() + vInvisibleGap;
            verticalScrollBar()->setValue(newValue);
            }

      // update scroll bars to show the cell hidden by the borders of main table view
      const int hInvisibleGap2 = width() - verticalHeader()->width() - 4 * frameWidth()
                  - ((verticalScrollBar()->isHidden()) ? 0 : verticalScrollBar()->width())
                  - visualRect(current).bottomRight().x();
      const int vInvisibleGap2 = height() - horizontalHeader()->height() - 4 * frameWidth()
                  - ((horizontalScrollBar()->isHidden()) ? 0 : horizontalScrollBar()->height())
                  - visualRect(current).bottomRight().y();

      if (current.column() != previous.column() && hInvisibleGap2 < 0) {
            const int newValue = horizontalScrollBar()->value() - hInvisibleGap2;
            horizontalScrollBar()->setValue(newValue);
            }
      if (current.row() != previous.row() && vInvisibleGap2 < 0) {
            const int newValue = verticalScrollBar()->value() - vInvisibleGap2;
            verticalScrollBar()->setValue(newValue);
            }
      }

int TracksView::frozenRowHeight()
      {
      int height = 0;
      for (int row = 0; row != _frozenRowCount; ++row)
            height += rowHeight(row);
      return height;
      }

int TracksView::frozenColWidth()
      {
      int width = 0;
      for (int col = 0; col != _frozenColCount; ++col)
            width += columnWidth(col);
      return width;
      }

int TracksView::frozenVTableWidth()
      {
      int width = 0;
      for (int col = 0; col != _frozenColCount; ++col)
            width += _frozenVTableView->columnWidth(col);
      return width;
      }

int TracksView::frozenHTableHeight()
      {
      int height = 0;
      for (int row = 0; row != _frozenRowCount; ++row)
            height += _frozenHTableView->rowHeight(row);
      return height;
      }

void TracksView::updateFrozenTableGeometry()
      {
      _frozenHTableView->setGeometry(
                        frameWidth(),
                        horizontalHeader()->height() + frameWidth(),
                        viewport()->width() + verticalHeader()->width(),
                        frozenRowHeight());

      _frozenVTableView->setGeometry(
                        verticalHeader()->width() + frameWidth(),
                        frameWidth(),
                        frozenColWidth(),
                        viewport()->height() + horizontalHeader()->height());

      _frozenCornerTableView->setGeometry(
                        verticalHeader()->width() + frameWidth(),
                        horizontalHeader()->height() + frameWidth(),
                        frozenColWidth(),
                        frozenRowHeight());
      }
