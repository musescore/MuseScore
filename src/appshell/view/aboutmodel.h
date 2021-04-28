/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
#ifndef MU_APPSHELL_ABOUTMODEL_H
#define MU_APPSHELL_ABOUTMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "iappshellconfiguration.h"

namespace mu::appshell {
class AboutModel : public QObject
{
    Q_OBJECT

    INJECT(appshell, IAppShellConfiguration, configuration)

public:
    explicit AboutModel(QObject* parent = nullptr);

    Q_INVOKABLE QString museScoreVersion() const;
    Q_INVOKABLE QString museScoreRevision() const;
    Q_INVOKABLE QVariantMap museScoreUrl() const;
    Q_INVOKABLE QVariantMap museScoreForumUrl() const;
    Q_INVOKABLE QVariantMap museScoreContributionUrl() const;

    Q_INVOKABLE QVariantMap musicXMLLicenseUrl() const;
    Q_INVOKABLE QVariantMap musicXMLLicenseDeedUrl() const;

    Q_INVOKABLE void copyRevisionToClipboard() const;

private:
    QVariantMap makeUrl(const QString& url, const QString& displayName) const;
};
}

#endif // MU_APPSHELL_ABOUTMODEL_H
