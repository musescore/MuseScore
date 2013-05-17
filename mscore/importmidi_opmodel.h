#ifndef IMPORTMIDI_OPMODEL_H
#define IMPORTMIDI_OPMODEL_H

#include <memory>


namespace Ms {

struct Node;
struct Controller;
struct TrackOperations;

struct OperationCol {
      enum {
            OPER_NAME,
            VALUE,
            COL_COUNT
            };
      };

class OperationsModel : public QAbstractItemModel
      {
      Q_OBJECT

   public:
      OperationsModel();
      ~OperationsModel();

      void setTrack(int trackIndex, const TrackOperations &opers);
      TrackOperations trackOperations() const;

      QModelIndex index(int row, int column, const QModelIndex &parent) const;
      QModelIndex parent(const QModelIndex &child) const;
      int rowCount(const QModelIndex &parent) const;
      int columnCount(const QModelIndex &parent) const;
      QVariant data(const QModelIndex &index, int role) const;
      QVariant headerData(int section, Qt::Orientation orientation, int role) const;
      Qt::ItemFlags flags(const QModelIndex &index) const;
      bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

   private slots:
      void onDataChanged(const QModelIndex &index);

   private:
      std::unique_ptr<Node> root;
      std::unique_ptr<Controller> controller;
      int trackIndex;

      Node* nodeFromIndex(const QModelIndex &index) const;
      };

} // namespace Ms


#endif // IMPORTMIDI_OPMODEL_H
