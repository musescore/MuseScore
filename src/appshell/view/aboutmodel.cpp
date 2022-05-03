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

#include <QClipboard>
#include <QUrl>
#include <QApplication>

using namespace mu::appshell;

AboutModel::AboutModel(QObject* parent)
    : QObject(parent)
{
}

QString AboutModel::museScoreVersion() const
{
    return (mu::framework::Version::unstable() ? qtrc("appshell",
                                                      "Unstable Prerelease for %1") : "%1").arg(QString::fromStdString(configuration()->
                                                                                                                       museScoreVersion()));
}

QString AboutModel::museScoreRevision() const
{
    return QString::fromStdString(configuration()->museScoreRevision());
}

QVariantMap AboutModel::museScoreUrl() const
{
    QUrl museScoreUrl(QString::fromStdString(configuration()->museScoreUrl()));
    return makeUrl(museScoreUrl.toString(), museScoreUrl.host());
}

QVariantMap AboutModel::museScoreForumUrl() const
{
    QUrl museScoreUrl(QString::fromStdString(configuration()->museScoreForumUrl()));
    return makeUrl(museScoreUrl.toString(), qtrc("appshell", "help"));
}

QVariantMap AboutModel::museScoreContributionUrl() const
{
    QUrl museScoreUrl(QString::fromStdString(configuration()->museScoreContributionUrl()));
    return makeUrl(museScoreUrl.toString(), qtrc("appshell", "contribute"));
}

QVariantMap AboutModel::museScorePrivacyPolicyUrl() const
{
    QUrl museScoreUrl(QString::fromStdString(configuration()->museScorePrivacyPolicyUrl()));
    return makeUrl(museScoreUrl.toString(), qtrc("appshell", "privacy policy"));
}

QVariantMap AboutModel::musicXMLLicenseUrl() const
{
    QUrl musicXMLLicenseUrl(QString::fromStdString(configuration()->musicXMLLicenseUrl()));
    return makeUrl(musicXMLLicenseUrl.toString(), musicXMLLicenseUrl.host() + musicXMLLicenseUrl.path());
}

QVariantMap AboutModel::musicXMLLicenseDeedUrl() const
{
    QUrl musicXMLLicenseDeedUrl(QString::fromStdString(configuration()->musicXMLLicenseDeedUrl()));
    return makeUrl(musicXMLLicenseDeedUrl.toString(), musicXMLLicenseDeedUrl.host() + musicXMLLicenseDeedUrl.path());
}

void AboutModel::copyRevisionToClipboard() const
{
    QApplication::clipboard()->setText(QString(
                                           "OS: %1, Arch.: %2, MuseScore version (%3-bit): %4-%5, revision: github-musescore-musescore-%6")
                                       .arg(QSysInfo::prettyProductName()).arg(QSysInfo::currentCpuArchitecture()).arg(QSysInfo::WordSize)
                                       .arg(VERSION).arg(BUILD_NUMBER).arg(MUSESCORE_REVISION));
}

QVariantMap AboutModel::makeUrl(const QString& url, const QString& displayName) const
{
    QVariantMap urlMap;
    urlMap["url"] = url;
    urlMap["displayName"] = displayName;

    return urlMap;
}
