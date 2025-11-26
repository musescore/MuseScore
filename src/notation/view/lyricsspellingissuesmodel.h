/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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
#ifndef MU_NOTATION_LYRICSSPELLINGISSUESMODEL_H
#define MU_NOTATION_LYRICSSPELLINGISSUESMODEL_H

#include <QAbstractListModel>

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "notation/ilyricsspellcheckservice.h"
#include "notation/inotationinteraction.h"

namespace mu::notation {
class LyricsSpellingIssuesModel : public QAbstractListModel, public muse::Injectable
{
    Q_OBJECT

    Q_PROPERTY(int issueCount READ issueCount NOTIFY issuesChanged)
    Q_PROPERTY(QString language READ language NOTIFY languageChanged)
    Q_PROPERTY(bool isAvailable READ isAvailable NOTIFY availabilityChanged)
    Q_PROPERTY(int totalWordsChecked READ totalWordsChecked NOTIFY issuesChanged)

    muse::Inject<context::IGlobalContext> globalContext = { this };
    muse::Inject<ILyricsSpellCheckService> spellCheckService = { this };

public:
    explicit LyricsSpellingIssuesModel(QObject* parent = nullptr);

    enum Roles {
        WordRole = Qt::UserRole + 1,
        CountRole,
        LocationRole
    };

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    int issueCount() const;
    QString language() const;
    bool isAvailable() const;
    int totalWordsChecked() const;

    Q_INVOKABLE void load();
    Q_INVOKABLE void rescan();
    Q_INVOKABLE void goToIssue(int index);

signals:
    void issuesChanged();
    void languageChanged();
    void availabilityChanged();

private:
    void setResult(const LyricsSpellCheckResult& result);
    INotationInteractionPtr interaction() const;

    LyricsSpellCheckResult m_result;
};
}

#endif // MU_NOTATION_LYRICSSPELLINGISSUESMODEL_H
