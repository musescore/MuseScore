

set (SF_VERSION_REMOTE_FILE ${PROJECT_BINARY_DIR}/SF_VERSION)
set (SF_VERSION_LOCAL_FILE ${PROJECT_SOURCE_DIR}/share/sound/SF_VERSION)
set (DOWNLOAD_URL_ROOT "https://ftp.osuosl.org/pub/musescore/soundfont/MuseScore_General")
set (SOUND_DIRECTORY ${PROJECT_SOURCE_DIR}/share/sound)

file(DOWNLOAD ${DOWNLOAD_URL_ROOT}/VERSION ${SF_VERSION_REMOTE_FILE} STATUS SF_VERSION_DL_STATUS_LIST)
list(GET SF_VERSION_DL_STATUS_LIST 0 SF_VERSION_DL_STATUS)

if(EXISTS ${SF_VERSION_REMOTE_FILE} AND SF_VERSION_DL_STATUS EQUAL 0)
  file (STRINGS ${SF_VERSION_REMOTE_FILE} SF_VERSION_REMOTE)

  set(SF_VERSION_LOCAL "0.0")
  if (EXISTS ${SF_VERSION_LOCAL_FILE})
    file (STRINGS ${SF_VERSION_LOCAL_FILE} SF_VERSION_LOCAL)
  endif ()

  string(COMPARE LESS ${SF_VERSION_LOCAL} ${SF_VERSION_REMOTE} DO_DOWNLOAD)
  if (DO_DOWNLOAD)
    message("Version ${SF_VERSION_LOCAL} of the MuseScore SoundFont is outdated, downloading version ${SF_VERSION_REMOTE}.")
    # delete soundfont and download new version
    ## TODO check STATUS of downloads
    file (REMOVE ${SOUND_DIRECTORY}/MuseScore_General.sf3
                 ${SOUND_DIRECTORY}/MuseScore_General_License.md
                 ${SOUND_DIRECTORY}/MuseScore_General_Changelog.md
                 ${SOUND_DIRECTORY}/MuseScore_General_Readme.md)
    file(DOWNLOAD ${DOWNLOAD_URL_ROOT}/MuseScore_General.sf3 ${SOUND_DIRECTORY}/MuseScore_General.sf3 SHOW_PROGRESS STATUS SF_DL_STATUS)
    list(GET SF_DL_STATUS 0 HTTP_STATUS)
    if (NOT HTTP_STATUS EQUAL 0)
      list(GET SF_DL_STATUS 1 HTTP_STATUS_TEXT)
      message(FATAL_ERROR "Cannot download new soundfont. Error: ${HTTP_STATUS_TEXT}")
    endif()
    file(DOWNLOAD ${DOWNLOAD_URL_ROOT}/MuseScore_General_License.md ${SOUND_DIRECTORY}/MuseScore_General_License.md)
    file(DOWNLOAD ${DOWNLOAD_URL_ROOT}/MuseScore_General_Changelog.md ${SOUND_DIRECTORY}/MuseScore_General_Changelog.txt)
    file(DOWNLOAD ${DOWNLOAD_URL_ROOT}/MuseScore_General_Readme.md ${SOUND_DIRECTORY}/MuseScore_General_Readme.md)
    # replace VERSION file (in any case, delete VERSION file)
    file (COPY ${SF_VERSION_REMOTE_FILE} DESTINATION ${SOUND_DIRECTORY})
    file (REMOVE ${SF_VERSION_REMOTE_FILE})
  else(DO_DOWNLOAD)
    message("MuseScore SoundFont is up to date.")
  endif(DO_DOWNLOAD)
endif()
