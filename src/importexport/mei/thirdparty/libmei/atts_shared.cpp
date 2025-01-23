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

#include "atts_shared.h"

//----------------------------------------------------------------------------

#include <cassert>

//----------------------------------------------------------------------------

namespace libmei {

//----------------------------------------------------------------------------
// AttAccidLog
//----------------------------------------------------------------------------

AttAccidLog::AttAccidLog() : Att()
{
    ResetAccidLog();
}

void AttAccidLog::ResetAccidLog()
{
    m_func = accidLog_FUNC_NONE;
}

bool AttAccidLog::ReadAccidLog(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("func")) {
        this->SetFunc(StrToAccidLogFunc(element.attribute("func").value()));
        if (removeAttr) element.remove_attribute("func");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttAccidLog::WriteAccidLog(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasFunc()) {
        element.append_attribute("func") = AccidLogFuncToStr(this->GetFunc()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttAccidLog::HasFunc() const
{
    return (m_func != accidLog_FUNC_NONE);
}

//----------------------------------------------------------------------------
// AttAccidental
//----------------------------------------------------------------------------

AttAccidental::AttAccidental() : Att()
{
    ResetAccidental();
}

void AttAccidental::ResetAccidental()
{
    m_accid = ACCIDENTAL_WRITTEN_NONE;
}

bool AttAccidental::ReadAccidental(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("accid")) {
        this->SetAccid(StrToAccidentalWritten(element.attribute("accid").value()));
        if (removeAttr) element.remove_attribute("accid");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttAccidental::WriteAccidental(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasAccid()) {
        element.append_attribute("accid") = AccidentalWrittenToStr(this->GetAccid()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttAccidental::HasAccid() const
{
    return (m_accid != ACCIDENTAL_WRITTEN_NONE);
}

//----------------------------------------------------------------------------
// AttArticulation
//----------------------------------------------------------------------------

AttArticulation::AttArticulation() : Att()
{
    ResetArticulation();
}

void AttArticulation::ResetArticulation()
{
    m_artic = std::vector<data_ARTICULATION>();
}

bool AttArticulation::ReadArticulation(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("artic")) {
        this->SetArtic(StrToArticulationList(element.attribute("artic").value()));
        if (removeAttr) element.remove_attribute("artic");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttArticulation::WriteArticulation(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasArtic()) {
        element.append_attribute("artic") = ArticulationListToStr(this->GetArtic()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttArticulation::HasArtic() const
{
    return (m_artic != std::vector<data_ARTICULATION>());
}

//----------------------------------------------------------------------------
// AttAugmentDots
//----------------------------------------------------------------------------

AttAugmentDots::AttAugmentDots() : Att()
{
    ResetAugmentDots();
}

void AttAugmentDots::ResetAugmentDots()
{
    m_dots = MEI_UNSET;
}

bool AttAugmentDots::ReadAugmentDots(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("dots")) {
        this->SetDots(StrToInt(element.attribute("dots").value()));
        if (removeAttr) element.remove_attribute("dots");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttAugmentDots::WriteAugmentDots(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasDots()) {
        element.append_attribute("dots") = IntToStr(this->GetDots()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttAugmentDots::HasDots() const
{
    return (m_dots != MEI_UNSET);
}

//----------------------------------------------------------------------------
// AttCalendared
//----------------------------------------------------------------------------

AttCalendared::AttCalendared() : Att()
{
    ResetCalendared();
}

void AttCalendared::ResetCalendared()
{
    m_calendar = "";
}

bool AttCalendared::ReadCalendared(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("calendar")) {
        this->SetCalendar(StrToStr(element.attribute("calendar").value()));
        if (removeAttr) element.remove_attribute("calendar");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttCalendared::WriteCalendared(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasCalendar()) {
        element.append_attribute("calendar") = StrToStr(this->GetCalendar()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttCalendared::HasCalendar() const
{
    return (m_calendar != "");
}

//----------------------------------------------------------------------------
// AttClefLog
//----------------------------------------------------------------------------

AttClefLog::AttClefLog() : Att()
{
    ResetClefLog();
}

void AttClefLog::ResetClefLog()
{
    m_cautionary = BOOLEAN_NONE;
}

bool AttClefLog::ReadClefLog(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("cautionary")) {
        this->SetCautionary(StrToBoolean(element.attribute("cautionary").value()));
        if (removeAttr) element.remove_attribute("cautionary");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttClefLog::WriteClefLog(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasCautionary()) {
        element.append_attribute("cautionary") = BooleanToStr(this->GetCautionary()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttClefLog::HasCautionary() const
{
    return (m_cautionary != BOOLEAN_NONE);
}

//----------------------------------------------------------------------------
// AttClefShape
//----------------------------------------------------------------------------

AttClefShape::AttClefShape() : Att()
{
    ResetClefShape();
}

void AttClefShape::ResetClefShape()
{
    m_shape = CLEFSHAPE_NONE;
}

bool AttClefShape::ReadClefShape(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("shape")) {
        this->SetShape(StrToClefshape(element.attribute("shape").value()));
        if (removeAttr) element.remove_attribute("shape");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttClefShape::WriteClefShape(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasShape()) {
        element.append_attribute("shape") = ClefshapeToStr(this->GetShape()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttClefShape::HasShape() const
{
    return (m_shape != CLEFSHAPE_NONE);
}

//----------------------------------------------------------------------------
// AttCleffingLog
//----------------------------------------------------------------------------

AttCleffingLog::AttCleffingLog() : Att()
{
    ResetCleffingLog();
}

void AttCleffingLog::ResetCleffingLog()
{
    m_clefShape = CLEFSHAPE_NONE;
    m_clefLine = 0;
    m_clefDis = OCTAVE_DIS_NONE;
    m_clefDisPlace = STAFFREL_basic_NONE;
}

bool AttCleffingLog::ReadCleffingLog(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("clef.shape")) {
        this->SetClefShape(StrToClefshape(element.attribute("clef.shape").value()));
        if (removeAttr) element.remove_attribute("clef.shape");
        hasAttribute = true;
    }
    if (element.attribute("clef.line")) {
        this->SetClefLine(StrToInt(element.attribute("clef.line").value()));
        if (removeAttr) element.remove_attribute("clef.line");
        hasAttribute = true;
    }
    if (element.attribute("clef.dis")) {
        this->SetClefDis(StrToOctaveDis(element.attribute("clef.dis").value()));
        if (removeAttr) element.remove_attribute("clef.dis");
        hasAttribute = true;
    }
    if (element.attribute("clef.dis.place")) {
        this->SetClefDisPlace(StrToStaffrelBasic(element.attribute("clef.dis.place").value()));
        if (removeAttr) element.remove_attribute("clef.dis.place");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttCleffingLog::WriteCleffingLog(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasClefShape()) {
        element.append_attribute("clef.shape") = ClefshapeToStr(this->GetClefShape()).c_str();
        wroteAttribute = true;
    }
    if (this->HasClefLine()) {
        element.append_attribute("clef.line") = IntToStr(this->GetClefLine()).c_str();
        wroteAttribute = true;
    }
    if (this->HasClefDis()) {
        element.append_attribute("clef.dis") = OctaveDisToStr(this->GetClefDis()).c_str();
        wroteAttribute = true;
    }
    if (this->HasClefDisPlace()) {
        element.append_attribute("clef.dis.place") = StaffrelBasicToStr(this->GetClefDisPlace()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttCleffingLog::HasClefShape() const
{
    return (m_clefShape != CLEFSHAPE_NONE);
}

bool AttCleffingLog::HasClefLine() const
{
    return (m_clefLine != 0);
}

bool AttCleffingLog::HasClefDis() const
{
    return (m_clefDis != OCTAVE_DIS_NONE);
}

bool AttCleffingLog::HasClefDisPlace() const
{
    return (m_clefDisPlace != STAFFREL_basic_NONE);
}

//----------------------------------------------------------------------------
// AttColor
//----------------------------------------------------------------------------

AttColor::AttColor() : Att()
{
    ResetColor();
}

void AttColor::ResetColor()
{
    m_color = "";
}

bool AttColor::ReadColor(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("color")) {
        this->SetColor(StrToStr(element.attribute("color").value()));
        if (removeAttr) element.remove_attribute("color");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttColor::WriteColor(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasColor()) {
        element.append_attribute("color") = StrToStr(this->GetColor()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttColor::HasColor() const
{
    return (m_color != "");
}

//----------------------------------------------------------------------------
// AttCue
//----------------------------------------------------------------------------

AttCue::AttCue() : Att()
{
    ResetCue();
}

void AttCue::ResetCue()
{
    m_cue = BOOLEAN_NONE;
}

bool AttCue::ReadCue(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("cue")) {
        this->SetCue(StrToBoolean(element.attribute("cue").value()));
        if (removeAttr) element.remove_attribute("cue");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttCue::WriteCue(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasCue()) {
        element.append_attribute("cue") = BooleanToStr(this->GetCue()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttCue::HasCue() const
{
    return (m_cue != BOOLEAN_NONE);
}

//----------------------------------------------------------------------------
// AttCurvature
//----------------------------------------------------------------------------

AttCurvature::AttCurvature() : Att()
{
    ResetCurvature();
}

void AttCurvature::ResetCurvature()
{
    m_curvedir = curvature_CURVEDIR_NONE;
}

bool AttCurvature::ReadCurvature(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("curvedir")) {
        this->SetCurvedir(StrToCurvatureCurvedir(element.attribute("curvedir").value()));
        if (removeAttr) element.remove_attribute("curvedir");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttCurvature::WriteCurvature(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasCurvedir()) {
        element.append_attribute("curvedir") = CurvatureCurvedirToStr(this->GetCurvedir()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttCurvature::HasCurvedir() const
{
    return (m_curvedir != curvature_CURVEDIR_NONE);
}

//----------------------------------------------------------------------------
// AttDataPointing
//----------------------------------------------------------------------------

AttDataPointing::AttDataPointing() : Att()
{
    ResetDataPointing();
}

void AttDataPointing::ResetDataPointing()
{
    m_data = "";
}

bool AttDataPointing::ReadDataPointing(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("data")) {
        this->SetData(StrToStr(element.attribute("data").value()));
        if (removeAttr) element.remove_attribute("data");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttDataPointing::WriteDataPointing(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasData()) {
        element.append_attribute("data") = StrToStr(this->GetData()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttDataPointing::HasData() const
{
    return (m_data != "");
}

//----------------------------------------------------------------------------
// AttDatable
//----------------------------------------------------------------------------

AttDatable::AttDatable() : Att()
{
    ResetDatable();
}

void AttDatable::ResetDatable()
{
    m_enddate = "";
    m_isodate = "";
    m_notafter = "";
    m_notbefore = "";
    m_startdate = "";
}

bool AttDatable::ReadDatable(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("enddate")) {
        this->SetEnddate(StrToStr(element.attribute("enddate").value()));
        if (removeAttr) element.remove_attribute("enddate");
        hasAttribute = true;
    }
    if (element.attribute("isodate")) {
        this->SetIsodate(StrToStr(element.attribute("isodate").value()));
        if (removeAttr) element.remove_attribute("isodate");
        hasAttribute = true;
    }
    if (element.attribute("notafter")) {
        this->SetNotafter(StrToStr(element.attribute("notafter").value()));
        if (removeAttr) element.remove_attribute("notafter");
        hasAttribute = true;
    }
    if (element.attribute("notbefore")) {
        this->SetNotbefore(StrToStr(element.attribute("notbefore").value()));
        if (removeAttr) element.remove_attribute("notbefore");
        hasAttribute = true;
    }
    if (element.attribute("startdate")) {
        this->SetStartdate(StrToStr(element.attribute("startdate").value()));
        if (removeAttr) element.remove_attribute("startdate");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttDatable::WriteDatable(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasEnddate()) {
        element.append_attribute("enddate") = StrToStr(this->GetEnddate()).c_str();
        wroteAttribute = true;
    }
    if (this->HasIsodate()) {
        element.append_attribute("isodate") = StrToStr(this->GetIsodate()).c_str();
        wroteAttribute = true;
    }
    if (this->HasNotafter()) {
        element.append_attribute("notafter") = StrToStr(this->GetNotafter()).c_str();
        wroteAttribute = true;
    }
    if (this->HasNotbefore()) {
        element.append_attribute("notbefore") = StrToStr(this->GetNotbefore()).c_str();
        wroteAttribute = true;
    }
    if (this->HasStartdate()) {
        element.append_attribute("startdate") = StrToStr(this->GetStartdate()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttDatable::HasEnddate() const
{
    return (m_enddate != "");
}

bool AttDatable::HasIsodate() const
{
    return (m_isodate != "");
}

bool AttDatable::HasNotafter() const
{
    return (m_notafter != "");
}

bool AttDatable::HasNotbefore() const
{
    return (m_notbefore != "");
}

bool AttDatable::HasStartdate() const
{
    return (m_startdate != "");
}

//----------------------------------------------------------------------------
// AttDurationAdditive
//----------------------------------------------------------------------------

AttDurationAdditive::AttDurationAdditive() : Att()
{
    ResetDurationAdditive();
}

void AttDurationAdditive::ResetDurationAdditive()
{
    m_dur = DURATION_NONE;
}

bool AttDurationAdditive::ReadDurationAdditive(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("dur")) {
        this->SetDur(StrToDuration(element.attribute("dur").value()));
        if (removeAttr) element.remove_attribute("dur");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttDurationAdditive::WriteDurationAdditive(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasDur()) {
        element.append_attribute("dur") = DurationToStr(this->GetDur()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttDurationAdditive::HasDur() const
{
    return (m_dur != DURATION_NONE);
}

//----------------------------------------------------------------------------
// AttDurationLog
//----------------------------------------------------------------------------

AttDurationLog::AttDurationLog() : Att()
{
    ResetDurationLog();
}

void AttDurationLog::ResetDurationLog()
{
    m_dur = DURATION_NONE;
}

bool AttDurationLog::ReadDurationLog(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("dur")) {
        this->SetDur(StrToDuration(element.attribute("dur").value()));
        if (removeAttr) element.remove_attribute("dur");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttDurationLog::WriteDurationLog(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasDur()) {
        element.append_attribute("dur") = DurationToStr(this->GetDur()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttDurationLog::HasDur() const
{
    return (m_dur != DURATION_NONE);
}

//----------------------------------------------------------------------------
// AttDurationRatio
//----------------------------------------------------------------------------

AttDurationRatio::AttDurationRatio() : Att()
{
    ResetDurationRatio();
}

void AttDurationRatio::ResetDurationRatio()
{
    m_num = MEI_UNSET;
    m_numbase = MEI_UNSET;
}

bool AttDurationRatio::ReadDurationRatio(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("num")) {
        this->SetNum(StrToInt(element.attribute("num").value()));
        if (removeAttr) element.remove_attribute("num");
        hasAttribute = true;
    }
    if (element.attribute("numbase")) {
        this->SetNumbase(StrToInt(element.attribute("numbase").value()));
        if (removeAttr) element.remove_attribute("numbase");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttDurationRatio::WriteDurationRatio(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasNum()) {
        element.append_attribute("num") = IntToStr(this->GetNum()).c_str();
        wroteAttribute = true;
    }
    if (this->HasNumbase()) {
        element.append_attribute("numbase") = IntToStr(this->GetNumbase()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttDurationRatio::HasNum() const
{
    return (m_num != MEI_UNSET);
}

bool AttDurationRatio::HasNumbase() const
{
    return (m_numbase != MEI_UNSET);
}

//----------------------------------------------------------------------------
// AttEnclosingChars
//----------------------------------------------------------------------------

AttEnclosingChars::AttEnclosingChars() : Att()
{
    ResetEnclosingChars();
}

void AttEnclosingChars::ResetEnclosingChars()
{
    m_enclose = ENCLOSURE_NONE;
}

bool AttEnclosingChars::ReadEnclosingChars(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("enclose")) {
        this->SetEnclose(StrToEnclosure(element.attribute("enclose").value()));
        if (removeAttr) element.remove_attribute("enclose");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttEnclosingChars::WriteEnclosingChars(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasEnclose()) {
        element.append_attribute("enclose") = EnclosureToStr(this->GetEnclose()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttEnclosingChars::HasEnclose() const
{
    return (m_enclose != ENCLOSURE_NONE);
}

//----------------------------------------------------------------------------
// AttExtender
//----------------------------------------------------------------------------

AttExtender::AttExtender() : Att()
{
    ResetExtender();
}

void AttExtender::ResetExtender()
{
    m_extender = BOOLEAN_NONE;
}

bool AttExtender::ReadExtender(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("extender")) {
        this->SetExtender(StrToBoolean(element.attribute("extender").value()));
        if (removeAttr) element.remove_attribute("extender");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttExtender::WriteExtender(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasExtender()) {
        element.append_attribute("extender") = BooleanToStr(this->GetExtender()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttExtender::HasExtender() const
{
    return (m_extender != BOOLEAN_NONE);
}

//----------------------------------------------------------------------------
// AttFormework
//----------------------------------------------------------------------------

AttFormework::AttFormework() : Att()
{
    ResetFormework();
}

void AttFormework::ResetFormework()
{
    m_func = PGFUNC_NONE;
}

bool AttFormework::ReadFormework(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("func")) {
        this->SetFunc(StrToPgfunc(element.attribute("func").value()));
        if (removeAttr) element.remove_attribute("func");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttFormework::WriteFormework(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasFunc()) {
        element.append_attribute("func") = PgfuncToStr(this->GetFunc()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttFormework::HasFunc() const
{
    return (m_func != PGFUNC_NONE);
}

//----------------------------------------------------------------------------
// AttHorizontalAlign
//----------------------------------------------------------------------------

AttHorizontalAlign::AttHorizontalAlign() : Att()
{
    ResetHorizontalAlign();
}

void AttHorizontalAlign::ResetHorizontalAlign()
{
    m_halign = HORIZONTALALIGNMENT_NONE;
}

bool AttHorizontalAlign::ReadHorizontalAlign(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("halign")) {
        this->SetHalign(StrToHorizontalalignment(element.attribute("halign").value()));
        if (removeAttr) element.remove_attribute("halign");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttHorizontalAlign::WriteHorizontalAlign(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasHalign()) {
        element.append_attribute("halign") = HorizontalalignmentToStr(this->GetHalign()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttHorizontalAlign::HasHalign() const
{
    return (m_halign != HORIZONTALALIGNMENT_NONE);
}

//----------------------------------------------------------------------------
// AttKeySigDefaultLog
//----------------------------------------------------------------------------

AttKeySigDefaultLog::AttKeySigDefaultLog() : Att()
{
    ResetKeySigDefaultLog();
}

void AttKeySigDefaultLog::ResetKeySigDefaultLog()
{
    m_keysig = std::make_pair(-1, ACCIDENTAL_WRITTEN_NONE);
}

bool AttKeySigDefaultLog::ReadKeySigDefaultLog(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("keysig")) {
        this->SetKeysig(StrToKeysignature(element.attribute("keysig").value()));
        if (removeAttr) element.remove_attribute("keysig");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttKeySigDefaultLog::WriteKeySigDefaultLog(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasKeysig()) {
        element.append_attribute("keysig") = KeysignatureToStr(this->GetKeysig()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttKeySigDefaultLog::HasKeysig() const
{
    return (m_keysig != std::make_pair(-1, ACCIDENTAL_WRITTEN_NONE));
}

//----------------------------------------------------------------------------
// AttLabelled
//----------------------------------------------------------------------------

AttLabelled::AttLabelled() : Att()
{
    ResetLabelled();
}

void AttLabelled::ResetLabelled()
{
    m_label = "";
}

bool AttLabelled::ReadLabelled(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("label")) {
        this->SetLabel(StrToStr(element.attribute("label").value()));
        if (removeAttr) element.remove_attribute("label");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttLabelled::WriteLabelled(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasLabel()) {
        element.append_attribute("label") = StrToStr(this->GetLabel()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttLabelled::HasLabel() const
{
    return (m_label != "");
}

//----------------------------------------------------------------------------
// AttLang
//----------------------------------------------------------------------------

AttLang::AttLang() : Att()
{
    ResetLang();
}

void AttLang::ResetLang()
{
    m_lang = "";
}

bool AttLang::ReadLang(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("xml:lang")) {
        this->SetLang(StrToStr(element.attribute("xml:lang").value()));
        if (removeAttr) element.remove_attribute("xml:lang");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttLang::WriteLang(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasLang()) {
        element.append_attribute("xml:lang") = StrToStr(this->GetLang()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttLang::HasLang() const
{
    return (m_lang != "");
}

//----------------------------------------------------------------------------
// AttLayerIdent
//----------------------------------------------------------------------------

AttLayerIdent::AttLayerIdent() : Att()
{
    ResetLayerIdent();
}

void AttLayerIdent::ResetLayerIdent()
{
    m_layer = MEI_UNSET;
}

bool AttLayerIdent::ReadLayerIdent(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("layer")) {
        this->SetLayer(StrToInt(element.attribute("layer").value()));
        if (removeAttr) element.remove_attribute("layer");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttLayerIdent::WriteLayerIdent(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasLayer()) {
        element.append_attribute("layer") = IntToStr(this->GetLayer()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttLayerIdent::HasLayer() const
{
    return (m_layer != MEI_UNSET);
}

//----------------------------------------------------------------------------
// AttLineLoc
//----------------------------------------------------------------------------

AttLineLoc::AttLineLoc() : Att()
{
    ResetLineLoc();
}

void AttLineLoc::ResetLineLoc()
{
    m_line = 0;
}

bool AttLineLoc::ReadLineLoc(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("line")) {
        this->SetLine(StrToInt(element.attribute("line").value()));
        if (removeAttr) element.remove_attribute("line");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttLineLoc::WriteLineLoc(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasLine()) {
        element.append_attribute("line") = IntToStr(this->GetLine()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttLineLoc::HasLine() const
{
    return (m_line != 0);
}

//----------------------------------------------------------------------------
// AttLineRend
//----------------------------------------------------------------------------

AttLineRend::AttLineRend() : Att()
{
    ResetLineRend();
}

void AttLineRend::ResetLineRend()
{
    m_lendsym = LINESTARTENDSYMBOL_NONE;
    m_lendsymSize = MEI_UNSET;
    m_lstartsym = LINESTARTENDSYMBOL_NONE;
    m_lstartsymSize = MEI_UNSET;
}

bool AttLineRend::ReadLineRend(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("lendsym")) {
        this->SetLendsym(StrToLinestartendsymbol(element.attribute("lendsym").value()));
        if (removeAttr) element.remove_attribute("lendsym");
        hasAttribute = true;
    }
    if (element.attribute("lendsym.size")) {
        this->SetLendsymSize(StrToInt(element.attribute("lendsym.size").value()));
        if (removeAttr) element.remove_attribute("lendsym.size");
        hasAttribute = true;
    }
    if (element.attribute("lstartsym")) {
        this->SetLstartsym(StrToLinestartendsymbol(element.attribute("lstartsym").value()));
        if (removeAttr) element.remove_attribute("lstartsym");
        hasAttribute = true;
    }
    if (element.attribute("lstartsym.size")) {
        this->SetLstartsymSize(StrToInt(element.attribute("lstartsym.size").value()));
        if (removeAttr) element.remove_attribute("lstartsym.size");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttLineRend::WriteLineRend(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasLendsym()) {
        element.append_attribute("lendsym") = LinestartendsymbolToStr(this->GetLendsym()).c_str();
        wroteAttribute = true;
    }
    if (this->HasLendsymSize()) {
        element.append_attribute("lendsym.size") = IntToStr(this->GetLendsymSize()).c_str();
        wroteAttribute = true;
    }
    if (this->HasLstartsym()) {
        element.append_attribute("lstartsym") = LinestartendsymbolToStr(this->GetLstartsym()).c_str();
        wroteAttribute = true;
    }
    if (this->HasLstartsymSize()) {
        element.append_attribute("lstartsym.size") = IntToStr(this->GetLstartsymSize()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttLineRend::HasLendsym() const
{
    return (m_lendsym != LINESTARTENDSYMBOL_NONE);
}

bool AttLineRend::HasLendsymSize() const
{
    return (m_lendsymSize != MEI_UNSET);
}

bool AttLineRend::HasLstartsym() const
{
    return (m_lstartsym != LINESTARTENDSYMBOL_NONE);
}

bool AttLineRend::HasLstartsymSize() const
{
    return (m_lstartsymSize != MEI_UNSET);
}

//----------------------------------------------------------------------------
// AttLineRendBase
//----------------------------------------------------------------------------

AttLineRendBase::AttLineRendBase() : Att()
{
    ResetLineRendBase();
}

void AttLineRendBase::ResetLineRendBase()
{
    m_lform = LINEFORM_NONE;
    m_lwidth = data_LINEWIDTH();
    m_lsegs = MEI_UNSET;
}

bool AttLineRendBase::ReadLineRendBase(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("lform")) {
        this->SetLform(StrToLineform(element.attribute("lform").value()));
        if (removeAttr) element.remove_attribute("lform");
        hasAttribute = true;
    }
    if (element.attribute("lwidth")) {
        this->SetLwidth(StrToLinewidth(element.attribute("lwidth").value()));
        if (removeAttr) element.remove_attribute("lwidth");
        hasAttribute = true;
    }
    if (element.attribute("lsegs")) {
        this->SetLsegs(StrToInt(element.attribute("lsegs").value()));
        if (removeAttr) element.remove_attribute("lsegs");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttLineRendBase::WriteLineRendBase(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasLform()) {
        element.append_attribute("lform") = LineformToStr(this->GetLform()).c_str();
        wroteAttribute = true;
    }
    if (this->HasLwidth()) {
        element.append_attribute("lwidth") = LinewidthToStr(this->GetLwidth()).c_str();
        wroteAttribute = true;
    }
    if (this->HasLsegs()) {
        element.append_attribute("lsegs") = IntToStr(this->GetLsegs()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttLineRendBase::HasLform() const
{
    return (m_lform != LINEFORM_NONE);
}

bool AttLineRendBase::HasLwidth() const
{
    return (m_lwidth.HasValue());
}

bool AttLineRendBase::HasLsegs() const
{
    return (m_lsegs != MEI_UNSET);
}

//----------------------------------------------------------------------------
// AttMeiVersion
//----------------------------------------------------------------------------

AttMeiVersion::AttMeiVersion() : Att()
{
    ResetMeiVersion();
}

void AttMeiVersion::ResetMeiVersion()
{
    m_meiversion = meiVersion_MEIVERSION_NONE;
}

bool AttMeiVersion::ReadMeiVersion(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("meiversion")) {
        this->SetMeiversion(StrToMeiVersionMeiversion(element.attribute("meiversion").value()));
        if (removeAttr) element.remove_attribute("meiversion");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttMeiVersion::WriteMeiVersion(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasMeiversion()) {
        element.append_attribute("meiversion") = MeiVersionMeiversionToStr(this->GetMeiversion()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttMeiVersion::HasMeiversion() const
{
    return (m_meiversion != meiVersion_MEIVERSION_NONE);
}

//----------------------------------------------------------------------------
// AttMeterConformanceBar
//----------------------------------------------------------------------------

AttMeterConformanceBar::AttMeterConformanceBar() : Att()
{
    ResetMeterConformanceBar();
}

void AttMeterConformanceBar::ResetMeterConformanceBar()
{
    m_metcon = BOOLEAN_NONE;
}

bool AttMeterConformanceBar::ReadMeterConformanceBar(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("metcon")) {
        this->SetMetcon(StrToBoolean(element.attribute("metcon").value()));
        if (removeAttr) element.remove_attribute("metcon");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttMeterConformanceBar::WriteMeterConformanceBar(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasMetcon()) {
        element.append_attribute("metcon") = BooleanToStr(this->GetMetcon()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttMeterConformanceBar::HasMetcon() const
{
    return (m_metcon != BOOLEAN_NONE);
}

//----------------------------------------------------------------------------
// AttMeterSigDefaultLog
//----------------------------------------------------------------------------

AttMeterSigDefaultLog::AttMeterSigDefaultLog() : Att()
{
    ResetMeterSigDefaultLog();
}

void AttMeterSigDefaultLog::ResetMeterSigDefaultLog()
{
    m_meterCount = std::pair<std::vector<int>, MeterCountSign>();
    m_meterUnit = MEI_UNSET;
    m_meterSym = METERSIGN_NONE;
}

bool AttMeterSigDefaultLog::ReadMeterSigDefaultLog(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("meter.count")) {
        this->SetMeterCount(StrToMetercountPair(element.attribute("meter.count").value()));
        if (removeAttr) element.remove_attribute("meter.count");
        hasAttribute = true;
    }
    if (element.attribute("meter.unit")) {
        this->SetMeterUnit(StrToInt(element.attribute("meter.unit").value()));
        if (removeAttr) element.remove_attribute("meter.unit");
        hasAttribute = true;
    }
    if (element.attribute("meter.sym")) {
        this->SetMeterSym(StrToMetersign(element.attribute("meter.sym").value()));
        if (removeAttr) element.remove_attribute("meter.sym");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttMeterSigDefaultLog::WriteMeterSigDefaultLog(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasMeterCount()) {
        element.append_attribute("meter.count") = MetercountPairToStr(this->GetMeterCount()).c_str();
        wroteAttribute = true;
    }
    if (this->HasMeterUnit()) {
        element.append_attribute("meter.unit") = IntToStr(this->GetMeterUnit()).c_str();
        wroteAttribute = true;
    }
    if (this->HasMeterSym()) {
        element.append_attribute("meter.sym") = MetersignToStr(this->GetMeterSym()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttMeterSigDefaultLog::HasMeterCount() const
{
    return (m_meterCount != std::pair<std::vector<int>, MeterCountSign>());
}

bool AttMeterSigDefaultLog::HasMeterUnit() const
{
    return (m_meterUnit != MEI_UNSET);
}

bool AttMeterSigDefaultLog::HasMeterSym() const
{
    return (m_meterSym != METERSIGN_NONE);
}

//----------------------------------------------------------------------------
// AttMmTempo
//----------------------------------------------------------------------------

AttMmTempo::AttMmTempo() : Att()
{
    ResetMmTempo();
}

void AttMmTempo::ResetMmTempo()
{
    m_mm = 0.0;
    m_mmUnit = DURATION_NONE;
    m_mmDots = MEI_UNSET;
}

bool AttMmTempo::ReadMmTempo(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("mm")) {
        this->SetMm(StrToDbl(element.attribute("mm").value()));
        if (removeAttr) element.remove_attribute("mm");
        hasAttribute = true;
    }
    if (element.attribute("mm.unit")) {
        this->SetMmUnit(StrToDuration(element.attribute("mm.unit").value()));
        if (removeAttr) element.remove_attribute("mm.unit");
        hasAttribute = true;
    }
    if (element.attribute("mm.dots")) {
        this->SetMmDots(StrToInt(element.attribute("mm.dots").value()));
        if (removeAttr) element.remove_attribute("mm.dots");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttMmTempo::WriteMmTempo(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasMm()) {
        element.append_attribute("mm") = DblToStr(this->GetMm()).c_str();
        wroteAttribute = true;
    }
    if (this->HasMmUnit()) {
        element.append_attribute("mm.unit") = DurationToStr(this->GetMmUnit()).c_str();
        wroteAttribute = true;
    }
    if (this->HasMmDots()) {
        element.append_attribute("mm.dots") = IntToStr(this->GetMmDots()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttMmTempo::HasMm() const
{
    return (m_mm != 0.0);
}

bool AttMmTempo::HasMmUnit() const
{
    return (m_mmUnit != DURATION_NONE);
}

bool AttMmTempo::HasMmDots() const
{
    return (m_mmDots != MEI_UNSET);
}

//----------------------------------------------------------------------------
// AttNInteger
//----------------------------------------------------------------------------

AttNInteger::AttNInteger() : Att()
{
    ResetNInteger();
}

void AttNInteger::ResetNInteger()
{
    m_n = MEI_UNSET;
}

bool AttNInteger::ReadNInteger(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("n")) {
        this->SetN(StrToInt(element.attribute("n").value()));
        if (removeAttr) element.remove_attribute("n");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttNInteger::WriteNInteger(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasN()) {
        element.append_attribute("n") = IntToStr(this->GetN()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttNInteger::HasN() const
{
    return (m_n != MEI_UNSET);
}

//----------------------------------------------------------------------------
// AttNNumberLike
//----------------------------------------------------------------------------

AttNNumberLike::AttNNumberLike() : Att()
{
    ResetNNumberLike();
}

void AttNNumberLike::ResetNNumberLike()
{
    m_n = "";
}

bool AttNNumberLike::ReadNNumberLike(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("n")) {
        this->SetN(StrToStr(element.attribute("n").value()));
        if (removeAttr) element.remove_attribute("n");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttNNumberLike::WriteNNumberLike(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasN()) {
        element.append_attribute("n") = StrToStr(this->GetN()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttNNumberLike::HasN() const
{
    return (m_n != "");
}

//----------------------------------------------------------------------------
// AttName
//----------------------------------------------------------------------------

AttName::AttName() : Att()
{
    ResetName();
}

void AttName::ResetName()
{
    m_nymref = "";
    m_role = "";
}

bool AttName::ReadName(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("nymref")) {
        this->SetNymref(StrToStr(element.attribute("nymref").value()));
        if (removeAttr) element.remove_attribute("nymref");
        hasAttribute = true;
    }
    if (element.attribute("role")) {
        this->SetRole(StrToStr(element.attribute("role").value()));
        if (removeAttr) element.remove_attribute("role");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttName::WriteName(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasNymref()) {
        element.append_attribute("nymref") = StrToStr(this->GetNymref()).c_str();
        wroteAttribute = true;
    }
    if (this->HasRole()) {
        element.append_attribute("role") = StrToStr(this->GetRole()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttName::HasNymref() const
{
    return (m_nymref != "");
}

bool AttName::HasRole() const
{
    return (m_role != "");
}

//----------------------------------------------------------------------------
// AttOctave
//----------------------------------------------------------------------------

AttOctave::AttOctave() : Att()
{
    ResetOctave();
}

void AttOctave::ResetOctave()
{
    m_oct = -127;
}

bool AttOctave::ReadOctave(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("oct")) {
        this->SetOct(StrToOctave(element.attribute("oct").value()));
        if (removeAttr) element.remove_attribute("oct");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttOctave::WriteOctave(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasOct()) {
        element.append_attribute("oct") = OctaveToStr(this->GetOct()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttOctave::HasOct() const
{
    return (m_oct != -127);
}

//----------------------------------------------------------------------------
// AttOctaveDisplacement
//----------------------------------------------------------------------------

AttOctaveDisplacement::AttOctaveDisplacement() : Att()
{
    ResetOctaveDisplacement();
}

void AttOctaveDisplacement::ResetOctaveDisplacement()
{
    m_dis = OCTAVE_DIS_NONE;
    m_disPlace = STAFFREL_basic_NONE;
}

bool AttOctaveDisplacement::ReadOctaveDisplacement(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("dis")) {
        this->SetDis(StrToOctaveDis(element.attribute("dis").value()));
        if (removeAttr) element.remove_attribute("dis");
        hasAttribute = true;
    }
    if (element.attribute("dis.place")) {
        this->SetDisPlace(StrToStaffrelBasic(element.attribute("dis.place").value()));
        if (removeAttr) element.remove_attribute("dis.place");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttOctaveDisplacement::WriteOctaveDisplacement(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasDis()) {
        element.append_attribute("dis") = OctaveDisToStr(this->GetDis()).c_str();
        wroteAttribute = true;
    }
    if (this->HasDisPlace()) {
        element.append_attribute("dis.place") = StaffrelBasicToStr(this->GetDisPlace()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttOctaveDisplacement::HasDis() const
{
    return (m_dis != OCTAVE_DIS_NONE);
}

bool AttOctaveDisplacement::HasDisPlace() const
{
    return (m_disPlace != STAFFREL_basic_NONE);
}

//----------------------------------------------------------------------------
// AttPitch
//----------------------------------------------------------------------------

AttPitch::AttPitch() : Att()
{
    ResetPitch();
}

void AttPitch::ResetPitch()
{
    m_pname = PITCHNAME_NONE;
}

bool AttPitch::ReadPitch(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("pname")) {
        this->SetPname(StrToPitchname(element.attribute("pname").value()));
        if (removeAttr) element.remove_attribute("pname");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttPitch::WritePitch(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasPname()) {
        element.append_attribute("pname") = PitchnameToStr(this->GetPname()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttPitch::HasPname() const
{
    return (m_pname != PITCHNAME_NONE);
}

//----------------------------------------------------------------------------
// AttPlacementRelEvent
//----------------------------------------------------------------------------

AttPlacementRelEvent::AttPlacementRelEvent() : Att()
{
    ResetPlacementRelEvent();
}

void AttPlacementRelEvent::ResetPlacementRelEvent()
{
    m_place = data_STAFFREL();
}

bool AttPlacementRelEvent::ReadPlacementRelEvent(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("place")) {
        this->SetPlace(StrToStaffrel(element.attribute("place").value()));
        if (removeAttr) element.remove_attribute("place");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttPlacementRelEvent::WritePlacementRelEvent(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasPlace()) {
        element.append_attribute("place") = StaffrelToStr(this->GetPlace()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttPlacementRelEvent::HasPlace() const
{
    return (m_place != data_STAFFREL());
}

//----------------------------------------------------------------------------
// AttPlacementRelStaff
//----------------------------------------------------------------------------

AttPlacementRelStaff::AttPlacementRelStaff() : Att()
{
    ResetPlacementRelStaff();
}

void AttPlacementRelStaff::ResetPlacementRelStaff()
{
    m_place = data_STAFFREL();
}

bool AttPlacementRelStaff::ReadPlacementRelStaff(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("place")) {
        this->SetPlace(StrToStaffrel(element.attribute("place").value()));
        if (removeAttr) element.remove_attribute("place");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttPlacementRelStaff::WritePlacementRelStaff(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasPlace()) {
        element.append_attribute("place") = StaffrelToStr(this->GetPlace()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttPlacementRelStaff::HasPlace() const
{
    return (m_place != data_STAFFREL());
}

//----------------------------------------------------------------------------
// AttPlist
//----------------------------------------------------------------------------

AttPlist::AttPlist() : Att()
{
    ResetPlist();
}

void AttPlist::ResetPlist()
{
    m_plist = std::vector<std::string>();
}

bool AttPlist::ReadPlist(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("plist")) {
        this->SetPlist(StrToXsdAnyURIList(element.attribute("plist").value()));
        if (removeAttr) element.remove_attribute("plist");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttPlist::WritePlist(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasPlist()) {
        element.append_attribute("plist") = XsdAnyURIListToStr(this->GetPlist()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttPlist::HasPlist() const
{
    return (m_plist != std::vector<std::string>());
}

//----------------------------------------------------------------------------
// AttRepeatMarkLog
//----------------------------------------------------------------------------

AttRepeatMarkLog::AttRepeatMarkLog() : Att()
{
    ResetRepeatMarkLog();
}

void AttRepeatMarkLog::ResetRepeatMarkLog()
{
    m_func = repeatMarkLog_FUNC_NONE;
}

bool AttRepeatMarkLog::ReadRepeatMarkLog(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("func")) {
        this->SetFunc(StrToRepeatMarkLogFunc(element.attribute("func").value()));
        if (removeAttr) element.remove_attribute("func");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttRepeatMarkLog::WriteRepeatMarkLog(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasFunc()) {
        element.append_attribute("func") = RepeatMarkLogFuncToStr(this->GetFunc()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttRepeatMarkLog::HasFunc() const
{
    return (m_func != repeatMarkLog_FUNC_NONE);
}

//----------------------------------------------------------------------------
// AttScalable
//----------------------------------------------------------------------------

AttScalable::AttScalable() : Att()
{
    ResetScalable();
}

void AttScalable::ResetScalable()
{
    m_scale = -1.0;
}

bool AttScalable::ReadScalable(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("scale")) {
        this->SetScale(StrToPercent(element.attribute("scale").value()));
        if (removeAttr) element.remove_attribute("scale");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttScalable::WriteScalable(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasScale()) {
        element.append_attribute("scale") = PercentToStr(this->GetScale()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttScalable::HasScale() const
{
    return (m_scale != -1.0);
}

//----------------------------------------------------------------------------
// AttStaffDefLog
//----------------------------------------------------------------------------

AttStaffDefLog::AttStaffDefLog() : Att()
{
    ResetStaffDefLog();
}

void AttStaffDefLog::ResetStaffDefLog()
{
    m_lines = MEI_UNSET;
}

bool AttStaffDefLog::ReadStaffDefLog(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("lines")) {
        this->SetLines(StrToInt(element.attribute("lines").value()));
        if (removeAttr) element.remove_attribute("lines");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttStaffDefLog::WriteStaffDefLog(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasLines()) {
        element.append_attribute("lines") = IntToStr(this->GetLines()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttStaffDefLog::HasLines() const
{
    return (m_lines != MEI_UNSET);
}

//----------------------------------------------------------------------------
// AttStaffGroupingSym
//----------------------------------------------------------------------------

AttStaffGroupingSym::AttStaffGroupingSym() : Att()
{
    ResetStaffGroupingSym();
}

void AttStaffGroupingSym::ResetStaffGroupingSym()
{
    m_symbol = staffGroupingSym_SYMBOL_NONE;
}

bool AttStaffGroupingSym::ReadStaffGroupingSym(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("symbol")) {
        this->SetSymbol(StrToStaffGroupingSymSymbol(element.attribute("symbol").value()));
        if (removeAttr) element.remove_attribute("symbol");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttStaffGroupingSym::WriteStaffGroupingSym(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasSymbol()) {
        element.append_attribute("symbol") = StaffGroupingSymSymbolToStr(this->GetSymbol()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttStaffGroupingSym::HasSymbol() const
{
    return (m_symbol != staffGroupingSym_SYMBOL_NONE);
}

//----------------------------------------------------------------------------
// AttStaffIdent
//----------------------------------------------------------------------------

AttStaffIdent::AttStaffIdent() : Att()
{
    ResetStaffIdent();
}

void AttStaffIdent::ResetStaffIdent()
{
    m_staff = std::vector<int>();
}

bool AttStaffIdent::ReadStaffIdent(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("staff")) {
        this->SetStaff(StrToXsdPositiveIntegerList(element.attribute("staff").value()));
        if (removeAttr) element.remove_attribute("staff");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttStaffIdent::WriteStaffIdent(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasStaff()) {
        element.append_attribute("staff") = XsdPositiveIntegerListToStr(this->GetStaff()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttStaffIdent::HasStaff() const
{
    return (m_staff != std::vector<int>());
}

//----------------------------------------------------------------------------
// AttStaffLocPitched
//----------------------------------------------------------------------------

AttStaffLocPitched::AttStaffLocPitched() : Att()
{
    ResetStaffLocPitched();
}

void AttStaffLocPitched::ResetStaffLocPitched()
{
    m_ploc = PITCHNAME_NONE;
    m_oloc = -127;
}

bool AttStaffLocPitched::ReadStaffLocPitched(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("ploc")) {
        this->SetPloc(StrToPitchname(element.attribute("ploc").value()));
        if (removeAttr) element.remove_attribute("ploc");
        hasAttribute = true;
    }
    if (element.attribute("oloc")) {
        this->SetOloc(StrToOctave(element.attribute("oloc").value()));
        if (removeAttr) element.remove_attribute("oloc");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttStaffLocPitched::WriteStaffLocPitched(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasPloc()) {
        element.append_attribute("ploc") = PitchnameToStr(this->GetPloc()).c_str();
        wroteAttribute = true;
    }
    if (this->HasOloc()) {
        element.append_attribute("oloc") = OctaveToStr(this->GetOloc()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttStaffLocPitched::HasPloc() const
{
    return (m_ploc != PITCHNAME_NONE);
}

bool AttStaffLocPitched::HasOloc() const
{
    return (m_oloc != -127);
}

//----------------------------------------------------------------------------
// AttStartEndId
//----------------------------------------------------------------------------

AttStartEndId::AttStartEndId() : Att()
{
    ResetStartEndId();
}

void AttStartEndId::ResetStartEndId()
{
    m_endid = "";
}

bool AttStartEndId::ReadStartEndId(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("endid")) {
        this->SetEndid(StrToStr(element.attribute("endid").value()));
        if (removeAttr) element.remove_attribute("endid");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttStartEndId::WriteStartEndId(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasEndid()) {
        element.append_attribute("endid") = StrToStr(this->GetEndid()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttStartEndId::HasEndid() const
{
    return (m_endid != "");
}

//----------------------------------------------------------------------------
// AttStartId
//----------------------------------------------------------------------------

AttStartId::AttStartId() : Att()
{
    ResetStartId();
}

void AttStartId::ResetStartId()
{
    m_startid = "";
}

bool AttStartId::ReadStartId(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("startid")) {
        this->SetStartid(StrToStr(element.attribute("startid").value()));
        if (removeAttr) element.remove_attribute("startid");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttStartId::WriteStartId(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasStartid()) {
        element.append_attribute("startid") = StrToStr(this->GetStartid()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttStartId::HasStartid() const
{
    return (m_startid != "");
}

//----------------------------------------------------------------------------
// AttStems
//----------------------------------------------------------------------------

AttStems::AttStems() : Att()
{
    ResetStems();
}

void AttStems::ResetStems()
{
    m_stemDir = STEMDIRECTION_NONE;
    m_stemLen = -1.0;
    m_stemMod = STEMMODIFIER_NONE;
}

bool AttStems::ReadStems(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("stem.dir")) {
        this->SetStemDir(StrToStemdirection(element.attribute("stem.dir").value()));
        if (removeAttr) element.remove_attribute("stem.dir");
        hasAttribute = true;
    }
    if (element.attribute("stem.len")) {
        this->SetStemLen(StrToDbl(element.attribute("stem.len").value()));
        if (removeAttr) element.remove_attribute("stem.len");
        hasAttribute = true;
    }
    if (element.attribute("stem.mod")) {
        this->SetStemMod(StrToStemmodifier(element.attribute("stem.mod").value()));
        if (removeAttr) element.remove_attribute("stem.mod");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttStems::WriteStems(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasStemDir()) {
        element.append_attribute("stem.dir") = StemdirectionToStr(this->GetStemDir()).c_str();
        wroteAttribute = true;
    }
    if (this->HasStemLen()) {
        element.append_attribute("stem.len") = DblToStr(this->GetStemLen()).c_str();
        wroteAttribute = true;
    }
    if (this->HasStemMod()) {
        element.append_attribute("stem.mod") = StemmodifierToStr(this->GetStemMod()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttStems::HasStemDir() const
{
    return (m_stemDir != STEMDIRECTION_NONE);
}

bool AttStems::HasStemLen() const
{
    return (m_stemLen != -1.0);
}

bool AttStems::HasStemMod() const
{
    return (m_stemMod != STEMMODIFIER_NONE);
}

//----------------------------------------------------------------------------
// AttSylLog
//----------------------------------------------------------------------------

AttSylLog::AttSylLog() : Att()
{
    ResetSylLog();
}

void AttSylLog::ResetSylLog()
{
    m_con = sylLog_CON_NONE;
    m_wordpos = sylLog_WORDPOS_NONE;
}

bool AttSylLog::ReadSylLog(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("con")) {
        this->SetCon(StrToSylLogCon(element.attribute("con").value()));
        if (removeAttr) element.remove_attribute("con");
        hasAttribute = true;
    }
    if (element.attribute("wordpos")) {
        this->SetWordpos(StrToSylLogWordpos(element.attribute("wordpos").value()));
        if (removeAttr) element.remove_attribute("wordpos");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttSylLog::WriteSylLog(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasCon()) {
        element.append_attribute("con") = SylLogConToStr(this->GetCon()).c_str();
        wroteAttribute = true;
    }
    if (this->HasWordpos()) {
        element.append_attribute("wordpos") = SylLogWordposToStr(this->GetWordpos()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttSylLog::HasCon() const
{
    return (m_con != sylLog_CON_NONE);
}

bool AttSylLog::HasWordpos() const
{
    return (m_wordpos != sylLog_WORDPOS_NONE);
}

//----------------------------------------------------------------------------
// AttTempoLog
//----------------------------------------------------------------------------

AttTempoLog::AttTempoLog() : Att()
{
    ResetTempoLog();
}

void AttTempoLog::ResetTempoLog()
{
    m_func = tempoLog_FUNC_NONE;
}

bool AttTempoLog::ReadTempoLog(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("func")) {
        this->SetFunc(StrToTempoLogFunc(element.attribute("func").value()));
        if (removeAttr) element.remove_attribute("func");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttTempoLog::WriteTempoLog(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasFunc()) {
        element.append_attribute("func") = TempoLogFuncToStr(this->GetFunc()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttTempoLog::HasFunc() const
{
    return (m_func != tempoLog_FUNC_NONE);
}

//----------------------------------------------------------------------------
// AttTextRendition
//----------------------------------------------------------------------------

AttTextRendition::AttTextRendition() : Att()
{
    ResetTextRendition();
}

void AttTextRendition::ResetTextRendition()
{
    m_altrend = "";
    m_rend = TEXTRENDITION_NONE;
}

bool AttTextRendition::ReadTextRendition(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("altrend")) {
        this->SetAltrend(StrToStr(element.attribute("altrend").value()));
        if (removeAttr) element.remove_attribute("altrend");
        hasAttribute = true;
    }
    if (element.attribute("rend")) {
        this->SetRend(StrToTextrendition(element.attribute("rend").value()));
        if (removeAttr) element.remove_attribute("rend");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttTextRendition::WriteTextRendition(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasAltrend()) {
        element.append_attribute("altrend") = StrToStr(this->GetAltrend()).c_str();
        wroteAttribute = true;
    }
    if (this->HasRend()) {
        element.append_attribute("rend") = TextrenditionToStr(this->GetRend()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttTextRendition::HasAltrend() const
{
    return (m_altrend != "");
}

bool AttTextRendition::HasRend() const
{
    return (m_rend != TEXTRENDITION_NONE);
}

//----------------------------------------------------------------------------
// AttTimestampLog
//----------------------------------------------------------------------------

AttTimestampLog::AttTimestampLog() : Att()
{
    ResetTimestampLog();
}

void AttTimestampLog::ResetTimestampLog()
{
    m_tstamp = -1.0;
}

bool AttTimestampLog::ReadTimestampLog(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("tstamp")) {
        this->SetTstamp(StrToDbl(element.attribute("tstamp").value()));
        if (removeAttr) element.remove_attribute("tstamp");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttTimestampLog::WriteTimestampLog(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasTstamp()) {
        element.append_attribute("tstamp") = DblToStr(this->GetTstamp()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttTimestampLog::HasTstamp() const
{
    return (m_tstamp != -1.0);
}

//----------------------------------------------------------------------------
// AttTimestamp2Log
//----------------------------------------------------------------------------

AttTimestamp2Log::AttTimestamp2Log() : Att()
{
    ResetTimestamp2Log();
}

void AttTimestamp2Log::ResetTimestamp2Log()
{
    m_tstamp2 = std::make_pair(-1, -1.0);
}

bool AttTimestamp2Log::ReadTimestamp2Log(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("tstamp2")) {
        this->SetTstamp2(StrToMeasurebeat(element.attribute("tstamp2").value()));
        if (removeAttr) element.remove_attribute("tstamp2");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttTimestamp2Log::WriteTimestamp2Log(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasTstamp2()) {
        element.append_attribute("tstamp2") = MeasurebeatToStr(this->GetTstamp2()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttTimestamp2Log::HasTstamp2() const
{
    return (m_tstamp2 != std::make_pair(-1, -1.0));
}

//----------------------------------------------------------------------------
// AttTransposition
//----------------------------------------------------------------------------

AttTransposition::AttTransposition() : Att()
{
    ResetTransposition();
}

void AttTransposition::ResetTransposition()
{
    m_transDiat = MEI_UNSET;
    m_transSemi = MEI_UNSET;
}

bool AttTransposition::ReadTransposition(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("trans.diat")) {
        this->SetTransDiat(StrToInt(element.attribute("trans.diat").value()));
        if (removeAttr) element.remove_attribute("trans.diat");
        hasAttribute = true;
    }
    if (element.attribute("trans.semi")) {
        this->SetTransSemi(StrToInt(element.attribute("trans.semi").value()));
        if (removeAttr) element.remove_attribute("trans.semi");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttTransposition::WriteTransposition(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasTransDiat()) {
        element.append_attribute("trans.diat") = IntToStr(this->GetTransDiat()).c_str();
        wroteAttribute = true;
    }
    if (this->HasTransSemi()) {
        element.append_attribute("trans.semi") = IntToStr(this->GetTransSemi()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttTransposition::HasTransDiat() const
{
    return (m_transDiat != MEI_UNSET);
}

bool AttTransposition::HasTransSemi() const
{
    return (m_transSemi != MEI_UNSET);
}

//----------------------------------------------------------------------------
// AttTuning
//----------------------------------------------------------------------------

AttTuning::AttTuning() : Att()
{
    ResetTuning();
}

void AttTuning::ResetTuning()
{
    m_tuneHz = 0.0;
    m_tunePname = PITCHNAME_NONE;
    m_tuneTemper = TEMPERAMENT_NONE;
}

bool AttTuning::ReadTuning(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("tune.Hz")) {
        this->SetTuneHz(StrToDbl(element.attribute("tune.Hz").value()));
        if (removeAttr) element.remove_attribute("tune.Hz");
        hasAttribute = true;
    }
    if (element.attribute("tune.pname")) {
        this->SetTunePname(StrToPitchname(element.attribute("tune.pname").value()));
        if (removeAttr) element.remove_attribute("tune.pname");
        hasAttribute = true;
    }
    if (element.attribute("tune.temper")) {
        this->SetTuneTemper(StrToTemperament(element.attribute("tune.temper").value()));
        if (removeAttr) element.remove_attribute("tune.temper");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttTuning::WriteTuning(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasTuneHz()) {
        element.append_attribute("tune.Hz") = DblToStr(this->GetTuneHz()).c_str();
        wroteAttribute = true;
    }
    if (this->HasTunePname()) {
        element.append_attribute("tune.pname") = PitchnameToStr(this->GetTunePname()).c_str();
        wroteAttribute = true;
    }
    if (this->HasTuneTemper()) {
        element.append_attribute("tune.temper") = TemperamentToStr(this->GetTuneTemper()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttTuning::HasTuneHz() const
{
    return (m_tuneHz != 0.0);
}

bool AttTuning::HasTunePname() const
{
    return (m_tunePname != PITCHNAME_NONE);
}

bool AttTuning::HasTuneTemper() const
{
    return (m_tuneTemper != TEMPERAMENT_NONE);
}

//----------------------------------------------------------------------------
// AttTyped
//----------------------------------------------------------------------------

AttTyped::AttTyped() : Att()
{
    ResetTyped();
}

void AttTyped::ResetTyped()
{
    m_type = "";
}

bool AttTyped::ReadTyped(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("type")) {
        this->SetType(StrToStr(element.attribute("type").value()));
        if (removeAttr) element.remove_attribute("type");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttTyped::WriteTyped(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasType()) {
        element.append_attribute("type") = StrToStr(this->GetType()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttTyped::HasType() const
{
    return (m_type != "");
}

//----------------------------------------------------------------------------
// AttTypography
//----------------------------------------------------------------------------

AttTypography::AttTypography() : Att()
{
    ResetTypography();
}

void AttTypography::ResetTypography()
{
    m_fontfam = "";
    m_fontname = "";
    m_fontsize = data_FONTSIZE();
    m_fontstyle = FONTSTYLE_NONE;
    m_fontweight = FONTWEIGHT_NONE;
    m_letterspacing = 0.0;
    m_lineheight = "";
}

bool AttTypography::ReadTypography(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("fontfam")) {
        this->SetFontfam(StrToStr(element.attribute("fontfam").value()));
        if (removeAttr) element.remove_attribute("fontfam");
        hasAttribute = true;
    }
    if (element.attribute("fontname")) {
        this->SetFontname(StrToStr(element.attribute("fontname").value()));
        if (removeAttr) element.remove_attribute("fontname");
        hasAttribute = true;
    }
    if (element.attribute("fontsize")) {
        this->SetFontsize(StrToFontsize(element.attribute("fontsize").value()));
        if (removeAttr) element.remove_attribute("fontsize");
        hasAttribute = true;
    }
    if (element.attribute("fontstyle")) {
        this->SetFontstyle(StrToFontstyle(element.attribute("fontstyle").value()));
        if (removeAttr) element.remove_attribute("fontstyle");
        hasAttribute = true;
    }
    if (element.attribute("fontweight")) {
        this->SetFontweight(StrToFontweight(element.attribute("fontweight").value()));
        if (removeAttr) element.remove_attribute("fontweight");
        hasAttribute = true;
    }
    if (element.attribute("letterspacing")) {
        this->SetLetterspacing(StrToDbl(element.attribute("letterspacing").value()));
        if (removeAttr) element.remove_attribute("letterspacing");
        hasAttribute = true;
    }
    if (element.attribute("lineheight")) {
        this->SetLineheight(StrToStr(element.attribute("lineheight").value()));
        if (removeAttr) element.remove_attribute("lineheight");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttTypography::WriteTypography(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasFontfam()) {
        element.append_attribute("fontfam") = StrToStr(this->GetFontfam()).c_str();
        wroteAttribute = true;
    }
    if (this->HasFontname()) {
        element.append_attribute("fontname") = StrToStr(this->GetFontname()).c_str();
        wroteAttribute = true;
    }
    if (this->HasFontsize()) {
        element.append_attribute("fontsize") = FontsizeToStr(this->GetFontsize()).c_str();
        wroteAttribute = true;
    }
    if (this->HasFontstyle()) {
        element.append_attribute("fontstyle") = FontstyleToStr(this->GetFontstyle()).c_str();
        wroteAttribute = true;
    }
    if (this->HasFontweight()) {
        element.append_attribute("fontweight") = FontweightToStr(this->GetFontweight()).c_str();
        wroteAttribute = true;
    }
    if (this->HasLetterspacing()) {
        element.append_attribute("letterspacing") = DblToStr(this->GetLetterspacing()).c_str();
        wroteAttribute = true;
    }
    if (this->HasLineheight()) {
        element.append_attribute("lineheight") = StrToStr(this->GetLineheight()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttTypography::HasFontfam() const
{
    return (m_fontfam != "");
}

bool AttTypography::HasFontname() const
{
    return (m_fontname != "");
}

bool AttTypography::HasFontsize() const
{
    return (m_fontsize.HasValue());
}

bool AttTypography::HasFontstyle() const
{
    return (m_fontstyle != FONTSTYLE_NONE);
}

bool AttTypography::HasFontweight() const
{
    return (m_fontweight != FONTWEIGHT_NONE);
}

bool AttTypography::HasLetterspacing() const
{
    return (m_letterspacing != 0.0);
}

bool AttTypography::HasLineheight() const
{
    return (m_lineheight != "");
}

//----------------------------------------------------------------------------
// AttVerticalAlign
//----------------------------------------------------------------------------

AttVerticalAlign::AttVerticalAlign() : Att()
{
    ResetVerticalAlign();
}

void AttVerticalAlign::ResetVerticalAlign()
{
    m_valign = VERTICALALIGNMENT_NONE;
}

bool AttVerticalAlign::ReadVerticalAlign(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("valign")) {
        this->SetValign(StrToVerticalalignment(element.attribute("valign").value()));
        if (removeAttr) element.remove_attribute("valign");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttVerticalAlign::WriteVerticalAlign(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasValign()) {
        element.append_attribute("valign") = VerticalalignmentToStr(this->GetValign()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttVerticalAlign::HasValign() const
{
    return (m_valign != VERTICALALIGNMENT_NONE);
}

//----------------------------------------------------------------------------
// AttVisualOffsetHo
//----------------------------------------------------------------------------

AttVisualOffsetHo::AttVisualOffsetHo() : Att()
{
    ResetVisualOffsetHo();
}

void AttVisualOffsetHo::ResetVisualOffsetHo()
{
    m_ho = data_MEASUREMENTSIGNED();
}

bool AttVisualOffsetHo::ReadVisualOffsetHo(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("ho")) {
        this->SetHo(StrToMeasurementsigned(element.attribute("ho").value()));
        if (removeAttr) element.remove_attribute("ho");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttVisualOffsetHo::WriteVisualOffsetHo(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasHo()) {
        element.append_attribute("ho") = MeasurementsignedToStr(this->GetHo()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttVisualOffsetHo::HasHo() const
{
    return (m_ho != data_MEASUREMENTSIGNED());
}

//----------------------------------------------------------------------------
// AttVisualOffsetVo
//----------------------------------------------------------------------------

AttVisualOffsetVo::AttVisualOffsetVo() : Att()
{
    ResetVisualOffsetVo();
}

void AttVisualOffsetVo::ResetVisualOffsetVo()
{
    m_vo = data_MEASUREMENTSIGNED();
}

bool AttVisualOffsetVo::ReadVisualOffsetVo(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("vo")) {
        this->SetVo(StrToMeasurementsigned(element.attribute("vo").value()));
        if (removeAttr) element.remove_attribute("vo");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttVisualOffsetVo::WriteVisualOffsetVo(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasVo()) {
        element.append_attribute("vo") = MeasurementsignedToStr(this->GetVo()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttVisualOffsetVo::HasVo() const
{
    return (m_vo != data_MEASUREMENTSIGNED());
}

//----------------------------------------------------------------------------
// AttVisualOffset2Ho
//----------------------------------------------------------------------------

AttVisualOffset2Ho::AttVisualOffset2Ho() : Att()
{
    ResetVisualOffset2Ho();
}

void AttVisualOffset2Ho::ResetVisualOffset2Ho()
{
    m_startho = data_MEASUREMENTSIGNED();
    m_endho = data_MEASUREMENTSIGNED();
}

bool AttVisualOffset2Ho::ReadVisualOffset2Ho(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("startho")) {
        this->SetStartho(StrToMeasurementsigned(element.attribute("startho").value()));
        if (removeAttr) element.remove_attribute("startho");
        hasAttribute = true;
    }
    if (element.attribute("endho")) {
        this->SetEndho(StrToMeasurementsigned(element.attribute("endho").value()));
        if (removeAttr) element.remove_attribute("endho");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttVisualOffset2Ho::WriteVisualOffset2Ho(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasStartho()) {
        element.append_attribute("startho") = MeasurementsignedToStr(this->GetStartho()).c_str();
        wroteAttribute = true;
    }
    if (this->HasEndho()) {
        element.append_attribute("endho") = MeasurementsignedToStr(this->GetEndho()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttVisualOffset2Ho::HasStartho() const
{
    return (m_startho != data_MEASUREMENTSIGNED());
}

bool AttVisualOffset2Ho::HasEndho() const
{
    return (m_endho != data_MEASUREMENTSIGNED());
}

//----------------------------------------------------------------------------
// AttVisualOffset2Vo
//----------------------------------------------------------------------------

AttVisualOffset2Vo::AttVisualOffset2Vo() : Att()
{
    ResetVisualOffset2Vo();
}

void AttVisualOffset2Vo::ResetVisualOffset2Vo()
{
    m_startvo = data_MEASUREMENTSIGNED();
    m_endvo = data_MEASUREMENTSIGNED();
}

bool AttVisualOffset2Vo::ReadVisualOffset2Vo(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("startvo")) {
        this->SetStartvo(StrToMeasurementsigned(element.attribute("startvo").value()));
        if (removeAttr) element.remove_attribute("startvo");
        hasAttribute = true;
    }
    if (element.attribute("endvo")) {
        this->SetEndvo(StrToMeasurementsigned(element.attribute("endvo").value()));
        if (removeAttr) element.remove_attribute("endvo");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttVisualOffset2Vo::WriteVisualOffset2Vo(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasStartvo()) {
        element.append_attribute("startvo") = MeasurementsignedToStr(this->GetStartvo()).c_str();
        wroteAttribute = true;
    }
    if (this->HasEndvo()) {
        element.append_attribute("endvo") = MeasurementsignedToStr(this->GetEndvo()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttVisualOffset2Vo::HasStartvo() const
{
    return (m_startvo != data_MEASUREMENTSIGNED());
}

bool AttVisualOffset2Vo::HasEndvo() const
{
    return (m_endvo != data_MEASUREMENTSIGNED());
}

//----------------------------------------------------------------------------
// AttVoltaGroupingSym
//----------------------------------------------------------------------------

AttVoltaGroupingSym::AttVoltaGroupingSym() : Att()
{
    ResetVoltaGroupingSym();
}

void AttVoltaGroupingSym::ResetVoltaGroupingSym()
{
    m_voltasym = voltaGroupingSym_VOLTASYM_NONE;
}

bool AttVoltaGroupingSym::ReadVoltaGroupingSym(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("voltasym")) {
        this->SetVoltasym(StrToVoltaGroupingSymVoltasym(element.attribute("voltasym").value()));
        if (removeAttr) element.remove_attribute("voltasym");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttVoltaGroupingSym::WriteVoltaGroupingSym(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasVoltasym()) {
        element.append_attribute("voltasym") = VoltaGroupingSymVoltasymToStr(this->GetVoltasym()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttVoltaGroupingSym::HasVoltasym() const
{
    return (m_voltasym != voltaGroupingSym_VOLTASYM_NONE);
}

//----------------------------------------------------------------------------
// AttWhitespace
//----------------------------------------------------------------------------

AttWhitespace::AttWhitespace() : Att()
{
    ResetWhitespace();
}

void AttWhitespace::ResetWhitespace()
{
    m_space = "";
}

bool AttWhitespace::ReadWhitespace(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("xml:space")) {
        this->SetSpace(StrToStr(element.attribute("xml:space").value()));
        if (removeAttr) element.remove_attribute("xml:space");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttWhitespace::WriteWhitespace(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasSpace()) {
        element.append_attribute("xml:space") = StrToStr(this->GetSpace()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttWhitespace::HasSpace() const
{
    return (m_space != "");
}

//----------------------------------------------------------------------------
// AttWidth
//----------------------------------------------------------------------------

AttWidth::AttWidth() : Att()
{
    ResetWidth();
}

void AttWidth::ResetWidth()
{
    m_width = data_MEASUREMENTUNSIGNED();
}

bool AttWidth::ReadWidth(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("width")) {
        this->SetWidth(StrToMeasurementunsigned(element.attribute("width").value()));
        if (removeAttr) element.remove_attribute("width");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttWidth::WriteWidth(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasWidth()) {
        element.append_attribute("width") = MeasurementunsignedToStr(this->GetWidth()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttWidth::HasWidth() const
{
    return (m_width != data_MEASUREMENTUNSIGNED());
}

} // namespace libmei
