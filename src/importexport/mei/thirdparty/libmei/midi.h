/////////////////////////////////////////////////////////////////////////////
// Authors:     Laurent Pugin and Rodolfo Zitellini
// Created:     2014
// Copyright (c) Authors and others. All rights reserved.
//
// Code generated using a modified version of libmei
// by Andrew Hankinson, Alastair Porter, and Others
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// NOTE: this file was generated with the Verovio libmei version and
// should not be edited because changes will be lost.
/////////////////////////////////////////////////////////////////////////////

#ifndef __LIBMEI_MIDI_H__
#define __LIBMEI_MIDI_H__

#include "att.h"
#include "attdef.h"
#include "pugixml.hpp"

//----------------------------------------------------------------------------

#include "element.h"
#include "atts_gestural.h"
#include "atts_midi.h"
#include "atts_shared.h"


namespace libmei {

/** MIDI instrument declaration. **/
class InstrDef : public Element, public AttLabelled, public AttNInteger, public AttTyped, public AttChannelized, public AttMidiInstrument, public AttSoundLocation {
    public:
        InstrDef();
        virtual ~InstrDef();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

} // namespace libmei

#endif // __LIBMEI_MIDI_H__
