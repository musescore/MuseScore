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
#include "accessibleitem.h"

#include "accessibleroot.h"
#include "../dom/score.h"
#include "../dom/measure.h"

#include "translation.h"
#include "log.h"

using namespace mu;
using namespace mu::engraving;
using namespace muse::accessibility;
using namespace mu::engraving;

bool AccessibleItem::enabled = true;

static QString readable(QString s)
{
    // Remove undesired pauses in screen reader speech output
    s.remove(u':');

    // Handle desired pauses
    s.replace(u';', u','); // NVDA doesn't pause for semicolon

    return s;
}

AccessibleItem::AccessibleItem(EngravingItem* e, Role role)
    : muse::Injectable(e->iocContext()), m_element(e), m_role(role)
{
}

AccessibleItem::~AccessibleItem()
{
    m_element = nullptr;

    if (m_registred && accessibilityController()) {
        accessibilityController()->unreg(this);
        m_registred = false;
    }
}

AccessibleItem* AccessibleItem::clone(EngravingItem* e) const
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

    if (m_element->isType(ElementType::ROOT_ITEM)) {
        return dynamic_cast<AccessibleRoot*>(m_element->accessible().get());
    }

    Score* score = m_element->score();
    if (!score) {
        return nullptr;
    }

    RootItem* rootItem = m_element->explicitParent() ? score->rootItem() : score->dummy()->rootItem();
    return dynamic_cast<AccessibleRoot*>(rootItem->accessible().get());
}

const EngravingItem* AccessibleItem::element() const
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
    if (!m_element) {
        return nullptr;
    }

    EngravingItem* p = m_element->parentItem(false /*not explicit*/);
    if (!p) {
        return nullptr;
    }

    return p->accessible().get();
}

