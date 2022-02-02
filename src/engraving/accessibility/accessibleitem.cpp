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
#include "accessibleitem.h"

#include "accessibleroot.h"
#include "../libmscore/score.h"

#include "log.h"

using namespace mu::engraving;
using namespace mu::accessibility;
using namespace Ms;

bool AccessibleItem::enabled = true;

AccessibleItem::AccessibleItem(Ms::EngravingItem* e, Role role)
    : m_element(e), m_role(role)
{
}

AccessibleItem::~AccessibleItem()
{
    AccessibleRoot* root = accessibleRoot();
    if (root && root->focusedElement() == this) {
        root->setFocusedElement(nullptr);
    }

    if (m_registred && accessibilityController()) {
        accessibilityController()->unreg(this);
        m_registred = false;
    }

    m_element = nullptr;
}

AccessibleItem* AccessibleItem::clone(Ms::EngravingItem* e) const
{
    return new AccessibleItem(e, m_role);
}

void AccessibleItem::setup()
{
    if (!AccessibleItem::enabled) {
        return;
    }

    if (!accessibilityController()) {
        return;
    }

    accessibilityController()->reg(this);
    m_registred = true;
}

AccessibleRoot* AccessibleItem::accessibleRoot() const
{
    if (!m_element) {
        return nullptr;
    }

    Ms::Score* score = m_element->score();
    if (!score) {
        return nullptr;
    }

    RootItem* rootItem = m_element->explicitParent() ? score->rootItem() : score->dummy()->rootItem();
    return dynamic_cast<AccessibleRoot*>(rootItem->accessible());
}

const Ms::EngravingItem* AccessibleItem::element() const
{
    return m_element;
}

bool AccessibleItem::registered() const
{
    return m_registred;
}

void AccessibleItem::notifyAboutFocus(bool focused)
{
    m_accessibleStateChanged.send(IAccessible::State::Focused, focused);
}

const IAccessible* AccessibleItem::accessibleParent() const
{
    Ms::EngravingObject* p = m_element->parent();
    if (!p || !p->isEngravingItem()) {
        return nullptr;
    }

    return static_cast<EngravingItem*>(p)->accessible();
}

size_t AccessibleItem::accessibleChildCount() const
{
    TRACEFUNC;
    size_t count = 0;
    for (const EngravingObject* obj : m_element->children()) {
        if (obj->isEngravingItem()) {
            AccessibleItem* access = Ms::toEngravingItem(obj)->accessible();
            if (access && access->registered()) {
                ++count;
            }
        }
    }
    return count;
}

const IAccessible* AccessibleItem::accessibleChild(size_t i) const
{
    TRACEFUNC;
    size_t count = 0;
    for (const EngravingObject* obj : m_element->children()) {
        if (obj->isEngravingItem()) {
            AccessibleItem* access = Ms::toEngravingItem(obj)->accessible();
            if (access && access->registered()) {
                if (count == i) {
                    return access;
                }
                ++count;
            }
        }
    }
    return nullptr;
}

IAccessible::Role AccessibleItem::accessibleRole() const
{
    return m_role;
}

QString AccessibleItem::accessibleName() const
{
    return m_element->accessibleInfo();
}

QString AccessibleItem::accessibleDescription() const
{
    QString result;
#ifdef Q_OS_MACOS
    result = accessibleName() + " ";
#endif

    result += m_element->accessibleExtraInfo();
    return result;
}

QVariant AccessibleItem::accessibleValue() const
{
    return QVariant();
}

QVariant AccessibleItem::accessibleMaximumValue() const
{
    return QVariant();
}

QVariant AccessibleItem::accessibleMinimumValue() const
{
    return QVariant();
}

QVariant AccessibleItem::accessibleValueStepSize() const
{
    return QVariant();
}

void AccessibleItem::accessibleSelection(int selectionIndex, int* startOffset, int* endOffset) const
{
    Ms::TextCursor* textCursor = this->textCursor();
    if (selectionIndex == 0 && textCursor && textCursor->hasSelection()) {
        Ms::TextCursor::Range selectionRange = textCursor->selectionRange();
        *startOffset = selectionRange.startPosition;
        *endOffset = selectionRange.endPosition;
    } else {
        *startOffset = 0;
        *endOffset = 0;
    }
}

int AccessibleItem::accessibleSelectionCount() const
{
    Ms::TextCursor* textCursor = this->textCursor();
    if (!textCursor) {
        return 0;
    }

    return textCursor->selectedText().length();
}

int AccessibleItem::accessibleCursorPosition() const
{
    Ms::TextCursor* textCursor = this->textCursor();
    if (!textCursor) {
        return 0;
    }

    return textCursor->currentPosition();
}

