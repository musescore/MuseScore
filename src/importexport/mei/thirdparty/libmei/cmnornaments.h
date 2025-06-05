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

#ifndef __LIBMEI_CMNORNAMENTS_H__
#define __LIBMEI_CMNORNAMENTS_H__

#include "att.h"
#include "attdef.h"
#include "pugixml.hpp"

//----------------------------------------------------------------------------

#include "element.h"
#include "atts_cmnornaments.h"
#include "atts_externalsymbols.h"
#include "atts_gestural.h"
#include "atts_shared.h"


namespace libmei {

/**
 * An ornament indicating rapid alternation of the main note with a secondary note,
 * usually a step below, but sometimes a step above.
 **/
class Mordent : public Element, public AttLabelled, public AttTyped, public AttMordentLog, public AttLayerIdent, public AttPlist, public AttStaffIdent, public AttTimestampLog, public AttStartEndId, public AttStartId, public AttOrnamentAccid, public AttOrnamentAccidGes, public AttColor, public AttEnclosingChars, public AttExtSymAuth, public AttExtSymNames, public AttPlacementRelStaff, public AttTypography, public AttVisualOffsetHo, public AttVisualOffsetVo {
    public:
        Mordent();
        virtual ~Mordent();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/**
 * Rapid alternation of a note with another (usually at the interval of a second
 * above).
 **/
class Trill : public Element, public AttLabelled, public AttTyped, public AttLayerIdent, public AttPlist, public AttStaffIdent, public AttTimestampLog, public AttDurationAdditive, public AttOrnamentAccid, public AttOrnamentAccidGes, public AttStartEndId, public AttStartId, public AttTimestamp2Log, public AttColor, public AttEnclosingChars, public AttExtender, public AttLineRend, public AttLineRendBase, public AttExtSymAuth, public AttExtSymNames, public AttPlacementRelStaff, public AttTypography, public AttVisualOffsetHo, public AttVisualOffsetVo, public AttVisualOffset2Ho {
    public:
        Trill();
        virtual ~Trill();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/**
 * An ornament consisting of four notes — the upper neighbor of the written note,
 * the written note, the lower neighbor, and the written note.
 **/
class Turn : public Element, public AttLabelled, public AttTyped, public AttTurnLog, public AttLayerIdent, public AttPlist, public AttStaffIdent, public AttTimestampLog, public AttOrnamentAccid, public AttOrnamentAccidGes, public AttStartId, public AttColor, public AttEnclosingChars, public AttExtSymAuth, public AttExtSymNames, public AttPlacementRelStaff, public AttTypography, public AttVisualOffsetHo, public AttVisualOffsetVo {
    public:
        Turn();
        virtual ~Turn();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

} // namespace libmei

#endif // __LIBMEI_CMNORNAMENTS_H__
