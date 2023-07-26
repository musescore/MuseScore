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

#ifndef __LIBMEI_HEADER_H__
#define __LIBMEI_HEADER_H__

#include "att.h"
#include "attdef.h"
#include "pugixml.hpp"

//----------------------------------------------------------------------------

#include "element.h"
#include "atts_shared.h"


namespace libmei {

/**
 * Groups elements that describe the availability of and access to a bibliographic
 * item, including an MEI-encoded document.
 **/
class Availability : public Element, public AttLabelled, public AttTyped, public AttDataPointing {
    public:
        Availability();
        virtual ~Availability();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** (file description) – Contains a full bibliographic description of the MEI file. **/
class FileDesc : public Element, public AttLabelled, public AttTyped {
    public:
        FileDesc();
        virtual ~FileDesc();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/**
 * (MEI header) – Supplies the descriptive and declarative metadata prefixed to
 * every MEI-conformant text.
 **/
class MeiHead : public Element, public AttLabelled, public AttLang {
    public:
        MeiHead();
        virtual ~MeiHead();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/**
 * (publication statement) – Container for information regarding the publication or
 * distribution of a bibliographic item, including the publisher's name and
 * address, the date of publication, and other relevant details.
 **/
class PubStmt : public Element, public AttLabelled, public AttTyped {
    public:
        PubStmt();
        virtual ~PubStmt();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

/** Container for title and responsibility meta-data. **/
class TitleStmt : public Element, public AttLabelled, public AttTyped {
    public:
        TitleStmt();
        virtual ~TitleStmt();

public:
    bool Read(pugi::xml_node element, bool removeAttr = false);
    bool Write(pugi::xml_node element, const std::string &xmlId = "");
    void Reset();
};

} // namespace libmei

#endif // __LIBMEI_HEADER_H__