QString AccessibleItem::accessibleText(int startOffset, int endOffset) const
{
    if (!m_element || !m_element->isTextBase()) {
        return QString();
    }

    Ms::TextCursor* textCursor = new Ms::TextCursor(Ms::toTextBase(m_element));
    auto startCoord = textCursor->positionToLocalCoord(startOffset);
    if (startCoord.first == -1 || startCoord.second == -1) {
        return QString();
    }

    textCursor->setRow(startCoord.first);
    textCursor->setColumn(startCoord.second);
    textCursor->movePosition(Ms::TextCursor::MoveOperation::Right, Ms::TextCursor::MoveMode::KeepAnchor, endOffset - startOffset);

    textCursor->setSelectLine(startCoord.first);
    textCursor->setSelectColumn(startCoord.second);

    QString text = textCursor->selectedText();
    delete textCursor;

    return text;
}

QString AccessibleItem::accessibleTextAtOffset(int offset, TextBoundaryType boundaryType, int* startOffset, int* endOffset) const
{
    if (!m_element || !m_element->isTextBase()) {
        return QString();
    }

    QString result;

    Ms::TextCursor* textCursor = new Ms::TextCursor(Ms::toTextBase(m_element));
    auto startCoord = textCursor->positionToLocalCoord(offset);
    if (startCoord.first == -1 || startCoord.second == -1) {
        return QString();
    }

    textCursor->setRow(startCoord.first);
    textCursor->setColumn(startCoord.second);

    switch (boundaryType) {
    case CharBoundary: {
        *startOffset = textCursor->currentPosition();
        textCursor->movePosition(Ms::TextCursor::MoveOperation::Right, Ms::TextCursor::MoveMode::KeepAnchor);
        *endOffset = textCursor->currentPosition();
        break;
    }
    case WordBoundary: {
        textCursor->movePosition(Ms::TextCursor::MoveOperation::WordLeft, Ms::TextCursor::MoveMode::MoveAnchor);
        *startOffset = textCursor->currentPosition();
        textCursor->movePosition(Ms::TextCursor::MoveOperation::NextWord, Ms::TextCursor::MoveMode::KeepAnchor);
        *endOffset = textCursor->currentPosition();
        break;
    }
    case LineBoundary: {
        textCursor->movePosition(Ms::TextCursor::MoveOperation::StartOfLine, Ms::TextCursor::MoveMode::MoveAnchor);
        *startOffset = textCursor->currentPosition();
        textCursor->movePosition(Ms::TextCursor::MoveOperation::EndOfLine, Ms::TextCursor::MoveMode::KeepAnchor);
        *endOffset = textCursor->currentPosition();
        break;
    }
    default:
        NOT_IMPLEMENTED;
        break;
    }

    textCursor->setSelectLine(startCoord.first);
    textCursor->setSelectColumn(startCoord.second);

    result = textCursor->selectedText();

    delete textCursor;
    return result;
}

int AccessibleItem::accessibleCharacterCount() const
{
    if (!m_element || !m_element->isTextBase()) {
        return 0;
    }

    Ms::TextBase* text = Ms::toTextBase(m_element);
    return text->plainText().length();
}

bool AccessibleItem::accessibleState(State st) const
{
    if (!registered()) {
        return false;
    }

    switch (st) {
    case IAccessible::State::Enabled: return true;
    case IAccessible::State::Active: return true;
    case IAccessible::State::Focused: {
        auto root = accessibleRoot();
        return root ? root->focusedElement() == this : false;
    }
    case IAccessible::State::Selected: return m_element->selected();
    default:
        break;
    }
    return false;
}

QRect AccessibleItem::accessibleRect() const
{
    if (!registered()) {
        return QRect();
    }

    RectF bbox = m_element->canvasBoundingRect();
    RectF canvasRect(m_element->canvasPos(), SizeF(bbox.width(), bbox.height()));

    auto rect = accessibleRoot()->toScreenRect(canvasRect).toQRect();
    return rect;
}

mu::async::Channel<IAccessible::Property, mu::Val> AccessibleItem::accessiblePropertyChanged() const
{
    return m_accessiblePropertyChanged;
}

mu::async::Channel<IAccessible::State, bool> AccessibleItem::accessibleStateChanged() const
{
    return m_accessibleStateChanged;
}

TextCursor* AccessibleItem::textCursor() const
{
    if (!m_element || !m_element->isTextBase()) {
        return nullptr;
    }

    Ms::TextBase* text = Ms::toTextBase(m_element);
    return text ? text->cursor() : nullptr;
}
