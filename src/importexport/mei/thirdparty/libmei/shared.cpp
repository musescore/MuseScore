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

#include "shared.h"

//----------------------------------------------------------------------------

#include <cassert>

//----------------------------------------------------------------------------

namespace libmei {

Accid::Accid() :
    Element("accid"), AttLabelled(), AttTyped(), AttAccidentalGes(), AttAccidLog(), AttAccidental(), AttLayerIdent(), AttPlist(), AttStaffIdent(), AttTimestampLog(), AttColor(), AttEnclosingChars(), AttExtSymAuth(), AttExtSymNames(), AttVisualOffsetHo(), AttVisualOffsetVo()
{
}

Accid::~Accid() {}

bool Accid::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadAccidentalGes(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadAccidLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadAccidental(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLayerIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadPlist(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStaffIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTimestampLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadColor(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadEnclosingChars(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymAuth(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymNames(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetHo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetVo(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Accid::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("accid");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteAccidentalGes(element) || hasAttribute);
    hasAttribute = (WriteAccidLog(element) || hasAttribute);
    hasAttribute = (WriteAccidental(element) || hasAttribute);
    hasAttribute = (WriteLayerIdent(element) || hasAttribute);
    hasAttribute = (WritePlist(element) || hasAttribute);
    hasAttribute = (WriteStaffIdent(element) || hasAttribute);
    hasAttribute = (WriteTimestampLog(element) || hasAttribute);
    hasAttribute = (WriteColor(element) || hasAttribute);
    hasAttribute = (WriteEnclosingChars(element) || hasAttribute);
    hasAttribute = (WriteExtSymAuth(element) || hasAttribute);
    hasAttribute = (WriteExtSymNames(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetHo(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetVo(element) || hasAttribute);
    return hasAttribute;
}

void Accid::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetAccidentalGes();
    ResetAccidLog();
    ResetAccidental();
    ResetLayerIdent();
    ResetPlist();
    ResetStaffIdent();
    ResetTimestampLog();
    ResetColor();
    ResetEnclosingChars();
    ResetExtSymAuth();
    ResetExtSymNames();
    ResetVisualOffsetHo();
    ResetVisualOffsetVo();
}

Arranger::Arranger() :
    Element("arranger"), AttLabelled(), AttTyped(), AttLang()
{
}

Arranger::~Arranger() {}

bool Arranger::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLang(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Arranger::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("arranger");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteLang(element) || hasAttribute);
    return hasAttribute;
}

void Arranger::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetLang();
}

Artic::Artic() :
    Element("artic"), AttLabelled(), AttTyped(), AttArticulation(), AttLayerIdent(), AttPlist(), AttStaffIdent(), AttTimestampLog(), AttColor(), AttEnclosingChars(), AttExtSymAuth(), AttExtSymNames(), AttPlacementRelEvent(), AttVisualOffsetHo(), AttVisualOffsetVo()
{
}

Artic::~Artic() {}

bool Artic::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadArticulation(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLayerIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadPlist(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStaffIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTimestampLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadColor(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadEnclosingChars(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymAuth(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymNames(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadPlacementRelEvent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetHo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetVo(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Artic::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("artic");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteArticulation(element) || hasAttribute);
    hasAttribute = (WriteLayerIdent(element) || hasAttribute);
    hasAttribute = (WritePlist(element) || hasAttribute);
    hasAttribute = (WriteStaffIdent(element) || hasAttribute);
    hasAttribute = (WriteTimestampLog(element) || hasAttribute);
    hasAttribute = (WriteColor(element) || hasAttribute);
    hasAttribute = (WriteEnclosingChars(element) || hasAttribute);
    hasAttribute = (WriteExtSymAuth(element) || hasAttribute);
    hasAttribute = (WriteExtSymNames(element) || hasAttribute);
    hasAttribute = (WritePlacementRelEvent(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetHo(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetVo(element) || hasAttribute);
    return hasAttribute;
}

void Artic::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetArticulation();
    ResetLayerIdent();
    ResetPlist();
    ResetStaffIdent();
    ResetTimestampLog();
    ResetColor();
    ResetEnclosingChars();
    ResetExtSymAuth();
    ResetExtSymNames();
    ResetPlacementRelEvent();
    ResetVisualOffsetHo();
    ResetVisualOffsetVo();
}

Body::Body() :
    Element("body"), AttLabelled(), AttTyped()
{
}

Body::~Body() {}

bool Body::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Body::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("body");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    return hasAttribute;
}

void Body::Reset() 
{     
    ResetLabelled();
    ResetTyped();
}

Caesura::Caesura() :
    Element("caesura"), AttLabelled(), AttTyped(), AttLayerIdent(), AttStaffIdent(), AttStartId(), AttTimestampLog(), AttColor(), AttExtSymAuth(), AttExtSymNames(), AttVisualOffsetHo(), AttVisualOffsetVo()
{
}

Caesura::~Caesura() {}

bool Caesura::Read(pugi::xml_node element, bool removeAttr)
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

bool Caesura::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("caesura");
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

void Caesura::Reset() 
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

Chord::Chord() :
    Element("chord"), AttLabelled(), AttTyped(), AttInstrumentIdent(), AttAugmentDots(), AttGraced(), AttCue(), AttDurationLog(), AttLayerIdent(), AttStaffIdent(), AttChordVis(), AttColor(), AttEnclosingChars(), AttExtSymAuth(), AttExtSymNames(), AttStems(), AttVisualOffsetHo(), AttBeamSecondary()
{
}

Chord::~Chord() {}

bool Chord::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadInstrumentIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadAugmentDots(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadGraced(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadCue(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadDurationLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLayerIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStaffIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadChordVis(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadColor(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadEnclosingChars(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymAuth(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymNames(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStems(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetHo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadBeamSecondary(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Chord::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("chord");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteInstrumentIdent(element) || hasAttribute);
    hasAttribute = (WriteAugmentDots(element) || hasAttribute);
    hasAttribute = (WriteGraced(element) || hasAttribute);
    hasAttribute = (WriteCue(element) || hasAttribute);
    hasAttribute = (WriteDurationLog(element) || hasAttribute);
    hasAttribute = (WriteLayerIdent(element) || hasAttribute);
    hasAttribute = (WriteStaffIdent(element) || hasAttribute);
    hasAttribute = (WriteChordVis(element) || hasAttribute);
    hasAttribute = (WriteColor(element) || hasAttribute);
    hasAttribute = (WriteEnclosingChars(element) || hasAttribute);
    hasAttribute = (WriteExtSymAuth(element) || hasAttribute);
    hasAttribute = (WriteExtSymNames(element) || hasAttribute);
    hasAttribute = (WriteStems(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetHo(element) || hasAttribute);
    hasAttribute = (WriteBeamSecondary(element) || hasAttribute);
    return hasAttribute;
}

void Chord::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetInstrumentIdent();
    ResetAugmentDots();
    ResetGraced();
    ResetCue();
    ResetDurationLog();
    ResetLayerIdent();
    ResetStaffIdent();
    ResetChordVis();
    ResetColor();
    ResetEnclosingChars();
    ResetExtSymAuth();
    ResetExtSymNames();
    ResetStems();
    ResetVisualOffsetHo();
    ResetBeamSecondary();
}

Clef::Clef() :
    Element("clef"), AttLabelled(), AttTyped(), AttLayerIdent(), AttStaffIdent(), AttClefLog(), AttClefShape(), AttLineLoc(), AttOctave(), AttOctaveDisplacement(), AttColor(), AttEnclosingChars(), AttExtSymAuth(), AttExtSymNames(), AttTypography(), AttVisualOffsetHo(), AttVisualOffsetVo()
{
}

Clef::~Clef() {}

bool Clef::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLayerIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStaffIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadClefLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadClefShape(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLineLoc(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadOctave(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadOctaveDisplacement(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadColor(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadEnclosingChars(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymAuth(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymNames(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTypography(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetHo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetVo(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Clef::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("clef");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteLayerIdent(element) || hasAttribute);
    hasAttribute = (WriteStaffIdent(element) || hasAttribute);
    hasAttribute = (WriteClefLog(element) || hasAttribute);
    hasAttribute = (WriteClefShape(element) || hasAttribute);
    hasAttribute = (WriteLineLoc(element) || hasAttribute);
    hasAttribute = (WriteOctave(element) || hasAttribute);
    hasAttribute = (WriteOctaveDisplacement(element) || hasAttribute);
    hasAttribute = (WriteColor(element) || hasAttribute);
    hasAttribute = (WriteEnclosingChars(element) || hasAttribute);
    hasAttribute = (WriteExtSymAuth(element) || hasAttribute);
    hasAttribute = (WriteExtSymNames(element) || hasAttribute);
    hasAttribute = (WriteTypography(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetHo(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetVo(element) || hasAttribute);
    return hasAttribute;
}

void Clef::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetLayerIdent();
    ResetStaffIdent();
    ResetClefLog();
    ResetClefShape();
    ResetLineLoc();
    ResetOctave();
    ResetOctaveDisplacement();
    ResetColor();
    ResetEnclosingChars();
    ResetExtSymAuth();
    ResetExtSymNames();
    ResetTypography();
    ResetVisualOffsetHo();
    ResetVisualOffsetVo();
}

ClefGrp::ClefGrp() :
    Element("clefGrp"), AttLabelled(), AttTyped(), AttLayerIdent(), AttStaffIdent()
{
}

ClefGrp::~ClefGrp() {}

bool ClefGrp::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLayerIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStaffIdent(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool ClefGrp::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("clefGrp");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteLayerIdent(element) || hasAttribute);
    hasAttribute = (WriteStaffIdent(element) || hasAttribute);
    return hasAttribute;
}

void ClefGrp::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetLayerIdent();
    ResetStaffIdent();
}

Composer::Composer() :
    Element("composer"), AttLabelled(), AttTyped(), AttLang()
{
}

Composer::~Composer() {}

bool Composer::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLang(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Composer::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("composer");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteLang(element) || hasAttribute);
    return hasAttribute;
}

void Composer::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetLang();
}

Date::Date() :
    Element("date"), AttLabelled(), AttTyped(), AttCalendared(), AttDatable(), AttLang()
{
}

Date::~Date() {}

bool Date::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadCalendared(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadDatable(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLang(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Date::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("date");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteCalendared(element) || hasAttribute);
    hasAttribute = (WriteDatable(element) || hasAttribute);
    hasAttribute = (WriteLang(element) || hasAttribute);
    return hasAttribute;
}

void Date::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetCalendared();
    ResetDatable();
    ResetLang();
}

Dir::Dir() :
    Element("dir"), AttLabelled(), AttTyped(), AttLang(), AttLayerIdent(), AttPlist(), AttStaffIdent(), AttTimestampLog(), AttDurationAdditive(), AttStartEndId(), AttStartId(), AttTimestamp2Log(), AttColor(), AttExtender(), AttLineRend(), AttLineRendBase(), AttPlacementRelStaff(), AttVisualOffsetHo(), AttVisualOffsetVo(), AttVisualOffset2Ho()
{
}

Dir::~Dir() {}

bool Dir::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLang(element, removeAttr) || hasAttribute);
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
    hasAttribute = (ReadPlacementRelStaff(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetHo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetVo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffset2Ho(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Dir::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("dir");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteLang(element) || hasAttribute);
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
    hasAttribute = (WritePlacementRelStaff(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetHo(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetVo(element) || hasAttribute);
    hasAttribute = (WriteVisualOffset2Ho(element) || hasAttribute);
    return hasAttribute;
}

void Dir::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetLang();
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
    ResetPlacementRelStaff();
    ResetVisualOffsetHo();
    ResetVisualOffsetVo();
    ResetVisualOffset2Ho();
}

Dynam::Dynam() :
    Element("dynam"), AttLabelled(), AttTyped(), AttLang(), AttLayerIdent(), AttPlist(), AttStaffIdent(), AttTimestampLog(), AttDurationAdditive(), AttStartEndId(), AttStartId(), AttTimestamp2Log(), AttColor(), AttExtender(), AttLineRend(), AttLineRendBase(), AttPlacementRelStaff(), AttVisualOffsetHo(), AttVisualOffsetVo(), AttVisualOffset2Ho()
{
}

Dynam::~Dynam() {}

bool Dynam::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLang(element, removeAttr) || hasAttribute);
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
    hasAttribute = (ReadPlacementRelStaff(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetHo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetVo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffset2Ho(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Dynam::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("dynam");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteLang(element) || hasAttribute);
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
    hasAttribute = (WritePlacementRelStaff(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetHo(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetVo(element) || hasAttribute);
    hasAttribute = (WriteVisualOffset2Ho(element) || hasAttribute);
    return hasAttribute;
}

void Dynam::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetLang();
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
    ResetPlacementRelStaff();
    ResetVisualOffsetHo();
    ResetVisualOffsetVo();
    ResetVisualOffset2Ho();
}

Ending::Ending() :
    Element("ending"), AttLabelled(), AttTyped(), AttLineRend(), AttLineRendBase()
{
}

Ending::~Ending() {}

bool Ending::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLineRend(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLineRendBase(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Ending::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("ending");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteLineRend(element) || hasAttribute);
    hasAttribute = (WriteLineRendBase(element) || hasAttribute);
    return hasAttribute;
}

void Ending::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetLineRend();
    ResetLineRendBase();
}

Label::Label() :
    Element("label"), AttLabelled(), AttTyped(), AttLang()
{
}

Label::~Label() {}

bool Label::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLang(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Label::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("label");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteLang(element) || hasAttribute);
    return hasAttribute;
}

void Label::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetLang();
}

LabelAbbr::LabelAbbr() :
    Element("labelAbbr"), AttLabelled(), AttTyped(), AttLang()
{
}

LabelAbbr::~LabelAbbr() {}

bool LabelAbbr::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLang(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool LabelAbbr::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("labelAbbr");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteLang(element) || hasAttribute);
    return hasAttribute;
}

void LabelAbbr::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetLang();
}

Layer::Layer() :
    Element("layer"), AttLabelled(), AttNInteger(), AttTyped()
{
}

Layer::~Layer() {}

bool Layer::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadNInteger(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Layer::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("layer");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteNInteger(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    return hasAttribute;
}

void Layer::Reset() 
{     
    ResetLabelled();
    ResetNInteger();
    ResetTyped();
}

Lb::Lb() :
    Element("lb"), AttLabelled(), AttTyped()
{
}

Lb::~Lb() {}

bool Lb::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Lb::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("lb");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    return hasAttribute;
}

void Lb::Reset() 
{     
    ResetLabelled();
    ResetTyped();
}

Lyricist::Lyricist() :
    Element("lyricist"), AttLabelled(), AttTyped(), AttLang()
{
}

Lyricist::~Lyricist() {}

bool Lyricist::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLang(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Lyricist::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("lyricist");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteLang(element) || hasAttribute);
    return hasAttribute;
}

void Lyricist::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetLang();
}

Mdiv::Mdiv() :
    Element("mdiv"), AttLabelled(), AttTyped(), AttAttacking()
{
}

Mdiv::~Mdiv() {}

bool Mdiv::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadAttacking(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Mdiv::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("mdiv");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteAttacking(element) || hasAttribute);
    return hasAttribute;
}

void Mdiv::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetAttacking();
}

Mei::Mei() :
    Element("mei"), AttMeiVersion()
{
}

Mei::~Mei() {}

bool Mei::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadMeiVersion(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Mei::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("mei");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteMeiVersion(element) || hasAttribute);
    return hasAttribute;
}

void Mei::Reset() 
{     
    ResetMeiVersion();
}

Music::Music() :
    Element("music"), AttLabelled(), AttTyped()
{
}

Music::~Music() {}

bool Music::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Music::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("music");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    return hasAttribute;
}

void Music::Reset() 
{     
    ResetLabelled();
    ResetTyped();
}

Note::Note() :
    Element("note"), AttLabelled(), AttTyped(), AttNoteGes(), AttInstrumentIdent(), AttMidiVelocity(), AttStringtab(), AttAugmentDots(), AttCue(), AttDurationLog(), AttLayerIdent(), AttStaffIdent(), AttGraced(), AttPitch(), AttOctave(), AttColor(), AttEnclosingChars(), AttExtSymAuth(), AttExtSymNames(), AttStems(), AttTypography(), AttVisualOffsetHo(), AttBeamSecondary()
{
}

Note::~Note() {}

bool Note::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadNoteGes(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadInstrumentIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadMidiVelocity(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStringtab(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadAugmentDots(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadCue(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadDurationLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLayerIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStaffIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadGraced(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadPitch(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadOctave(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadColor(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadEnclosingChars(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymAuth(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymNames(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStems(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTypography(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetHo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadBeamSecondary(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Note::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("note");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteNoteGes(element) || hasAttribute);
    hasAttribute = (WriteInstrumentIdent(element) || hasAttribute);
    hasAttribute = (WriteMidiVelocity(element) || hasAttribute);
    hasAttribute = (WriteStringtab(element) || hasAttribute);
    hasAttribute = (WriteAugmentDots(element) || hasAttribute);
    hasAttribute = (WriteCue(element) || hasAttribute);
    hasAttribute = (WriteDurationLog(element) || hasAttribute);
    hasAttribute = (WriteLayerIdent(element) || hasAttribute);
    hasAttribute = (WriteStaffIdent(element) || hasAttribute);
    hasAttribute = (WriteGraced(element) || hasAttribute);
    hasAttribute = (WritePitch(element) || hasAttribute);
    hasAttribute = (WriteOctave(element) || hasAttribute);
    hasAttribute = (WriteColor(element) || hasAttribute);
    hasAttribute = (WriteEnclosingChars(element) || hasAttribute);
    hasAttribute = (WriteExtSymAuth(element) || hasAttribute);
    hasAttribute = (WriteExtSymNames(element) || hasAttribute);
    hasAttribute = (WriteStems(element) || hasAttribute);
    hasAttribute = (WriteTypography(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetHo(element) || hasAttribute);
    hasAttribute = (WriteBeamSecondary(element) || hasAttribute);
    return hasAttribute;
}

void Note::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetNoteGes();
    ResetInstrumentIdent();
    ResetMidiVelocity();
    ResetStringtab();
    ResetAugmentDots();
    ResetCue();
    ResetDurationLog();
    ResetLayerIdent();
    ResetStaffIdent();
    ResetGraced();
    ResetPitch();
    ResetOctave();
    ResetColor();
    ResetEnclosingChars();
    ResetExtSymAuth();
    ResetExtSymNames();
    ResetStems();
    ResetTypography();
    ResetVisualOffsetHo();
    ResetBeamSecondary();
}

Ornam::Ornam() :
    Element("ornam"), AttLabelled(), AttTyped(), AttLayerIdent(), AttPlist(), AttStaffIdent(), AttTimestampLog(), AttDurationAdditive(), AttOrnamentAccid(), AttOrnamentAccidGes(), AttStartEndId(), AttStartId(), AttTimestamp2Log(), AttColor(), AttEnclosingChars(), AttPlacementRelStaff(), AttVisualOffsetHo(), AttVisualOffsetVo(), AttVisualOffset2Ho()
{
}

Ornam::~Ornam() {}

bool Ornam::Read(pugi::xml_node element, bool removeAttr)
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
    hasAttribute = (ReadOrnamentAccid(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadOrnamentAccidGes(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStartEndId(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStartId(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTimestamp2Log(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadColor(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadEnclosingChars(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadPlacementRelStaff(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetHo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetVo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffset2Ho(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Ornam::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("ornam");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteLayerIdent(element) || hasAttribute);
    hasAttribute = (WritePlist(element) || hasAttribute);
    hasAttribute = (WriteStaffIdent(element) || hasAttribute);
    hasAttribute = (WriteTimestampLog(element) || hasAttribute);
    hasAttribute = (WriteDurationAdditive(element) || hasAttribute);
    hasAttribute = (WriteOrnamentAccid(element) || hasAttribute);
    hasAttribute = (WriteOrnamentAccidGes(element) || hasAttribute);
    hasAttribute = (WriteStartEndId(element) || hasAttribute);
    hasAttribute = (WriteStartId(element) || hasAttribute);
    hasAttribute = (WriteTimestamp2Log(element) || hasAttribute);
    hasAttribute = (WriteColor(element) || hasAttribute);
    hasAttribute = (WriteEnclosingChars(element) || hasAttribute);
    hasAttribute = (WritePlacementRelStaff(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetHo(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetVo(element) || hasAttribute);
    hasAttribute = (WriteVisualOffset2Ho(element) || hasAttribute);
    return hasAttribute;
}

void Ornam::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetLayerIdent();
    ResetPlist();
    ResetStaffIdent();
    ResetTimestampLog();
    ResetDurationAdditive();
    ResetOrnamentAccid();
    ResetOrnamentAccidGes();
    ResetStartEndId();
    ResetStartId();
    ResetTimestamp2Log();
    ResetColor();
    ResetEnclosingChars();
    ResetPlacementRelStaff();
    ResetVisualOffsetHo();
    ResetVisualOffsetVo();
    ResetVisualOffset2Ho();
}

Pb::Pb() :
    Element("pb"), AttLabelled(), AttTyped(), AttPbVis()
{
}

Pb::~Pb() {}

bool Pb::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadPbVis(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Pb::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("pb");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WritePbVis(element) || hasAttribute);
    return hasAttribute;
}

void Pb::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetPbVis();
}

PgFoot::PgFoot() :
    Element("pgFoot"), AttLabelled(), AttTyped(), AttFormework(), AttHorizontalAlign(), AttLang()
{
}

PgFoot::~PgFoot() {}

bool PgFoot::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadFormework(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadHorizontalAlign(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLang(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool PgFoot::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("pgFoot");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteFormework(element) || hasAttribute);
    hasAttribute = (WriteHorizontalAlign(element) || hasAttribute);
    hasAttribute = (WriteLang(element) || hasAttribute);
    return hasAttribute;
}

void PgFoot::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetFormework();
    ResetHorizontalAlign();
    ResetLang();
}

PgHead::PgHead() :
    Element("pgHead"), AttLabelled(), AttTyped(), AttFormework(), AttHorizontalAlign(), AttLang()
{
}

PgHead::~PgHead() {}

bool PgHead::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadFormework(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadHorizontalAlign(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLang(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool PgHead::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("pgHead");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteFormework(element) || hasAttribute);
    hasAttribute = (WriteHorizontalAlign(element) || hasAttribute);
    hasAttribute = (WriteLang(element) || hasAttribute);
    return hasAttribute;
}

void PgHead::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetFormework();
    ResetHorizontalAlign();
    ResetLang();
}

PubPlace::PubPlace() :
    Element("pubPlace"), AttLabelled(), AttTyped(), AttLang()
{
}

PubPlace::~PubPlace() {}

bool PubPlace::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLang(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool PubPlace::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("pubPlace");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteLang(element) || hasAttribute);
    return hasAttribute;
}

void PubPlace::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetLang();
}

Rend::Rend() :
    Element("rend"), AttColor(), AttLabelled(), AttTyped(), AttExtSymAuth(), AttHorizontalAlign(), AttLang(), AttTextRendition(), AttTypography(), AttVerticalAlign(), AttWhitespace()
{
}

Rend::~Rend() {}

bool Rend::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadColor(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymAuth(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadHorizontalAlign(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLang(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTextRendition(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTypography(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVerticalAlign(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadWhitespace(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Rend::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("rend");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteColor(element) || hasAttribute);
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteExtSymAuth(element) || hasAttribute);
    hasAttribute = (WriteHorizontalAlign(element) || hasAttribute);
    hasAttribute = (WriteLang(element) || hasAttribute);
    hasAttribute = (WriteTextRendition(element) || hasAttribute);
    hasAttribute = (WriteTypography(element) || hasAttribute);
    hasAttribute = (WriteVerticalAlign(element) || hasAttribute);
    hasAttribute = (WriteWhitespace(element) || hasAttribute);
    return hasAttribute;
}

void Rend::Reset() 
{     
    ResetColor();
    ResetLabelled();
    ResetTyped();
    ResetExtSymAuth();
    ResetHorizontalAlign();
    ResetLang();
    ResetTextRendition();
    ResetTypography();
    ResetVerticalAlign();
    ResetWhitespace();
}

RespStmt::RespStmt() :
    Element("respStmt"), AttLabelled(), AttTyped()
{
}

RespStmt::~RespStmt() {}

bool RespStmt::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool RespStmt::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("respStmt");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    return hasAttribute;
}

void RespStmt::Reset() 
{     
    ResetLabelled();
    ResetTyped();
}

Rest::Rest() :
    Element("rest"), AttLabelled(), AttTyped(), AttInstrumentIdent(), AttAugmentDots(), AttCue(), AttDurationLog(), AttLayerIdent(), AttStaffIdent(), AttColor(), AttEnclosingChars(), AttBeamSecondary(), AttStaffLocPitched(), AttVisualOffsetHo(), AttVisualOffsetVo()
{
}

Rest::~Rest() {}

bool Rest::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadInstrumentIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadAugmentDots(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadCue(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadDurationLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLayerIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStaffIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadColor(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadEnclosingChars(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadBeamSecondary(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStaffLocPitched(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetHo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetVo(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Rest::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("rest");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteInstrumentIdent(element) || hasAttribute);
    hasAttribute = (WriteAugmentDots(element) || hasAttribute);
    hasAttribute = (WriteCue(element) || hasAttribute);
    hasAttribute = (WriteDurationLog(element) || hasAttribute);
    hasAttribute = (WriteLayerIdent(element) || hasAttribute);
    hasAttribute = (WriteStaffIdent(element) || hasAttribute);
    hasAttribute = (WriteColor(element) || hasAttribute);
    hasAttribute = (WriteEnclosingChars(element) || hasAttribute);
    hasAttribute = (WriteBeamSecondary(element) || hasAttribute);
    hasAttribute = (WriteStaffLocPitched(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetHo(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetVo(element) || hasAttribute);
    return hasAttribute;
}

void Rest::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetInstrumentIdent();
    ResetAugmentDots();
    ResetCue();
    ResetDurationLog();
    ResetLayerIdent();
    ResetStaffIdent();
    ResetColor();
    ResetEnclosingChars();
    ResetBeamSecondary();
    ResetStaffLocPitched();
    ResetVisualOffsetHo();
    ResetVisualOffsetVo();
}

Sb::Sb() :
    Element("sb"), AttLabelled(), AttTyped(), AttSbVis(), AttExtSymAuth(), AttExtSymNames(), AttTypography()
{
}

Sb::~Sb() {}

bool Sb::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadSbVis(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymAuth(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymNames(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTypography(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Sb::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("sb");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteSbVis(element) || hasAttribute);
    hasAttribute = (WriteExtSymAuth(element) || hasAttribute);
    hasAttribute = (WriteExtSymNames(element) || hasAttribute);
    hasAttribute = (WriteTypography(element) || hasAttribute);
    return hasAttribute;
}

void Sb::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetSbVis();
    ResetExtSymAuth();
    ResetExtSymNames();
    ResetTypography();
}

Score::Score() :
    Element("score"), AttLabelled(), AttTyped()
{
}

Score::~Score() {}

bool Score::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Score::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("score");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    return hasAttribute;
}

void Score::Reset() 
{     
    ResetLabelled();
    ResetTyped();
}

ScoreDef::ScoreDef() :
    Element("scoreDef"), AttLabelled(), AttTyped(), AttKeySigDefaultLog(), AttMeterSigDefaultLog()
{
}

ScoreDef::~ScoreDef() {}

bool ScoreDef::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadKeySigDefaultLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadMeterSigDefaultLog(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool ScoreDef::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("scoreDef");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteKeySigDefaultLog(element) || hasAttribute);
    hasAttribute = (WriteMeterSigDefaultLog(element) || hasAttribute);
    return hasAttribute;
}

void ScoreDef::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetKeySigDefaultLog();
    ResetMeterSigDefaultLog();
}

Section::Section() :
    Element("section"), AttLabelled(), AttTyped(), AttAttacking(), AttSectionVis()
{
}

Section::~Section() {}

bool Section::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadAttacking(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadSectionVis(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Section::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("section");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteAttacking(element) || hasAttribute);
    hasAttribute = (WriteSectionVis(element) || hasAttribute);
    return hasAttribute;
}

void Section::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetAttacking();
    ResetSectionVis();
}

Space::Space() :
    Element("space"), AttLabelled(), AttTyped(), AttAugmentDots(), AttDurationLog(), AttLayerIdent(), AttStaffIdent(), AttSpaceVis(), AttCutout()
{
}

Space::~Space() {}

bool Space::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadAugmentDots(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadDurationLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLayerIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStaffIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadSpaceVis(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadCutout(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Space::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("space");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteAugmentDots(element) || hasAttribute);
    hasAttribute = (WriteDurationLog(element) || hasAttribute);
    hasAttribute = (WriteLayerIdent(element) || hasAttribute);
    hasAttribute = (WriteStaffIdent(element) || hasAttribute);
    hasAttribute = (WriteSpaceVis(element) || hasAttribute);
    hasAttribute = (WriteCutout(element) || hasAttribute);
    return hasAttribute;
}

void Space::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetAugmentDots();
    ResetDurationLog();
    ResetLayerIdent();
    ResetStaffIdent();
    ResetSpaceVis();
    ResetCutout();
}

Staff::Staff() :
    Element("staff"), AttLabelled(), AttNInteger(), AttTyped()
{
}

Staff::~Staff() {}

bool Staff::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadNInteger(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Staff::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("staff");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteNInteger(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    return hasAttribute;
}

void Staff::Reset() 
{     
    ResetLabelled();
    ResetNInteger();
    ResetTyped();
}

StaffDef::StaffDef() :
    Element("staffDef"), AttLabelled(), AttNInteger(), AttTyped(), AttInstrumentIdent(), AttStringtabTuning(), AttTimeBase(), AttTuning(), AttStaffDefLog(), AttCleffingLog(), AttKeySigDefaultLog(), AttMeterSigDefaultLog(), AttTransposition(), AttStaffDefVis(), AttScalable()
{
}

StaffDef::~StaffDef() {}

bool StaffDef::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadNInteger(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadInstrumentIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStringtabTuning(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTimeBase(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTuning(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStaffDefLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadCleffingLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadKeySigDefaultLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadMeterSigDefaultLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTransposition(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStaffDefVis(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadScalable(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool StaffDef::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("staffDef");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteNInteger(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteInstrumentIdent(element) || hasAttribute);
    hasAttribute = (WriteStringtabTuning(element) || hasAttribute);
    hasAttribute = (WriteTimeBase(element) || hasAttribute);
    hasAttribute = (WriteTuning(element) || hasAttribute);
    hasAttribute = (WriteStaffDefLog(element) || hasAttribute);
    hasAttribute = (WriteCleffingLog(element) || hasAttribute);
    hasAttribute = (WriteKeySigDefaultLog(element) || hasAttribute);
    hasAttribute = (WriteMeterSigDefaultLog(element) || hasAttribute);
    hasAttribute = (WriteTransposition(element) || hasAttribute);
    hasAttribute = (WriteStaffDefVis(element) || hasAttribute);
    hasAttribute = (WriteScalable(element) || hasAttribute);
    return hasAttribute;
}

void StaffDef::Reset() 
{     
    ResetLabelled();
    ResetNInteger();
    ResetTyped();
    ResetInstrumentIdent();
    ResetStringtabTuning();
    ResetTimeBase();
    ResetTuning();
    ResetStaffDefLog();
    ResetCleffingLog();
    ResetKeySigDefaultLog();
    ResetMeterSigDefaultLog();
    ResetTransposition();
    ResetStaffDefVis();
    ResetScalable();
}

StaffGrp::StaffGrp() :
    Element("staffGrp"), AttLabelled(), AttTyped(), AttStaffGrpVis(), AttStaffGroupingSym(), AttInstrumentIdent()
{
}

StaffGrp::~StaffGrp() {}

bool StaffGrp::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStaffGrpVis(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStaffGroupingSym(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadInstrumentIdent(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool StaffGrp::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("staffGrp");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteStaffGrpVis(element) || hasAttribute);
    hasAttribute = (WriteStaffGroupingSym(element) || hasAttribute);
    hasAttribute = (WriteInstrumentIdent(element) || hasAttribute);
    return hasAttribute;
}

void StaffGrp::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetStaffGrpVis();
    ResetStaffGroupingSym();
    ResetInstrumentIdent();
}

Syl::Syl() :
    Element("syl"), AttLabelled(), AttTyped(), AttLang(), AttSylLog(), AttPlacementRelStaff(), AttTypography(), AttVisualOffsetHo(), AttVisualOffsetVo(), AttHorizontalAlign()
{
}

Syl::~Syl() {}

bool Syl::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLang(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadSylLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadPlacementRelStaff(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTypography(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetHo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetVo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadHorizontalAlign(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Syl::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("syl");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteLang(element) || hasAttribute);
    hasAttribute = (WriteSylLog(element) || hasAttribute);
    hasAttribute = (WritePlacementRelStaff(element) || hasAttribute);
    hasAttribute = (WriteTypography(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetHo(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetVo(element) || hasAttribute);
    hasAttribute = (WriteHorizontalAlign(element) || hasAttribute);
    return hasAttribute;
}

void Syl::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetLang();
    ResetSylLog();
    ResetPlacementRelStaff();
    ResetTypography();
    ResetVisualOffsetHo();
    ResetVisualOffsetVo();
    ResetHorizontalAlign();
}

Symbol::Symbol() :
    Element("symbol"), AttLabelled(), AttTyped(), AttColor(), AttExtSymAuth(), AttExtSymNames(), AttScalable(), AttVisualOffsetHo(), AttVisualOffsetVo()
{
}

Symbol::~Symbol() {}

bool Symbol::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadColor(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymAuth(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtSymNames(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadScalable(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetHo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetVo(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Symbol::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("symbol");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteColor(element) || hasAttribute);
    hasAttribute = (WriteExtSymAuth(element) || hasAttribute);
    hasAttribute = (WriteExtSymNames(element) || hasAttribute);
    hasAttribute = (WriteScalable(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetHo(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetVo(element) || hasAttribute);
    return hasAttribute;
}

void Symbol::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetColor();
    ResetExtSymAuth();
    ResetExtSymNames();
    ResetScalable();
    ResetVisualOffsetHo();
    ResetVisualOffsetVo();
}

Tempo::Tempo() :
    Element("tempo"), AttLabelled(), AttTyped(), AttLang(), AttMidiTempo(), AttTempoLog(), AttLayerIdent(), AttPlist(), AttStaffIdent(), AttTimestampLog(), AttMmTempo(), AttStartEndId(), AttStartId(), AttTimestamp2Log(), AttColor(), AttExtender(), AttLineRend(), AttLineRendBase(), AttPlacementRelStaff(), AttVisualOffsetHo(), AttVisualOffsetVo(), AttVisualOffset2Ho()
{
}

Tempo::~Tempo() {}

bool Tempo::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTyped(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLang(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadMidiTempo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTempoLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLayerIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadPlist(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStaffIdent(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTimestampLog(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadMmTempo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStartEndId(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadStartId(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadTimestamp2Log(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadColor(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadExtender(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLineRend(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLineRendBase(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadPlacementRelStaff(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetHo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffsetVo(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadVisualOffset2Ho(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Tempo::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("tempo");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteTyped(element) || hasAttribute);
    hasAttribute = (WriteLang(element) || hasAttribute);
    hasAttribute = (WriteMidiTempo(element) || hasAttribute);
    hasAttribute = (WriteTempoLog(element) || hasAttribute);
    hasAttribute = (WriteLayerIdent(element) || hasAttribute);
    hasAttribute = (WritePlist(element) || hasAttribute);
    hasAttribute = (WriteStaffIdent(element) || hasAttribute);
    hasAttribute = (WriteTimestampLog(element) || hasAttribute);
    hasAttribute = (WriteMmTempo(element) || hasAttribute);
    hasAttribute = (WriteStartEndId(element) || hasAttribute);
    hasAttribute = (WriteStartId(element) || hasAttribute);
    hasAttribute = (WriteTimestamp2Log(element) || hasAttribute);
    hasAttribute = (WriteColor(element) || hasAttribute);
    hasAttribute = (WriteExtender(element) || hasAttribute);
    hasAttribute = (WriteLineRend(element) || hasAttribute);
    hasAttribute = (WriteLineRendBase(element) || hasAttribute);
    hasAttribute = (WritePlacementRelStaff(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetHo(element) || hasAttribute);
    hasAttribute = (WriteVisualOffsetVo(element) || hasAttribute);
    hasAttribute = (WriteVisualOffset2Ho(element) || hasAttribute);
    return hasAttribute;
}

void Tempo::Reset() 
{     
    ResetLabelled();
    ResetTyped();
    ResetLang();
    ResetMidiTempo();
    ResetTempoLog();
    ResetLayerIdent();
    ResetPlist();
    ResetStaffIdent();
    ResetTimestampLog();
    ResetMmTempo();
    ResetStartEndId();
    ResetStartId();
    ResetTimestamp2Log();
    ResetColor();
    ResetExtender();
    ResetLineRend();
    ResetLineRendBase();
    ResetPlacementRelStaff();
    ResetVisualOffsetHo();
    ResetVisualOffsetVo();
    ResetVisualOffset2Ho();
}

Title::Title() :
    Element("title"), AttLabelled(), AttLang(), AttNNumberLike()
{
}

Title::~Title() {}

bool Title::Read(pugi::xml_node element, bool removeAttr)
{
    if (element.attribute("xml:id")) m_xmlId = element.attribute("xml:id").value();
    bool hasAttribute = false;
    hasAttribute = (ReadLabelled(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadLang(element, removeAttr) || hasAttribute);
    hasAttribute = (ReadNNumberLike(element, removeAttr) || hasAttribute);
    return hasAttribute;
}

bool Title::Write(pugi::xml_node element, const std::string &xmlId)
{
    element.set_name("title");
    if (xmlId.size() > 0) element.append_attribute("xml:id") = xmlId.c_str();
    bool hasAttribute = false;
    hasAttribute = (WriteLabelled(element) || hasAttribute);
    hasAttribute = (WriteLang(element) || hasAttribute);
    hasAttribute = (WriteNNumberLike(element) || hasAttribute);
    return hasAttribute;
}

void Title::Reset() 
{     
    ResetLabelled();
    ResetLang();
    ResetNNumberLike();
}



} // namespace libmei
