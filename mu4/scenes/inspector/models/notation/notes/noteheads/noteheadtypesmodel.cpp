#include "noteheadtypesmodel.h"

NoteheadTypesModel::NoteheadTypesModel(QObject* parent)
    : QAbstractListModel(parent)
{
    m_roleNames.insert(HeadGroupRole, "headGroupRole");
    m_roleNames.insert(HintRole, "hintRole");
}

void NoteheadTypesModel::load()
{
    if (!m_noteheadTypeDataList.isEmpty()) {
        return;
    }

    for (int i = 0; i < static_cast<int>(Ms::NoteHead::Group::HEAD_DO_WALKER); ++i) {
        HeadTypeData headTypeData;

        headTypeData.group = static_cast<Ms::NoteHead::Group>(i);
        headTypeData.hint = Ms::NoteHead::group2userName(headTypeData.group);

        m_noteheadTypeDataList << headTypeData;
    }
}

QHash<int, QByteArray> NoteheadTypesModel::roleNames() const
{
    return m_roleNames;
}

int NoteheadTypesModel::rowCount(const QModelIndex&) const
{
    return m_noteheadTypeDataList.count();
}

QVariant NoteheadTypesModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount() || m_noteheadTypeDataList.isEmpty()) {
        return QVariant();
    }

    HeadTypeData headTypeData = m_noteheadTypeDataList.at(index.row());

    switch (role) {
    case HeadGroupRole: return static_cast<int>(headTypeData.group);
    case HintRole: return headTypeData.hint;
    default: return QVariant();
    }
}

void NoteheadTypesModel::init(const Ms::NoteHead::Group noteHeadGroup)
{
    emit layoutAboutToBeChanged();

    m_selectedHeadTypeIndex = indexOfHeadGroup(noteHeadGroup);
    emit selectedHeadTypeIndexChanged(m_selectedHeadTypeIndex);

    load();

    emit layoutChanged();
}

int NoteheadTypesModel::selectedHeadTypeIndex() const
{
    return m_selectedHeadTypeIndex;
}

void NoteheadTypesModel::setSelectedHeadTypeIndex(int selectedHeadTypeIndex)
{
    if (m_selectedHeadTypeIndex == selectedHeadTypeIndex) {
        return;
    }

    m_selectedHeadTypeIndex = selectedHeadTypeIndex;
    emit selectedHeadTypeIndexChanged(m_selectedHeadTypeIndex);

    emit noteHeadGroupSelected(static_cast<int>(m_noteheadTypeDataList.at(selectedHeadTypeIndex).group));
}

int NoteheadTypesModel::indexOfHeadGroup(const Ms::NoteHead::Group group) const
{
    for (int i = 0; i < m_noteheadTypeDataList.count(); ++i) {
        if (m_noteheadTypeDataList.at(i).group == group) {
            return i;
        }
    }

    return -1;
}
