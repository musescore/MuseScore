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
#include "logapi.h"

#include "log.h"

using namespace muse::api;

/** APIDOC namespace: log
 * Write messages to log and console
 * @namespace
 */
LogApi::LogApi(api::IApiEngine* e)
    : ApiObject(e)
{
}

/** APIDOC method
 * Write error message with default tag
 * @param {String} message Message
 */
void LogApi::error(const QString& message)
{
    error("Api", message);
}

/** APIDOC method
 * Write warning message with default tag
 * @param {String} message Message
 */
void LogApi::warn(const QString& message)
{
    warn("Api", message);
}

/** APIDOC method
 * Write info message with default tag
 * @param {String} message Message
 */
void LogApi::info(const QString& message)
{
    info("Api", message);
}

/** APIDOC method
 * Write debug message with default tag
 * @param {String} message Message
 */
void LogApi::debug(const QString& message)
{
    debug("Api", message);
}

/** APIDOC method
 * Write error message with tag
 * @param {String} tag Tag
 * @param {String} message Message
 */
void LogApi::error(const QString& tag, const QString& message)
{
    LOGE_T(tag.toStdString())() << message;
}

/** APIDOC method
 * Write warning message with tag
 * @param {String} tag Tag
 * @param {String} message Message
 */
void LogApi::warn(const QString& tag, const QString& message)
{
    LOGW_T(tag.toStdString())() << message;
}

/** APIDOC method
 * Write info message with tag
 * @param {String} tag Tag
 * @param {String} message Message
 */
void LogApi::info(const QString& tag, const QString& message)
{
    LOGI_T(tag.toStdString())() << message;
}

/** APIDOC method
 * Write debug message with tag
 * @param {String} tag Tag
 * @param {String} message Message
 */
void LogApi::debug(const QString& tag, const QString& message)
{
    LOGD_T(tag.toStdString())() << message;
}
