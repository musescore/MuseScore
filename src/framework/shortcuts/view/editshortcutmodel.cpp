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

#include "editshortcutmodel.h"

#include <QKeySequence>

#include "translation.h"
#include "shortcutstypes.h"
#include "log.h"

using namespace mu::shortcuts;

static const QString ANY_CONTEXT("any");

EditShortcutModel::EditShortcutModel(QObject* parent)
    : QObject(parent)
{
}

void EditShortcutModel::load(const QVariant& originShortcut, const QVariantList& allShortcuts)
{
    clear();

    QVariantMap originShortcutMap = originShortcut.toMap();
    QString originCtx = originShortcutMap.value("context", ANY_CONTEXT).toString();
    bool isOriginCtxAny = originCtx == ANY_CONTEXT;

    for (const QVariant& shortcut : allShortcuts) {
        if (isOriginCtxAny) {
            m_potentialConflictShortcuts << shortcut;
            continue;
        }

        QVariantMap map = shortcut.toMap();
        QString ctx = map.value("context", ANY_CONTEXT).toString();

        if (ctx == ANY_CONTEXT) {
            m_potentialConflictShortcuts << shortcut;
            continue;
        }

        if (ctx == originCtx) {
            m_potentialConflictShortcuts << shortcut;
        }
    }

    m_originSequence = originShortcutMap.value("sequence").toString();

    emit originSequenceChanged(originSequenceInNativeFormat());
}

void EditShortcutModel::clear()
{
    m_inputtedSequence = QKeySequence();
    m_errorMessage.clear();

    emit inputtedSequenceChanged(QString());
}

void EditShortcutModel::inputKey(int key, Qt::KeyboardModifiers modifiers)
{
    std::pair<int, Qt::KeyboardModifiers> correctedKeyInput = correctKeyInput(key, modifiers);
    int newKey = correctedKeyInput.first;
    int newModifiers = correctedKeyInput.second;

    if (needIgnoreKey(newKey)) {
        return;
    }

    newKey += newModifiers;

    for (int i = 0; i < m_inputtedSequence.count(); i++) {
        if (m_inputtedSequence[i] == key) {
            return;
        }
    }

    switch (m_inputtedSequence.count()) {
    case 0:
        m_inputtedSequence = QKeySequence(newKey);
        break;
    case 1:
        m_inputtedSequence = QKeySequence(m_inputtedSequence[0], newKey);
        break;
    case 2:
        m_inputtedSequence = QKeySequence(m_inputtedSequence[0], m_inputtedSequence[1], newKey);
        break;
    case 3:
        m_inputtedSequence = QKeySequence(m_inputtedSequence[0], m_inputtedSequence[1], m_inputtedSequence[2], newKey);
        break;
    }

    validateInputtedSequence();

    emit inputtedSequenceChanged(inputtedSequenceInNativeFormat());
}

void EditShortcutModel::validateInputtedSequence()
{
    m_errorMessage.clear();

    QString input = inputtedSequence();

    for (const QVariant& shortcut : m_potentialConflictShortcuts) {
        QVariantMap sc = shortcut.toMap();

        if (sc.value("sequence").toString() == input) {
            QString title = sc.value("title").toString();
            m_errorMessage = qtrc("shortcuts", "Shortcut conflicts with %1").arg(title);
            return;
        }
    }
}

QString EditShortcutModel::originSequenceInNativeFormat() const
{
    std::vector<std::string> sequences = Shortcut::sequencesFromString(m_originSequence.toStdString());

    return sequencesToNativeText(sequences);
}

QString EditShortcutModel::inputtedSequenceInNativeFormat() const
{
    return m_inputtedSequence.toString(QKeySequence::NativeText);
}

QString EditShortcutModel::errorMessage() const
{
    return m_errorMessage;
}

bool EditShortcutModel::canApplyInputtedSequence() const
{
    return m_errorMessage.isEmpty() && !m_inputtedSequence.isEmpty();
}

void EditShortcutModel::replaceOriginSequence()
{
    m_originSequence = inputtedSequence();
    emit applyNewSequenceRequested(m_originSequence);
}

void EditShortcutModel::addToOriginSequence()
{
    if (!m_originSequence.isEmpty()) {
        m_originSequence += "; ";
    }
    m_originSequence += inputtedSequence();

    emit applyNewSequenceRequested(m_originSequence);
}

QString EditShortcutModel::inputtedSequence() const
{
    return m_inputtedSequence.toString();
}
