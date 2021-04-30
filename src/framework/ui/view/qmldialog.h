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
#ifndef MU_UI_QMLDIALOG_H
#define MU_UI_QMLDIALOG_H

#include <QQuickItem>
#include <QQmlComponent>
#include <QQuickView>
#include <QJSValue>

#include "ret.h"
#include "navigationsection.h"
#include "navigationcontrol.h"

#include "modularity/ioc.h"
#include "ui/inavigationcontroller.h"

namespace mu::ui {
class QDialogWrap;
class QmlDialog : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QQmlComponent * content READ content WRITE setContent NOTIFY contentChanged)

    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString objectID READ objectID WRITE setObjectID NOTIFY objectIDChanged)
    Q_PROPERTY(bool modal READ modal WRITE setModal NOTIFY modalChanged)
    Q_PROPERTY(QSize fixedSize READ fixedSize WRITE setFixedSize NOTIFY fixedSizeChanged)

    Q_PROPERTY(QVariantMap ret READ ret WRITE setRet NOTIFY retChanged)

    Q_PROPERTY(mu::ui::NavigationSection* navigation READ navigation NOTIFY navigationChanged)
    Q_PROPERTY(mu::ui::NavigationControl* parentControl READ parentControl WRITE setParentControl NOTIFY parentControlChanged)

    Q_CLASSINFO("DefaultProperty", "content")

    INJECT(ui, INavigationController, navigationController)

public:
    QmlDialog(QQuickItem* parent = nullptr);

    QQmlComponent* content() const;
    QString title() const;
    QString objectID() const;
    bool modal() const;
    QSize fixedSize() const;
    QVariantMap ret() const;
    NavigationSection* navigation() const;
    NavigationControl* parentControl() const;

    Q_INVOKABLE void exec();
    Q_INVOKABLE void show();
    Q_INVOKABLE void hide();
    Q_INVOKABLE void reject();

public slots:
    void setContent(QQmlComponent* component);
    void setTitle(QString title);
    void setObjectID(QString objectID);
    void setModal(bool modal);
    void setFixedSize(QSize size);
    void setRet(QVariantMap ret);
    void setParentControl(NavigationControl* parentControl);

signals:
    void contentChanged();
    void titleChanged(QString title);
    void objectIDChanged(QString objectID);
    void modalChanged(bool modal);
    void fixedSizeChanged(QSize size);
    void retChanged(QVariantMap ret);
    void navigationChanged();
    void parentControlChanged();

    void opened();
    void closed();

private:
    void setErrCode(Ret::Code code);

    void componentComplete() override;
    bool eventFilter(QObject* watched, QEvent* event) override;

    void onShow();
    void onClose();

    QQuickView* m_view = nullptr;
    QWidget* m_widgetContainer = nullptr;
    QQmlComponent* m_content = nullptr;
    QDialogWrap* m_dialog = nullptr;
    QString m_objectID;
    QVariantMap m_ret;
    QSize m_fixedSize;
    NavigationSection* m_navigation = nullptr;
    NavigationControl* m_parentControl = nullptr;
};
}

#endif // MU_UI_QMLDIALOG_H
