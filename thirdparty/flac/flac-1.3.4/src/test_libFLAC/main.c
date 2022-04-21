/* test_libFLAC - Unit tester for libFLAC
 * Copyright (C) 2000-2009  Josh Coalson
 * Copyright (C) 2011-2018  Xiph.Org Foundation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "bitreader.h"
#include "bitwriter.h"
#include "crc.h"
#include "decoders.h"
#include "encoders.h"
#include "endswap.h"
#include "format.h"
#include "metadata.h"
#include "md5.h"

int main(void)
{
	if(!test_endswap())
		return 1;

	if(!test_crc())
		return 1;

	if(!test_md5())
		return 1;

	if(!test_bitreader())
		return 1;

	if(!test_bitwriter())
		return 1;

	if(!test_format())
		return 1;

	if(!test_encoders())
		return 1;

	if(!test_decoders())
		return 1;

	if(!test_metadata())
		return 1;

	return 0;
}
