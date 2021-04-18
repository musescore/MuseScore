

if (BUILD_64)
    set(ARCH_TYPE "_x64")
else (BUILD_64)
    set(ARCH_TYPE "_x86")
endif (BUILD_64)

if (APPLE)
       find_package(Sparkle) #needed for SPARKLE_FOUND variable
       if(SPARKLE_FOUND)
             set(MAC_SPARKLE_ENABLED 1)
             set(OsxFrameworks ${OsxFrameworks} ${SPARKLE_LIBRARY})
             message("Current OSX version is ${CURRENT_OSX_VERSION}")
             set(MAC_APPCAST_URL "https://sparkle.musescore.org/${MSCORE_RELEASE_CHANNEL}/3/macos/appcast.xml")
       endif(SPARKLE_FOUND)
elseif (MSVC)
       if ((NOT MSCORE_UNSTABLE) AND (NOT DEFINED WIN_PORTABLE)) # do not include WinSparkle in unstable and portable builds
             include(FindWinSparkle)
             add_library(winsparkledll SHARED IMPORTED)
             set_target_properties(winsparkledll PROPERTIES IMPORTED_IMPLIB ${WINSPARKLE_LIBRARY})
             set(WIN_SPARKLE_ENABLED 1)
             set(WIN_SPARKLE_APPCAST_URL "https://sparkle.musescore.org/${MSCORE_RELEASE_CHANNEL}/4/win/appcast${ARCH_TYPE}.xml")
             message("Win Sparkle Url: " ${WIN_SPARKLE_APPCAST_URL})
       endif ((NOT MSCORE_UNSTABLE) AND (NOT DEFINED WIN_PORTABLE))
else (APPLE)
       message("Sparkle is not supported on your system.")
endif (APPLE)
