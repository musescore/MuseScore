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

#include "musesoundsdevtoolsmodel.h"

#include <QFile>

#include "global/translation.h"
#include "global/log.h"

using namespace mu::musesounds;
using namespace muse;

static QString langToCode(const QString& lang)
{
    static QMap<QString, QString> map = {
        { "English", "en" },
        { "Spanish", "es" },
        { "Chinese (Simplified)", "zh" },
        { "Chinese", "zh" },
        { "Mandarin", "zh" },
        { "Portuguese (BR)", "pt" },
        { "Portuguese", "pt" },
        { "Russian", "ru" },
        { "Japanese", "ja" },
        { "German", "de" },
        { "Korean", "ko" },
        { "Italian", "it" },
        { "French", "fr" },
        { "Dutch", "nl" }
    };

    return map.value(lang.trimmed(), lang.toLower());
}

MuseSoundsDevToolsModel::MuseSoundsDevToolsModel(QObject* parent)
    : QObject(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
}

bool MuseSoundsDevToolsModel::enableTestMode() const
{
    return configuration()->getSoundsTestMode();
}

void MuseSoundsDevToolsModel::setEnableTestMode(bool enabled)
{
    if (enableTestMode() == enabled) {
        return;
    }
    configuration()->setSoundsTestMode(enabled);
    emit enableTestModeChanged();
}

QString MuseSoundsDevToolsModel::heroImagePath() const
{
    return m_heroImagePath;
}

void MuseSoundsDevToolsModel::setHeroImagePath(const QString& path)
{
    if (m_heroImagePath == path) {
        return;
    }
    m_heroImagePath = path;
    emit heroImagePathChanged();
}

QString MuseSoundsDevToolsModel::updateNotes() const
{
    return m_updateNotes;
}

void MuseSoundsDevToolsModel::setUpdateNotes(const QString& notes)
{
    if (m_updateNotes == notes) {
        return;
    }
    m_updateNotes = notes;
    emit updateNotesChanged();
}

QString MuseSoundsDevToolsModel::ctaLink() const
{
    return m_ctaLink;
}

QString MuseSoundsDevToolsModel::selectedLanguage() const
{
    return m_selectedLanguage;
}

void MuseSoundsDevToolsModel::setSelectedLanguage(const QString& lang)
{
    if (m_selectedLanguage == lang) {
        return;
    }
    m_selectedLanguage = lang;
    emit selectedLanguageChanged();
}

QStringList MuseSoundsDevToolsModel::availableLanguages() const
{
    return m_availableLanguages;
}

void MuseSoundsDevToolsModel::updateAvailableLanguages()
{
    if (m_currentUpdateData.isNull()) {
        return;
    }

    QJsonObject dataObj = m_currentUpdateData.object();
    QJsonObject content = dataObj.value("content").toObject();
    QStringList langs = content.keys();
    if (m_availableLanguages != langs) {
        m_availableLanguages = langs;
        emit availableLanguagesChanged();
        if (!m_selectedLanguage.isEmpty() && !langs.contains(m_selectedLanguage) && !langs.isEmpty()) {
            setSelectedLanguage(langs.first());
        } else if (m_selectedLanguage.isEmpty() && !langs.isEmpty()) {
            setSelectedLanguage(langs.first());
        }
    }
}

void MuseSoundsDevToolsModel::setCtaLink(const QString& link)
{
    if (m_ctaLink == link) {
        return;
    }
    m_ctaLink = link;
    emit ctaLinkChanged();
}

int MuseSoundsDevToolsModel::version() const
{
    return m_version;
}

void MuseSoundsDevToolsModel::setVersion(int version)
{
    if (m_version == version) {
        return;
    }
    m_version = version;
    emit versionChanged();
}

QString MuseSoundsDevToolsModel::heroImageToBase64() const
{
    if (m_heroImagePath.isEmpty()) {
        return QString();
    }

    QFile file(m_heroImagePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }

    QByteArray imageData = file.readAll();
    file.close();

    return QString("data:image/png;base64,%1").arg(QString::fromUtf8(imageData.toBase64()));
}

