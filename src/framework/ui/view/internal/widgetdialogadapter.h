/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore BVBA and others
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
#pragma once

#include <QObject>

namespace muse::ui {
class WidgetDialogAdapter : public QObject
{
public:
    WidgetDialogAdapter(QDialog* parent, QWindow* window, bool staysOnTop = true);

    WidgetDialogAdapter& onShow(const std::function<void()>& func);
    WidgetDialogAdapter& onHide(const std::function<void()>& func);

    bool eventFilter(QObject* watched, QEvent* event) override;

private:
#ifdef Q_OS_MAC
    void updateStayOnTopHint();
#endif

    QDialog* m_dialog = nullptr;
    QWindow* m_window = nullptr;
    bool m_staysOnTop = true;
    std::function<void()> m_onShownCallBack;
    std::function<void()> m_onHideCallBack;
};
}
