/*
    Copyright (C) 2003-2008 Fons Adriaensen <fons@kokkinizita.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#ifndef __CALLBACKS_H
#define __CALLBACKS_H

enum
{
    CB_GLOB_SAVE = 0x1000,
    CB_GLOB_MOFF,
    CB_SHOW_INSW,
    CB_SHOW_AUDW,
    CB_SHOW_MIDW,
    CB_MAIN_MSG,
    CB_MAIN_END,
    CB_EDIT_REQ,
    CB_EDIT_APP,
    CB_EDIT_END,
    CB_AUDIO_ACT,
    CB_DIVIS_ACT,
    CB_MIDI_MODCONF,
    CB_MIDI_SETCONF,
    CB_MIDI_GETCONF,
    CB_RETUNE,
    CB_SC_HARM,
    CB_SC_NOTE,
    CB_MS_SEL,
    CB_MS_UPD,
    CB_MS_DEF,
    CB_MS_UND,
    CB_FW_SEL,
    CB_FW_UPD,
    CB_FW_DEF,
    CB_FW_UND
};


#endif
