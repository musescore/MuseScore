//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#include "aboutmodel.h"

#include "translation.h"

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
    return QString::fromStdString(configuration()->museScoreVersion());
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

QVariantMap AboutModel::museScoreContributionUrl() const
{
    QUrl museScoreUrl(QString::fromStdString(configuration()->museScoreContributionUrl()));
    return makeUrl(museScoreUrl.toString(), qtrc("appshell", "contribution"));
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
    QApplication::clipboard()->setText(museScoreRevision());
}

QVariantMap AboutModel::makeUrl(const QString& url, const QString& displayName) const
{
    QVariantMap urlMap;
    urlMap["url"] = url;
    urlMap["displayName"] = displayName;

    return urlMap;
}
