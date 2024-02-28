
#pragma once

/*
 * MULTI-CHANNEL SIGNED DISTANCE FIELD GENERATOR v1.5 (2017-07-23) - extensions
 * ----------------------------------------------------------------------------
 * A utility by Viktor Chlumsky, (c) 2014 - 2017
 *
 * The extension module provides ways to easily load input and save output using popular formats.
 *
 * Third party dependencies in extension module:
 * - FreeType 2
 *   (to load input font files)
 * - TinyXML 2 by Lee Thomason
 *   (to aid in parsing input SVG files)
 * - LodePNG by Lode Vandevenne
 *   (to save output PNG images)
 *
 */

#include "ext/save-png.h"
#include "ext/import-svg.h"
#include "ext/import-font.h"
