	int i, j;
	(void) lag;
	FLAC__ASSERT(lag <= MAX_LAG);

        for(i = 0; i < MAX_LAG; i++)
                autoc[i] = 0.0;

        for(i = 0; i < MAX_LAG; i++)
                for(j = 0; j <= i; j++)
                        autoc[j] += (double)data[i] * (double)data[i-j];

        for(i = MAX_LAG; i < (int)data_len; i++)
		for(j = 0; j < MAX_LAG; j++)
	                autoc[j] += (double)data[i] * (double)data[i-j];
