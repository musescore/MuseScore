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

#include "atts_visual.h"

//----------------------------------------------------------------------------

#include <cassert>

//----------------------------------------------------------------------------

namespace libmei {

//----------------------------------------------------------------------------
// AttArpegVis
//----------------------------------------------------------------------------

AttArpegVis::AttArpegVis() : Att()
{
    ResetArpegVis();
}

void AttArpegVis::ResetArpegVis()
{
    m_arrow = BOOLEAN_NONE;
    m_arrowShape = LINESTARTENDSYMBOL_NONE;
    m_arrowSize = MEI_UNSET;
    m_arrowColor = "";
    m_arrowFillcolor = "";
}

bool AttArpegVis::ReadArpegVis(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("arrow")) {
        this->SetArrow(StrToBoolean(element.attribute("arrow").value()));
        if (removeAttr) element.remove_attribute("arrow");
        hasAttribute = true;
    }
    if (element.attribute("arrow.shape")) {
        this->SetArrowShape(StrToLinestartendsymbol(element.attribute("arrow.shape").value()));
        if (removeAttr) element.remove_attribute("arrow.shape");
        hasAttribute = true;
    }
    if (element.attribute("arrow.size")) {
        this->SetArrowSize(StrToInt(element.attribute("arrow.size").value()));
        if (removeAttr) element.remove_attribute("arrow.size");
        hasAttribute = true;
    }
    if (element.attribute("arrow.color")) {
        this->SetArrowColor(StrToStr(element.attribute("arrow.color").value()));
        if (removeAttr) element.remove_attribute("arrow.color");
        hasAttribute = true;
    }
    if (element.attribute("arrow.fillcolor")) {
        this->SetArrowFillcolor(StrToStr(element.attribute("arrow.fillcolor").value()));
        if (removeAttr) element.remove_attribute("arrow.fillcolor");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttArpegVis::WriteArpegVis(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasArrow()) {
        element.append_attribute("arrow") = BooleanToStr(this->GetArrow()).c_str();
        wroteAttribute = true;
    }
    if (this->HasArrowShape()) {
        element.append_attribute("arrow.shape") = LinestartendsymbolToStr(this->GetArrowShape()).c_str();
        wroteAttribute = true;
    }
    if (this->HasArrowSize()) {
        element.append_attribute("arrow.size") = IntToStr(this->GetArrowSize()).c_str();
        wroteAttribute = true;
    }
    if (this->HasArrowColor()) {
        element.append_attribute("arrow.color") = StrToStr(this->GetArrowColor()).c_str();
        wroteAttribute = true;
    }
    if (this->HasArrowFillcolor()) {
        element.append_attribute("arrow.fillcolor") = StrToStr(this->GetArrowFillcolor()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttArpegVis::HasArrow() const
{
    return (m_arrow != BOOLEAN_NONE);
}

bool AttArpegVis::HasArrowShape() const
{
    return (m_arrowShape != LINESTARTENDSYMBOL_NONE);
}

bool AttArpegVis::HasArrowSize() const
{
    return (m_arrowSize != MEI_UNSET);
}

bool AttArpegVis::HasArrowColor() const
{
    return (m_arrowColor != "");
}

bool AttArpegVis::HasArrowFillcolor() const
{
    return (m_arrowFillcolor != "");
}

//----------------------------------------------------------------------------
// AttBeatRptVis
//----------------------------------------------------------------------------

AttBeatRptVis::AttBeatRptVis() : Att()
{
    ResetBeatRptVis();
}

void AttBeatRptVis::ResetBeatRptVis()
{
    m_slash = BEATRPT_REND_NONE;
}

bool AttBeatRptVis::ReadBeatRptVis(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("slash")) {
        this->SetSlash(StrToBeatrptRend(element.attribute("slash").value()));
        if (removeAttr) element.remove_attribute("slash");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttBeatRptVis::WriteBeatRptVis(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasSlash()) {
        element.append_attribute("slash") = BeatrptRendToStr(this->GetSlash()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttBeatRptVis::HasSlash() const
{
    return (m_slash != BEATRPT_REND_NONE);
}

//----------------------------------------------------------------------------
// AttChordVis
//----------------------------------------------------------------------------

AttChordVis::AttChordVis() : Att()
{
    ResetChordVis();
}

void AttChordVis::ResetChordVis()
{
    m_cluster = CLUSTER_NONE;
}

bool AttChordVis::ReadChordVis(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("cluster")) {
        this->SetCluster(StrToCluster(element.attribute("cluster").value()));
        if (removeAttr) element.remove_attribute("cluster");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttChordVis::WriteChordVis(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasCluster()) {
        element.append_attribute("cluster") = ClusterToStr(this->GetCluster()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttChordVis::HasCluster() const
{
    return (m_cluster != CLUSTER_NONE);
}

//----------------------------------------------------------------------------
// AttFermataVis
//----------------------------------------------------------------------------

AttFermataVis::AttFermataVis() : Att()
{
    ResetFermataVis();
}

void AttFermataVis::ResetFermataVis()
{
    m_form = fermataVis_FORM_NONE;
    m_shape = fermataVis_SHAPE_NONE;
}

bool AttFermataVis::ReadFermataVis(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("form")) {
        this->SetForm(StrToFermataVisForm(element.attribute("form").value()));
        if (removeAttr) element.remove_attribute("form");
        hasAttribute = true;
    }
    if (element.attribute("shape")) {
        this->SetShape(StrToFermataVisShape(element.attribute("shape").value()));
        if (removeAttr) element.remove_attribute("shape");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttFermataVis::WriteFermataVis(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasForm()) {
        element.append_attribute("form") = FermataVisFormToStr(this->GetForm()).c_str();
        wroteAttribute = true;
    }
    if (this->HasShape()) {
        element.append_attribute("shape") = FermataVisShapeToStr(this->GetShape()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttFermataVis::HasForm() const
{
    return (m_form != fermataVis_FORM_NONE);
}

bool AttFermataVis::HasShape() const
{
    return (m_shape != fermataVis_SHAPE_NONE);
}

//----------------------------------------------------------------------------
// AttHairpinVis
//----------------------------------------------------------------------------

AttHairpinVis::AttHairpinVis() : Att()
{
    ResetHairpinVis();
}

void AttHairpinVis::ResetHairpinVis()
{
    m_opening = data_MEASUREMENTUNSIGNED();
    m_closed = BOOLEAN_NONE;
    m_openingVertical = BOOLEAN_NONE;
    m_angleOptimize = BOOLEAN_NONE;
}

bool AttHairpinVis::ReadHairpinVis(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("opening")) {
        this->SetOpening(StrToMeasurementunsigned(element.attribute("opening").value()));
        if (removeAttr) element.remove_attribute("opening");
        hasAttribute = true;
    }
    if (element.attribute("closed")) {
        this->SetClosed(StrToBoolean(element.attribute("closed").value()));
        if (removeAttr) element.remove_attribute("closed");
        hasAttribute = true;
    }
    if (element.attribute("opening.vertical")) {
        this->SetOpeningVertical(StrToBoolean(element.attribute("opening.vertical").value()));
        if (removeAttr) element.remove_attribute("opening.vertical");
        hasAttribute = true;
    }
    if (element.attribute("angle.optimize")) {
        this->SetAngleOptimize(StrToBoolean(element.attribute("angle.optimize").value()));
        if (removeAttr) element.remove_attribute("angle.optimize");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttHairpinVis::WriteHairpinVis(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasOpening()) {
        element.append_attribute("opening") = MeasurementunsignedToStr(this->GetOpening()).c_str();
        wroteAttribute = true;
    }
    if (this->HasClosed()) {
        element.append_attribute("closed") = BooleanToStr(this->GetClosed()).c_str();
        wroteAttribute = true;
    }
    if (this->HasOpeningVertical()) {
        element.append_attribute("opening.vertical") = BooleanToStr(this->GetOpeningVertical()).c_str();
        wroteAttribute = true;
    }
    if (this->HasAngleOptimize()) {
        element.append_attribute("angle.optimize") = BooleanToStr(this->GetAngleOptimize()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttHairpinVis::HasOpening() const
{
    return (m_opening != data_MEASUREMENTUNSIGNED());
}

bool AttHairpinVis::HasClosed() const
{
    return (m_closed != BOOLEAN_NONE);
}

bool AttHairpinVis::HasOpeningVertical() const
{
    return (m_openingVertical != BOOLEAN_NONE);
}

bool AttHairpinVis::HasAngleOptimize() const
{
    return (m_angleOptimize != BOOLEAN_NONE);
}

//----------------------------------------------------------------------------
// AttHarmVis
//----------------------------------------------------------------------------

AttHarmVis::AttHarmVis() : Att()
{
    ResetHarmVis();
}

void AttHarmVis::ResetHarmVis()
{
    m_rendgrid = harmVis_RENDGRID_NONE;
}

bool AttHarmVis::ReadHarmVis(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("rendgrid")) {
        this->SetRendgrid(StrToHarmVisRendgrid(element.attribute("rendgrid").value()));
        if (removeAttr) element.remove_attribute("rendgrid");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttHarmVis::WriteHarmVis(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasRendgrid()) {
        element.append_attribute("rendgrid") = HarmVisRendgridToStr(this->GetRendgrid()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttHarmVis::HasRendgrid() const
{
    return (m_rendgrid != harmVis_RENDGRID_NONE);
}

//----------------------------------------------------------------------------
// AttMultiRestVis
//----------------------------------------------------------------------------

AttMultiRestVis::AttMultiRestVis() : Att()
{
    ResetMultiRestVis();
}

void AttMultiRestVis::ResetMultiRestVis()
{
    m_block = BOOLEAN_NONE;
}

bool AttMultiRestVis::ReadMultiRestVis(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("block")) {
        this->SetBlock(StrToBoolean(element.attribute("block").value()));
        if (removeAttr) element.remove_attribute("block");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttMultiRestVis::WriteMultiRestVis(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasBlock()) {
        element.append_attribute("block") = BooleanToStr(this->GetBlock()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttMultiRestVis::HasBlock() const
{
    return (m_block != BOOLEAN_NONE);
}

//----------------------------------------------------------------------------
// AttPbVis
//----------------------------------------------------------------------------

AttPbVis::AttPbVis() : Att()
{
    ResetPbVis();
}

void AttPbVis::ResetPbVis()
{
    m_folium = pbVis_FOLIUM_NONE;
}

bool AttPbVis::ReadPbVis(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("folium")) {
        this->SetFolium(StrToPbVisFolium(element.attribute("folium").value()));
        if (removeAttr) element.remove_attribute("folium");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttPbVis::WritePbVis(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasFolium()) {
        element.append_attribute("folium") = PbVisFoliumToStr(this->GetFolium()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttPbVis::HasFolium() const
{
    return (m_folium != pbVis_FOLIUM_NONE);
}

//----------------------------------------------------------------------------
// AttPedalVis
//----------------------------------------------------------------------------

AttPedalVis::AttPedalVis() : Att()
{
    ResetPedalVis();
}

void AttPedalVis::ResetPedalVis()
{
    m_form = PEDALSTYLE_NONE;
}

bool AttPedalVis::ReadPedalVis(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("form")) {
        this->SetForm(StrToPedalstyle(element.attribute("form").value()));
        if (removeAttr) element.remove_attribute("form");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttPedalVis::WritePedalVis(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasForm()) {
        element.append_attribute("form") = PedalstyleToStr(this->GetForm()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttPedalVis::HasForm() const
{
    return (m_form != PEDALSTYLE_NONE);
}

//----------------------------------------------------------------------------
// AttSbVis
//----------------------------------------------------------------------------

AttSbVis::AttSbVis() : Att()
{
    ResetSbVis();
}

void AttSbVis::ResetSbVis()
{
    m_form = sbVis_FORM_NONE;
}

bool AttSbVis::ReadSbVis(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("form")) {
        this->SetForm(StrToSbVisForm(element.attribute("form").value()));
        if (removeAttr) element.remove_attribute("form");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttSbVis::WriteSbVis(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasForm()) {
        element.append_attribute("form") = SbVisFormToStr(this->GetForm()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttSbVis::HasForm() const
{
    return (m_form != sbVis_FORM_NONE);
}

//----------------------------------------------------------------------------
// AttSectionVis
//----------------------------------------------------------------------------

AttSectionVis::AttSectionVis() : Att()
{
    ResetSectionVis();
}

void AttSectionVis::ResetSectionVis()
{
    m_restart = BOOLEAN_NONE;
}

bool AttSectionVis::ReadSectionVis(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("restart")) {
        this->SetRestart(StrToBoolean(element.attribute("restart").value()));
        if (removeAttr) element.remove_attribute("restart");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttSectionVis::WriteSectionVis(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasRestart()) {
        element.append_attribute("restart") = BooleanToStr(this->GetRestart()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttSectionVis::HasRestart() const
{
    return (m_restart != BOOLEAN_NONE);
}

//----------------------------------------------------------------------------
// AttSpaceVis
//----------------------------------------------------------------------------

AttSpaceVis::AttSpaceVis() : Att()
{
    ResetSpaceVis();
}

void AttSpaceVis::ResetSpaceVis()
{
    m_compressable = BOOLEAN_NONE;
}

bool AttSpaceVis::ReadSpaceVis(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("compressable")) {
        this->SetCompressable(StrToBoolean(element.attribute("compressable").value()));
        if (removeAttr) element.remove_attribute("compressable");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttSpaceVis::WriteSpaceVis(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasCompressable()) {
        element.append_attribute("compressable") = BooleanToStr(this->GetCompressable()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttSpaceVis::HasCompressable() const
{
    return (m_compressable != BOOLEAN_NONE);
}

//----------------------------------------------------------------------------
// AttStaffDefVis
//----------------------------------------------------------------------------

AttStaffDefVis::AttStaffDefVis() : Att()
{
    ResetStaffDefVis();
}

void AttStaffDefVis::ResetStaffDefVis()
{
    m_linesColor = "";
    m_linesVisible = BOOLEAN_NONE;
}

bool AttStaffDefVis::ReadStaffDefVis(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("lines.color")) {
        this->SetLinesColor(StrToStr(element.attribute("lines.color").value()));
        if (removeAttr) element.remove_attribute("lines.color");
        hasAttribute = true;
    }
    if (element.attribute("lines.visible")) {
        this->SetLinesVisible(StrToBoolean(element.attribute("lines.visible").value()));
        if (removeAttr) element.remove_attribute("lines.visible");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttStaffDefVis::WriteStaffDefVis(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasLinesColor()) {
        element.append_attribute("lines.color") = StrToStr(this->GetLinesColor()).c_str();
        wroteAttribute = true;
    }
    if (this->HasLinesVisible()) {
        element.append_attribute("lines.visible") = BooleanToStr(this->GetLinesVisible()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttStaffDefVis::HasLinesColor() const
{
    return (m_linesColor != "");
}

bool AttStaffDefVis::HasLinesVisible() const
{
    return (m_linesVisible != BOOLEAN_NONE);
}

//----------------------------------------------------------------------------
// AttStaffGrpVis
//----------------------------------------------------------------------------

AttStaffGrpVis::AttStaffGrpVis() : Att()
{
    ResetStaffGrpVis();
}

void AttStaffGrpVis::ResetStaffGrpVis()
{
    m_barThru = BOOLEAN_NONE;
}

bool AttStaffGrpVis::ReadStaffGrpVis(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("bar.thru")) {
        this->SetBarThru(StrToBoolean(element.attribute("bar.thru").value()));
        if (removeAttr) element.remove_attribute("bar.thru");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttStaffGrpVis::WriteStaffGrpVis(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasBarThru()) {
        element.append_attribute("bar.thru") = BooleanToStr(this->GetBarThru()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttStaffGrpVis::HasBarThru() const
{
    return (m_barThru != BOOLEAN_NONE);
}

//----------------------------------------------------------------------------
// AttTupletVis
//----------------------------------------------------------------------------

AttTupletVis::AttTupletVis() : Att()
{
    ResetTupletVis();
}

void AttTupletVis::ResetTupletVis()
{
    m_bracketPlace = STAFFREL_basic_NONE;
    m_bracketVisible = BOOLEAN_NONE;
    m_durVisible = BOOLEAN_NONE;
    m_numFormat = tupletVis_NUMFORMAT_NONE;
}

bool AttTupletVis::ReadTupletVis(pugi::xml_node element, bool removeAttr)
{
    bool hasAttribute = false;
    if (element.attribute("bracket.place")) {
        this->SetBracketPlace(StrToStaffrelBasic(element.attribute("bracket.place").value()));
        if (removeAttr) element.remove_attribute("bracket.place");
        hasAttribute = true;
    }
    if (element.attribute("bracket.visible")) {
        this->SetBracketVisible(StrToBoolean(element.attribute("bracket.visible").value()));
        if (removeAttr) element.remove_attribute("bracket.visible");
        hasAttribute = true;
    }
    if (element.attribute("dur.visible")) {
        this->SetDurVisible(StrToBoolean(element.attribute("dur.visible").value()));
        if (removeAttr) element.remove_attribute("dur.visible");
        hasAttribute = true;
    }
    if (element.attribute("num.format")) {
        this->SetNumFormat(StrToTupletVisNumformat(element.attribute("num.format").value()));
        if (removeAttr) element.remove_attribute("num.format");
        hasAttribute = true;
    }
    return hasAttribute;
}

bool AttTupletVis::WriteTupletVis(pugi::xml_node element)
{
    bool wroteAttribute = false;
    if (this->HasBracketPlace()) {
        element.append_attribute("bracket.place") = StaffrelBasicToStr(this->GetBracketPlace()).c_str();
        wroteAttribute = true;
    }
    if (this->HasBracketVisible()) {
        element.append_attribute("bracket.visible") = BooleanToStr(this->GetBracketVisible()).c_str();
        wroteAttribute = true;
    }
    if (this->HasDurVisible()) {
        element.append_attribute("dur.visible") = BooleanToStr(this->GetDurVisible()).c_str();
        wroteAttribute = true;
    }
    if (this->HasNumFormat()) {
        element.append_attribute("num.format") = TupletVisNumformatToStr(this->GetNumFormat()).c_str();
        wroteAttribute = true;
    }
    return wroteAttribute;
}

bool AttTupletVis::HasBracketPlace() const
{
    return (m_bracketPlace != STAFFREL_basic_NONE);
}

bool AttTupletVis::HasBracketVisible() const
{
    return (m_bracketVisible != BOOLEAN_NONE);
}

bool AttTupletVis::HasDurVisible() const
{
    return (m_durVisible != BOOLEAN_NONE);
}

bool AttTupletVis::HasNumFormat() const
{
    return (m_numFormat != tupletVis_NUMFORMAT_NONE);
}

} // namespace libmei
