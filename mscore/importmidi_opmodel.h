#ifndef IMPORTMIDI_OPMODEL_H
#define IMPORTMIDI_OPMODEL_H

#include "importmidi_operation.h"
#include "importmidi_fraction.h"

#include <memory>


namespace Ms {

enum {
      OperationTypeRole = 100,
      DataRole
      };

struct Node;
struct Controller;
struct TrackOperations;
struct DefinedTrackOperations;

enum OperationCol {
            OPER_NAME,
            VALUE,
            OCOL_COUNT
      };

class OperationsModel : public QAbstractItemModel
      {
      Q_OBJECT

   public:
      OperationsModel();
      ~OperationsModel();

      void setTimeSigVisibility(bool value);
      void setTrackData(const QString &trackLabel, const Ms::DefinedTrackOperations &opers);

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
      void updateQuantValue();

   private:
      std::unique_ptr<Node> root;
      std::unique_ptr<Controller> controller;
      QString trackLabel;
      QTimer *updateQuantTimer;
      ReducedFraction prefQuantValue;

      Node* nodeFromIndex(const QModelIndex &index) const;
      };

} // namespace Ms


#endif // IMPORTMIDI_OPMODEL_H
