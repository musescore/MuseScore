::
:: // SPDX-License-Identifier: GPL-3.0-only
:: // MuseScore-CLA-applies
:: //=============================================================================
:: //  MuseScore
:: //  Music Composition & Notation
:: //
:: //  Copyright (C) 2021 MuseScore BVBA and others
:: //
:: //  This program is free software: you can redistribute it and/or modify
:: //  it under the terms of the GNU General Public License version 3 as
:: //  published by the Free Software Foundation.
:: //
:: //  This program is distributed in the hope that it will be useful,
:: //  but WITHOUT ANY WARRANTY; without even the implied warranty of
:: //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
:: //  GNU General Public License for more details.
:: //
:: //  You should have received a copy of the GNU General Public License
:: //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
:: //=============================================================================::
set P=..\..\build.debug\mtest\guitarpro

xcopy /y %P%\testIrrTuplet.gp4.mscx testIrrTuplet.gp4-ref.mscx
xcopy /y %P%\slur.gp4.mscx slur.gp4-ref.mscx
xcopy /y %P%\sforzato.gp4.mscx sforzato.gp4-ref.mscx
xcopy /y %P%\heavy-accent.gp5.mscx heavy-accent.gp5-ref.mscx
xcopy /y %P%\tremolos.gp5.mscx tremolos.gp5-ref.mscx
xcopy /y %P%\trill.gp4.mscx trill.gp4-ref.mscx
xcopy /y %P%\dynamic.gp5.mscx dynamic.gp5-ref.mscx
xcopy /y %P%\arpeggio_up_down.gp4.mscx arpeggio_up_down.gp4-ref.mscx
xcopy /y %P%\ghost_note.gp3.mscx ghost_note.gp3-ref.mscx
xcopy /y %P%\grace.gp5.mscx grace.gp5-ref.mscx
xcopy /y %P%\volta.gp5.mscx volta.gp5-ref.mscx
xcopy /y %P%\volta.gp4.mscx volta.gp4-ref.mscx
xcopy /y %P%\volta.gp3.mscx volta.gp3-ref.mscx
xcopy /y %P%\copyright.gp5.mscx copyright.gp5-ref.mscx
xcopy /y %P%\copyright.gp4.mscx copyright.gp4-ref.mscx
xcopy /y %P%\copyright.gp3.mscx copyright.gp3-ref.mscx
xcopy /y %P%\tempo.gp5.mscx tempo.gp5-ref.mscx
xcopy /y %P%\tempo.gp4.mscx tempo.gp4-ref.mscx
xcopy /y %P%\tempo.gp3.mscx tempo.gp3-ref.mscx
xcopy /y %P%\basic-bend.gp5.mscx basic-bend.gp5-ref.mscx
xcopy /y %P%\bend.gp5.mscx bend.gp5-ref.mscx
xcopy /y %P%\bend.gp4.mscx bend.gp4-ref.mscx
xcopy /y %P%\bend.gp3.mscx bend.gp3-ref.mscx
