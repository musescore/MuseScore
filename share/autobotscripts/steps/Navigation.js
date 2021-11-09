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

function doCkeckControlIsActive(action)
{
    if (api.navigation.activeControl() === "") {
        api.autobot.error("navigation error: no control after call: " + action)
    }
}

module.exports = {
    nextPanel: function()
    {
        api.navigation.nextPanel()
        doCkeckControlIsActive("nextPanel")
    },
    prevPanel: function()
    {
        api.navigation.prevPanel()
        doCkeckControlIsActive("prevPanel")
    },
    right: function()
    {
        api.navigation.right()
        doCkeckControlIsActive("right")
    },
    left: function()
    {
        api.navigation.left()
        doCkeckControlIsActive("left")
    },
    up: function()
    {
        api.navigation.up()
        doCkeckControlIsActive("up")
    },
    down: function()
    {
        api.navigation.down()
        doCkeckControlIsActive("down")
    },
    escape: function()
    {
        api.navigation.down()
        doCkeckControlIsActive("escape")
    },
    goToControl: function(section, panel, contolNameOrIndex)
    {
        if (!api.navigation.goToControl(section, panel, contolNameOrIndex)) {
            api.autobot.error("navigation error: not found control: " + contolNameOrIndex)
        }
    },
    trigger: function()
    {
        if (api.navigation.activeControl() === "") {
            api.autobot.error("navigation error: unable trigger, no active control")
            return
        }
        api.navigation.trigger()
    },
    triggerControl: function(section, panel, contolNameOrIndex)
    {
        if (!api.navigation.triggerControl(section, panel, contolNameOrIndex)) {
            api.autobot.error("navigation error: not found control: " + contolNameOrIndex)
        }
    },
    activeSection: api.navigation.activeSection,
    activePanel: api.navigation.activePanel,
    activeControl: api.navigation.activeControl,
}
