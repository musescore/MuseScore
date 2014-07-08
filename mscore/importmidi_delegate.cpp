#include "importmidi_delegate.h"


namespace Ms {

// values:
//    [0] == "__MultiValue__"
//  the rule for next values:
//    odd index - value to show
//    even index - "true" or "false" to define is the previous value checked or not

MultiValue::MultiValue(const QStringList &values, QWidget *parent)
      : QWidget(parent)
      {

      Q_ASSERT_X(values[0] == "__MultiValue__",
                 "Midi delegate - MultiValue class", "Invalid input values");

      QHBoxLayout *contentLayout = new QHBoxLayout();
      contentLayout->setSpacing(0);
      contentLayout->setContentsMargins(0, 0, 0, 0);

      _allButton = new QPushButton(QCoreApplication::translate(
                                         "Multi value editor", "All"));
      _allButton->setMinimumWidth(24);
      _allButton->setMinimumHeight(24);
      _allButton->setCheckable(true);
      contentLayout->addWidget(_allButton);

      for (int i = 1; i < values.size() - 1; i += 2) {
            QPushButton *button = new QPushButton;
            button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            button->setMinimumWidth(24);
            button->setMinimumHeight(24);
            button->setText(values[i]);
            button->setCheckable(true);
            button->setChecked(values[i + 1] == "true");
            if (button->isChecked() && !_allButton->isChecked())
                  _allButton->setChecked(true);
            connect(button, SIGNAL(clicked()), SLOT(buttonClicked()));
            contentLayout->addWidget(button);
            _buttons.append(button);
            }

      connect(_allButton, SIGNAL(toggled(bool)), SLOT(checkBoxToggled(bool)));

      QPushButton *okButton = new QPushButton(QCoreApplication::translate(
                                                    "Multi value editor", "OK"));
      okButton->setMinimumWidth(22);
      connect(okButton, SIGNAL(clicked()), SIGNAL(okClicked()));
      contentLayout->addWidget(okButton);

      setLayout(contentLayout);
      }

QStringList MultiValue::data() const
      {
      QStringList values;
      for (const auto *b: _buttons)
            values.append(b->isChecked() ? "true" : "false");
      return values;
      }

void MultiValue::buttonClicked()
      {
      disconnect(_allButton, SIGNAL(toggled(bool)), this, SLOT(checkBoxToggled(bool)));
      for (const auto *b: _buttons) {
            if (b->isChecked()) {
                  _allButton->setChecked(true);
                  return;
                  }
            }
      _allButton->setChecked(false);
      connect(_allButton, SIGNAL(toggled(bool)), SLOT(checkBoxToggled(bool)));
      }

void MultiValue::checkBoxToggled(bool value)
      {
      for (auto *b: _buttons) {
            disconnect(b, SIGNAL(clicked()), this, SLOT(buttonClicked()));
            b->setChecked(value);
            connect(b, SIGNAL(clicked()), SLOT(buttonClicked()));
            }
      }

//----------------------------------------------------------------------------------

OperationsDelegate::OperationsDelegate(QWidget *appWindow, bool rightArrowAlign)
      : appWindow(appWindow)
      , rightArrowAlign(rightArrowAlign)
      {}

void OperationsDelegate::paint(QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
      {
      QStyledItemDelegate::paint(painter, option, index);
                  // draw small arrow that symbolizes list
      QVariant value = index.data(Qt::EditRole);
      if (value.type() == QVariant::StringList)
            {
            QStringList list = qvariant_cast<QStringList>(value);
            if (list.size() > 1)
                  drawArrow(painter, option, index);
            }
      }

void OperationsDelegate::drawArrow(
            QPainter *painter,
            const QStyleOptionViewItem &option,
            const QModelIndex &index) const
      {
      painter->save();

      QFontMetrics fm(painter->font());

      const int gap = 10;
      const int height = 4;
      const int width = 8;

      const int textWidth = fm.width(index.data(Qt::DisplayRole).toString());
      const int x = rightArrowAlign
                  ? option.rect.right() - width - gap
                  : option.rect.left() + textWidth + gap;
      const int y = option.rect.top() + option.rect.height() / 2 + 1;

      QPoint p1(x, y - height / 2);
      QPoint p2(x + width, y - height / 2);
      QPoint p3(x + width / 2, y + height / 2);

      QPen pen = painter->pen();
      painter->setPen(pen);
      pen.setWidth(1);
      painter->drawLine(p1, p2);
      pen.setWidth(2);
      painter->setPen(pen);
      painter->drawLine(QPoint(p2.x() - 1, p2.y() + 1), QPoint(p3.x() + 1, p3.y() - 1));
      pen.setWidth(1);
      painter->setPen(pen);
      painter->drawLine(p3, p1);

      painter->restore();
      }

QWidget* OperationsDelegate::createEditor(QWidget *parent,
                                          const QStyleOptionViewItem &option,
                                          const QModelIndex &index) const
      {
      const QVariant value = index.data(Qt::EditRole);
      if (value.type() == QVariant::StringList) {     // list of possible values
            const QStringList list = qvariant_cast<QStringList>(value);
            if (!list.isEmpty()) {
                  QWidget *editor = nullptr;

                  if (list[0] == "__MultiValue__") {
                        MultiValue *mv = new MultiValue(list, parent);
                        connect(mv, SIGNAL(okClicked()),
                                this, SLOT(commitAndCloseEditor()));
                        editor = mv;
                        }
                  else {
                        QListWidget *lw = new QListWidget(parent);
                        for (const auto &item: list)
                              lw->addItem(item);
                        connect(lw, SIGNAL(itemClicked(QListWidgetItem*)),
                                this, SLOT(commitAndCloseEditor()));
                        editor = lw;
                        }
                  return editor;
                  }
            }
                  // single value
      return QStyledItemDelegate::createEditor(parent, option, index);
      }

void OperationsDelegate::setEditorData(QWidget *editor,
                                       const QModelIndex &index) const
      {
      const QVariant value = index.data(Qt::EditRole);
      if (value.type() == QVariant::StringList) {
            QListWidget *lw = qobject_cast<QListWidget *>(editor);
            if (lw) {
                  const auto items = lw->findItems(index.data(Qt::DisplayRole).toString(), Qt::MatchExactly);
                  lw->setCurrentItem(items.empty() ? lw->item(0) : items.first());

                  const int extraWidth = 25;
                  const int extraHeight = 6;
                  lw->setMinimumWidth(lw->sizeHintForColumn(0) + extraWidth);

                              // to prevent possible hiding bottom part of the list
                  const int h = lw->count() * (lw->visualItemRect(lw->currentItem()).height() + extraHeight);
                  const int y = (lw->parentWidget() && (lw->parentWidget()->rect().bottom() < lw->y() + h))
                              ? lw->parentWidget()->rect().bottom() - h - extraHeight : lw->y();
                  lw->setGeometry(lw->x(), y, lw->width(), h);

                              // now lw can be partially hidden behind the view
                              // if the view has small rect, so set parent of lw
                              // to app window and map coordinates accordingly to leave lw in place
                  const auto globalCoord = lw->parentWidget()->mapToGlobal(lw->geometry().topLeft());
                  lw->setParent(appWindow);
                  const auto newLocalCoord = appWindow->mapFromGlobal(globalCoord);
                  lw->setGeometry(newLocalCoord.x(), newLocalCoord.y(), lw->width(), h);
                  }
            else {
                  MultiValue *mv = qobject_cast<MultiValue *>(editor);

                  Q_ASSERT_X(mv, "Midi delegate - setEditorData", "Unknown editor type");

                              // now mv can be partially hidden behind the view
                              // if the view has small rect, so set parent of mv
                              // to app window and map coordinates accordingly to leave mv in place
                  const auto globalCoord = mv->parentWidget()->mapToGlobal(mv->geometry().topLeft());
                  mv->setParent(appWindow);
                  const auto newLocalCoord = appWindow->mapFromGlobal(globalCoord);
                  const int newWidth = mv->sizeHint().width();
                  mv->setGeometry(newLocalCoord.x() + (mv->width() - newWidth) / 2, newLocalCoord.y(),
                                  newWidth, mv->height());
                  }
            }
      else {       // single value
            QStyledItemDelegate::setEditorData(editor, index);
            }
      }

void OperationsDelegate::setModelData(QWidget *editor,
                                      QAbstractItemModel *model,
                                      const QModelIndex &index) const
      {
      const QVariant value = index.data(Qt::EditRole);
      if (value.type() == QVariant::StringList) {
            const QListWidget *lw = qobject_cast<QListWidget *>(editor);
            if (lw) {
                  model->setData(index, lw->currentRow());
                  }
            else {
                  const MultiValue *mv = qobject_cast<MultiValue *>(editor);

                  Q_ASSERT_X(mv, "Midi delegate - setModelData", "Unknown editor type");

                  model->setData(index, mv->data());
                  }
            }
      else {
            QStyledItemDelegate::setModelData(editor, model, index);
            }
      }

void OperationsDelegate::commitAndCloseEditor()
      {
      QWidget *editor = qobject_cast<QWidget *>(sender());
      emit commitData(editor);
      emit closeEditor(editor);
      editor->parentWidget()->setFocus();
      }

} // namespace Ms
