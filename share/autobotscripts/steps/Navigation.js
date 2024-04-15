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

function doCheckControlIsActive(action)
{
    if (api.navigation.activeControl() === "") {
        api.autobot.error("navigation error: no control after call: " + action)
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
        api.navigation.down()
        doCheckControlIsActive("escape")
    },
    goToControl: function(section, panel, controlNameOrIndex)
    {
        if (!api.navigation.goToControl(section, panel, controlNameOrIndex)) {
            api.autobot.error("navigation error: not found control: " + controlNameOrIndex)
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
    triggerControl: function(section, panel, controlNameOrIndex)
    {
        if (!api.navigation.triggerControl(section, panel, controlNameOrIndex)) {
            api.autobot.error("navigation error: not found control: " + controlNameOrIndex)
        }
    },
    activeSection: api.navigation.activeSection,
    activePanel: api.navigation.activePanel,
    activeControl: api.navigation.activeControl,
}
