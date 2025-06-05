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

#ifndef __LIBMEI_FINGERING_H__
#define __LIBMEI_FINGERING_H__

#include "att.h"
#include "attdef.h"
#include "pugixml.hpp"

//----------------------------------------------------------------------------

#include "element.h"
#include "atts_fingering.h"
#include "atts_shared.h"


namespace libmei {

/** An individual finger in a fingering indication. **/
class Fing : public Element, public AttLabelled, public AttTyped, public AttLayerIdent, public AttPlist, public AttStaffIdent, public AttTimestampLog, public AttDurationAdditive, public AttStartEndId, public AttStartId, public AttTimestamp2Log, public AttColor, public AttExtender, public AttLineRend, public AttLineRendBase, public AttPlacementRelStaff, public AttVisualOffsetHo, public AttVisualOffsetVo {
    public:
        Fing();
        virtual ~Fing();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** A group of individual fingers in a fingering indication. **/
class FingGrp : public Element, public AttLabelled, public AttTyped, public AttFingGrpLog, public AttLayerIdent, public AttPlist, public AttStaffIdent, public AttTimestampLog, public AttDurationAdditive, public AttStartEndId, public AttStartId, public AttTimestamp2Log {
    public:
        FingGrp();
        virtual ~FingGrp();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

} // namespace libmei

#endif // __LIBMEI_FINGERING_H__
