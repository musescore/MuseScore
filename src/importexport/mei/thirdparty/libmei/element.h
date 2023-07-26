/////////////////////////////////////////////////////////////////////////////
// Name:        element.h
// Author:      Laurent Pugin
// Created:     30/01/2023
// Copyright (c) Authors and others. All rights reserved.
/////////////////////////////////////////////////////////////////////////////

#ifndef __LIBMEI_ELEMENT_H__
#define __LIBMEI_ELEMENT_H__

#include <string>

namespace libmei {

//----------------------------------------------------------------------------
// Element
//----------------------------------------------------------------------------

/**
 * This class models the MEI element.
 */
class Element {
public:
    Element(std::string name) { m_name = name; }
    virtual ~Element() {}

    std::string m_name;
    std::string m_xmlId;
};

} // namespace libmei

#endif // __LIBMEI_ELEMENT_H__
