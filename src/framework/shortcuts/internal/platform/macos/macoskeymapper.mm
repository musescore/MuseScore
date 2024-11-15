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
#include "macoskeymapper.h"

#import <Carbon/Carbon.h>

#include "global/containers.h"

#include "log.h"

using namespace muse::shortcuts;

static const std::map<UInt32, QString> specialKeysMap = {
    { kVK_F1, "F1" },
    { kVK_F2, "F2" },
    { kVK_F3, "F3" },
    { kVK_F4, "F4" },
    { kVK_F5, "F5" },
    { kVK_F6, "F6" },
    { kVK_F7, "F7" },
    { kVK_F8, "F8" },
    { kVK_F9, "F9" },
    { kVK_F10, "F10" },
    { kVK_F11, "F11" },
    { kVK_F12, "F12" },
    { kVK_F13, "F13" },
    { kVK_F14, "F14" },
    { kVK_F15, "F15" },
    { kVK_F16, "F16" },
    { kVK_F17, "F17" },
    { kVK_F18, "F18" },
    { kVK_F19, "F19" },
    { kVK_F20, "F20" },
    { kVK_Space, "Space" },
    { kVK_Escape, "Esc" },
    { kVK_Delete, "Backspace" },
    { kVK_ForwardDelete, "Delete" },
    { kVK_LeftArrow, "Left" },
    { kVK_RightArrow, "Right" },
    { kVK_UpArrow, "Up" },
    { kVK_DownArrow, "Down" },
    { kVK_Help, "" },
    { kVK_PageUp, "PgUp" },
    { kVK_PageDown, "PgDown" },
    { kVK_Tab, "Tab" },
    { kVK_Return, "Return" },
    { kVK_Home, "Home" },
    { kVK_End, "End" },
    { kVK_ANSI_Keypad0, "0" },
    { kVK_ANSI_Keypad1, "1" },
    { kVK_ANSI_Keypad2, "2" },
    { kVK_ANSI_Keypad3, "3" },
    { kVK_ANSI_Keypad4, "4" },
    { kVK_ANSI_Keypad5, "5" },
    { kVK_ANSI_Keypad6, "6" },
    { kVK_ANSI_Keypad7, "7" },
    { kVK_ANSI_Keypad8, "8" },
    { kVK_ANSI_Keypad9, "9" },
    { kVK_ANSI_KeypadDecimal, "." },
    { kVK_ANSI_KeypadMultiply, "*" },
    { kVK_ANSI_KeypadPlus, "+" },
    { kVK_ANSI_KeypadClear, "Clear" },
    { kVK_ANSI_KeypadDivide, "/" },
    { kVK_ANSI_KeypadEnter, "Enter" },
    { kVK_ANSI_KeypadMinus, "-" },
    { kVK_ANSI_KeypadEquals, "=" }
};

static UCKeyboardLayout* keyboardLayout()
{
    static TISInputSourceRef (* TISCopyCurrentKeyboardLayoutInputSource)(void);
    static void*(* TISGetInputSourceProperty)(TISInputSourceRef inputSource, CFStringRef propertyKey);

    CFBundleRef bundle = CFBundleGetBundleWithIdentifier(CFSTR("com.apple.Carbon"));

    if (bundle) {
        *(void**)& TISGetInputSourceProperty = CFBundleGetFunctionPointerForName(bundle, CFSTR("TISGetInputSourceProperty"));
        *(void**)& TISCopyCurrentKeyboardLayoutInputSource
            = CFBundleGetFunctionPointerForName(bundle, CFSTR("TISCopyCurrentKeyboardLayoutInputSource"));
    }

    if (!TISCopyCurrentKeyboardLayoutInputSource || !TISGetInputSourceProperty) {
        LOGE() << "Error getting functions from Carbon framework";
        return 0;
    }

    TISInputSourceRef currentKeyboard = TISCopyCurrentKeyboardLayoutInputSource();
    CFDataRef uchr = (CFDataRef)TISGetInputSourceProperty(currentKeyboard, CFSTR("TISPropertyUnicodeKeyLayoutData"));

    return (UCKeyboardLayout*)CFDataGetBytePtr(uchr);
}

static UCKeyboardLayout* englishKeyboardLayout()
{
    static CFArrayRef (* TISCreateInputSourceList)(CFDictionaryRef, Boolean);
    static void*(* TISGetInputSourceProperty)(TISInputSourceRef inputSource, CFStringRef propertyKey);

    CFBundleRef bundle = CFBundleGetBundleWithIdentifier(CFSTR("com.apple.Carbon"));

    if (bundle) {
        *(void**)& TISGetInputSourceProperty = CFBundleGetFunctionPointerForName(bundle, CFSTR("TISGetInputSourceProperty"));
        *(void**)& TISCreateInputSourceList
            = CFBundleGetFunctionPointerForName(bundle, CFSTR("TISCreateInputSourceList"));
    }

    if (!TISCreateInputSourceList || !TISGetInputSourceProperty) {
        LOGE() << "Error getting functions from Carbon framework";
        return 0;
    }

    CFArrayRef sources = TISCreateInputSourceList(nil, false);
    //! need to check that English always comes first in the source list
    TISInputSourceRef keyboard = (TISInputSourceRef)CFArrayGetValueAtIndex(sources, 0);
    CFDataRef uchr = (CFDataRef)TISGetInputSourceProperty(keyboard, CFSTR("TISPropertyUnicodeKeyLayoutData"));

    return (UCKeyboardLayout*)CFDataGetBytePtr(uchr);
}

