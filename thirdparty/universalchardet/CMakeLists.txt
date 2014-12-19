if (APPLE)
      file(GLOB_RECURSE INCS "*.h")
else (APPLE)
      set(INCS "")
endif (APPLE)

add_library (
      universalchardet STATIC
      CharDistribution.cpp
      JpCntx.cpp
      LangBulgarianModel.cpp
      LangCyrillicModel.cpp
      LangGreekModel.cpp
      LangHungarianModel.cpp
      LangHebrewModel.cpp
      LangThaiModel.cpp
      nsHebrewProber.cpp
      nsCharSetProber.cpp
      nsBig5Prober.cpp
      nsEUCJPProber.cpp
      nsEUCKRProber.cpp
      nsEUCTWProber.cpp
      nsEscCharsetProber.cpp
      nsEscSM.cpp
      nsGB2312Prober.cpp
      nsMBCSGroupProber.cpp
      nsMBCSSM.cpp
      nsSBCSGroupProber.cpp
      nsSBCharSetProber.cpp
      nsSJISProber.cpp
      nsUTF8Prober.cpp
      nsLatin1Prober.cpp
      nsUniversalDetector.cpp
  )

set_target_properties( universalchardet
   PROPERTIES
      COMPILE_FLAGS "${PCH_INCLUDE} -g -Wall -Wextra -Winvalid-pch"
   )

xcode_pch(universalchardet all)

ADD_DEPENDENCIES(universalchardet mops1)
ADD_DEPENDENCIES(universalchardet mops2)