QJsonObject MuseSoundsDevToolsModel::processUpdateNotes() const
{
    // ---------- temp storage ----------
    QMap<QString, QString> notes1;
    QMap<QString, QString> notes2;
    QList<QMap<QString, QString> > features;
    QMap<QString, QString> actionTitle;
    QMap<QString, QString> cancelTitle;

    QString currentBlock;
    QMap<QString, QString> currentFeature;

    auto flushFeature = [&]() {
        if (!currentFeature.isEmpty()) {
            features.append(currentFeature);
            currentFeature.clear();
        }
    };

    QStringList lines = m_updateNotes.split('\n');
    QString pendingLang;

    for (QString line : lines) {
        line = line.trimmed();
        if (line.isEmpty()) {
            continue;
        }

        QRegularExpression re("^\\[(.+)\\]");
        auto match = re.match(line);

        if (match.hasMatch()) {
            flushFeature();

            QString name = match.captured(1).toLower();

            if (name == "notes 1") {
                currentBlock = "notes_1";
            } else if (name == "notes 2") {
                currentBlock = "notes_2";
            } else if (name == "feature") {
                currentBlock = "feature";
            } else if (name == "action_title") {
                currentBlock = "action_title";
            } else if (name == "cancel_title") {
                currentBlock = "cancel_title";
            } else {
                currentBlock.clear();
            }

            continue;
        }

        if (line == "Language" || line == "Final Version" || line == "Translation") {
            continue;
        }

        // cancel_title special format
        if (currentBlock == "cancel_title" && line.contains(":")) {
            auto parts = line.split(":");
            QString code = langToCode(parts[0]);
            QString text = parts.mid(1).join(":").trimmed();

            cancelTitle[code] = text;
            continue;
        }

        if (pendingLang.isEmpty()) {
            pendingLang = line;
        } else {
            QString code = langToCode(pendingLang);

            if (currentBlock == "notes_1") {
                notes1[code] = line;
            } else if (currentBlock == "notes_2") {
                notes2[code] = line;
            } else if (currentBlock == "feature") {
                currentFeature[code] = line;
            } else if (currentBlock == "action_title") {
                actionTitle[code] = line;
            }

            pendingLang.clear();
        }
    }

    flushFeature();

    // ---------- build final pivot ----------
    QJsonObject result;

    QSet<QString> languages;

    languages.unite(QSet<QString>(notes1.keyBegin(), notes1.keyEnd()));
    languages.unite(QSet<QString>(actionTitle.keyBegin(), actionTitle.keyEnd()));
    languages.unite(QSet<QString>(cancelTitle.keyBegin(), cancelTitle.keyEnd()));

    for (const QString& lang : languages) {
        QJsonObject langObj;

        // notes combine
        QString html
            ="<p align=\"center\"><strong>" + notes1.value(lang)
              + "</strong></p><p align=\"center\">"
              + notes2.value(lang) + "</p>";

        langObj.insert("notes", html);

        // features array
        QJsonArray featArr;

        for (const auto& f : features) {
            if (f.contains(lang)) {
                featArr.append(f.value(lang));
            }
        }

        langObj.insert("features", featArr);

        // titles
        langObj.insert("action_title", actionTitle.value(lang));
        langObj.insert("cancel_title", cancelTitle.value(lang));

        result.insert(lang, langObj);
    }

    return result;
}

QJsonObject MuseSoundsDevToolsModel::processCTALink() const
{
    QJsonArray ctaLinkArray;
    if (!m_ctaLink.isEmpty()) {
        ctaLinkArray.append(m_ctaLink);
    }

    QJsonObject result;
    result["windows"] = ctaLinkArray;
    result["macos"] = ctaLinkArray;
    result["linux"] = ctaLinkArray;

    return result;
}

void MuseSoundsDevToolsModel::applyUpdateData()
{
    QJsonObject obj;
    obj["version"] = QString::number(m_version);
    obj["image_url"] = heroImageToBase64();
    obj["content"] = processUpdateNotes();
    obj["actions"] = processCTALink();

    setCurrentUpdateData(QJsonDocument(obj));

    updateAvailableLanguages();
}

void MuseSoundsDevToolsModel::openUpdateDialog()
{
    if (m_currentUpdateData.isNull()) {
        return;
    }

    UriQuery query("musescore://musesounds/musesoundsreleaseinfo");

    QJsonObject dataObj = m_currentUpdateData.object();
    QJsonObject content = dataObj.value("content").toObject();
    QString notes;
    ValList features;
    QString actionTitle = muse::qtrc("musesounds", "Take me to MuseHub");
    QString cancelTitle = muse::qtrc("musesounds", "No thanks");

    if (!m_selectedLanguage.isEmpty() && content.contains(m_selectedLanguage)) {
        QJsonObject langContent = content[m_selectedLanguage].toObject();
        notes = langContent["notes"].toString();
        if (langContent.contains("features")) {
            QJsonArray featArr = langContent["features"].toArray();
            for (const QJsonValue& v : featArr) {
                features.push_back(Val(v.toString().toStdString()));
            }
        }
        if (!langContent["action_title"].toString().isEmpty()) {
            actionTitle = langContent["action_title"].toString();
        }
        if (!langContent["cancel_title"].toString().isEmpty()) {
            cancelTitle = langContent["cancel_title"].toString();
        }
    }

    query.addParam("notes", Val(notes.toStdString()));
    query.addParam("features", Val(features));
    query.addParam("actionTitle", Val(actionTitle.toStdString()));
    query.addParam("cancelTitle", Val(cancelTitle.toStdString()));

    QString imageUrl;
    if (dataObj.contains("image_url")) {
        QString _imageUrl = dataObj.value("image_url").toString();
        if (!_imageUrl.isEmpty()) {
            imageUrl = _imageUrl;
        }
    }
    if (imageUrl.isEmpty()) {
        imageUrl = "qrc:/qt/qml/MuseScore/MuseSounds/resources/muse_sounds_promo.png";
    }
    query.addParam("imageUrl", Val(imageUrl.toStdString()));

    RetVal<Val> rv = interactive()->openSync(query);
    if (!rv.ret) {
        LOGD() << rv.ret.toString();
        return;
    }

    std::string actionCode = rv.val.toString();
    if (actionCode == "openMuseHub" && !m_ctaLink.isEmpty()) {
        interactive()->openUrl(m_ctaLink.toStdString());
    }
}

QString MuseSoundsDevToolsModel::currentUpdateData_property() const
{
    return m_currentUpdateData.toJson();
}

void MuseSoundsDevToolsModel::setCurrentUpdateData_property(const QString& data)
{
    auto newJsonDoc = QJsonDocument::fromJson(data.toUtf8());
    if (newJsonDoc == m_currentUpdateData) {
        return;
    }

    m_currentUpdateData = newJsonDoc;
    updateAvailableLanguages();
    emit currentUpdateDataChanged();
}

void MuseSoundsDevToolsModel::setCurrentUpdateData(const QJsonDocument& data)
{
    if (m_currentUpdateData == data) {
        return;
    }

    m_currentUpdateData = data;
    emit currentUpdateDataChanged();
}
