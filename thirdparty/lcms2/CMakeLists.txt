add_library(lcms2 STATIC
	src/cmsintrp.c
	src/cmsio0.c
	src/cmsio1.c
	src/cmslut.c
	src/cmsmd5.c
	src/cmsmtrx.c
	src/cmsnamed.c
	src/cmsopt.c
	src/cmspack.c
	src/cmspcs.c
	src/cmsplugin.c
	src/cmsps2.c
	src/cmssamp.c
	src/cmssm.c
	src/cmsvirt.c
	src/cmswtpnt.c
	src/cmsxform.c
   )

set_target_properties(lcms2
      PROPERTIES
      COMPILE_FLAGS
            "-O2 -Wno-write-strings -ansi -Wnon-virtual-dtor -Woverloaded-virtual"
      )

include_directories(
      ${PROJECT_SOURCE_DIR}/thirdparty/lcms2/include
	#${PROJECT_SOURCE_DIR}/thirdparty/lcms2/src
      )
