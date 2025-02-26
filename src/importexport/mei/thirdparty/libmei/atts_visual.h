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

#ifndef __LIBMEI_ATTS_VISUAL_H__
#define __LIBMEI_ATTS_VISUAL_H__

#include "att.h"
#include "attdef.h"
#include "pugixml.hpp"

//----------------------------------------------------------------------------

#include <string>

namespace libmei {

//----------------------------------------------------------------------------
// AttArpegVis
//----------------------------------------------------------------------------

class AttArpegVis : public Att {
protected:
    AttArpegVis();
    ~AttArpegVis() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetArpegVis();

    /** Read the values for the attribute class **/
    bool ReadArpegVis(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteArpegVis(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetArrow(data_BOOLEAN arrow_) { m_arrow = arrow_; }
    data_BOOLEAN GetArrow() const { return m_arrow; }
    bool HasArrow() const;
    //
    void SetArrowShape(data_LINESTARTENDSYMBOL arrowShape_) { m_arrowShape = arrowShape_; }
    data_LINESTARTENDSYMBOL GetArrowShape() const { return m_arrowShape; }
    bool HasArrowShape() const;
    //
    void SetArrowSize(int arrowSize_) { m_arrowSize = arrowSize_; }
    int GetArrowSize() const { return m_arrowSize; }
    bool HasArrowSize() const;
    //
    void SetArrowColor(std::string arrowColor_) { m_arrowColor = arrowColor_; }
    std::string GetArrowColor() const { return m_arrowColor; }
    bool HasArrowColor() const;
    //
    void SetArrowFillcolor(std::string arrowFillcolor_) { m_arrowFillcolor = arrowFillcolor_; }
    std::string GetArrowFillcolor() const { return m_arrowFillcolor; }
    bool HasArrowFillcolor() const;
    ///@}

private:
    /** Indicates if an arrowhead is to be drawn as part of the arpeggiation symbol. **/
    data_BOOLEAN m_arrow;
    /** Symbol rendered at end of the line. **/
    data_LINESTARTENDSYMBOL m_arrowShape;
    /** Holds the relative size of the arrow symbol. **/
    int m_arrowSize;
    /** Captures the overall color of the arrow. **/
    std::string m_arrowColor;
    /** Captures the fill color of the arrow if different from the line color. **/
    std::string m_arrowFillcolor;
};

//----------------------------------------------------------------------------
// InstArpegVis
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttArpegVis
 */

class InstArpegVis : public AttArpegVis {
public:
    InstArpegVis() = default;
    virtual ~InstArpegVis() = default;
};

//----------------------------------------------------------------------------
// AttBeatRptVis
//----------------------------------------------------------------------------

class AttBeatRptVis : public Att {
protected:
    AttBeatRptVis();
    ~AttBeatRptVis() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetBeatRptVis();

    /** Read the values for the attribute class **/
    bool ReadBeatRptVis(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteBeatRptVis(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetSlash(data_BEATRPT_REND slash_) { m_slash = slash_; }
    data_BEATRPT_REND GetSlash() const { return m_slash; }
    bool HasSlash() const;
    ///@}

private:
    /**
     * Indicates the number of slashes required to render the appropriate beat repeat
     * symbol.
     * When a single beat consisting of a single note or chord is repeated, the
     * repetition symbol is a single thick, slanting slash; therefore, the value 1
     * should be used. When the beat is divided into even notes, the following values
     * should be used: 4ths or 8ths=1, 16ths=2, 32nds=3, 64ths=4, 128ths=5. When the
     * beat is comprised of mixed duration values, the default rendition is 2 slashes
     * and 2 dots.
     **/
    data_BEATRPT_REND m_slash;
};

//----------------------------------------------------------------------------
// InstBeatRptVis
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttBeatRptVis
 */

class InstBeatRptVis : public AttBeatRptVis {
public:
    InstBeatRptVis() = default;
    virtual ~InstBeatRptVis() = default;
};

//----------------------------------------------------------------------------
// AttChordVis
//----------------------------------------------------------------------------

class AttChordVis : public Att {
protected:
    AttChordVis();
    ~AttChordVis() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetChordVis();

    /** Read the values for the attribute class **/
    bool ReadChordVis(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteChordVis(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetCluster(data_CLUSTER cluster_) { m_cluster = cluster_; }
    data_CLUSTER GetCluster() const { return m_cluster; }
    bool HasCluster() const;
    ///@}

private:
    /**
     * Indicates a single, alternative note head should be displayed instead of
     * individual note heads.
     * The highest and lowest notes of the chord usually indicate the upper and lower
     * boundaries of the cluster note head.
     **/
    data_CLUSTER m_cluster;
};

//----------------------------------------------------------------------------
// InstChordVis
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttChordVis
 */

class InstChordVis : public AttChordVis {
public:
    InstChordVis() = default;
    virtual ~InstChordVis() = default;
};

//----------------------------------------------------------------------------
// AttFermataVis
//----------------------------------------------------------------------------

class AttFermataVis : public Att {
protected:
    AttFermataVis();
    ~AttFermataVis() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetFermataVis();

    /** Read the values for the attribute class **/
    bool ReadFermataVis(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteFermataVis(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetForm(fermataVis_FORM form_) { m_form = form_; }
    fermataVis_FORM GetForm() const { return m_form; }
    bool HasForm() const;
    //
    void SetShape(fermataVis_SHAPE shape_) { m_shape = shape_; }
    fermataVis_SHAPE GetShape() const { return m_shape; }
    bool HasShape() const;
    ///@}

private:
    /**
     * Captures the visual rendition and function of the hairpin; that is, whether it
     * indicates an increase or a decrease in volume.
     **/
    fermataVis_FORM m_form;
    /** Describes a clef’s shape. **/
    fermataVis_SHAPE m_shape;
};

//----------------------------------------------------------------------------
// InstFermataVis
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttFermataVis
 */

class InstFermataVis : public AttFermataVis {
public:
    InstFermataVis() = default;
    virtual ~InstFermataVis() = default;
};

//----------------------------------------------------------------------------
// AttHairpinVis
//----------------------------------------------------------------------------

class AttHairpinVis : public Att {
protected:
    AttHairpinVis();
    ~AttHairpinVis() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetHairpinVis();

    /** Read the values for the attribute class **/
    bool ReadHairpinVis(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteHairpinVis(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetOpening(data_MEASUREMENTUNSIGNED opening_) { m_opening = opening_; }
    data_MEASUREMENTUNSIGNED GetOpening() const { return m_opening; }
    bool HasOpening() const;
    //
    void SetClosed(data_BOOLEAN closed_) { m_closed = closed_; }
    data_BOOLEAN GetClosed() const { return m_closed; }
    bool HasClosed() const;
    //
    void SetOpeningVertical(data_BOOLEAN openingVertical_) { m_openingVertical = openingVertical_; }
    data_BOOLEAN GetOpeningVertical() const { return m_openingVertical; }
    bool HasOpeningVertical() const;
    //
    void SetAngleOptimize(data_BOOLEAN angleOptimize_) { m_angleOptimize = angleOptimize_; }
    data_BOOLEAN GetAngleOptimize() const { return m_angleOptimize; }
    bool HasAngleOptimize() const;
    ///@}

private:
    /**
     * Specifies the distance between the lines at the open end of a hairpin dynamic
     * mark.
     **/
    data_MEASUREMENTUNSIGNED m_opening;
    /**
     * Applies to a "Rossini" hairpin, i.e., one where the normally open side is closed
     * by a connecting line.
     **/
    data_BOOLEAN m_closed;
    /**
     * Indicates that the opening points are aligned with an imaginary line that is
     * always 90° perpendicular to the horizontal plane, regardless of any angle or
     * start/end adjustments, including when the hairpin is angled with @angle.optimize
     * or through @endvo/@startvo adjustments.
     **/
    data_BOOLEAN m_openingVertical;
    /**
     * Indicates that the slope of the hairpin can be adjusted to follow the content in
     * order to optimize spacing.
     **/
    data_BOOLEAN m_angleOptimize;
};

//----------------------------------------------------------------------------
// InstHairpinVis
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttHairpinVis
 */

class InstHairpinVis : public AttHairpinVis {
public:
    InstHairpinVis() = default;
    virtual ~InstHairpinVis() = default;
};

//----------------------------------------------------------------------------
// AttHarmVis
//----------------------------------------------------------------------------

class AttHarmVis : public Att {
protected:
    AttHarmVis();
    ~AttHarmVis() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetHarmVis();

    /** Read the values for the attribute class **/
    bool ReadHarmVis(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteHarmVis(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetRendgrid(harmVis_RENDGRID rendgrid_) { m_rendgrid = rendgrid_; }
    harmVis_RENDGRID GetRendgrid() const { return m_rendgrid; }
    bool HasRendgrid() const;
    ///@}

private:
    /** Describes how the harmonic indication should be rendered. **/
    harmVis_RENDGRID m_rendgrid;
};

//----------------------------------------------------------------------------
// InstHarmVis
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttHarmVis
 */

class InstHarmVis : public AttHarmVis {
public:
    InstHarmVis() = default;
    virtual ~InstHarmVis() = default;
};

//----------------------------------------------------------------------------
// AttMultiRestVis
//----------------------------------------------------------------------------

class AttMultiRestVis : public Att {
protected:
    AttMultiRestVis();
    ~AttMultiRestVis() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetMultiRestVis();

    /** Read the values for the attribute class **/
    bool ReadMultiRestVis(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteMultiRestVis(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetBlock(data_BOOLEAN block_) { m_block = block_; }
    data_BOOLEAN GetBlock() const { return m_block; }
    bool HasBlock() const;
    ///@}

private:
    /**
     * The block attribute controls whether the multimeasure rest should be rendered as
     * a block rest or as church rests ("Kirchenpausen"), that are combinations of
     * longa, breve and semibreve rests.
     **/
    data_BOOLEAN m_block;
};

//----------------------------------------------------------------------------
// InstMultiRestVis
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttMultiRestVis
 */

class InstMultiRestVis : public AttMultiRestVis {
public:
    InstMultiRestVis() = default;
    virtual ~InstMultiRestVis() = default;
};

//----------------------------------------------------------------------------
// AttPbVis
//----------------------------------------------------------------------------

class AttPbVis : public Att {
protected:
    AttPbVis();
    ~AttPbVis() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetPbVis();

    /** Read the values for the attribute class **/
    bool ReadPbVis(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WritePbVis(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetFolium(pbVis_FOLIUM folium_) { m_folium = folium_; }
    pbVis_FOLIUM GetFolium() const { return m_folium; }
    bool HasFolium() const;
    ///@}

private:
    /**
     * States the side of a leaf (as in a manuscript) on which the content following
     * the pb element occurs.
     **/
    pbVis_FOLIUM m_folium;
};

//----------------------------------------------------------------------------
// InstPbVis
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttPbVis
 */

class InstPbVis : public AttPbVis {
public:
    InstPbVis() = default;
    virtual ~InstPbVis() = default;
};

//----------------------------------------------------------------------------
// AttPedalVis
//----------------------------------------------------------------------------

class AttPedalVis : public Att {
protected:
    AttPedalVis();
    ~AttPedalVis() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetPedalVis();

    /** Read the values for the attribute class **/
    bool ReadPedalVis(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WritePedalVis(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetForm(data_PEDALSTYLE form_) { m_form = form_; }
    data_PEDALSTYLE GetForm() const { return m_form; }
    bool HasForm() const;
    ///@}

private:
    /**
     * Captures the visual rendition and function of the hairpin; that is, whether it
     * indicates an increase or a decrease in volume.
     **/
    data_PEDALSTYLE m_form;
};

//----------------------------------------------------------------------------
// InstPedalVis
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttPedalVis
 */

class InstPedalVis : public AttPedalVis {
public:
    InstPedalVis() = default;
    virtual ~InstPedalVis() = default;
};

//----------------------------------------------------------------------------
// AttSbVis
//----------------------------------------------------------------------------

class AttSbVis : public Att {
protected:
    AttSbVis();
    ~AttSbVis() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetSbVis();

    /** Read the values for the attribute class **/
    bool ReadSbVis(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteSbVis(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetForm(sbVis_FORM form_) { m_form = form_; }
    sbVis_FORM GetForm() const { return m_form; }
    bool HasForm() const;
    ///@}

private:
    /**
     * Captures the visual rendition and function of the hairpin; that is, whether it
     * indicates an increase or a decrease in volume.
     **/
    sbVis_FORM m_form;
};

//----------------------------------------------------------------------------
// InstSbVis
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttSbVis
 */

class InstSbVis : public AttSbVis {
public:
    InstSbVis() = default;
    virtual ~InstSbVis() = default;
};

//----------------------------------------------------------------------------
// AttSectionVis
//----------------------------------------------------------------------------

class AttSectionVis : public Att {
protected:
    AttSectionVis();
    ~AttSectionVis() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetSectionVis();

    /** Read the values for the attribute class **/
    bool ReadSectionVis(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteSectionVis(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetRestart(data_BOOLEAN restart_) { m_restart = restart_; }
    data_BOOLEAN GetRestart() const { return m_restart; }
    bool HasRestart() const;
    ///@}

private:
    /** Indicates that staves begin again with this section. **/
    data_BOOLEAN m_restart;
};

//----------------------------------------------------------------------------
// InstSectionVis
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttSectionVis
 */

class InstSectionVis : public AttSectionVis {
public:
    InstSectionVis() = default;
    virtual ~InstSectionVis() = default;
};

//----------------------------------------------------------------------------
// AttSpaceVis
//----------------------------------------------------------------------------

class AttSpaceVis : public Att {
protected:
    AttSpaceVis();
    ~AttSpaceVis() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetSpaceVis();

    /** Read the values for the attribute class **/
    bool ReadSpaceVis(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteSpaceVis(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetCompressable(data_BOOLEAN compressable_) { m_compressable = compressable_; }
    data_BOOLEAN GetCompressable() const { return m_compressable; }
    bool HasCompressable() const;
    ///@}

private:
    /**
     * Indicates whether a space is 'compressible', i.e., if it may be removed at the
     * discretion of processing software.
     **/
    data_BOOLEAN m_compressable;
};

//----------------------------------------------------------------------------
// InstSpaceVis
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttSpaceVis
 */

class InstSpaceVis : public AttSpaceVis {
public:
    InstSpaceVis() = default;
    virtual ~InstSpaceVis() = default;
};

//----------------------------------------------------------------------------
// AttStaffDefVis
//----------------------------------------------------------------------------

class AttStaffDefVis : public Att {
protected:
    AttStaffDefVis();
    ~AttStaffDefVis() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetStaffDefVis();

    /** Read the values for the attribute class **/
    bool ReadStaffDefVis(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteStaffDefVis(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetLinesColor(std::string linesColor_) { m_linesColor = linesColor_; }
    std::string GetLinesColor() const { return m_linesColor; }
    bool HasLinesColor() const;
    //
    void SetLinesVisible(data_BOOLEAN linesVisible_) { m_linesVisible = linesVisible_; }
    data_BOOLEAN GetLinesVisible() const { return m_linesVisible; }
    bool HasLinesVisible() const;
    ///@}

private:
    /** Captures the colors of the staff lines. **/
    std::string m_linesColor;
    /** Records whether all staff lines are visible. **/
    data_BOOLEAN m_linesVisible;
};

//----------------------------------------------------------------------------
// InstStaffDefVis
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttStaffDefVis
 */

class InstStaffDefVis : public AttStaffDefVis {
public:
    InstStaffDefVis() = default;
    virtual ~InstStaffDefVis() = default;
};

//----------------------------------------------------------------------------
// AttStaffGrpVis
//----------------------------------------------------------------------------

class AttStaffGrpVis : public Att {
protected:
    AttStaffGrpVis();
    ~AttStaffGrpVis() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetStaffGrpVis();

    /** Read the values for the attribute class **/
    bool ReadStaffGrpVis(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteStaffGrpVis(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetBarThru(data_BOOLEAN barThru_) { m_barThru = barThru_; }
    data_BOOLEAN GetBarThru() const { return m_barThru; }
    bool HasBarThru() const;
    ///@}

private:
    /**
     * Indicates whether bar lines go across the space between staves (true) or are
     * only drawn across the lines of each staff (false).
     **/
    data_BOOLEAN m_barThru;
};

//----------------------------------------------------------------------------
// InstStaffGrpVis
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttStaffGrpVis
 */

class InstStaffGrpVis : public AttStaffGrpVis {
public:
    InstStaffGrpVis() = default;
    virtual ~InstStaffGrpVis() = default;
};

//----------------------------------------------------------------------------
// AttTupletVis
//----------------------------------------------------------------------------

class AttTupletVis : public Att {
protected:
    AttTupletVis();
    ~AttTupletVis() = default;

public:
    /** Reset the default values for the attribute class **/
    void ResetTupletVis();

    /** Read the values for the attribute class **/
    bool ReadTupletVis(pugi::xml_node element, bool removeAttr = true);

    /** Write the values for the attribute class **/
    bool WriteTupletVis(pugi::xml_node element);

    /**
     * @name Setters, getters and presence checker for class members.
     * The checker returns true if the attribute class is set (e.g., not equal
     * to the default value)
     **/
    ///@{
    void SetBracketPlace(data_STAFFREL_basic bracketPlace_) { m_bracketPlace = bracketPlace_; }
    data_STAFFREL_basic GetBracketPlace() const { return m_bracketPlace; }
    bool HasBracketPlace() const;
    //
    void SetBracketVisible(data_BOOLEAN bracketVisible_) { m_bracketVisible = bracketVisible_; }
    data_BOOLEAN GetBracketVisible() const { return m_bracketVisible; }
    bool HasBracketVisible() const;
    //
    void SetDurVisible(data_BOOLEAN durVisible_) { m_durVisible = durVisible_; }
    data_BOOLEAN GetDurVisible() const { return m_durVisible; }
    bool HasDurVisible() const;
    //
    void SetNumFormat(tupletVis_NUMFORMAT numFormat_) { m_numFormat = numFormat_; }
    tupletVis_NUMFORMAT GetNumFormat() const { return m_numFormat; }
    bool HasNumFormat() const;
    ///@}

private:
    /**
     * Used to state where a tuplet bracket will be placed in relation to the note
     * heads.
     **/
    data_STAFFREL_basic m_bracketPlace;
    /** States whether a bracket should be rendered with a tuplet. **/
    data_BOOLEAN m_bracketVisible;
    /** Determines if the tuplet duration is visible. **/
    data_BOOLEAN m_durVisible;
    /** Controls how the num:numbase ratio is to be displayed. **/
    tupletVis_NUMFORMAT m_numFormat;
};

//----------------------------------------------------------------------------
// InstTupletVis
//----------------------------------------------------------------------------

/**
 * Instantiable version of AttTupletVis
 */

class InstTupletVis : public AttTupletVis {
public:
    InstTupletVis() = default;
    virtual ~InstTupletVis() = default;
};

} // namespace libmei

#endif // __LIBMEI_ATTS_VISUAL_H__
