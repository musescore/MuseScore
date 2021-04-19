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

import QtQuick 2.0
import MuseScore 3.0

MuseScore {
      menuPath: "Plugins.run"
      version:  "3.0"
      description: "This demo plugin runs an external command."
      requiresScore: false

      QProcess {
        id: proc
        }

      onRun: {
            console.log("run ls");
            //proc.start("/bin/ls"); // Linux, Mac(?)
            proc.start("cmd.exe /c dir"); // Windows
            var val = proc.waitForFinished(30000);
            if (val)
                  console.log(proc.readAllStandardOutput());
            Qt.quit()
            }
      }

