#include "importmidi_opdelegate.h"


namespace Ms {

OperationsDelegate::OperationsDelegate(QWidget *appWindow)
      : appWindow(appWindow)
      {}

QWidget* OperationsDelegate::createEditor(QWidget *parent,
                                          const QStyleOptionViewItem &option,
                                          const QModelIndex &index) const
      {
      QVariant value = index.data(Qt::EditRole);
      if (value.type() == QVariant::StringList) { // list of possible values
            QStringList list = qvariant_cast<QStringList>(value);
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
      QVariant value = index.data(Qt::EditRole);
      if (value.type() == QVariant::StringList) {
            QListWidget *lw = qobject_cast<QListWidget *>(editor);
            auto items = lw->findItems(index.data(Qt::DisplayRole).toString(), Qt::MatchExactly);
            if (!items.empty())
                  lw->setCurrentItem(items.first());

            const int EXTRA_WIDTH = 25;
            const int EXTRA_HEIGHT = 6;
            lw->setMinimumWidth(lw->sizeHintForColumn(0) + EXTRA_WIDTH);
            // to prevent possible hiding bottom part of the list
            int h = lw->count() * (lw->visualItemRect(lw->currentItem()).height() + EXTRA_HEIGHT);
            int y = (lw->parentWidget() && (lw->parentWidget()->rect().bottom() < lw->y() + h))
                        ? lw->parentWidget()->rect().bottom() - h - EXTRA_HEIGHT : lw->y();
            lw->setGeometry(lw->x(), y, lw->width(), h);

            // now lw can be partially hidden behind the tree view
            // if tree view has small rect, so set parent of lw
            // to app window and map coordinates accordingly to leave lw in place
            auto globalCoord = lw->parentWidget()->mapToGlobal(lw->geometry().topLeft());
            lw->setParent(appWindow);
            auto newLocalCoord = appWindow->mapFromGlobal(globalCoord);
            lw->setGeometry(newLocalCoord.x(), newLocalCoord.y(), lw->width(), h);
            }
      else // single value
            QStyledItemDelegate::setEditorData(editor, index);
      }

void OperationsDelegate::setModelData(QWidget *editor,
                                      QAbstractItemModel *model,
                                      const QModelIndex &index) const
      {
      QVariant value = index.data(Qt::EditRole);
      if (value.type() == QVariant::StringList) {
            QListWidget *lw = qobject_cast<QListWidget *>(editor);
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
