#ifndef MU_INSPECTOR_BEAMTYPESMODEL_H
#define MU_INSPECTOR_BEAMTYPESMODEL_H

#include <QAbstractListModel>
#include "types/beamtypes.h"

namespace mu::inspector {
class BeamModeListModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int selectedTypeIndex READ selectedTypeIndex WRITE setSelectedTypeIndex NOTIFY selectedTypeIndexChanged)

public:
    enum RoleNames {
        BeamModeRole = Qt::UserRole + 1,
        HintRole
    };

    explicit BeamModeListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setSelectedBeamMode(const BeamTypes::Mode beamMode);

    int selectedTypeIndex() const;

public slots:
    void setSelectedTypeIndex(int selectedTypeIndex);

signals:
    void selectedTypeIndexChanged(int selectedTypeIndex);
    void beamModeSelected(BeamTypes::Mode beamMode);

private:
    struct BeamTypesData {
        BeamTypes::Mode mode;
        QString hint;
    };

    int indexOfBeamMode(const BeamTypes::Mode beamMode) const;

    QHash<int, QByteArray> m_roleNames;

    QVector<BeamTypesData> m_beamTypesDataList;
    int m_selectedTypeIndex = -1;
};
}

#endif // MU_INSPECTOR_BEAMTYPESMODEL_H
