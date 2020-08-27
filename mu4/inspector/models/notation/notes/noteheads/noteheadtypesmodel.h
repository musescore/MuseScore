#ifndef NOTEHEADTYPESMODEL_H
#define NOTEHEADTYPESMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include "note.h"
#include "types/noteheadtypes.h"

class NoteheadTypesModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(int selectedHeadTypeIndex READ selectedHeadTypeIndex WRITE setSelectedHeadTypeIndex NOTIFY selectedHeadTypeIndexChanged)

public:
    enum RoleNames {
        HeadGroupRole = Qt::UserRole + 1,
        HintRole
    };

    explicit NoteheadTypesModel(QObject* parent = nullptr);

    void load();

    QHash<int, QByteArray> roleNames() const;
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role) const;

    void init(const Ms::NoteHead::Group noteHeadGroup);

    int selectedHeadTypeIndex() const;

public slots:
    void setSelectedHeadTypeIndex(int selectedHeadTypeIndex);

signals:
    void noteHeadGroupSelected(int headGroup);
    void selectedHeadTypeIndexChanged(int selectedHeadTypeIndex);

private:
    struct HeadTypeData {
        Ms::NoteHead::Group group = Ms::NoteHead::Group::HEAD_INVALID;
        QString hint;
    };

    int indexOfHeadGroup(const Ms::NoteHead::Group group) const;

    QList<HeadTypeData> m_noteheadTypeDataList;
    QHash<int, QByteArray> m_roleNames;
    Ms::Score* m_score = nullptr;
    int m_selectedHeadTypeIndex = 0;
};

#endif // NOTEHEADTYPESMODEL_H
