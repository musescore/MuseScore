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
#include "lyricsspellingissuesmodel.h"

#include "engraving/dom/lyrics.h"

#include "log.h"

using namespace mu::notation;

LyricsSpellingIssuesModel::LyricsSpellingIssuesModel(QObject* parent)
    : QAbstractListModel(parent)
    , muse::Injectable(muse::iocCtxForQmlObject(this))
{
}

void LyricsSpellingIssuesModel::load()
{
    rescan();
}

void LyricsSpellingIssuesModel::rescan()
{
    if (!spellCheckService()) {
        LOGE() << "SpellCheckService not available";
        return;
    }

    auto notation = globalContext()->currentNotation();
    if (!notation) {
        LOGW() << "No notation available";
        return;
    }

    LyricsSpellCheckResult result = spellCheckService()->checkLyrics(notation);
    setResult(result);
}

void LyricsSpellingIssuesModel::setResult(const LyricsSpellCheckResult& result)
{
    beginResetModel();
    m_result = result;
    endResetModel();

    emit issuesChanged();
    emit languageChanged();
    emit availabilityChanged();
}

QVariant LyricsSpellingIssuesModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_result.issues.size()) {
        return QVariant();
    }

    const LyricsSpellCheckIssue& issue = m_result.issues.at(index.row());

    switch (role) {
    case WordRole:
        return issue.word;
    case CountRole:
        return issue.occurrenceCount;
    case LocationRole:
        return issue.firstLocation;
    }

    return QVariant();
}

int LyricsSpellingIssuesModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_result.issues.size();
}

QHash<int, QByteArray> LyricsSpellingIssuesModel::roleNames() const
{
    return {
        { WordRole, "word" },
        { CountRole, "count" },
        { LocationRole, "location" }
    };
}

int LyricsSpellingIssuesModel::issueCount() const
{
    return m_result.issues.size();
}

QString LyricsSpellingIssuesModel::language() const
{
    return m_result.language;
}

bool LyricsSpellingIssuesModel::isAvailable() const
{
    return m_result.spellCheckerAvailable;
}

int LyricsSpellingIssuesModel::totalWordsChecked() const
{
    return m_result.totalWordsChecked;
}

void LyricsSpellingIssuesModel::goToIssue(int index)
{
    if (index < 0 || index >= m_result.issues.size()) {
        return;
    }

    const LyricsSpellCheckIssue& issue = m_result.issues.at(index);
    mu::engraving::Lyrics* lyrics = issue.firstLyrics;

    if (!lyrics) {
        LOGW() << "No lyrics element for issue at index" << index;
        return;
    }

    auto inter = interaction();
    if (!inter) {
        LOGW() << "No interaction available";
        return;
    }

    // Select the lyrics element
    inter->clearSelection();
    inter->select({ lyrics }, SelectType::SINGLE, lyrics->staffIdx());

    // Scroll to show it
    inter->showItem(lyrics);

    LOGD() << "Navigated to lyrics:" << issue.word << "at" << issue.firstLocation;
}

INotationInteractionPtr LyricsSpellingIssuesModel::interaction() const
{
    auto notation = globalContext()->currentNotation();
    return notation ? notation->interaction() : nullptr;
}
