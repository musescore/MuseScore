#include "inspectorlistmodel.h"

#include "general/generalsettingsmodel.h"
#include "notation/notationinspectorproxymodel.h"

InspectorListModel::InspectorListModel(QObject *parent) : QAbstractListModel(parent)
      {
      m_roleNames.insert(InspectorDataRole, "inspectorData");

      m_repository = new ElementRepositoryService(this);
      }

void InspectorListModel::setElementList(const QList<Ms::Element*>& elementList)
      {

      if (elementList.isEmpty() && !m_modelList.isEmpty()) {

            beginResetModel();

            m_repository->updateElementList(elementList);

            qDeleteAll(m_modelList);
            m_modelList.clear();

            endResetModel();

            return;
      }

      for (const Ms::Element* element : elementList) {
            createModelsByElementType(element->type());
      }

      m_repository->updateElementList(elementList);
      }

int InspectorListModel::rowCount(const QModelIndex&) const
      {
      return m_modelList.count();
      }

QVariant InspectorListModel::data(const QModelIndex &index, int) const
      {
      if (!index.isValid() || index.row() >= rowCount() || m_modelList.isEmpty())
            return QVariant();

      AbstractInspectorModel* model = m_modelList.at(index.row());

      QObject* result = qobject_cast<QObject*>(model);

      return QVariant::fromValue(result);
      }

QHash<int, QByteArray> InspectorListModel::roleNames() const
      {
      return m_roleNames;
      }

int InspectorListModel::columnCount(const QModelIndex&) const
      {
      return 1;
      }

void InspectorListModel::createModelsByElementType(const Ms::ElementType elementType)
      {
      using ModelTypes = AbstractInspectorModel::InspectorModelType;

      QList<ModelTypes> modelTypeList = { AbstractInspectorModel::GENERAL };

      modelTypeList << AbstractInspectorModel::modelTypeFromElementType(elementType);

      for (const ModelTypes modelType : modelTypeList) {

          if (modelType == AbstractInspectorModel::UNDEFINED)
              continue;

          if (isModelAlreadyExists(modelType))
              continue;

          beginInsertRows(QModelIndex(), rowCount(), rowCount());

          AbstractInspectorModel* newModel = nullptr;
          connect(newModel, &AbstractInspectorModel::elementsModified, this, &InspectorListModel::elementsModified);

          switch (modelType) {
          case AbstractInspectorModel::GENERAL: newModel = new GeneralSettingsModel(this, m_repository); break;
          case AbstractInspectorModel::NOTATION: newModel = new NotationInspectorProxyModel(this, m_repository); break;
          default: break;
          }

          if (newModel) {
                m_modelList << newModel;
          }

          endInsertRows();
      }
}

bool InspectorListModel::isModelAlreadyExists(const AbstractInspectorModel::InspectorModelType modelType) const
      {
      for (const AbstractInspectorModel* model : m_modelList) {
            if (model->type() == modelType)
                  return true;
            }

      return false;
      }
