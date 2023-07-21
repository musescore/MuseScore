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

#include "atts_midi.h"

//----------------------------------------------------------------------------

#include <cassert>

//----------------------------------------------------------------------------

namespace libmei {

//----------------------------------------------------------------------------
// AttChannelized
//----------------------------------------------------------------------------

AttChannelized::AttChannelized() : Att()
{
    ResetChannelized();
}

void AttChannelized::ResetChannelized()
{
    m_midiChannel = -1;
    m_midiDuty = -1.0;
    m_midiPort = data_MIDIVALUE_NAME();
    m_midiTrack = MEI_UNSET;
}

bool AttChannelized::ReadChannelized(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("midi.channel")) {
        this->SetMidiChannel(StrToMidichannel(element.attribute("midi.channel").value()));
        if (removeAttr) element.remove_attribute("midi.channel");
        hasAttribute = true;
    }
    if (element.attribute("midi.duty")) {
        this->SetMidiDuty(StrToPercentLimited(element.attribute("midi.duty").value()));
        if (removeAttr) element.remove_attribute("midi.duty");
        hasAttribute = true;
    }
    if (element.attribute("midi.port")) {
        this->SetMidiPort(StrToMidivalueName(element.attribute("midi.port").value()));
        if (removeAttr) element.remove_attribute("midi.port");
        hasAttribute = true;
    }
    if (element.attribute("midi.track")) {
        this->SetMidiTrack(StrToInt(element.attribute("midi.track").value()));
        if (removeAttr) element.remove_attribute("midi.track");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttChannelized::WriteChannelized(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasMidiChannel()) {
        element.append_attribute("midi.channel") = MidichannelToStr(this->GetMidiChannel()).c_str();
        wroteAttribute = true;
    }
    if (this->HasMidiDuty()) {
        element.append_attribute("midi.duty") = PercentLimitedToStr(this->GetMidiDuty()).c_str();
        wroteAttribute = true;
    }
    if (this->HasMidiPort()) {
        element.append_attribute("midi.port") = MidivalueNameToStr(this->GetMidiPort()).c_str();
        wroteAttribute = true;
    }
    if (this->HasMidiTrack()) {
        element.append_attribute("midi.track") = IntToStr(this->GetMidiTrack()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttChannelized::HasMidiChannel() const
{
    return (m_midiChannel != -1);
}

bool AttChannelized::HasMidiDuty() const
{
    return (m_midiDuty != -1.0);
}

bool AttChannelized::HasMidiPort() const
{
    return (m_midiPort != data_MIDIVALUE_NAME());
}

bool AttChannelized::HasMidiTrack() const
{
    return (m_midiTrack != MEI_UNSET);
}

//----------------------------------------------------------------------------
// AttInstrumentIdent
//----------------------------------------------------------------------------

AttInstrumentIdent::AttInstrumentIdent() : Att()
{
    ResetInstrumentIdent();
}

void AttInstrumentIdent::ResetInstrumentIdent()
{
    m_instr = "";
}

bool AttInstrumentIdent::ReadInstrumentIdent(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("instr")) {
        this->SetInstr(StrToStr(element.attribute("instr").value()));
        if (removeAttr) element.remove_attribute("instr");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttInstrumentIdent::WriteInstrumentIdent(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasInstr()) {
        element.append_attribute("instr") = StrToStr(this->GetInstr()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttInstrumentIdent::HasInstr() const
{
    return (m_instr != "");
}

//----------------------------------------------------------------------------
// AttMidiInstrument
//----------------------------------------------------------------------------

AttMidiInstrument::AttMidiInstrument() : Att()
{
    ResetMidiInstrument();
}

void AttMidiInstrument::ResetMidiInstrument()
{
    m_midiInstrnum = -1;
    m_midiInstrname = MIDINAMES_NONE;
    m_midiPan = data_MIDIVALUE_PAN();
    m_midiPatchname = "";
    m_midiPatchnum = -1;
    m_midiVolume = -1.0;
}

bool AttMidiInstrument::ReadMidiInstrument(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("midi.instrnum")) {
        this->SetMidiInstrnum(StrToMidivalue(element.attribute("midi.instrnum").value()));
        if (removeAttr) element.remove_attribute("midi.instrnum");
        hasAttribute = true;
    }
    if (element.attribute("midi.instrname")) {
        this->SetMidiInstrname(StrToMidinames(element.attribute("midi.instrname").value()));
        if (removeAttr) element.remove_attribute("midi.instrname");
        hasAttribute = true;
    }
    if (element.attribute("midi.pan")) {
        this->SetMidiPan(StrToMidivaluePan(element.attribute("midi.pan").value()));
        if (removeAttr) element.remove_attribute("midi.pan");
        hasAttribute = true;
    }
    if (element.attribute("midi.patchname")) {
        this->SetMidiPatchname(StrToStr(element.attribute("midi.patchname").value()));
        if (removeAttr) element.remove_attribute("midi.patchname");
        hasAttribute = true;
    }
    if (element.attribute("midi.patchnum")) {
        this->SetMidiPatchnum(StrToMidivalue(element.attribute("midi.patchnum").value()));
        if (removeAttr) element.remove_attribute("midi.patchnum");
        hasAttribute = true;
    }
    if (element.attribute("midi.volume")) {
        this->SetMidiVolume(StrToPercent(element.attribute("midi.volume").value()));
        if (removeAttr) element.remove_attribute("midi.volume");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttMidiInstrument::WriteMidiInstrument(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasMidiInstrnum()) {
        element.append_attribute("midi.instrnum") = MidivalueToStr(this->GetMidiInstrnum()).c_str();
        wroteAttribute = true;
    }
    if (this->HasMidiInstrname()) {
        element.append_attribute("midi.instrname") = MidinamesToStr(this->GetMidiInstrname()).c_str();
        wroteAttribute = true;
    }
    if (this->HasMidiPan()) {
        element.append_attribute("midi.pan") = MidivaluePanToStr(this->GetMidiPan()).c_str();
        wroteAttribute = true;
    }
    if (this->HasMidiPatchname()) {
        element.append_attribute("midi.patchname") = StrToStr(this->GetMidiPatchname()).c_str();
        wroteAttribute = true;
    }
    if (this->HasMidiPatchnum()) {
        element.append_attribute("midi.patchnum") = MidivalueToStr(this->GetMidiPatchnum()).c_str();
        wroteAttribute = true;
    }
    if (this->HasMidiVolume()) {
        element.append_attribute("midi.volume") = PercentToStr(this->GetMidiVolume()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttMidiInstrument::HasMidiInstrnum() const
{
    return (m_midiInstrnum != -1);
}

bool AttMidiInstrument::HasMidiInstrname() const
{
    return (m_midiInstrname != MIDINAMES_NONE);
}

bool AttMidiInstrument::HasMidiPan() const
{
    return (m_midiPan != data_MIDIVALUE_PAN());
}

bool AttMidiInstrument::HasMidiPatchname() const
{
    return (m_midiPatchname != "");
}

bool AttMidiInstrument::HasMidiPatchnum() const
{
    return (m_midiPatchnum != -1);
}

bool AttMidiInstrument::HasMidiVolume() const
{
    return (m_midiVolume != -1.0);
}

//----------------------------------------------------------------------------
// AttMidiTempo
//----------------------------------------------------------------------------

AttMidiTempo::AttMidiTempo() : Att()
{
    ResetMidiTempo();
}

void AttMidiTempo::ResetMidiTempo()
{
    m_midiBpm = 0.0;
    m_midiMspb = -1;
}

bool AttMidiTempo::ReadMidiTempo(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("midi.bpm")) {
        this->SetMidiBpm(StrToDbl(element.attribute("midi.bpm").value()));
        if (removeAttr) element.remove_attribute("midi.bpm");
        hasAttribute = true;
    }
    if (element.attribute("midi.mspb")) {
        this->SetMidiMspb(StrToMidimspb(element.attribute("midi.mspb").value()));
        if (removeAttr) element.remove_attribute("midi.mspb");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttMidiTempo::WriteMidiTempo(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasMidiBpm()) {
        element.append_attribute("midi.bpm") = DblToStr(this->GetMidiBpm()).c_str();
        wroteAttribute = true;
    }
    if (this->HasMidiMspb()) {
        element.append_attribute("midi.mspb") = MidimspbToStr(this->GetMidiMspb()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttMidiTempo::HasMidiBpm() const
{
    return (m_midiBpm != 0.0);
}

bool AttMidiTempo::HasMidiMspb() const
{
    return (m_midiMspb != -1);
}

//----------------------------------------------------------------------------
// AttMidiValue
//----------------------------------------------------------------------------

AttMidiValue::AttMidiValue() : Att()
{
    ResetMidiValue();
}

void AttMidiValue::ResetMidiValue()
{
    m_val = -1;
}

bool AttMidiValue::ReadMidiValue(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("val")) {
        this->SetVal(StrToMidivalue(element.attribute("val").value()));
        if (removeAttr) element.remove_attribute("val");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttMidiValue::WriteMidiValue(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasVal()) {
        element.append_attribute("val") = MidivalueToStr(this->GetVal()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttMidiValue::HasVal() const
{
    return (m_val != -1);
}

//----------------------------------------------------------------------------
// AttMidiValue2
//----------------------------------------------------------------------------

AttMidiValue2::AttMidiValue2() : Att()
{
    ResetMidiValue2();
}

void AttMidiValue2::ResetMidiValue2()
{
    m_val2 = -1;
}

bool AttMidiValue2::ReadMidiValue2(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("val2")) {
        this->SetVal2(StrToMidivalue(element.attribute("val2").value()));
        if (removeAttr) element.remove_attribute("val2");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttMidiValue2::WriteMidiValue2(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasVal2()) {
        element.append_attribute("val2") = MidivalueToStr(this->GetVal2()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttMidiValue2::HasVal2() const
{
    return (m_val2 != -1);
}

//----------------------------------------------------------------------------
// AttMidiVelocity
//----------------------------------------------------------------------------

AttMidiVelocity::AttMidiVelocity() : Att()
{
    ResetMidiVelocity();
}

void AttMidiVelocity::ResetMidiVelocity()
{
    m_vel = -1;
}

bool AttMidiVelocity::ReadMidiVelocity(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("vel")) {
        this->SetVel(StrToMidivalue(element.attribute("vel").value()));
        if (removeAttr) element.remove_attribute("vel");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttMidiVelocity::WriteMidiVelocity(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasVel()) {
        element.append_attribute("vel") = MidivalueToStr(this->GetVel()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttMidiVelocity::HasVel() const
{
    return (m_vel != -1);
}

//----------------------------------------------------------------------------
// AttTimeBase
//----------------------------------------------------------------------------

AttTimeBase::AttTimeBase() : Att()
{
    ResetTimeBase();
}

void AttTimeBase::ResetTimeBase()
{
    m_ppq = MEI_UNSET;
}

bool AttTimeBase::ReadTimeBase(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("ppq")) {
        this->SetPpq(StrToInt(element.attribute("ppq").value()));
        if (removeAttr) element.remove_attribute("ppq");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttTimeBase::WriteTimeBase(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasPpq()) {
        element.append_attribute("ppq") = IntToStr(this->GetPpq()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttTimeBase::HasPpq() const
{
    return (m_ppq != MEI_UNSET);
}

} // namespace libmei
