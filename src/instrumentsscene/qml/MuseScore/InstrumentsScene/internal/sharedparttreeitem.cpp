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
#include "sharedparttreeitem.h"

#include <algorithm>
#include <limits>

#include <QRegularExpression>

#include "engraving/dom/sharedpart.h"

#include "notation/inotation.h"

using namespace mu::instrumentsscene;
using namespace mu::notation;
using namespace muse;

SharedPartTreeItem::SharedPartTreeItem(IMasterNotationPtr masterNotation, INotationPtr notation, QObject* parent)
    : PartTreeItem(masterNotation, notation, parent, LayoutPanelItemType::ItemType::SHARED_PART)
{
    setSettingsAvailable(false);

    listenEnabledChanged();
}

void SharedPartTreeItem::init(const notation::Part* part)
{
    PartTreeItem::init(part);

    setTitle(groupTitle());
    setSettingsAvailable(false);
    setIsExpandable(true);
    setIsEnabled(engraving::toSharedPart(part)->enabled());

    m_ignoreEnabledChanged = false;
}

void SharedPartTreeItem::onScoreChanged(const mu::engraving::ScoreChanges& changes)
{
    updateVisibility();

    if (isTitleAffected(changes)) {
        setTitle(groupTitle());
    }

    m_ignoreEnabledChanged = true;
    setIsEnabled(engraving::toSharedPart(part())->enabled());
    m_ignoreEnabledChanged = false;
}

void SharedPartTreeItem::listenEnabledChanged()
{
    connect(this, &AbstractLayoutPanelTreeItem::isEnabledChanged, this, [this](bool enable) {
        if (m_ignoreEnabledChanged) {
            return;
        }

        INotationPartsPtr parts = notation()->parts();
        if (!parts) {
            return;
        }

        if (parts->partExists(id())) {
            parts->setSharedPartEnabled(id(), enable);
        }
    });
}

bool SharedPartTreeItem::isTitleAffected(const mu::engraving::ScoreChanges& changes) const
{
    const engraving::SharedPart* sharedPart = engraving::toSharedPart(part());
    const std::vector<engraving::Part*>& originParts = sharedPart->originParts();

    for (const auto& changed : changes.changedObjects) {
        const engraving::EngravingObject* object = changed.first;
        if (object == sharedPart || std::find(originParts.cbegin(), originParts.cend(), object) != originParts.cend()) {
            return true;
        }
    }

    return false;
}

QString SharedPartTreeItem::groupTitle() const
{
    static const QRegularExpression trailingNumber(QStringLiteral("^(.*?)\\s*(\\d+)$"));

    const engraving::SharedPart* sharedPart = engraving::toSharedPart(part());

    QString baseName;
    bool hasMixedBaseNames = false;
    int firstNumber = std::numeric_limits<int>::max();
    int lastNumber = std::numeric_limits<int>::min();

    for (const engraving::Part* originPart : sharedPart->originParts()) {
        const QString name = originPart->partName().toQString().simplified();
        const QRegularExpressionMatch match = trailingNumber.match(name);

        int number = originPart->instrument()->number();
        if (number <= 0 && match.hasMatch()) {
            number = match.captured(2).toInt();
        }

        const QString originBaseName = match.hasMatch() ? match.captured(1) : name;
        if (baseName.isEmpty()) {
            baseName = originBaseName;
        } else if (originBaseName != baseName) {
            hasMixedBaseNames = true;
        }

        if (number > 0) {
            firstNumber = std::min(firstNumber, number);
            lastNumber = std::max(lastNumber, number);
        }
    }

    if (hasMixedBaseNames || baseName.isEmpty() || firstNumber >= lastNumber) {
        return sharedPart->partName().toQString().simplified();
    }

    return baseName + QChar(u' ') + QString::number(firstNumber) + QChar(u'–') + QString::number(lastNumber);
}
