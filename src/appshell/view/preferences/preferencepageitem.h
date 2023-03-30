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
#ifndef MU_APPSHELL_PREFERENCEPAGEITEM_H
#define MU_APPSHELL_PREFERENCEPAGEITEM_H

#include <QObject>
#include <QVariantMap>

#include "types/translatablestring.h"

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "languages/ilanguagesservice.h"

#include "ui/view/iconcodes.h"

namespace mu::appshell {
class PreferencePageItem : public QObject, public async::Asyncable
{
    Q_OBJECT

    INJECT(appshell, languages::ILanguagesService, languagesService)

    Q_PROPERTY(QString id READ id NOTIFY idChanged)
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)
    Q_PROPERTY(int icon READ icon NOTIFY iconChanged)
    Q_PROPERTY(QString path READ path NOTIFY pathChanged)
    Q_PROPERTY(bool expanded READ expanded NOTIFY expandedChanged)

public:
    explicit PreferencePageItem(QObject* parent = nullptr);
    virtual ~PreferencePageItem();

    QString id() const;
    QString title() const;
    int icon() const;
    QString path() const;
    bool expanded() const;

    PreferencePageItem* parentItem() const;
    void setParentItem(PreferencePageItem* parent);

    QList<PreferencePageItem*> childrenItems() const;
    bool isEmpty() const;

    PreferencePageItem* childAtRow(const int row) const;

    void appendChild(PreferencePageItem* child);

    int childCount() const;
    int row() const;

public slots:
    void setTitle(TranslatableString title);
    void setId(QString id);
    void setIcon(ui::IconCode::Code icon);
    void setPath(QString path);
    void setExpanded(bool expanded);

signals:
    void idChanged(QString id);
    void titleChanged();
    void iconChanged(int icon);
    void pathChanged(QString path);
    void expandedChanged(bool expanded);

private:
    QList<PreferencePageItem*> m_children;
    PreferencePageItem* m_parent = nullptr;

    TranslatableString m_title;
    QString m_id;
    ui::IconCode::Code m_icon = ui::IconCode::Code::NONE;
    QString m_path;
    bool m_expanded = false;
};
}

#endif // MU_APPSHELL_PREFERENCEPAGEITEM_H