static UInt32 nativeKeycode(UCKeyboardLayout* keyboard, Qt::Key keyCode, bool& found)
{
    found = true;

    switch (keyCode) {
    case Qt::Key_Return:
        return kVK_Return;
    case Qt::Key_Enter:
        return kVK_ANSI_KeypadEnter;
    case Qt::Key_Tab:
        return kVK_Tab;
    case Qt::Key_Space:
        return kVK_Space;
    case Qt::Key_Backspace:
        return kVK_Delete;
    case Qt::Key_Escape:
        return kVK_Escape;
    case Qt::Key_CapsLock:
        return kVK_CapsLock;
    case Qt::Key_Option:
        return kVK_Option;
    case Qt::Key_F17:
        return kVK_F17;
    case Qt::Key_VolumeUp:
        return kVK_VolumeUp;
    case Qt::Key_VolumeDown:
        return kVK_VolumeDown;
    case Qt::Key_F18:
        return kVK_F18;
    case Qt::Key_F19:
        return kVK_F19;
    case Qt::Key_F20:
        return kVK_F20;
    case Qt::Key_F5:
        return kVK_F5;
    case Qt::Key_F6:
        return kVK_F6;
    case Qt::Key_F7:
        return kVK_F7;
    case Qt::Key_F3:
        return kVK_F3;
    case Qt::Key_F8:
        return kVK_F8;
    case Qt::Key_F9:
        return kVK_F9;
    case Qt::Key_F11:
        return kVK_F11;
    case Qt::Key_F13:
        return kVK_F13;
    case Qt::Key_F16:
        return kVK_F16;
    case Qt::Key_F14:
        return kVK_F14;
    case Qt::Key_F10:
        return kVK_F10;
    case Qt::Key_F12:
        return kVK_F12;
    case Qt::Key_F15:
        return kVK_F15;
    case Qt::Key_Help:
        return kVK_Help;
    case Qt::Key_Home:
        return kVK_Home;
    case Qt::Key_PageUp:
        return kVK_PageUp;
    case Qt::Key_Delete:
        return kVK_ForwardDelete;
    case Qt::Key_F4:
        return kVK_F4;
    case Qt::Key_End:
        return kVK_End;
    case Qt::Key_F2:
        return kVK_F2;
    case Qt::Key_PageDown:
        return kVK_PageDown;
    case Qt::Key_F1:
        return kVK_F1;
    case Qt::Key_Left:
        return kVK_LeftArrow;
    case Qt::Key_Right:
        return kVK_RightArrow;
    case Qt::Key_Down:
        return kVK_DownArrow;
    case Qt::Key_Up:
        return kVK_UpArrow;
    default:
        break;
    }

    if (keyCode < 0 || keyCode > 0xFFFF) {
        LOGW() << "Unhandled key code: " << keyCode;
        found = false;
        return 0;
    }

    UTF16Char keyCodeChar = keyCode;
    UCKeyboardTypeHeader* table = keyboard->keyboardTypeList;

    uint8_t* data = (uint8_t*)keyboard;
    for (UInt32 i = 0; i < keyboard->keyboardTypeCount; i++) {
        UCKeyStateRecordsIndex* stateRec = 0;
        if (table[i].keyStateRecordsIndexOffset != 0) {
            stateRec = reinterpret_cast<UCKeyStateRecordsIndex*>(data + table[i].keyStateRecordsIndexOffset);
            if (stateRec->keyStateRecordsIndexFormat != kUCKeyStateRecordsIndexFormat) {
                stateRec = 0;
            }
        }

        UCKeyToCharTableIndex* charTable = reinterpret_cast<UCKeyToCharTableIndex*>(data + table[i].keyToCharTableIndexOffset);
        if (charTable->keyToCharTableIndexFormat != kUCKeyToCharTableIndexFormat) {
            continue;
        }

        for (UInt32 j = 0; j < charTable->keyToCharTableCount; j++) {
            UCKeyOutput* keyToChar = reinterpret_cast<UCKeyOutput*>(data + charTable->keyToCharTableOffsets[j]);
            for (UInt32 k = 0; k < charTable->keyToCharTableSize; k++) {
                if (keyToChar[k] & kUCKeyOutputTestForIndexMask) {
                    long idx = keyToChar[k] & kUCKeyOutputGetIndexMask;
                    if (stateRec && idx < stateRec->keyStateRecordCount) {
                        UCKeyStateRecord* rec = reinterpret_cast<UCKeyStateRecord*>(data + stateRec->keyStateRecordOffsets[idx]);
                        if (rec->stateZeroCharData == keyCodeChar) {
                            return k;
                        }
                    }
                } else if (!(keyToChar[k] & kUCKeyOutputSequenceIndexMask) && keyToChar[k] < 0xFFFE) {
                    if (keyToChar[k] == keyCodeChar) {
                        return k;
                    }
                }
            }
        }
    }

    found = false;
    return 0;
}

