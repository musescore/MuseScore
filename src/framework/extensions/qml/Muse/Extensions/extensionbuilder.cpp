/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited and others
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
#include "extensionbuilder.h"

#include <QQmlEngine>

#include "global/types/number.h"
#include "global/async/async.h"
#include "global/io/file.h"

#include "api/v1/ipluginapiv1.h"

#include "log.h"

using namespace muse::extensions;

const QString ERROR_MSG_PATH = ":/qml/Muse/Extensions/ExtensionErrorMessage.qml";

ExtensionBuilder::ExtensionBuilder(QObject* parent)
    : QObject(parent), Injectable(muse::iocCtxForQmlObject(this))
{}

QString ExtensionBuilder::validateImports(const io::path_t& qmlFilePath) const
{
    ByteArray data;
    Ret ret = io::File::readFile(qmlFilePath, data);
    if (!ret) {
        return "failed read file: " + qmlFilePath.toQString() + ", err: " + QString::fromStdString(ret.toString());
    }

    QByteArray qdata = data.toQByteArrayNoCopy();
    QString content = QString::fromUtf8(qdata);
    QStringList lines = content.split('\n');
    for (const QString& line_ : std::as_const(lines)) {
        QString line = line_.trimmed();
        if (line.startsWith("import") && line.contains("Muse.")) {
            return "Importing 'Muse.' modules is prohibited.";
        }
    }
    return QString();
}

void ExtensionBuilder::load(const QString& uri, QObject* itemParent)
{
    const UriQuery q = UriQuery(uri.toStdString());
    const Action a = provider()->action(q);
    if (!a.isValid()) {
        LOGE() << "Not found action, uri: " << uri;
        return;
    }

    setTitle(a.title);

    QQmlEngine* engin = nullptr;
    if (a.apiversion == 1) {
        engin = engine()->qmlEngineApiV1();
    } else {
        engin = engine()->qmlEngine();
    }

    const Manifest& manifest = provider()->manifest(q.uri());
    QObject* qmlObj = nullptr;

    QString errorString;
    if (manifest.apiversion != 1) {
        errorString = validateImports(a.path);
    }

    if (errorString.isEmpty()) {
        //! NOTE We create extension UI using a separate engine to control what we provide,
        //! making it easier to maintain backward compatibility and stability.
        QQmlComponent component = QQmlComponent(engin, a.path.toQString());
        if (component.isReady()) {
            qmlObj = component.createWithInitialProperties({ { "parent", QVariant::fromValue(itemParent) } });
        } else {
            errorString = component.errorString();
            LOGE() << "Failed to load QML file: " << a.path << ", from extension: " << uri;
        }
    }

    if (!errorString.isEmpty()) {
        LOGE() << errorString;

        QQmlComponent component = QQmlComponent(engin, ERROR_MSG_PATH);
        if (component.isReady()) {
            qmlObj = component.createWithInitialProperties({
                { "parent", QVariant::fromValue(itemParent) },
                { "text", errorString }
            });
        } else {
            LOGE() << "Failed to load QML file: " << a.path << ", from extension: " << uri;
            LOGE() << component.errorString();
            return;
        }
    }

    m_contentItem = qobject_cast<QQuickItem*>(qmlObj);
    if (!m_contentItem) {
        LOGE() << "Component not QuickItem, file: " << a.path << ", from extension: " << uri;
    }

    if (m_contentItem) {
        if (muse::is_zero(m_contentItem->implicitHeight())) {
            m_contentItem->setImplicitHeight(m_contentItem->height());
            if (muse::is_zero(m_contentItem->implicitHeight())) {
                m_contentItem->setImplicitHeight(480);
            }
        }

        if (muse::is_zero(m_contentItem->implicitWidth())) {
            m_contentItem->setImplicitWidth(m_contentItem->width());
            if (muse::is_zero(m_contentItem->implicitWidth())) {
                m_contentItem->setImplicitWidth(600);
            }
        }
    }

    emit contentItemChanged();

    if (a.apiversion == 1) {
        apiv1::IPluginApiV1* plugin = dynamic_cast<apiv1::IPluginApiV1*>(m_contentItem);

        plugin->setup(engin);

        plugin->closeRequest().onNotify(this, [this]() {
            emit closeRequested();
        });

        //! NOTE For version 1 plugins we need to call run
        async::Async::call(this, [plugin, a, uri]() {
            if (!plugin) {
                LOGE() << "Qml Object not MuseScore plugin: " << a.path
                       << ", from extension: " << uri;
                return;
            }
            plugin->runPlugin();
        });
    }
}

QQuickItem* ExtensionBuilder::contentItem() const
{
    return m_contentItem;
}

void ExtensionBuilder::setTitle(QString title)
{
    if (m_title == title) {
        return;
    }
    m_title = title;
    emit titleChanged();
}

QString ExtensionBuilder::title() const
{
    return m_title;
}
