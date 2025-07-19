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

#include "cmn.h"

//----------------------------------------------------------------------------

#include <cassert>

//----------------------------------------------------------------------------

namespace libmei {

Arpeg::Arpeg() :
    Element("arpeg"), AttLabelled(), AttTyped(), AttArpegLog(), AttLayerIdent(), AttPlist(), AttStaffIdent(), AttTimestampLog(), AttStartId(), AttArpegVis(), AttColor(), AttEnclosingChars(), AttExtSymAuth(), AttExtSymNames(), AttLineRendBase(), AttTypography(), AttVisualOffsetHo(), AttVisualOffsetVo()
{
}

Arpeg::~Arpeg() {}

bool Arpeg::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadArpegLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLayerIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadPlist(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStaffIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTimestampLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStartId(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadArpegVis(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadColor(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadEnclosingChars(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymAuth(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymNames(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLineRendBase(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTypography(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetHo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetVo(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Arpeg::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("arpeg");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteArpegLog(element) || hasAttribute);
    hasAttribute = (WriteLayerIdent(element) || hasAttribute);
    hasAttribute = (WritePlist(element) || hasAttribute);
    hasAttribute = (WriteStaffIdent(element) || hasAttribute);
    hasAttribute = (WriteTimestampLog(element) || hasAttribute);
    hasAttribute = (WriteStartId(element) || hasAttribute);
    hasAttribute = (WriteArpegVis(element) || hasAttribute);
    hasAttribute = (WriteColor(element) || hasAttribute);
    hasAttribute = (WriteEnclosingChars(element) || hasAttribute);
    hasAttribute = (WriteExtSymAuth(element) || hasAttribute);
    hasAttribute = (WriteExtSymNames(element) || hasAttribute);
    hasAttribute = (WriteLineRendBase(element) || hasAttribute);
    hasAttribute = (WriteTypography(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetHo(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetVo(element) || hasAttribute);
    return hasAttribute;
}

void Arpeg::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetArpegLog();
    ResetLayerIdent();
    ResetPlist();
    ResetStaffIdent();
    ResetTimestampLog();
    ResetStartId();
    ResetArpegVis();
    ResetColor();
    ResetEnclosingChars();
    ResetExtSymAuth();
    ResetExtSymNames();
    ResetLineRendBase();
    ResetTypography();
    ResetVisualOffsetHo();
    ResetVisualOffsetVo();
}

BTrem::BTrem() :
    Element("bTrem"), AttLabelled(), AttTyped(), AttLayerIdent(), AttStaffIdent(), AttAugmentDots(), AttDurationLog(), AttNumbered(), AttTremForm(), AttNumberPlacement(), AttTremMeasured()
{
}

BTrem::~BTrem() {}

bool BTrem::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLayerIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStaffIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadAugmentDots(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadDurationLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadNumbered(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTremForm(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadNumberPlacement(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTremMeasured(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool BTrem::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("bTrem");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteLayerIdent(element) || hasAttribute);
    hasAttribute = (WriteStaffIdent(element) || hasAttribute);
    hasAttribute = (WriteAugmentDots(element) || hasAttribute);
    hasAttribute = (WriteDurationLog(element) || hasAttribute);
    hasAttribute = (WriteNumbered(element) || hasAttribute);
    hasAttribute = (WriteTremForm(element) || hasAttribute);
    hasAttribute = (WriteNumberPlacement(element) || hasAttribute);
    hasAttribute = (WriteTremMeasured(element) || hasAttribute);
    return hasAttribute;
}

void BTrem::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetLayerIdent();
    ResetStaffIdent();
    ResetAugmentDots();
    ResetDurationLog();
    ResetNumbered();
    ResetTremForm();
    ResetNumberPlacement();
    ResetTremMeasured();
}

Beam::Beam() :
    Element("beam"), AttLabelled(), AttTyped(), AttLayerIdent(), AttStaffIdent(), AttColor(), AttCue()
{
}

Beam::~Beam() {}

bool Beam::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLayerIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStaffIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadColor(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadCue(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Beam::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("beam");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteLayerIdent(element) || hasAttribute);
    hasAttribute = (WriteStaffIdent(element) || hasAttribute);
    hasAttribute = (WriteColor(element) || hasAttribute);
    hasAttribute = (WriteCue(element) || hasAttribute);
    return hasAttribute;
}

void Beam::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetLayerIdent();
    ResetStaffIdent();
    ResetColor();
    ResetCue();
}

BeatRpt::BeatRpt() :
    Element("beatRpt"), AttLabelled(), AttTyped(), AttBeatRptLog(), AttLayerIdent(), AttStaffIdent(), AttBeatRptVis(), AttColor(), AttExpandable(), AttExtSymAuth(), AttExtSymNames(), AttPlist()
{
}

BeatRpt::~BeatRpt() {}

bool BeatRpt::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadBeatRptLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLayerIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStaffIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadBeatRptVis(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadColor(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExpandable(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymAuth(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymNames(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadPlist(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool BeatRpt::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("beatRpt");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteBeatRptLog(element) || hasAttribute);
    hasAttribute = (WriteLayerIdent(element) || hasAttribute);
    hasAttribute = (WriteStaffIdent(element) || hasAttribute);
    hasAttribute = (WriteBeatRptVis(element) || hasAttribute);
    hasAttribute = (WriteColor(element) || hasAttribute);
    hasAttribute = (WriteExpandable(element) || hasAttribute);
    hasAttribute = (WriteExtSymAuth(element) || hasAttribute);
    hasAttribute = (WriteExtSymNames(element) || hasAttribute);
    hasAttribute = (WritePlist(element) || hasAttribute);
    return hasAttribute;
}

void BeatRpt::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetBeatRptLog();
    ResetLayerIdent();
    ResetStaffIdent();
    ResetBeatRptVis();
    ResetColor();
    ResetExpandable();
    ResetExtSymAuth();
    ResetExtSymNames();
    ResetPlist();
}

Breath::Breath() :
    Element("breath"), AttLabelled(), AttTyped(), AttLayerIdent(), AttStaffIdent(), AttStartId(), AttTimestampLog(), AttColor(), AttExtSymAuth(), AttExtSymNames(), AttVisualOffsetHo(), AttVisualOffsetVo()
{
}

Breath::~Breath() {}

bool Breath::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLayerIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStaffIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStartId(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTimestampLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadColor(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymAuth(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymNames(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetHo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetVo(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Breath::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("breath");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteLayerIdent(element) || hasAttribute);
    hasAttribute = (WriteStaffIdent(element) || hasAttribute);
    hasAttribute = (WriteStartId(element) || hasAttribute);
    hasAttribute = (WriteTimestampLog(element) || hasAttribute);
    hasAttribute = (WriteColor(element) || hasAttribute);
    hasAttribute = (WriteExtSymAuth(element) || hasAttribute);
    hasAttribute = (WriteExtSymNames(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetHo(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetVo(element) || hasAttribute);
    return hasAttribute;
}

void Breath::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetLayerIdent();
    ResetStaffIdent();
    ResetStartId();
    ResetTimestampLog();
    ResetColor();
    ResetExtSymAuth();
    ResetExtSymNames();
    ResetVisualOffsetHo();
    ResetVisualOffsetVo();
}

FTrem::FTrem() :
    Element("fTrem"), AttLabelled(), AttTyped(), AttLayerIdent(), AttStaffIdent(), AttAugmentDots(), AttDurationLog(), AttTremForm(), AttTremMeasured()
{
}

FTrem::~FTrem() {}

bool FTrem::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLayerIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStaffIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadAugmentDots(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadDurationLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTremForm(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTremMeasured(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool FTrem::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("fTrem");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteLayerIdent(element) || hasAttribute);
    hasAttribute = (WriteStaffIdent(element) || hasAttribute);
    hasAttribute = (WriteAugmentDots(element) || hasAttribute);
    hasAttribute = (WriteDurationLog(element) || hasAttribute);
    hasAttribute = (WriteTremForm(element) || hasAttribute);
    hasAttribute = (WriteTremMeasured(element) || hasAttribute);
    return hasAttribute;
}

void FTrem::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetLayerIdent();
    ResetStaffIdent();
    ResetAugmentDots();
    ResetDurationLog();
    ResetTremForm();
    ResetTremMeasured();
}

Fermata::Fermata() :
    Element("fermata"), AttLabelled(), AttTyped(), AttLayerIdent(), AttPlist(), AttStaffIdent(), AttTimestampLog(), AttStartEndId(), AttStartId(), AttFermataVis(), AttColor(), AttExtSymAuth(), AttExtSymNames(), AttPlacementRelStaff(), AttVisualOffsetHo(), AttVisualOffsetVo()
{
}

Fermata::~Fermata() {}

bool Fermata::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLayerIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadPlist(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStaffIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTimestampLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStartEndId(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStartId(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadFermataVis(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadColor(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymAuth(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymNames(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadPlacementRelStaff(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetHo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetVo(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Fermata::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("fermata");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteLayerIdent(element) || hasAttribute);
    hasAttribute = (WritePlist(element) || hasAttribute);
    hasAttribute = (WriteStaffIdent(element) || hasAttribute);
    hasAttribute = (WriteTimestampLog(element) || hasAttribute);
    hasAttribute = (WriteStartEndId(element) || hasAttribute);
    hasAttribute = (WriteStartId(element) || hasAttribute);
    hasAttribute = (WriteFermataVis(element) || hasAttribute);
    hasAttribute = (WriteColor(element) || hasAttribute);
    hasAttribute = (WriteExtSymAuth(element) || hasAttribute);
    hasAttribute = (WriteExtSymNames(element) || hasAttribute);
    hasAttribute = (WritePlacementRelStaff(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetHo(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetVo(element) || hasAttribute);
    return hasAttribute;
}

void Fermata::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetLayerIdent();
    ResetPlist();
    ResetStaffIdent();
    ResetTimestampLog();
    ResetStartEndId();
    ResetStartId();
    ResetFermataVis();
    ResetColor();
    ResetExtSymAuth();
    ResetExtSymNames();
    ResetPlacementRelStaff();
    ResetVisualOffsetHo();
    ResetVisualOffsetVo();
}

Gliss::Gliss() :
    Element("gliss"), AttLabelled(), AttTyped(), AttLayerIdent(), AttPlist(), AttStaffIdent(), AttTimestampLog(), AttDurationAdditive(), AttStartEndId(), AttStartId(), AttTimestamp2Log(), AttColor(), AttExtSymAuth(), AttExtSymNames(), AttTypography(), AttVisualOffsetHo(), AttVisualOffsetVo(), AttVisualOffset2Ho(), AttVisualOffset2Vo(), AttLineRend(), AttLineRendBase()
{
}

Gliss::~Gliss() {}

bool Gliss::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLayerIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadPlist(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStaffIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTimestampLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadDurationAdditive(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStartEndId(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStartId(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTimestamp2Log(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadColor(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymAuth(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymNames(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTypography(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetHo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetVo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffset2Ho(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffset2Vo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLineRend(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLineRendBase(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Gliss::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("gliss");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteLayerIdent(element) || hasAttribute);
    hasAttribute = (WritePlist(element) || hasAttribute);
    hasAttribute = (WriteStaffIdent(element) || hasAttribute);
    hasAttribute = (WriteTimestampLog(element) || hasAttribute);
    hasAttribute = (WriteDurationAdditive(element) || hasAttribute);
    hasAttribute = (WriteStartEndId(element) || hasAttribute);
    hasAttribute = (WriteStartId(element) || hasAttribute);
    hasAttribute = (WriteTimestamp2Log(element) || hasAttribute);
    hasAttribute = (WriteColor(element) || hasAttribute);
    hasAttribute = (WriteExtSymAuth(element) || hasAttribute);
    hasAttribute = (WriteExtSymNames(element) || hasAttribute);
    hasAttribute = (WriteTypography(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetHo(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetVo(element) || hasAttribute);
    hasAttribute = (WriteVisualOffset2Ho(element) || hasAttribute);
    hasAttribute = (WriteVisualOffset2Vo(element) || hasAttribute);
    hasAttribute = (WriteLineRend(element) || hasAttribute);
    hasAttribute = (WriteLineRendBase(element) || hasAttribute);
    return hasAttribute;
}

void Gliss::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetLayerIdent();
    ResetPlist();
    ResetStaffIdent();
    ResetTimestampLog();
    ResetDurationAdditive();
    ResetStartEndId();
    ResetStartId();
    ResetTimestamp2Log();
    ResetColor();
    ResetExtSymAuth();
    ResetExtSymNames();
    ResetTypography();
    ResetVisualOffsetHo();
    ResetVisualOffsetVo();
    ResetVisualOffset2Ho();
    ResetVisualOffset2Vo();
    ResetLineRend();
    ResetLineRendBase();
}

GraceGrp::GraceGrp() :
    Element("graceGrp"), AttLabelled(), AttTyped(), AttGraceGrpLog(), AttLayerIdent(), AttStaffIdent(), AttGraced(), AttColor()
{
}

GraceGrp::~GraceGrp() {}

bool GraceGrp::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadGraceGrpLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLayerIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStaffIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadGraced(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadColor(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool GraceGrp::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("graceGrp");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteGraceGrpLog(element) || hasAttribute);
    hasAttribute = (WriteLayerIdent(element) || hasAttribute);
    hasAttribute = (WriteStaffIdent(element) || hasAttribute);
    hasAttribute = (WriteGraced(element) || hasAttribute);
    hasAttribute = (WriteColor(element) || hasAttribute);
    return hasAttribute;
}

void GraceGrp::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetGraceGrpLog();
    ResetLayerIdent();
    ResetStaffIdent();
    ResetGraced();
    ResetColor();
}

Hairpin::Hairpin() :
    Element("hairpin"), AttLabelled(), AttTyped(), AttHairpinLog(), AttLayerIdent(), AttPlist(), AttStaffIdent(), AttTimestampLog(), AttDurationAdditive(), AttStartEndId(), AttStartId(), AttTimestamp2Log(), AttHairpinVis(), AttColor(), AttLineRendBase(), AttPlacementRelStaff(), AttVisualOffsetHo(), AttVisualOffsetVo(), AttVisualOffset2Ho(), AttVisualOffset2Vo(), AttMidiValue(), AttMidiValue2()
{
}

Hairpin::~Hairpin() {}

bool Hairpin::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadHairpinLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLayerIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadPlist(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStaffIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTimestampLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadDurationAdditive(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStartEndId(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStartId(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTimestamp2Log(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadHairpinVis(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadColor(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLineRendBase(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadPlacementRelStaff(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetHo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetVo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffset2Ho(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffset2Vo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadMidiValue(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadMidiValue2(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Hairpin::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("hairpin");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteHairpinLog(element) || hasAttribute);
    hasAttribute = (WriteLayerIdent(element) || hasAttribute);
    hasAttribute = (WritePlist(element) || hasAttribute);
    hasAttribute = (WriteStaffIdent(element) || hasAttribute);
    hasAttribute = (WriteTimestampLog(element) || hasAttribute);
    hasAttribute = (WriteDurationAdditive(element) || hasAttribute);
    hasAttribute = (WriteStartEndId(element) || hasAttribute);
    hasAttribute = (WriteStartId(element) || hasAttribute);
    hasAttribute = (WriteTimestamp2Log(element) || hasAttribute);
    hasAttribute = (WriteHairpinVis(element) || hasAttribute);
    hasAttribute = (WriteColor(element) || hasAttribute);
    hasAttribute = (WriteLineRendBase(element) || hasAttribute);
    hasAttribute = (WritePlacementRelStaff(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetHo(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetVo(element) || hasAttribute);
    hasAttribute = (WriteVisualOffset2Ho(element) || hasAttribute);
    hasAttribute = (WriteVisualOffset2Vo(element) || hasAttribute);
    hasAttribute = (WriteMidiValue(element) || hasAttribute);
    hasAttribute = (WriteMidiValue2(element) || hasAttribute);
    return hasAttribute;
}

void Hairpin::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetHairpinLog();
    ResetLayerIdent();
    ResetPlist();
    ResetStaffIdent();
    ResetTimestampLog();
    ResetDurationAdditive();
    ResetStartEndId();
    ResetStartId();
    ResetTimestamp2Log();
    ResetHairpinVis();
    ResetColor();
    ResetLineRendBase();
    ResetPlacementRelStaff();
    ResetVisualOffsetHo();
    ResetVisualOffsetVo();
    ResetVisualOffset2Ho();
    ResetVisualOffset2Vo();
    ResetMidiValue();
    ResetMidiValue2();
}

HalfmRpt::HalfmRpt() :
    Element("halfmRpt"), AttLabelled(), AttTyped(), AttLayerIdent(), AttStaffIdent(), AttDurationAdditive(), AttColor(), AttExpandable(), AttExtSymAuth(), AttExtSymNames(), AttTypography(), AttVisualOffsetHo(), AttVisualOffsetVo()
{
}

HalfmRpt::~HalfmRpt() {}

bool HalfmRpt::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLayerIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStaffIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadDurationAdditive(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadColor(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExpandable(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymAuth(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymNames(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTypography(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetHo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetVo(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool HalfmRpt::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("halfmRpt");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteLayerIdent(element) || hasAttribute);
    hasAttribute = (WriteStaffIdent(element) || hasAttribute);
    hasAttribute = (WriteDurationAdditive(element) || hasAttribute);
    hasAttribute = (WriteColor(element) || hasAttribute);
    hasAttribute = (WriteExpandable(element) || hasAttribute);
    hasAttribute = (WriteExtSymAuth(element) || hasAttribute);
    hasAttribute = (WriteExtSymNames(element) || hasAttribute);
    hasAttribute = (WriteTypography(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetHo(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetVo(element) || hasAttribute);
    return hasAttribute;
}

void HalfmRpt::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetLayerIdent();
    ResetStaffIdent();
    ResetDurationAdditive();
    ResetColor();
    ResetExpandable();
    ResetExtSymAuth();
    ResetExtSymNames();
    ResetTypography();
    ResetVisualOffsetHo();
    ResetVisualOffsetVo();
}

HarpPedal::HarpPedal() :
    Element("harpPedal"), AttLabelled(), AttTyped(), AttHarpPedalLog(), AttLayerIdent(), AttPlist(), AttStaffIdent(), AttTimestampLog(), AttStartEndId(), AttStartId(), AttColor(), AttExtSymAuth(), AttExtSymNames(), AttPlacementRelStaff(), AttTypography(), AttVisualOffsetHo(), AttVisualOffsetVo()
{
}

HarpPedal::~HarpPedal() {}

bool HarpPedal::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadHarpPedalLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLayerIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadPlist(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStaffIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTimestampLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStartEndId(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStartId(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadColor(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymAuth(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymNames(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadPlacementRelStaff(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTypography(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetHo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetVo(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool HarpPedal::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("harpPedal");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteHarpPedalLog(element) || hasAttribute);
    hasAttribute = (WriteLayerIdent(element) || hasAttribute);
    hasAttribute = (WritePlist(element) || hasAttribute);
    hasAttribute = (WriteStaffIdent(element) || hasAttribute);
    hasAttribute = (WriteTimestampLog(element) || hasAttribute);
    hasAttribute = (WriteStartEndId(element) || hasAttribute);
    hasAttribute = (WriteStartId(element) || hasAttribute);
    hasAttribute = (WriteColor(element) || hasAttribute);
    hasAttribute = (WriteExtSymAuth(element) || hasAttribute);
    hasAttribute = (WriteExtSymNames(element) || hasAttribute);
    hasAttribute = (WritePlacementRelStaff(element) || hasAttribute);
    hasAttribute = (WriteTypography(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetHo(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetVo(element) || hasAttribute);
    return hasAttribute;
}

void HarpPedal::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetHarpPedalLog();
    ResetLayerIdent();
    ResetPlist();
    ResetStaffIdent();
    ResetTimestampLog();
    ResetStartEndId();
    ResetStartId();
    ResetColor();
    ResetExtSymAuth();
    ResetExtSymNames();
    ResetPlacementRelStaff();
    ResetTypography();
    ResetVisualOffsetHo();
    ResetVisualOffsetVo();
}

Lv::Lv() :
    Element("lv"), AttLabelled(), AttTyped(), AttLayerIdent(), AttPlist(), AttStaffIdent(), AttTimestampLog(), AttStartEndId(), AttStartId(), AttTimestamp2Log(), AttColor(), AttCurvature(), AttLineRendBase(), AttVisualOffsetHo(), AttVisualOffsetVo(), AttVisualOffset2Ho(), AttVisualOffset2Vo()
{
}

Lv::~Lv() {}

bool Lv::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLayerIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadPlist(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStaffIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTimestampLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStartEndId(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStartId(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTimestamp2Log(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadColor(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadCurvature(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLineRendBase(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetHo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetVo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffset2Ho(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffset2Vo(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Lv::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("lv");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteLayerIdent(element) || hasAttribute);
    hasAttribute = (WritePlist(element) || hasAttribute);
    hasAttribute = (WriteStaffIdent(element) || hasAttribute);
    hasAttribute = (WriteTimestampLog(element) || hasAttribute);
    hasAttribute = (WriteStartEndId(element) || hasAttribute);
    hasAttribute = (WriteStartId(element) || hasAttribute);
    hasAttribute = (WriteTimestamp2Log(element) || hasAttribute);
    hasAttribute = (WriteColor(element) || hasAttribute);
    hasAttribute = (WriteCurvature(element) || hasAttribute);
    hasAttribute = (WriteLineRendBase(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetHo(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetVo(element) || hasAttribute);
    hasAttribute = (WriteVisualOffset2Ho(element) || hasAttribute);
    hasAttribute = (WriteVisualOffset2Vo(element) || hasAttribute);
    return hasAttribute;
}

void Lv::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetLayerIdent();
    ResetPlist();
    ResetStaffIdent();
    ResetTimestampLog();
    ResetStartEndId();
    ResetStartId();
    ResetTimestamp2Log();
    ResetColor();
    ResetCurvature();
    ResetLineRendBase();
    ResetVisualOffsetHo();
    ResetVisualOffsetVo();
    ResetVisualOffset2Ho();
    ResetVisualOffset2Vo();
}

MNum::MNum() :
    Element("mNum"), AttLabelled(), AttTyped(), AttLang(), AttColor(), AttPlacementRelStaff(), AttTypography(), AttVisualOffsetHo(), AttVisualOffsetVo()
{
}

MNum::~MNum() {}

bool MNum::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLang(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadColor(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadPlacementRelStaff(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTypography(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetHo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetVo(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool MNum::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("mNum");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteLang(element) || hasAttribute);
    hasAttribute = (WriteColor(element) || hasAttribute);
    hasAttribute = (WritePlacementRelStaff(element) || hasAttribute);
    hasAttribute = (WriteTypography(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetHo(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetVo(element) || hasAttribute);
    return hasAttribute;
}

void MNum::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetLang();
    ResetColor();
    ResetPlacementRelStaff();
    ResetTypography();
    ResetVisualOffsetHo();
    ResetVisualOffsetVo();
}

MRest::MRest() :
    Element("mRest"), AttLabelled(), AttTyped(), AttCue(), AttDurationAdditive(), AttLayerIdent(), AttStaffIdent(), AttColor(), AttCutout(), AttExtSymAuth(), AttExtSymNames(), AttStaffLocPitched(), AttTypography(), AttVisualOffsetHo(), AttVisualOffsetVo()
{
}

MRest::~MRest() {}

bool MRest::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadCue(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadDurationAdditive(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLayerIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStaffIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadColor(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadCutout(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymAuth(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymNames(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStaffLocPitched(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTypography(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetHo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetVo(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool MRest::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("mRest");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteCue(element) || hasAttribute);
    hasAttribute = (WriteDurationAdditive(element) || hasAttribute);
    hasAttribute = (WriteLayerIdent(element) || hasAttribute);
    hasAttribute = (WriteStaffIdent(element) || hasAttribute);
    hasAttribute = (WriteColor(element) || hasAttribute);
    hasAttribute = (WriteCutout(element) || hasAttribute);
    hasAttribute = (WriteExtSymAuth(element) || hasAttribute);
    hasAttribute = (WriteExtSymNames(element) || hasAttribute);
    hasAttribute = (WriteStaffLocPitched(element) || hasAttribute);
    hasAttribute = (WriteTypography(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetHo(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetVo(element) || hasAttribute);
    return hasAttribute;
}

void MRest::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetCue();
    ResetDurationAdditive();
    ResetLayerIdent();
    ResetStaffIdent();
    ResetColor();
    ResetCutout();
    ResetExtSymAuth();
    ResetExtSymNames();
    ResetStaffLocPitched();
    ResetTypography();
    ResetVisualOffsetHo();
    ResetVisualOffsetVo();
}

MRpt::MRpt() :
    Element("mRpt"), AttLabelled(), AttTyped(), AttLayerIdent(), AttStaffIdent(), AttNumbered(), AttColor(), AttExpandable(), AttExtSymAuth(), AttExtSymNames(), AttNumberPlacement(), AttTypography()
{
}

MRpt::~MRpt() {}

bool MRpt::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLayerIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStaffIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadNumbered(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadColor(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExpandable(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymAuth(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymNames(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadNumberPlacement(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTypography(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool MRpt::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("mRpt");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteLayerIdent(element) || hasAttribute);
    hasAttribute = (WriteStaffIdent(element) || hasAttribute);
    hasAttribute = (WriteNumbered(element) || hasAttribute);
    hasAttribute = (WriteColor(element) || hasAttribute);
    hasAttribute = (WriteExpandable(element) || hasAttribute);
    hasAttribute = (WriteExtSymAuth(element) || hasAttribute);
    hasAttribute = (WriteExtSymNames(element) || hasAttribute);
    hasAttribute = (WriteNumberPlacement(element) || hasAttribute);
    hasAttribute = (WriteTypography(element) || hasAttribute);
    return hasAttribute;
}

void MRpt::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetLayerIdent();
    ResetStaffIdent();
    ResetNumbered();
    ResetColor();
    ResetExpandable();
    ResetExtSymAuth();
    ResetExtSymNames();
    ResetNumberPlacement();
    ResetTypography();
}

Measure::Measure() :
    Element("measure"), AttLabelled(), AttTyped(), AttMeasureLog(), AttMeterConformanceBar(), AttNNumberLike()
{
}

Measure::~Measure() {}

bool Measure::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadMeasureLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadMeterConformanceBar(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadNNumberLike(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Measure::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("measure");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteMeasureLog(element) || hasAttribute);
    hasAttribute = (WriteMeterConformanceBar(element) || hasAttribute);
    hasAttribute = (WriteNNumberLike(element) || hasAttribute);
    return hasAttribute;
}

void Measure::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetMeasureLog();
    ResetMeterConformanceBar();
    ResetNNumberLike();
}

MultiRest::MultiRest() :
    Element("multiRest"), AttLabelled(), AttTyped(), AttLayerIdent(), AttStaffIdent(), AttNumbered(), AttMultiRestVis(), AttColor(), AttExtSymAuth(), AttExtSymNames(), AttNumberPlacement(), AttStaffLocPitched(), AttTypography(), AttWidth()
{
}

MultiRest::~MultiRest() {}

bool MultiRest::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLayerIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStaffIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadNumbered(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadMultiRestVis(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadColor(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymAuth(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymNames(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadNumberPlacement(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStaffLocPitched(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTypography(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadWidth(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool MultiRest::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("multiRest");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteLayerIdent(element) || hasAttribute);
    hasAttribute = (WriteStaffIdent(element) || hasAttribute);
    hasAttribute = (WriteNumbered(element) || hasAttribute);
    hasAttribute = (WriteMultiRestVis(element) || hasAttribute);
    hasAttribute = (WriteColor(element) || hasAttribute);
    hasAttribute = (WriteExtSymAuth(element) || hasAttribute);
    hasAttribute = (WriteExtSymNames(element) || hasAttribute);
    hasAttribute = (WriteNumberPlacement(element) || hasAttribute);
    hasAttribute = (WriteStaffLocPitched(element) || hasAttribute);
    hasAttribute = (WriteTypography(element) || hasAttribute);
    hasAttribute = (WriteWidth(element) || hasAttribute);
    return hasAttribute;
}

void MultiRest::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetLayerIdent();
    ResetStaffIdent();
    ResetNumbered();
    ResetMultiRestVis();
    ResetColor();
    ResetExtSymAuth();
    ResetExtSymNames();
    ResetNumberPlacement();
    ResetStaffLocPitched();
    ResetTypography();
    ResetWidth();
}

MultiRpt::MultiRpt() :
    Element("multiRpt"), AttLabelled(), AttTyped(), AttLayerIdent(), AttStaffIdent(), AttNumbered(), AttColor(), AttExpandable(), AttExtSymAuth(), AttExtSymNames(), AttTypography()
{
}

MultiRpt::~MultiRpt() {}

bool MultiRpt::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLayerIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStaffIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadNumbered(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadColor(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExpandable(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymAuth(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymNames(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTypography(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool MultiRpt::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("multiRpt");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteLayerIdent(element) || hasAttribute);
    hasAttribute = (WriteStaffIdent(element) || hasAttribute);
    hasAttribute = (WriteNumbered(element) || hasAttribute);
    hasAttribute = (WriteColor(element) || hasAttribute);
    hasAttribute = (WriteExpandable(element) || hasAttribute);
    hasAttribute = (WriteExtSymAuth(element) || hasAttribute);
    hasAttribute = (WriteExtSymNames(element) || hasAttribute);
    hasAttribute = (WriteTypography(element) || hasAttribute);
    return hasAttribute;
}

void MultiRpt::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetLayerIdent();
    ResetStaffIdent();
    ResetNumbered();
    ResetColor();
    ResetExpandable();
    ResetExtSymAuth();
    ResetExtSymNames();
    ResetTypography();
}

Octave::Octave() :
    Element("octave"), AttLabelled(), AttTyped(), AttOctaveLog(), AttLayerIdent(), AttPlist(), AttStaffIdent(), AttTimestampLog(), AttDurationAdditive(), AttOctaveDisplacement(), AttStartEndId(), AttStartId(), AttTimestamp2Log(), AttColor(), AttExtender(), AttLineRend(), AttLineRendBase(), AttVisualOffsetHo(), AttVisualOffsetVo(), AttVisualOffset2Ho()
{
}

Octave::~Octave() {}

bool Octave::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadOctaveLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLayerIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadPlist(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStaffIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTimestampLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadDurationAdditive(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadOctaveDisplacement(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStartEndId(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStartId(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTimestamp2Log(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadColor(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtender(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLineRend(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLineRendBase(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetHo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetVo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffset2Ho(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Octave::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("octave");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteOctaveLog(element) || hasAttribute);
    hasAttribute = (WriteLayerIdent(element) || hasAttribute);
    hasAttribute = (WritePlist(element) || hasAttribute);
    hasAttribute = (WriteStaffIdent(element) || hasAttribute);
    hasAttribute = (WriteTimestampLog(element) || hasAttribute);
    hasAttribute = (WriteDurationAdditive(element) || hasAttribute);
    hasAttribute = (WriteOctaveDisplacement(element) || hasAttribute);
    hasAttribute = (WriteStartEndId(element) || hasAttribute);
    hasAttribute = (WriteStartId(element) || hasAttribute);
    hasAttribute = (WriteTimestamp2Log(element) || hasAttribute);
    hasAttribute = (WriteColor(element) || hasAttribute);
    hasAttribute = (WriteExtender(element) || hasAttribute);
    hasAttribute = (WriteLineRend(element) || hasAttribute);
    hasAttribute = (WriteLineRendBase(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetHo(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetVo(element) || hasAttribute);
    hasAttribute = (WriteVisualOffset2Ho(element) || hasAttribute);
    return hasAttribute;
}

void Octave::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetOctaveLog();
    ResetLayerIdent();
    ResetPlist();
    ResetStaffIdent();
    ResetTimestampLog();
    ResetDurationAdditive();
    ResetOctaveDisplacement();
    ResetStartEndId();
    ResetStartId();
    ResetTimestamp2Log();
    ResetColor();
    ResetExtender();
    ResetLineRend();
    ResetLineRendBase();
    ResetVisualOffsetHo();
    ResetVisualOffsetVo();
    ResetVisualOffset2Ho();
}

Pedal::Pedal() :
    Element("pedal"), AttLabelled(), AttTyped(), AttPedalLog(), AttLayerIdent(), AttPlist(), AttStaffIdent(), AttTimestampLog(), AttStartEndId(), AttStartId(), AttTimestamp2Log(), AttPedalVis(), AttColor(), AttExtSymAuth(), AttExtSymNames(), AttLineRend(), AttLineRendBase(), AttPlacementRelStaff(), AttTypography(), AttVisualOffsetHo(), AttVisualOffsetVo()
{
}

Pedal::~Pedal() {}

bool Pedal::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadPedalLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLayerIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadPlist(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStaffIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTimestampLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStartEndId(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStartId(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTimestamp2Log(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadPedalVis(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadColor(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymAuth(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymNames(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLineRend(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLineRendBase(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadPlacementRelStaff(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTypography(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetHo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetVo(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Pedal::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("pedal");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WritePedalLog(element) || hasAttribute);
    hasAttribute = (WriteLayerIdent(element) || hasAttribute);
    hasAttribute = (WritePlist(element) || hasAttribute);
    hasAttribute = (WriteStaffIdent(element) || hasAttribute);
    hasAttribute = (WriteTimestampLog(element) || hasAttribute);
    hasAttribute = (WriteStartEndId(element) || hasAttribute);
    hasAttribute = (WriteStartId(element) || hasAttribute);
    hasAttribute = (WriteTimestamp2Log(element) || hasAttribute);
    hasAttribute = (WritePedalVis(element) || hasAttribute);
    hasAttribute = (WriteColor(element) || hasAttribute);
    hasAttribute = (WriteExtSymAuth(element) || hasAttribute);
    hasAttribute = (WriteExtSymNames(element) || hasAttribute);
    hasAttribute = (WriteLineRend(element) || hasAttribute);
    hasAttribute = (WriteLineRendBase(element) || hasAttribute);
    hasAttribute = (WritePlacementRelStaff(element) || hasAttribute);
    hasAttribute = (WriteTypography(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetHo(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetVo(element) || hasAttribute);
    return hasAttribute;
}

void Pedal::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetPedalLog();
    ResetLayerIdent();
    ResetPlist();
    ResetStaffIdent();
    ResetTimestampLog();
    ResetStartEndId();
    ResetStartId();
    ResetTimestamp2Log();
    ResetPedalVis();
    ResetColor();
    ResetExtSymAuth();
    ResetExtSymNames();
    ResetLineRend();
    ResetLineRendBase();
    ResetPlacementRelStaff();
    ResetTypography();
    ResetVisualOffsetHo();
    ResetVisualOffsetVo();
}

Reh::Reh() :
    Element("reh"), AttLabelled(), AttTyped(), AttLang(), AttStaffIdent(), AttStartId(), AttTimestampLog(), AttColor(), AttPlacementRelStaff(), AttTypography(), AttVisualOffsetHo(), AttVisualOffsetVo()
{
}

Reh::~Reh() {}

bool Reh::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLang(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStaffIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStartId(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTimestampLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadColor(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadPlacementRelStaff(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTypography(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetHo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetVo(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Reh::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("reh");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteLang(element) || hasAttribute);
    hasAttribute = (WriteStaffIdent(element) || hasAttribute);
    hasAttribute = (WriteStartId(element) || hasAttribute);
    hasAttribute = (WriteTimestampLog(element) || hasAttribute);
    hasAttribute = (WriteColor(element) || hasAttribute);
    hasAttribute = (WritePlacementRelStaff(element) || hasAttribute);
    hasAttribute = (WriteTypography(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetHo(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetVo(element) || hasAttribute);
    return hasAttribute;
}

void Reh::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetLang();
    ResetStaffIdent();
    ResetStartId();
    ResetTimestampLog();
    ResetColor();
    ResetPlacementRelStaff();
    ResetTypography();
    ResetVisualOffsetHo();
    ResetVisualOffsetVo();
}

RepeatMark::RepeatMark() :
    Element("repeatMark"), AttLabelled(), AttTyped(), AttLang(), AttRepeatMarkLog(), AttLayerIdent(), AttPlist(), AttStaffIdent(), AttTimestampLog(), AttDurationAdditive(), AttStartEndId(), AttStartId(), AttTimestamp2Log(), AttColor(), AttExtender(), AttLineRend(), AttLineRendBase(), AttExtSymAuth(), AttExtSymNames(), AttPlacementRelStaff(), AttVisualOffsetHo(), AttVisualOffsetVo(), AttVisualOffset2Ho()
{
}

RepeatMark::~RepeatMark() {}

bool RepeatMark::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLang(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadRepeatMarkLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLayerIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadPlist(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStaffIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTimestampLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadDurationAdditive(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStartEndId(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStartId(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTimestamp2Log(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadColor(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtender(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLineRend(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLineRendBase(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymAuth(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymNames(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadPlacementRelStaff(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetHo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetVo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffset2Ho(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool RepeatMark::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("repeatMark");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteLang(element) || hasAttribute);
    hasAttribute = (WriteRepeatMarkLog(element) || hasAttribute);
    hasAttribute = (WriteLayerIdent(element) || hasAttribute);
    hasAttribute = (WritePlist(element) || hasAttribute);
    hasAttribute = (WriteStaffIdent(element) || hasAttribute);
    hasAttribute = (WriteTimestampLog(element) || hasAttribute);
    hasAttribute = (WriteDurationAdditive(element) || hasAttribute);
    hasAttribute = (WriteStartEndId(element) || hasAttribute);
    hasAttribute = (WriteStartId(element) || hasAttribute);
    hasAttribute = (WriteTimestamp2Log(element) || hasAttribute);
    hasAttribute = (WriteColor(element) || hasAttribute);
    hasAttribute = (WriteExtender(element) || hasAttribute);
    hasAttribute = (WriteLineRend(element) || hasAttribute);
    hasAttribute = (WriteLineRendBase(element) || hasAttribute);
    hasAttribute = (WriteExtSymAuth(element) || hasAttribute);
    hasAttribute = (WriteExtSymNames(element) || hasAttribute);
    hasAttribute = (WritePlacementRelStaff(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetHo(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetVo(element) || hasAttribute);
    hasAttribute = (WriteVisualOffset2Ho(element) || hasAttribute);
    return hasAttribute;
}

void RepeatMark::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetLang();
    ResetRepeatMarkLog();
    ResetLayerIdent();
    ResetPlist();
    ResetStaffIdent();
    ResetTimestampLog();
    ResetDurationAdditive();
    ResetStartEndId();
    ResetStartId();
    ResetTimestamp2Log();
    ResetColor();
    ResetExtender();
    ResetLineRend();
    ResetLineRendBase();
    ResetExtSymAuth();
    ResetExtSymNames();
    ResetPlacementRelStaff();
    ResetVisualOffsetHo();
    ResetVisualOffsetVo();
    ResetVisualOffset2Ho();
}

Slur::Slur() :
    Element("slur"), AttLabelled(), AttTyped(), AttLayerIdent(), AttPlist(), AttStaffIdent(), AttTimestampLog(), AttDurationAdditive(), AttStartEndId(), AttStartId(), AttTimestamp2Log(), AttColor(), AttCurvature(), AttLineRendBase(), AttVisualOffsetHo(), AttVisualOffsetVo(), AttVisualOffset2Ho(), AttVisualOffset2Vo()
{
}

Slur::~Slur() {}

bool Slur::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLayerIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadPlist(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStaffIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTimestampLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadDurationAdditive(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStartEndId(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStartId(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTimestamp2Log(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadColor(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadCurvature(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLineRendBase(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetHo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetVo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffset2Ho(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffset2Vo(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Slur::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("slur");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteLayerIdent(element) || hasAttribute);
    hasAttribute = (WritePlist(element) || hasAttribute);
    hasAttribute = (WriteStaffIdent(element) || hasAttribute);
    hasAttribute = (WriteTimestampLog(element) || hasAttribute);
    hasAttribute = (WriteDurationAdditive(element) || hasAttribute);
    hasAttribute = (WriteStartEndId(element) || hasAttribute);
    hasAttribute = (WriteStartId(element) || hasAttribute);
    hasAttribute = (WriteTimestamp2Log(element) || hasAttribute);
    hasAttribute = (WriteColor(element) || hasAttribute);
    hasAttribute = (WriteCurvature(element) || hasAttribute);
    hasAttribute = (WriteLineRendBase(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetHo(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetVo(element) || hasAttribute);
    hasAttribute = (WriteVisualOffset2Ho(element) || hasAttribute);
    hasAttribute = (WriteVisualOffset2Vo(element) || hasAttribute);
    return hasAttribute;
}

void Slur::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetLayerIdent();
    ResetPlist();
    ResetStaffIdent();
    ResetTimestampLog();
    ResetDurationAdditive();
    ResetStartEndId();
    ResetStartId();
    ResetTimestamp2Log();
    ResetColor();
    ResetCurvature();
    ResetLineRendBase();
    ResetVisualOffsetHo();
    ResetVisualOffsetVo();
    ResetVisualOffset2Ho();
    ResetVisualOffset2Vo();
}

Tie::Tie() :
    Element("tie"), AttLabelled(), AttTyped(), AttLayerIdent(), AttPlist(), AttStaffIdent(), AttTimestampLog(), AttStartEndId(), AttStartId(), AttTimestamp2Log(), AttColor(), AttCurvature(), AttLineRendBase(), AttVisualOffsetHo(), AttVisualOffsetVo(), AttVisualOffset2Ho(), AttVisualOffset2Vo()
{
}

Tie::~Tie() {}

bool Tie::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLayerIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadPlist(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStaffIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTimestampLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStartEndId(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStartId(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTimestamp2Log(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadColor(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadCurvature(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLineRendBase(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetHo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetVo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffset2Ho(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffset2Vo(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Tie::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("tie");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteLayerIdent(element) || hasAttribute);
    hasAttribute = (WritePlist(element) || hasAttribute);
    hasAttribute = (WriteStaffIdent(element) || hasAttribute);
    hasAttribute = (WriteTimestampLog(element) || hasAttribute);
    hasAttribute = (WriteStartEndId(element) || hasAttribute);
    hasAttribute = (WriteStartId(element) || hasAttribute);
    hasAttribute = (WriteTimestamp2Log(element) || hasAttribute);
    hasAttribute = (WriteColor(element) || hasAttribute);
    hasAttribute = (WriteCurvature(element) || hasAttribute);
    hasAttribute = (WriteLineRendBase(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetHo(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetVo(element) || hasAttribute);
    hasAttribute = (WriteVisualOffset2Ho(element) || hasAttribute);
    hasAttribute = (WriteVisualOffset2Vo(element) || hasAttribute);
    return hasAttribute;
}

void Tie::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetLayerIdent();
    ResetPlist();
    ResetStaffIdent();
    ResetTimestampLog();
    ResetStartEndId();
    ResetStartId();
    ResetTimestamp2Log();
    ResetColor();
    ResetCurvature();
    ResetLineRendBase();
    ResetVisualOffsetHo();
    ResetVisualOffsetVo();
    ResetVisualOffset2Ho();
    ResetVisualOffset2Vo();
}

Tuplet::Tuplet() :
    Element("tuplet"), AttLabelled(), AttTyped(), AttDurationAdditive(), AttDurationRatio(), AttLayerIdent(), AttStaffIdent(), AttStartEndId(), AttStartId(), AttTupletVis(), AttColor(), AttNumberPlacement()
{
}

Tuplet::~Tuplet() {}

bool Tuplet::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadDurationAdditive(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadDurationRatio(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLayerIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStaffIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStartEndId(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStartId(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTupletVis(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadColor(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadNumberPlacement(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Tuplet::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("tuplet");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteDurationAdditive(element) || hasAttribute);
    hasAttribute = (WriteDurationRatio(element) || hasAttribute);
    hasAttribute = (WriteLayerIdent(element) || hasAttribute);
    hasAttribute = (WriteStaffIdent(element) || hasAttribute);
    hasAttribute = (WriteStartEndId(element) || hasAttribute);
    hasAttribute = (WriteStartId(element) || hasAttribute);
    hasAttribute = (WriteTupletVis(element) || hasAttribute);
    hasAttribute = (WriteColor(element) || hasAttribute);
    hasAttribute = (WriteNumberPlacement(element) || hasAttribute);
    return hasAttribute;
}

void Tuplet::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetDurationAdditive();
    ResetDurationRatio();
    ResetLayerIdent();
    ResetStaffIdent();
    ResetStartEndId();
    ResetStartId();
    ResetTupletVis();
    ResetColor();
    ResetNumberPlacement();
}



} // namespace libmei
