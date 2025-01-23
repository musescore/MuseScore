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

#include "atts_gestural.h"

//----------------------------------------------------------------------------

#include <cassert>

//----------------------------------------------------------------------------

namespace libmei {

//----------------------------------------------------------------------------
// AttAccidentalGes
//----------------------------------------------------------------------------

AttAccidentalGes::AttAccidentalGes() : Att()
{
    ResetAccidentalGes();
}

void AttAccidentalGes::ResetAccidentalGes()
{
    m_accidGes = ACCIDENTAL_GESTURAL_NONE;
}

bool AttAccidentalGes::ReadAccidentalGes(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("accid.ges")) {
        this->SetAccidGes(StrToAccidentalGestural(element.attribute("accid.ges").value()));
        if (removeAttr) element.remove_attribute("accid.ges");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttAccidentalGes::WriteAccidentalGes(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasAccidGes()) {
        element.append_attribute("accid.ges") = AccidentalGesturalToStr(this->GetAccidGes()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttAccidentalGes::HasAccidGes() const
{
    return (m_accidGes != ACCIDENTAL_GESTURAL_NONE);
}

//----------------------------------------------------------------------------
// AttAttacking
//----------------------------------------------------------------------------

AttAttacking::AttAttacking() : Att()
{
    ResetAttacking();
}

void AttAttacking::ResetAttacking()
{
    m_attacca = BOOLEAN_NONE;
}

bool AttAttacking::ReadAttacking(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("attacca")) {
        this->SetAttacca(StrToBoolean(element.attribute("attacca").value()));
        if (removeAttr) element.remove_attribute("attacca");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttAttacking::WriteAttacking(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasAttacca()) {
        element.append_attribute("attacca") = BooleanToStr(this->GetAttacca()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttAttacking::HasAttacca() const
{
    return (m_attacca != BOOLEAN_NONE);
}

//----------------------------------------------------------------------------
// AttNoteGes
//----------------------------------------------------------------------------

AttNoteGes::AttNoteGes() : Att()
{
    ResetNoteGes();
}

void AttNoteGes::ResetNoteGes()
{
    m_octGes = -127;
    m_pnameGes = PITCHNAME_NONE;
}

bool AttNoteGes::ReadNoteGes(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("oct.ges")) {
        this->SetOctGes(StrToOctave(element.attribute("oct.ges").value()));
        if (removeAttr) element.remove_attribute("oct.ges");
        hasAttribute = true;
    }
    if (element.attribute("pname.ges")) {
        this->SetPnameGes(StrToPitchname(element.attribute("pname.ges").value()));
        if (removeAttr) element.remove_attribute("pname.ges");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttNoteGes::WriteNoteGes(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasOctGes()) {
        element.append_attribute("oct.ges") = OctaveToStr(this->GetOctGes()).c_str();
        wroteAttribute = true;
    }
    if (this->HasPnameGes()) {
        element.append_attribute("pname.ges") = PitchnameToStr(this->GetPnameGes()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttNoteGes::HasOctGes() const
{
    return (m_octGes != -127);
}

bool AttNoteGes::HasPnameGes() const
{
    return (m_pnameGes != PITCHNAME_NONE);
}

//----------------------------------------------------------------------------
// AttOrnamentAccidGes
//----------------------------------------------------------------------------

AttOrnamentAccidGes::AttOrnamentAccidGes() : Att()
{
    ResetOrnamentAccidGes();
}

void AttOrnamentAccidGes::ResetOrnamentAccidGes()
{
    m_accidupperGes = ACCIDENTAL_GESTURAL_NONE;
    m_accidlowerGes = ACCIDENTAL_GESTURAL_NONE;
}

bool AttOrnamentAccidGes::ReadOrnamentAccidGes(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("accidupper.ges")) {
        this->SetAccidupperGes(StrToAccidentalGestural(element.attribute("accidupper.ges").value()));
        if (removeAttr) element.remove_attribute("accidupper.ges");
        hasAttribute = true;
    }
    if (element.attribute("accidlower.ges")) {
        this->SetAccidlowerGes(StrToAccidentalGestural(element.attribute("accidlower.ges").value()));
        if (removeAttr) element.remove_attribute("accidlower.ges");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttOrnamentAccidGes::WriteOrnamentAccidGes(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasAccidupperGes()) {
        element.append_attribute("accidupper.ges") = AccidentalGesturalToStr(this->GetAccidupperGes()).c_str();
        wroteAttribute = true;
    }
    if (this->HasAccidlowerGes()) {
        element.append_attribute("accidlower.ges") = AccidentalGesturalToStr(this->GetAccidlowerGes()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttOrnamentAccidGes::HasAccidupperGes() const
{
    return (m_accidupperGes != ACCIDENTAL_GESTURAL_NONE);
}

bool AttOrnamentAccidGes::HasAccidlowerGes() const
{
    return (m_accidlowerGes != ACCIDENTAL_GESTURAL_NONE);
}

//----------------------------------------------------------------------------
// AttSoundLocation
//----------------------------------------------------------------------------

AttSoundLocation::AttSoundLocation() : Att()
{
    ResetSoundLocation();
}

void AttSoundLocation::ResetSoundLocation()
{
    m_azimuth = MEI_UNSET;
    m_elevation = MEI_UNSET;
}

bool AttSoundLocation::ReadSoundLocation(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("azimuth")) {
        this->SetAzimuth(StrToDegrees(element.attribute("azimuth").value()));
        if (removeAttr) element.remove_attribute("azimuth");
        hasAttribute = true;
    }
    if (element.attribute("elevation")) {
        this->SetElevation(StrToDegrees(element.attribute("elevation").value()));
        if (removeAttr) element.remove_attribute("elevation");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttSoundLocation::WriteSoundLocation(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasAzimuth()) {
        element.append_attribute("azimuth") = DegreesToStr(this->GetAzimuth()).c_str();
        wroteAttribute = true;
    }
    if (this->HasElevation()) {
        element.append_attribute("elevation") = DegreesToStr(this->GetElevation()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttSoundLocation::HasAzimuth() const
{
    return (m_azimuth != MEI_UNSET);
}

bool AttSoundLocation::HasElevation() const
{
    return (m_elevation != MEI_UNSET);
}

} // namespace libmei
