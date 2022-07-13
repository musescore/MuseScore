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
#include "aboutmodel.h"

#include "translation.h"
#include "version.h"
#include "config.h"

#include <QApplication>
#include <QClipboard>
#include <QUrl>

using namespace mu::appshell;

AboutModel::AboutModel(QObject* parent)
    : QObject(parent)
{
}

QString AboutModel::museScoreVersion() const
{
    QString version = QString::fromStdString(configuration()->museScoreVersion());
    return mu::framework::Version::unstable()
           ? qtrc("appshell", "Unstable Prerelease for %1").arg(version)
           : version;
}

QString AboutModel::museScoreRevision() const
{
    return QString::fromStdString(configuration()->museScoreRevision());
}

QVariantMap AboutModel::museScoreUrl() const
{
    QUrl museScoreUrl(QString::fromStdString(configuration()->museScoreUrl()));
    return makeUrl(museScoreUrl, false);
}

QVariantMap AboutModel::museScoreForumUrl() const
{
    QUrl museScoreForumUrl(QString::fromStdString(configuration()->museScoreForumUrl()));
    return makeUrl(museScoreForumUrl);
}

QVariantMap AboutModel::museScoreContributionUrl() const
{
    QUrl museScoreContributionUrl(QString::fromStdString(configuration()->museScoreContributionUrl()));
    return makeUrl(museScoreContributionUrl);
}

QVariantMap AboutModel::museScorePrivacyPolicyUrl() const
{
    QUrl museScorePrivacyPolicyUrl(QString::fromStdString(configuration()->museScorePrivacyPolicyUrl()));
    return makeUrl(museScorePrivacyPolicyUrl);
}

QVariantMap AboutModel::musicXMLLicenseUrl() const
{
    QUrl musicXMLLicenseUrl(QString::fromStdString(configuration()->musicXMLLicenseUrl()));
    return makeUrl(musicXMLLicenseUrl);
}

QVariantMap AboutModel::musicXMLLicenseDeedUrl() const
{
    QUrl musicXMLLicenseDeedUrl(QString::fromStdString(configuration()->musicXMLLicenseDeedUrl()));
    return makeUrl(musicXMLLicenseDeedUrl);
}

void AboutModel::copyRevisionToClipboard() const
{
    QApplication::clipboard()->setText(
        QString("OS: %1, Arch.: %2, MuseScore version (%3-bit): %4-%5, revision: github-musescore-musescore-%6")
        .arg(QSysInfo::prettyProductName())
        .arg(QSysInfo::currentCpuArchitecture())
        .arg(QSysInfo::WordSize)
        .arg(VERSION)
        .arg(BUILD_NUMBER)
        .arg(MUSESCORE_REVISION));
}

QVariantMap AboutModel::makeUrl(const QUrl& url, bool showPath) const
{
    QVariantMap urlMap;
    urlMap["url"] = url.toString();
    urlMap["displayName"] = showPath ? url.host() + url.path() : url.host();

    return urlMap;
}
