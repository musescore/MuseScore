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

#ifndef MU_PLAYBACK_ABSTRACTAUDIORESOURCEITEM_H
#define MU_PLAYBACK_ABSTRACTAUDIORESOURCEITEM_H

#include <QObject>

#include "async/asyncable.h"

#include "audio/audiotypes.h"

#include "types/uri.h"

namespace mu::playback {
class AbstractAudioResourceItem : public QObject, public muse::async::Asyncable
{
    Q_OBJECT

    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(bool isBlank READ isBlank NOTIFY isBlankChanged)
    Q_PROPERTY(bool isActive READ isActive NOTIFY isActiveChanged)
    Q_PROPERTY(bool hasNativeEditorSupport READ hasNativeEditorSupport NOTIFY hasNativeEditorSupportChanged)

public:
    explicit AbstractAudioResourceItem(QObject* parent);
    ~AbstractAudioResourceItem() override;

    virtual Q_INVOKABLE void requestAvailableResources() {}
    virtual Q_INVOKABLE void requestToLaunchNativeEditorView();
    virtual Q_INVOKABLE void handleMenuItem(const QString& menuItemId) { Q_UNUSED(menuItemId) }

    virtual QString title() const;
    virtual bool isBlank() const;
    virtual bool isActive() const;
    virtual bool hasNativeEditorSupport() const;

    const muse::UriQuery& editorUri() const;
    void setEditorUri(const muse::UriQuery& uri);

signals:
    void titleChanged();
    void isBlankChanged();
    void isActiveChanged();
    void hasNativeEditorSupportChanged();

    void nativeEditorViewLaunchRequested();
    void nativeEditorViewCloseRequested();
    void availableResourceListResolved(const QVariantList& resources);

protected:
    QVariantMap buildMenuItem(const QString& itemId, const QString& title, const bool checked,
                              const QVariantList& subItems = QVariantList()) const;

    QVariantMap buildSeparator() const;

    QVariantMap buildExternalLinkMenuItem(const QString& menuId, const QString& title) const;

    void sortResourcesList(muse::audio::AudioResourceMetaList& list);

    void updateNativeEditorView();

private:
    void doRequestToLaunchNativeEditorView();

    muse::UriQuery m_editorUri;
};
}

#endif // MU_PLAYBACK_ABSTRACTAUDIORESOURCEITEM_H
