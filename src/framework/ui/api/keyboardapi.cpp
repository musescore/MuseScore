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
#include "keyboardapi.h"

#include <QGuiApplication>
#include <QKeyEvent>
#include <QWindow>
#include <QKeySequence>
#include <QTimer>

#include "log.h"

using namespace muse::api;

KeyboardApi::KeyboardApi(api::IApiEngine* e)
    : api::ApiObject(e)
{
}

static int keyCode(const QString& key)
{
    QKeySequence seq = QKeySequence::fromString(key.toUpper());
    if (seq.count() == 0) {
        LOGE() << "not recognized key: " << key;
        return -1;
    }
    int code = seq[0].toCombined();
    return code;
}

static Qt::KeyboardModifier keyMod(const QString& key)
{
    static QMap<QString, Qt::KeyboardModifier> map = {
        { "SHIFT", Qt::ShiftModifier },
        { "CTRL", Qt::ControlModifier },
        { "ALT", Qt::AltModifier },
        { "META", Qt::MetaModifier },
        { "KEYPAD", Qt::KeypadModifier },
        { "GROUPSWITCH", Qt::GroupSwitchModifier }
    };

    return map.value(key.toUpper(), Qt::NoModifier);
}

QWindow* KeyboardApi::window() const
{
    QWindow* w = qApp->focusWindow();
    if (!w) {
        w = mainWindow()->qWindow();
    }

    return w;
}

void KeyboardApi::doPressKey(QWindow* w, int key, Qt::KeyboardModifier mod, const QString& text)
{
    QKeyEvent pressEvent(QEvent::KeyPress, key, mod, text);
    qApp->sendEvent(w, &pressEvent);
}

void KeyboardApi::doReleaseKey(QWindow* w, int key, Qt::KeyboardModifier mod, const QString& text)
{
    QKeyEvent* releaseEvent = new QKeyEvent(QEvent::KeyRelease, key, mod, text);
    qApp->postEvent(w, releaseEvent);
}

void KeyboardApi::key(const QString& key, const QString& mod)
{
    LOGD() << key;

    int code = keyCode(key);
    if (code < 0) {
        return;
    }

    QWindow* w = window();

    doPressKey(w, code, keyMod(mod), key);
    doReleaseKey(w, code, keyMod(mod), key);
}

void KeyboardApi::pressKey(const QString& key, const QString& mod)
{
    LOGD() << key;

    int code = keyCode(key);
    if (code < 0) {
        return;
    }

    QWindow* w = window();

    doPressKey(w, code, keyMod(mod), key);
}

void KeyboardApi::releaseKey(const QString& key, const QString& mod)
{
    LOGD() << key;

    int code = keyCode(key);
    if (code < 0) {
        return;
    }

    QWindow* w = window();

    doReleaseKey(w, code, keyMod(mod), key);
}

void KeyboardApi::repeatKey(const QString& k, int count)
{
    for (int i = 0; i < count; ++i) {
        QTimer::singleShot(10, this, [this, k]() {
            key(k);
        });
    }
}

void KeyboardApi::text(const QString& text)
{
    for (const QChar& ch : text) {
        QTimer::singleShot(10, this, [this, ch]() {
            key(ch);
        });
    }
}