size_t AccessibleItem::accessibleChildCount() const
{
    TRACEFUNC;

    if (!m_element) {
        return 0;
    }

    size_t count = 0;
    for (const EngravingObject* obj : m_element->children()) {
        if (obj->isEngravingItem()) {
            AccessibleItemPtr access = toEngravingItem(obj)->accessible();
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

    if (!m_element) {
        return nullptr;
    }

    size_t count = 0;
    for (const EngravingObject* obj : m_element->children()) {
        if (obj->isEngravingItem()) {
            AccessibleItemPtr access = toEngravingItem(obj)->accessible();
            if (access && access->registered()) {
                if (count == i) {
                    return access.get();
                }
                ++count;
            }
        }
    }
    return nullptr;
}

QWindow* AccessibleItem::accessibleWindow() const
{
    return nullptr;
}

muse::modularity::ContextPtr AccessibleItem::iocContext() const
{
    return muse::Injectable::iocContext();
}

IAccessible::Role AccessibleItem::accessibleRole() const
{
    return m_role;
}

QString AccessibleItem::accessibleName() const
{
    if (!m_element) {
        return QString();
    }

    AccessibleRoot* root = accessibleRoot();
    QString commandInfo = root ? root->commandInfo() : "";
    QString staffInfo = root ? root->staffInfo() : "";
    QString barsAndBeats = m_element->formatBarsAndBeats();

    barsAndBeats.remove(u';'); // Too many pauses in speech

    QString name = QString("%1%2%3%4%5%6")
                   .arg(!commandInfo.isEmpty() ? (commandInfo + "; ") : "")
                   .arg(!staffInfo.isEmpty() ? (staffInfo + "; ") : "")
                   .arg(m_element->screenReaderInfo().toQString())
                   .arg(m_element->visible() ? "" : " " + muse::qtrc("engraving", "invisible"))
                   .arg(!barsAndBeats.isEmpty() ? ("; " + barsAndBeats) : "")
                   .arg(root->isRangeSelection() ? ("; " + muse::qtrc("engraving", "selected")) : "");

    return readable(name);
}

QString AccessibleItem::accessibleDescription() const
{
    if (!m_element) {
        return QString();
    }

    AccessibleRoot* root = accessibleRoot();
    if (root->isRangeSelection()) {
        return readable(root->rangeSelectionInfo());
    }

    return readable(m_element->accessibleExtraInfo());
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
    TextCursor* textCursor = this->textCursor();
    if (selectionIndex == 0 && textCursor && textCursor->hasSelection()) {
        TextCursor::Range selectionRange = textCursor->selectionRange();
        *startOffset = selectionRange.startPosition;
        *endOffset = selectionRange.endPosition;
    } else {
        *startOffset = -1;
        *endOffset = -1;
    }
}

int AccessibleItem::accessibleSelectionCount() const
{
    TextCursor* textCursor = this->textCursor();
    if (!textCursor) {
        return 0;
    }

    return static_cast<int>(textCursor->selectedText().size());
}

int AccessibleItem::accessibleCursorPosition() const
{
    TextCursor* textCursor = this->textCursor();
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

    TextCursor* textCursor = new TextCursor(toTextBase(m_element));
    auto startCoord = textCursor->positionToLocalCoord(startOffset);
    if (startCoord.first == muse::nidx || startCoord.second == muse::nidx) {
        return QString();
    }

    textCursor->setRow(startCoord.first);
    textCursor->setColumn(startCoord.second);
    textCursor->movePosition(TextCursor::MoveOperation::Right, TextCursor::MoveMode::KeepAnchor,
                             endOffset - startOffset);

    textCursor->setSelectLine(startCoord.first);
    textCursor->setSelectColumn(startCoord.second);

    QString text = textCursor->selectedText();
    delete textCursor;

    return text;
}

QString AccessibleItem::accessibleTextBeforeOffset(int, TextBoundaryType, int* startOffset, int* endOffset) const
{
    NOT_IMPLEMENTED;

    *startOffset = -1;
    *endOffset = -1;
    return QString();
}

QString AccessibleItem::accessibleTextAfterOffset(int, TextBoundaryType, int* startOffset, int* endOffset) const
{
    NOT_IMPLEMENTED;

    *startOffset = -1;
    *endOffset = -1;
    return QString();
}

QString AccessibleItem::accessibleTextAtOffset(int offset, TextBoundaryType boundaryType, int* startOffset, int* endOffset) const
{
    if (!m_element || !m_element->isTextBase()) {
        return QString();
    }

    QString result;

    TextCursor* textCursor = new TextCursor(toTextBase(m_element));
    auto startCoord = textCursor->positionToLocalCoord(offset);
    if (startCoord.first == muse::nidx || startCoord.second == muse::nidx) {
        return QString();
    }

    textCursor->setRow(startCoord.first);
    textCursor->setColumn(startCoord.second);

    switch (boundaryType) {
    case CharBoundary: {
        *startOffset = textCursor->currentPosition();
        textCursor->movePosition(TextCursor::MoveOperation::Right, TextCursor::MoveMode::KeepAnchor);
        *endOffset = textCursor->currentPosition();
        break;
    }
    case WordBoundary: {
        textCursor->movePosition(TextCursor::MoveOperation::WordLeft, TextCursor::MoveMode::MoveAnchor);
        *startOffset = textCursor->currentPosition();
        textCursor->movePosition(TextCursor::MoveOperation::NextWord, TextCursor::MoveMode::KeepAnchor);
        *endOffset = textCursor->currentPosition();
        break;
    }
    case LineBoundary: {
        textCursor->movePosition(TextCursor::MoveOperation::StartOfLine, TextCursor::MoveMode::MoveAnchor);
        *startOffset = textCursor->currentPosition();
        textCursor->movePosition(TextCursor::MoveOperation::EndOfLine, TextCursor::MoveMode::KeepAnchor);
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

    TextBase* text = toTextBase(m_element);
    return static_cast<int>(text->plainText().size());
}

int AccessibleItem::accessibleRowIndex() const
{
    NOT_IMPLEMENTED;
    return 0;
}

bool AccessibleItem::accessibleState(State st) const
{
    if (!registered()) {
        return false;
    }

    AccessibleRoot* root = accessibleRoot();
    if (!root || !root->enabled()) {
        return false;
    }

    switch (st) {
    case IAccessible::State::Enabled: return true;
    case IAccessible::State::Active: return true;
    case IAccessible::State::Focused: {
        if (AccessibleItemPtr focusedElement = root->focusedElement().lock()) {
            return focusedElement->element() == element();
        }
        return false;
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

    EngravingItem* element = m_element;
    Measure* measure = element->findMeasure();
    if (measure) {
        element = measure;
    }

    RectF bbox = element->canvasBoundingRect();
    RectF canvasRect(element->canvasPos(), SizeF(bbox.width(), bbox.height()));

    auto rect = accessibleRoot()->toScreenRect(canvasRect).toQRect();
    return rect;
}

bool AccessibleItem::accessibleIgnored() const
{
    return false;
}

muse::async::Channel<IAccessible::Property, muse::Val> AccessibleItem::accessiblePropertyChanged() const
{
    return m_accessiblePropertyChanged;
}

muse::async::Channel<IAccessible::State, bool> AccessibleItem::accessibleStateChanged() const
{
    return m_accessibleStateChanged;
}

void AccessibleItem::setState(State state, bool arg)
{
    if (state != State::Focused) {
        return;
    }

    AccessibleRoot* root = accessibleRoot();
    if (arg) {
        root->setFocusedElement(shared_from_this(), false /*voiceStaffInfoChange*/);
    }
}

TextCursor* AccessibleItem::textCursor() const
{
    if (!m_element || !m_element->isTextBase()) {
        return nullptr;
    }

    TextBase* text = toTextBase(m_element);
    return text ? text->cursor() : nullptr;
}
