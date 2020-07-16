/*
SoLoud audio engine
Copyright (c) 2013-2020 Jari Komppa

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
   distribution.
*/

/*
This is a "hack" header to fool third party code to use our File stuff instead
of stdio FILE* stuff.
You can use soloud_file_hack_off.h to undef the stuff defined here.
*/

#ifndef SEEK_SET
#error soloud_file_hack_on must be included after stdio, otherwise the #define hacks will break stdio.
#endif

typedef void* Soloud_Filehack;

#ifdef __cplusplus
extern "C" {
#endif

extern int Soloud_Filehack_fgetc(Soloud_Filehack *f);
extern int Soloud_Filehack_fread(void *dst, int s, int c, Soloud_Filehack *f);
extern int Soloud_Filehack_fseek(Soloud_Filehack *f, int idx, int base);
extern int Soloud_Filehack_ftell(Soloud_Filehack *f);
extern int Soloud_Filehack_fclose(Soloud_Filehack *f);
extern Soloud_Filehack * Soloud_Filehack_fopen(const char *aFilename, char *aMode);
extern int Soloud_Filehack_fopen_s(Soloud_Filehack **f, const char* aFilename, char* aMode);

#ifdef __cplusplus
}
#endif

#define FILE Soloud_Filehack
#define fgetc Soloud_Filehack_fgetc
#define fread Soloud_Filehack_fread
#define fseek Soloud_Filehack_fseek
#define ftell Soloud_Filehack_ftell
#define fclose Soloud_Filehack_fclose
#define fopen Soloud_Filehack_fopen
#define fopen_s Soloud_Filehack_fopen_s
