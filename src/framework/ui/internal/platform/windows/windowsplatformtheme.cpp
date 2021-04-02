//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "windowsplatformtheme.h"
#include "log.h"

#include <Windows.h>

using namespace mu::ui;
using namespace mu::async;

static const std::wstring windowsThemesKey = L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes";
static const std::wstring windowsThemesPersonalizeKey = L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize";
static const std::wstring windowsThemesPersonalizeSubkey = L"AppsUseLightTheme";

HKEY hKey = nullptr;

HANDLE hThemeChangeEvent = nullptr;
HANDLE hStopListeningEvent = nullptr;

WindowsPlatformTheme::WindowsPlatformTheme()
{
    using fnRtlGetNtVersionNumbers = void(WINAPI*)(LPDWORD major, LPDWORD minor, LPDWORD build);
    auto rtlGetNtVersionNumbers
        = reinterpret_cast<fnRtlGetNtVersionNumbers>(GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlGetNtVersionNumbers"));
    if (rtlGetNtVersionNumbers) {
        DWORD major, minor, buildNumber;
        rtlGetNtVersionNumbers(&major, &minor, &buildNumber);
        m_buildNumber = buildNumber & ~0xF0000000;
    }
}

void WindowsPlatformTheme::startListening()
{
    if (m_isListening) {
        return;
    }
    m_isListening = true;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, windowsThemesKey.c_str(), 0,
                      KEY_NOTIFY | KEY_CREATE_SUB_KEY | KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE | KEY_WOW64_64KEY,
                      &hKey) == ERROR_SUCCESS) {
        m_listenThread = std::thread([this]() { th_listen(); });
    } else {
        LOGW() << "Failed opening key for listening dark theme changes.";
    }
}

void WindowsPlatformTheme::stopListening()
{
    if (!m_isListening) {
        return;
    }
    m_isListening = false;
    // The following _might_ fail; in that case, the app won't respond
    // for max. 4000 ms (or whatever value is specified in th_listen()).
    SetEvent(hStopListeningEvent);
    m_listenThread.join();
}

bool WindowsPlatformTheme::isFollowSystemThemeAvailable() const
{
    return m_buildNumber >= 17763; // Dark theme was introduced in Windows 1809
}

ThemeCode WindowsPlatformTheme::themeCode() const
{
    if (isSystemHighContrast()) {
        return HIGH_CONTRAST_THEME_CODE;
    }

    if (isSystemDarkMode()) {
        return DARK_THEME_CODE;
    }

    //! NOTE When system is in light mode, don't automatically use
    //! high contrast theme, because it is too dark.
    //! Light high contrast theme would be nice.
    return LIGHT_THEME_CODE;
}

Channel<ThemeCode> WindowsPlatformTheme::themeCodeChanged() const
{
    return m_channel;
}

bool WindowsPlatformTheme::isSystemDarkMode() const
{
    DWORD data {};
    DWORD datasize = sizeof(data);
    if (RegGetValue(HKEY_CURRENT_USER, windowsThemesPersonalizeKey.c_str(), windowsThemesPersonalizeSubkey.c_str(),
                    RRF_RT_REG_DWORD, nullptr, &data, &datasize) == ERROR_SUCCESS) {
        return data == 0;
    }
    return false;
}

bool WindowsPlatformTheme::isSystemHighContrast() const
{
    HIGHCONTRAST info;
    info.cbSize = sizeof(HIGHCONTRAST);
    if (SystemParametersInfoW(SPI_GETHIGHCONTRAST, 0, &info, 0)) {
        return info.dwFlags & HCF_HIGHCONTRASTON;
    }
    return false;
}

void WindowsPlatformTheme::th_listen()
{
    static const DWORD dwFilter = REG_NOTIFY_CHANGE_NAME
                                  | REG_NOTIFY_CHANGE_ATTRIBUTES
                                  | REG_NOTIFY_CHANGE_LAST_SET
                                  | REG_NOTIFY_CHANGE_SECURITY;

    while (m_isListening) {
        hStopListeningEvent = CreateEvent(NULL, FALSE, TRUE, TEXT("StopListening"));
        hThemeChangeEvent = CreateEvent(NULL, FALSE, TRUE, TEXT("ThemeSettingChange"));
        if (RegNotifyChangeKeyValue(hKey, TRUE, dwFilter, hThemeChangeEvent, TRUE) == ERROR_SUCCESS) {
            ThemeCode oldBestSuited = themeCode();
            HANDLE handles[2] = { hStopListeningEvent, hThemeChangeEvent };
            DWORD dwRet = WaitForMultipleObjects(2, handles, FALSE, 4000);
            if (dwRet != WAIT_TIMEOUT && dwRet != WAIT_FAILED) {
                if (m_isListening) {
                    //! NOTE There might be some delay before `isSystemHighContrast` returns the correct value
                    Sleep(100);
                    ThemeCode newBestSuited = themeCode();
                    if (newBestSuited != oldBestSuited) {
                        m_channel.send(newBestSuited);
                    }
                }
                // Else, the received event must have been a stop event
            }
        } else {
            LOGD() << "Failed registering for dark theme change notifications.";
        }
    }
    RegCloseKey(hKey);
}

void WindowsPlatformTheme::applyPlatformStyleOnAppForTheme(ThemeCode)
{
}

void WindowsPlatformTheme::applyPlatformStyleOnWindowForTheme(QWidget*, ThemeCode)
{
}
