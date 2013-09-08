#include "importmidi_opdelegate.h"
#include "importmidi_opmodel.h"


namespace Ms {

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
            }
      }

QWidget* OperationsDelegate::createEditor(QWidget *parent,
                                          const QStyleOptionViewItem &option,
                                          const QModelIndex &index) const
      {
      const QVariant value = index.data(Qt::EditRole);
      if (value.type() == QVariant::StringList) { // list of possible values
            const QStringList list = qvariant_cast<QStringList>(value);
            QListWidget *lw = new QListWidget(parent);
            for (const auto &p: list)
                  lw->addItem(p);
            connect(lw, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(commitAndCloseEditor()));
            return lw;
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
            const auto items = lw->findItems(index.data(Qt::DisplayRole).toString(), Qt::MatchExactly);
            if (!items.empty())
                  lw->setCurrentItem(items.first());
            else
                  lw->setCurrentItem(lw->item(0));

            const int extraWidth = 25;
            const int extraHeight = 6;
            lw->setMinimumWidth(lw->sizeHintForColumn(0) + extraWidth);
                        // to prevent possible hiding bottom part of the list
            const int h = lw->count() * (lw->visualItemRect(lw->currentItem()).height() + extraHeight);
            const int y = (lw->parentWidget() && (lw->parentWidget()->rect().bottom() < lw->y() + h))
                        ? lw->parentWidget()->rect().bottom() - h - extraHeight : lw->y();
            lw->setGeometry(lw->x(), y, lw->width(), h);
                        // now lw can be partially hidden behind the tree view
                        // if tree view has small rect, so set parent of lw
                        // to app window and map coordinates accordingly to leave lw in place
            const auto globalCoord = lw->parentWidget()->mapToGlobal(lw->geometry().topLeft());
            lw->setParent(appWindow);
            const auto newLocalCoord = appWindow->mapFromGlobal(globalCoord);
            lw->setGeometry(newLocalCoord.x(), newLocalCoord.y(), lw->width(), h);
            }
      else        // single value
            QStyledItemDelegate::setEditorData(editor, index);
      }

void OperationsDelegate::setModelData(QWidget *editor,
                                      QAbstractItemModel *model,
                                      const QModelIndex &index) const
      {
      const QVariant value = index.data(Qt::EditRole);
      if (value.type() == QVariant::StringList) {
            const QListWidget *lw = qobject_cast<QListWidget *>(editor);
            model->setData(index, lw->currentRow());
            }
      else
            QStyledItemDelegate::setModelData(editor, model, index);
      }

void OperationsDelegate::commitAndCloseEditor()
      {
      QListWidget *editor = qobject_cast<QListWidget *>(sender());
      emit commitData(editor);
      emit closeEditor(editor);
      }

} // namespace Ms
