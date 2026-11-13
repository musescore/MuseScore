/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

// How long to wait for a control to appear (dialogs and lists are loaded asynchronously)
const CONTROL_TIMEOUT_MSEC = 5000

// How long to wait for a popup to open or close, and how often to check it
const POPUP_TIMEOUT_MSEC = 5000
const POPUP_CHECK_MSEC = 50

//! NOTE The navigation section of the last opened popup
var openedPopupSection = ""

function names(arr)
{
    var list = []
    for (var i = 0; i < arr.length; i++) {
        list.push(arr[i].name)
    }
    return list.join(", ")
}

//! NOTE List what is actually there, so a renamed control is easy to fix
function notFoundError(section, panel, controlNameOrIndex)
{
    var msg = "navigation error: not found control: " + section + "/" + panel + "/" + controlNameOrIndex
    var controls = api.navigation.controls(section, panel)
    if (controls.length > 0) {
        msg += "; controls in the panel: " + names(controls)
    } else {
        msg += "; no panel, panels in the section: " + names(api.navigation.panels(section))
    }
    api.testflow.error(msg)
}

//! NOTE Call after an action that opens a popup, returns its navigation section.
//! With the panel it waits until the popup with the panel is opened, otherwise just some time
function popupOpened(panel)
{
    if (!panel) {
        api.testflow.waitPopup()
    } else {
        for (var t = 0; t < POPUP_TIMEOUT_MSEC; t += POPUP_CHECK_MSEC) {
            if (api.navigation.controls(api.navigation.activeSection(), panel).length > 0) {
                break
            }
            api.testflow.sleep(POPUP_CHECK_MSEC)
        }
    }

    openedPopupSection = api.navigation.activeSection()
    return openedPopupSection
}

function closePopupIfOpened()
{
    api.testflow.sleep(100) // a popup can be closed asynchronously
    if (openedPopupSection !== "" && api.navigation.activeSection() === openedPopupSection) {
        api.navigation.escape()
        for (var t = 0; t < POPUP_TIMEOUT_MSEC && api.navigation.activeSection() === openedPopupSection; t += POPUP_CHECK_MSEC) {
            api.testflow.sleep(POPUP_CHECK_MSEC)
        }
    }
    openedPopupSection = ""
}

//! NOTE Without waiting, for checking what is there
function hasControl(section, panel, controlName)
{
    var controls = api.navigation.controls(section, panel)
    for (var c = 0; c < controls.length; ++c) {
        if (controls[c].name === controlName) {
            return true
        }
    }
    return false
}

//! NOTE A popup (menu, dropdown) has its own navigation section,
//! which becomes active when the popup is opened
function goToControlInPopup(panel, controlNameOrIndex)
{
    var section = popupOpened(panel)
    if (!api.navigation.goToControl(section, panel, controlNameOrIndex, CONTROL_TIMEOUT_MSEC)) {
        notFoundError(section, panel, controlNameOrIndex)
    }
}

//! NOTE Clicking inside a popup does not always close it (it may have more controls),
//! a popup left opened breaks the next steps, so close it
function triggerControlInPopup(panel, controlNameOrIndex)
{
    goToControlInPopup(panel, controlNameOrIndex)
    api.navigation.trigger()
    closePopupIfOpened()
}

function doCheckControlIsActive(action)
{
    if (api.navigation.activeControl() === "") {
        api.testflow.error("navigation error: no control after call: " + action)
    }
}

module.exports = {
    nextPanel: function()
    {
        api.navigation.nextPanel()
        doCheckControlIsActive("nextPanel")
    },
    prevPanel: function()
    {
        api.navigation.prevPanel()
        doCheckControlIsActive("prevPanel")
    },
    right: function()
    {
        api.navigation.right()
        doCheckControlIsActive("right")
    },
    left: function()
    {
        api.navigation.left()
        doCheckControlIsActive("left")
    },
    up: function()
    {
        api.navigation.up()
        doCheckControlIsActive("up")
    },
    down: function()
    {
        api.navigation.down()
        doCheckControlIsActive("down")
    },
    escape: function()
    {
        api.navigation.escape()
    },
    goToControl: function(section, panel, controlNameOrIndex)
    {
        if (!api.navigation.goToControl(section, panel, controlNameOrIndex, CONTROL_TIMEOUT_MSEC)) {
            notFoundError(section, panel, controlNameOrIndex)
        }
    },
    trigger: function()
    {
        if (api.navigation.activeControl() === "") {
            api.testflow.error("navigation error: unable trigger, no active control")
            return
        }
        api.navigation.trigger()
    },
    triggerControl: function(section, panel, controlNameOrIndex)
    {
        if (!api.navigation.triggerControl(section, panel, controlNameOrIndex, CONTROL_TIMEOUT_MSEC)) {
            notFoundError(section, panel, controlNameOrIndex)
        }
    },
    goToControlInPopup: goToControlInPopup,
    triggerControlInPopup: triggerControlInPopup,
    popupOpened: popupOpened,
    closePopupIfOpened: closePopupIfOpened,
    hasControl: hasControl,
    activeSection: api.navigation.activeSection,
    activePanel: api.navigation.activePanel,
    activeControl: api.navigation.activeControl,
}
