# - Find Oss
# Find Oss headers and libraries.
#
#  OSS_INCLUDE_DIR  -  where to find soundcard.h, etc.
#  OSS_FOUND        - True if Oss found.


FIND_PATH(LINUX_OSS_INCLUDE_DIR "linux/soundcard.h"
  "/usr/include" "/usr/local/include"
)

FIND_PATH(SYS_OSS_INCLUDE_DIR "sys/soundcard.h"
  "/usr/include" "/usr/local/include"
)

FIND_PATH(MACHINE_OSS_INCLUDE_DIR "machine/soundcard.h"
  "/usr/include" "/usr/local/include"
)

SET(OSS_FOUND FALSE)


if ( NOT WIN32 )

    IF(LINUX_OSS_INCLUDE_DIR)
            SET(OSS_FOUND TRUE)
            SET(OSS_INCLUDE_DIR ${LINUX_OSS_INCLUDE_DIR})
            SET(HAVE_LINUX_SOUNDCARD_H 1)
    ENDIF()

    IF(SYS_OSS_INCLUDE_DIR)
            SET(OSS_FOUND TRUE)
            SET(OSS_INCLUDE_DIR ${SYS_OSS_INCLUDE_DIR})
            SET(HAVE_SYS_SOUNDCARD_H 1)
    ENDIF()

    IF(MACHINE_OSS_INCLUDE_DIR)
            SET(OSS_FOUND TRUE)
            SET(OSS_INCLUDE_DIR ${MACHINE_OSS_INCLUDE_DIR})
            SET(HAVE_MACHINE_SOUNDCARD_H 1)
    ENDIF()

ENDIF(NOT WIN32)

MARK_AS_ADVANCED (
	OSS_FOUND
	OSS_INCLUDE_DIR
)
