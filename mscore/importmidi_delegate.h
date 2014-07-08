#ifndef IMPORTMIDI_DELEGATE_H
#define IMPORTMIDI_DELEGATE_H


namespace Ms {

// class for multiple value representation
// each value is a button that can be checked or unchecked

class MultiValue : public QWidget
      {
      Q_OBJECT

   public:
      MultiValue(const QStringList &values, QWidget *parent = nullptr);

      QStringList data() const;

   signals:
      void okClicked();

   private slots:
      void buttonClicked();
      void checkBoxToggled(bool);

   private:
      QList<QPushButton *> _buttons;
      QPushButton *_allButton;
      };


class OperationsDelegate : public QStyledItemDelegate
      {
      Q_OBJECT

   public:
      explicit OperationsDelegate(QWidget *appWindow, bool rightArrowAlign);

      void paint(QPainter *painter,
                 const QStyleOptionViewItem &option,
                 const QModelIndex &index) const;

      QWidget* createEditor(QWidget *parent,
                            const QStyleOptionViewItem &option,
                            const QModelIndex &index) const;

      void setEditorData(QWidget *editor, const QModelIndex &index) const;
      void setModelData(QWidget *editor, QAbstractItemModel *model,
                        const QModelIndex &index) const;

   private slots:
      void commitAndCloseEditor();

   private:
      void drawArrow(QPainter *painter,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) const;

      QWidget *appWindow;
      const bool rightArrowAlign;
      };

} // namespace Ms


#endif // IMPORTMIDI_DELEGATE_H
