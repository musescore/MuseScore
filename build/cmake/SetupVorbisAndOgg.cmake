

if (APPLE)

      ##
      ## libvorbis
      ##
      PKGCONFIG1 (vorbis 1.3.3 VORBIS_INCDIR VORBIS_LIBDIR VORBIS_LIB VORBIS_CPP)
      if (VORBIS_INCDIR)
          message("libvorbis detected ${VORBIS_INCDIR} ${VORBIS_LIBDIR} ${VORBIS_LIB}")
      else (VORBIS_INCDIR)
          message("libvorbis not found\n")
      endif (VORBIS_INCDIR)


      ##
      ## libogg
      ##
      PKGCONFIG1 (ogg 1.3.0 OGG_INCDIR OGG_LIBDIR OGG_LIB OGG_CPP)
      if (OGG_INCDIR)
          message("libogg detected ${OGG_INCDIR} ${OGG_LIBDIR} ${OGG_LIB}")
      else (OGG_INCDIR)
          message("libogg not found\n")
      endif (OGG_INCDIR)

else(APPLE)
   if (MINGW OR MSVC)
      include(FindVorbis)
      add_library(vorbisdll SHARED IMPORTED)
      set_target_properties(vorbisdll PROPERTIES IMPORTED_IMPLIB ${VORBIS_LIBRARY})

      include(FindVorbisfile)
      add_library(vorbisfiledll SHARED IMPORTED)
      set_target_properties(vorbisfiledll PROPERTIES IMPORTED_IMPLIB ${VORBISFILE_LIBRARY})

      include(FindOgg)
      add_library(oggdll SHARED IMPORTED)
      set_target_properties(oggdll PROPERTIES IMPORTED_IMPLIB ${OGG_LIBRARY})
   endif (MINGW OR MSVC)
endif(APPLE)
