\page tpc Tonal Pitch Class

The Tonal Pitch Class (tpc) is used to differentiate among notes with the same MIDI pitch but with different representation (also covering enharmony). The tpc is used by the following properties:
- \ref Ms::PluginAPI::Note::tpc1
- \ref Ms::PluginAPI::Note::tpc2
- \ref Ms::PluginAPI::Note::tpc

### Tonal Pitch Classes in numeric order

tpc	| name	| tpc	| name	| tpc	| name	| tpc	| name	| tpc	| name
---:|:------|------:|:------|------:|:------|------:|:------|------:|:-----
-1	| F♭♭	| 6		| F♭	| 13	| F 	| 20	| F♯	| 27	| F♯♯
0	| C♭♭	| 7		| C♭	| 14	| C 	| 21	| C♯	| 28	| C♯♯
1	| G♭♭	| 8		| G♭	| 15	| G 	| 22	| G♯	| 29	| G♯♯
2	| D♭♭	| 9		| D♭	| 16	| D 	| 23	| D♯	| 30	| D♯♯
3	| A♭♭	| 10	| A♭	| 17	| A 	| 24	| A♯	| 31	| A♯♯
4	| E♭♭	| 11	| E♭	| 18	| E 	| 25	| E♯	| 32	| E♯♯
5	| B♭♭	| 12	| B♭	| 19	| B 	| 26	| B♯	| 33	| B♯♯

### Tonal Pitch Classes in pitch order

pitch	| tpc	| name	| tpc	| name	| tpc	| name
:------:|------:|:------|------:|:------|------:|:-----
11		| 31	| A♯♯	| 19	| B 	| 7 	| C♭
10		| 24	| A♯	| 12	| B♭	| 0 	| C♭♭
9		| 29	| G♯♯	| 17	| A 	| 5 	| B♭♭
8		| 22	| G♯	|   	|   	| 10	| A♭
7		| 27	| F♯♯	| 15	| G 	| 3 	| A♭♭
6		| 32	| E♯♯	| 20	| F♯	| 8 	| G♭
5		| 25	| E♯	| 13	| F 	| 1 	| G♭♭
4		| 30	| D♯♯	| 18	| E 	| 6 	| F♭
3		| 23	| D♯	| 11	| E♭	| -1	| F♭♭
2		| 28	| C♯♯	| 16	| D 	| 4 	| E♭♭
1		| 33	| B♯♯	| 21	| C♯	| 9 	| D♭
0		| 26	| B♯	| 14	| C 	| 2 	| D♭♭
