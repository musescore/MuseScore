/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef MU_INSTRUMENTSSCENE_INSTRUMENTLISTMODEL_H
#define MU_INSTRUMENTSSCENE_INSTRUMENTLISTMODEL_H

#include <QAbstractListModel>

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "notation/iinstrumentsrepository.h"

#include "uicomponents/view/itemmultiselectionmodel.h"

namespace mu::instrumentsscene {
class InstrumentListModel : public QAbstractListModel, public muse::async::Asyncable
{
    Q_OBJECT

    INJECT(notation::IInstrumentsRepository, repository)

    Q_PROPERTY(QStringList genres READ genres NOTIFY genresChanged)
    Q_PROPERTY(QStringList groups READ groups NOTIFY groupsChanged)

    Q_PROPERTY(int currentGenreIndex READ currentGenreIndex WRITE setCurrentGenreIndex NOTIFY currentGenreIndexChanged)
    Q_PROPERTY(int currentGroupIndex READ currentGroupIndex WRITE setCurrentGroupIndex NOTIFY currentGroupIndexChanged)

    Q_PROPERTY(bool hasSelection READ hasSelection NOTIFY selectionChanged)
    Q_PROPERTY(QVariant selectedInstrument READ selectedInstrument NOTIFY selectionChanged)

public:
    InstrumentListModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    QStringList genres() const;
    QStringList groups() const;

    int currentGenreIndex() const;
    int currentGroupIndex() const;

    bool hasSelection() const;
    QVariant selectedInstrument() const;

    Q_INVOKABLE void load(bool canSelectMultipleInstruments, const QString& currentInstrumentId);

    Q_INVOKABLE void saveCurrentGroup();
    Q_INVOKABLE void setSearchText(const QString& text);

    Q_INVOKABLE void selectInstrument(int instrumentIndex);
    Q_INVOKABLE QStringList selectedInstrumentIdList() const;

public slots:
    void setCurrentGenreIndex(int index);
    void setCurrentGroupIndex(int index);

signals:
    void genresChanged();
    void groupsChanged();

    void currentGenreIndexChanged();
    void currentGroupIndexChanged();

    void selectionChanged();

    void focusRequested(int groupIndex, int instrumentIndex);

private:
    enum Roles {
        RoleName = Qt::UserRole + 1,
        RoleDescription,
        RoleIsSelected,
        RoleTraits,
        RoleCurrentTraitIndex
    };

    struct CombinedInstrument
    {
        QString name;
        notation::InstrumentTemplateList templates;
        size_t currentTemplateIndex = 0;

        bool operator==(const CombinedInstrument& instrument) const
        {
            return name == instrument.name && templates == instrument.templates;
        }
    };

    using Instruments = QList<CombinedInstrument>;

    void init(const QString& genreId, const QString& groupId);

    void loadGenres();
    void loadGroups();
    void loadInstruments();
    void sortInstruments(Instruments& instruments) const;

    bool isSearching() const;

    void updateStateBySearch();

    bool isInstrumentAccepted(const notation::InstrumentTemplate& instrument, bool compareWithCurrentGroup = true) const;
    bool isInstrumentIndexValid(int index) const;

    void setCurrentGenre(const QString& genreId);
    void setCurrentGroup(const QString& groupId);
    void doSetCurrentGroup(const QString& groupId);

    QString m_currentGenreId;
    QString m_savedGenreId;
    QString m_currentGroupId;
    QString m_savedGroupId;
    QString m_searchText;

    Instruments m_instruments;
    muse::uicomponents::ItemMultiSelectionModel* m_selection = nullptr;

    notation::InstrumentGenreList m_genres;
    notation::InstrumentGroupList m_groups;

    bool m_instrumentsLoadingAllowed = false;
    bool m_saveCurrentGroup = false;
};
}

#endif // MU_INSTRUMENTSSCENE_INSTRUMENTLISTMODEL_H