static UInt32 nativeModifiers(Qt::KeyboardModifiers modifiers)
{
    UInt32 result = 0;
    if (modifiers & Qt::ShiftModifier) {
        result |= shiftKey;
    }
    if (modifiers & Qt::ControlModifier) {
        result |= cmdKey;
    }
    if (modifiers & Qt::AltModifier) {
        result |= optionKey;
    }
    if (modifiers & Qt::MetaModifier) {
        result |= controlKey;
    }
    if (modifiers & Qt::KeypadModifier) {
        result |= kEventKeyModifierNumLockMask;
    }

    return result;
}

static QString keyCodeToString(UCKeyboardLayout* keyboard, UInt32 keyNativeCode)
{
    if (muse::contains(specialKeysMap, keyNativeCode)) {
        return specialKeysMap.at(keyNativeCode);
    }

    static UInt8 (* LMGetKbdType)(void);

    CFBundleRef bundle = CFBundleGetBundleWithIdentifier(CFSTR("com.apple.Carbon"));

    if (bundle) {
        *(void**)& LMGetKbdType = CFBundleGetFunctionPointerForName(bundle, CFSTR("LMGetKbdType"));
    }

    UInt32 deadKeyState = 0;
    UniCharCount actualLength = 0;
    const int maxLength = 4;

    UniChar actualString[maxLength] = { 0 };

    OSStatus error = UCKeyTranslate(keyboard,
                                    UInt16(keyNativeCode),
                                    UInt16(kUCKeyActionDisplay),
                                    UInt32((0 >> 8) & 0xFF),
                                    UInt32(LMGetKbdType()),
                                    OptionBits(kUCKeyTranslateNoDeadKeysBit),
                                    &deadKeyState,
                                    maxLength,
                                    &actualLength,
                                    actualString);
    if (error == 0) {
        NSString* nsString = [NSString stringWithCharacters:actualString length:(NSUInteger)actualLength];
        return QString::fromNSString(nsString).toUpper();
    }

    return "";
}

static QString keyModifiersToString(UInt32 keyNativeModifiers)
{
    static const std::map<int, QString> qtModifiers = {
        { shiftKey, "Shift" },
        { rightShiftKey, "Shift" },
        { controlKey, "Meta" },
        { rightControlKey, "Meta" },
        { cmdKey, "Ctrl" },
        { optionKey, "Alt" },
        { rightOptionKey, "Alt" },
        { kEventKeyModifierNumLockMask, "Num" }
    };

    QString result;
    for (const auto& [key, value] : qtModifiers) {
        if (keyNativeModifiers & key) {
            if (!result.isEmpty()) {
                result += "+";
            }
            result += value;
        }
    }

    return result;
}

QString MacOSKeyMapper::translateToCurrentKeyboardLayout(const QKeySequence& sequence)
{
    const QKeyCombination keyCombination = sequence[0];

    const Qt::Key qKey = keyCombination.key();

    UCKeyboardLayout* keyboard = keyboardLayout();
    if (!keyboard) {
        LOGE() << "The keyboard layout is not valid";
        return {};
    }

    bool found = false;
    UInt32 keyNativeCode = nativeKeycode(keyboard, qKey, found);
    if (!found) {
        LOGW() << "Key " << qKey << " not found in the keyboard layout";
        return {};
    }

    Qt::KeyboardModifiers modifiers = keyCombination.keyboardModifiers();
    UInt32 keyNativeModifiers = nativeModifiers(modifiers);

    QString keyStr = keyCodeToString(keyboard, keyNativeCode);
    QString modifStr = keyModifiersToString(keyNativeModifiers);

    return (modifStr.isEmpty() ? "" : modifStr + "+") + keyStr;
}

QString MacOSKeyMapper::translateToEnglishKeyboardLayout(const QKeySequence& sequence)
{
    const QKeyCombination keyCombination = sequence[0];

    const Qt::Key qKey = keyCombination.key();

    UCKeyboardLayout* englishKeyboard = englishKeyboardLayout();
    UCKeyboardLayout* currentKeyboard = keyboardLayout();
    if (!englishKeyboard || !currentKeyboard) {
        LOGE() << "The keyboard layout is not valid";
        return {};
    }

    //! find native key code and modifiers in current keyboard

    bool found = false;
    UInt32 keyNativeCode = nativeKeycode(currentKeyboard, qKey, found);
    if (!found) {
        LOGW() << "Key " << qKey << " not found in the keyboard layout";
        return {};
    }

    Qt::KeyboardModifiers modifiers = keyCombination.keyboardModifiers();
    UInt32 keyNativeModifiers = nativeModifiers(modifiers);

    //! map to english keyboard
    QString keyStr = keyCodeToString(englishKeyboard, keyNativeCode);
    QString modifStr = keyModifiersToString(keyNativeModifiers);

    return (modifStr.isEmpty() ? "" : modifStr + "+") + keyStr;
}
