{
	/* try and get br->consumed_words and br->consumed_bits into register;
	 * must remember to flush them back to *br before calling other
	 * bitreader functions that use them, and before returning */
	uint32_t cwords, words, lsbs, msbs, x, y, limit;
	uint32_t ucbits; /* keep track of the number of unconsumed bits in word */
	brword b;
	int *val, *end;

	FLAC__ASSERT(0 != br);
	FLAC__ASSERT(0 != br->buffer);
	/* WATCHOUT: code does not work with <32bit words; we can make things much faster with this assertion */
	FLAC__ASSERT(FLAC__BITS_PER_WORD >= 32);
	FLAC__ASSERT(parameter < 32);
	/* the above two asserts also guarantee that the binary part never straddles more than 2 words, so we don't have to loop to read it */

	limit = UINT32_MAX >> parameter; /* Maximal msbs that can occur with residual bounded to int32_t */

	val = vals;
	end = vals + nvals;

	if(parameter == 0) {
		while(val < end) {
			/* read the unary MSBs and end bit */
			if(!FLAC__bitreader_read_unary_unsigned(br, &msbs))
				return false;
			/* Checking limit here would be overzealous: coding UINT32_MAX
			 * with parameter == 0 would take 4GiB */
			*val++ = (int)(msbs >> 1) ^ -(int)(msbs & 1);
		}

		return true;
	}

	FLAC__ASSERT(parameter > 0);

	cwords = br->consumed_words;
	words = br->words;

	/* if we've not consumed up to a partial tail word... */
	if(cwords >= words) {
		x = 0;
		goto process_tail;
	}

	ucbits = FLAC__BITS_PER_WORD - br->consumed_bits;
	b = br->buffer[cwords] << br->consumed_bits;  /* keep unconsumed bits aligned to left */

	while(val < end) {
		/* read the unary MSBs and end bit */
		x = y = COUNT_ZERO_MSBS2(b);
		if(x == FLAC__BITS_PER_WORD) {
			x = ucbits;
			do {
				/* didn't find stop bit yet, have to keep going... */
				cwords++;
				if (cwords >= words)
					goto incomplete_msbs;
				b = br->buffer[cwords];
				y = COUNT_ZERO_MSBS2(b);
				x += y;
			} while(y == FLAC__BITS_PER_WORD);
		}
		b <<= y;
		b <<= 1; /* account for stop bit */
		ucbits = (ucbits - x - 1) % FLAC__BITS_PER_WORD;
		msbs = x;

		if(x > limit)
			return false;

		/* read the binary LSBs */
		x = (FLAC__uint32)(b >> (FLAC__BITS_PER_WORD - parameter)); /* parameter < 32, so we can cast to 32-bit uint32_t */
		if(parameter <= ucbits) {
			ucbits -= parameter;
			b <<= parameter;
		} else {
			/* there are still bits left to read, they will all be in the next word */
			cwords++;
			if (cwords >= words)
				goto incomplete_lsbs;
			b = br->buffer[cwords];
			ucbits += FLAC__BITS_PER_WORD - parameter;
			x |= (FLAC__uint32)(b >> ucbits);
			b <<= FLAC__BITS_PER_WORD - ucbits;
		}
		lsbs = x;

		/* compose the value */
		x = (msbs << parameter) | lsbs;
		*val++ = (int)(x >> 1) ^ -(int)(x & 1);

		continue;

		/* at this point we've eaten up all the whole words */
process_tail:
		do {
			if(0) {
incomplete_msbs:
				br->consumed_bits = 0;
				br->consumed_words = cwords;
			}

			/* read the unary MSBs and end bit */
			if(!FLAC__bitreader_read_unary_unsigned(br, &msbs))
				return false;
			msbs += x;
			x = ucbits = 0;

			if(0) {
incomplete_lsbs:
				br->consumed_bits = 0;
				br->consumed_words = cwords;
			}

			/* read the binary LSBs */
			if(!FLAC__bitreader_read_raw_uint32(br, &lsbs, parameter - ucbits))
				return false;
			lsbs = x | lsbs;

			/* compose the value */
			x = (msbs << parameter) | lsbs;
			*val++ = (int)(x >> 1) ^ -(int)(x & 1);
			x = 0;

			cwords = br->consumed_words;
			words = br->words;
			ucbits = FLAC__BITS_PER_WORD - br->consumed_bits;
			b = cwords < br->capacity ? br->buffer[cwords] << br->consumed_bits : 0;
		} while(cwords >= words && val < end);
	}

	if(ucbits == 0 && cwords < words) {
		/* don't leave the head word with no unconsumed bits */
		cwords++;
		ucbits = FLAC__BITS_PER_WORD;
	}

	br->consumed_bits = FLAC__BITS_PER_WORD - ucbits;
	br->consumed_words = cwords;

	return true;
}
