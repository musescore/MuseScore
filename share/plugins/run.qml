//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Color notehead plugin
//	Noteheads are colored according to pitch. User can change to color by
//  modifying the colors array. First element is C, second C# etc...
//
//  Copyright (C)2012 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

import QtQuick 1.0
import MuseScore 1.0

MuseScore {
      menuPath: "Plugins.run"

      onRun: {
            console.log("run ls");
            var proc = newQProcess();
            proc.start("/bin/ls");
            var val = proc.waitForFinished(30000);
            if (val)
                  console.log(proc.readAllStandardOutput());
            Qt.quit()
            }
      }

