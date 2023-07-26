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

#include "atts_cmnornaments.h"

//----------------------------------------------------------------------------

#include <cassert>

//----------------------------------------------------------------------------

namespace libmei {

//----------------------------------------------------------------------------
// AttMordentLog
//----------------------------------------------------------------------------

AttMordentLog::AttMordentLog() : Att()
{
    ResetMordentLog();
}

void AttMordentLog::ResetMordentLog()
{
    m_form = mordentLog_FORM_NONE;
    m_long = BOOLEAN_NONE;
}

bool AttMordentLog::ReadMordentLog(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("form")) {
        this->SetForm(StrToMordentLogForm(element.attribute("form").value()));
        if (removeAttr) element.remove_attribute("form");
        hasAttribute = true;
    }
    if (element.attribute("long")) {
        this->SetLong(StrToBoolean(element.attribute("long").value()));
        if (removeAttr) element.remove_attribute("long");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttMordentLog::WriteMordentLog(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasForm()) {
        element.append_attribute("form") = MordentLogFormToStr(this->GetForm()).c_str();
        wroteAttribute = true;
    }
    if (this->HasLong()) {
        element.append_attribute("long") = BooleanToStr(this->GetLong()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttMordentLog::HasForm() const
{
    return (m_form != mordentLog_FORM_NONE);
}

bool AttMordentLog::HasLong() const
{
    return (m_long != BOOLEAN_NONE);
}

//----------------------------------------------------------------------------
// AttOrnamentAccid
//----------------------------------------------------------------------------

AttOrnamentAccid::AttOrnamentAccid() : Att()
{
    ResetOrnamentAccid();
}

void AttOrnamentAccid::ResetOrnamentAccid()
{
    m_accidupper = ACCIDENTAL_WRITTEN_NONE;
    m_accidlower = ACCIDENTAL_WRITTEN_NONE;
}

bool AttOrnamentAccid::ReadOrnamentAccid(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("accidupper")) {
        this->SetAccidupper(StrToAccidentalWritten(element.attribute("accidupper").value()));
        if (removeAttr) element.remove_attribute("accidupper");
        hasAttribute = true;
    }
    if (element.attribute("accidlower")) {
        this->SetAccidlower(StrToAccidentalWritten(element.attribute("accidlower").value()));
        if (removeAttr) element.remove_attribute("accidlower");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttOrnamentAccid::WriteOrnamentAccid(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasAccidupper()) {
        element.append_attribute("accidupper") = AccidentalWrittenToStr(this->GetAccidupper()).c_str();
        wroteAttribute = true;
    }
    if (this->HasAccidlower()) {
        element.append_attribute("accidlower") = AccidentalWrittenToStr(this->GetAccidlower()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttOrnamentAccid::HasAccidupper() const
{
    return (m_accidupper != ACCIDENTAL_WRITTEN_NONE);
}

bool AttOrnamentAccid::HasAccidlower() const
{
    return (m_accidlower != ACCIDENTAL_WRITTEN_NONE);
}

//----------------------------------------------------------------------------
// AttTurnLog
//----------------------------------------------------------------------------

AttTurnLog::AttTurnLog() : Att()
{
    ResetTurnLog();
}

void AttTurnLog::ResetTurnLog()
{
    m_delayed = BOOLEAN_NONE;
    m_form = turnLog_FORM_NONE;
}

bool AttTurnLog::ReadTurnLog(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("delayed")) {
        this->SetDelayed(StrToBoolean(element.attribute("delayed").value()));
        if (removeAttr) element.remove_attribute("delayed");
        hasAttribute = true;
    }
    if (element.attribute("form")) {
        this->SetForm(StrToTurnLogForm(element.attribute("form").value()));
        if (removeAttr) element.remove_attribute("form");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttTurnLog::WriteTurnLog(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasDelayed()) {
        element.append_attribute("delayed") = BooleanToStr(this->GetDelayed()).c_str();
        wroteAttribute = true;
    }
    if (this->HasForm()) {
        element.append_attribute("form") = TurnLogFormToStr(this->GetForm()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttTurnLog::HasDelayed() const
{
    return (m_delayed != BOOLEAN_NONE);
}

bool AttTurnLog::HasForm() const
{
    return (m_form != turnLog_FORM_NONE);
}

} // namespace libmei
