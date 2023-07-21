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

#include "midi.h"

//----------------------------------------------------------------------------

#include <cassert>

//----------------------------------------------------------------------------

namespace libmei {

InstrDef::InstrDef() :
    Element("instrDef"), AttLabelled(), AttNInteger(), AttTyped(), AttChannelized(), AttMidiInstrument(), AttSoundLocation()
{
}

InstrDef::~InstrDef() {}

bool InstrDef::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadNInteger(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadChannelized(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadMidiInstrument(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadSoundLocation(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool InstrDef::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("instrDef");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteNInteger(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteChannelized(element) || hasAttribute);
    hasAttribute = (WriteMidiInstrument(element) || hasAttribute);
    hasAttribute = (WriteSoundLocation(element) || hasAttribute);
    return hasAttribute;
}

void InstrDef::Reset() 
{     
    ResetLabelled();
    ResetNInteger();
    ResetTyped();
    ResetChannelized();
    ResetMidiInstrument();
    ResetSoundLocation();
}



} // namespace libmei
