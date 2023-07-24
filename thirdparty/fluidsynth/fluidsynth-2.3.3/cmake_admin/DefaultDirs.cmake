# Provides install directory variables as defined by the GNU Coding Standards
include ( GNUInstallDirs )

# Several directory names used by FluidSynth to install files
# the variable names are similar to the KDE4 build system

# DEFAULT_SOUNDFONT - automatically loaded in some use cases
if ( WIN32 )
  set (DEFAULT_SOUNDFONT "C:\\\\ProgramData\\\\soundfonts\\\\default.sf2" CACHE STRING
       "Default soundfont file")
else ( WIN32 )
  set (DEFAULT_SOUNDFONT "${CMAKE_INSTALL_FULL_DATADIR}/soundfonts/default.sf2" CACHE STRING
       "Default soundfont file")
endif ( WIN32 )
mark_as_advanced (DEFAULT_SOUNDFONT)

set(FRAMEWORK_INSTALL_PREFIX "")
if ( CMAKE_VERSION VERSION_GREATER "3.7.0" AND NOT CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT )
  set(FRAMEWORK_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
endif()

# BUNDLE_INSTALL_DIR - Mac only: the directory for application bundles 
set (BUNDLE_INSTALL_DIR "Applications" CACHE STRING
     "The install dir for application bundles")
mark_as_advanced (BUNDLE_INSTALL_DIR)

# FRAMEWORK_INSTALL_DIR - Mac only: the directory for framework bundles
set (FRAMEWORK_INSTALL_DIR "Library/Frameworks" CACHE STRING
     "The install dir for framework bundles")
mark_as_advanced (FRAMEWORK_INSTALL_DIR)

# XDG_APPS_INSTALL_DIR - the XDG apps dir, where .desktop files are installed
set (XDG_APPS_INSTALL_DIR "${CMAKE_INSTALL_DATADIR}/applications" CACHE STRING "The XDG apps dir")
mark_as_advanced (XDG_APPS_INSTALL_DIR) 

# XDG_MIME_INSTALL_DIR - the XDG mimetypes install dir
set (XDG_MIME_INSTALL_DIR "${CMAKE_INSTALL_DATADIR}/mime/packages" CACHE STRING
     "The install dir for the xdg mimetypes")
mark_as_advanced (XDG_MIME_INSTALL_DIR)

# DBUS_INTERFACES_INSTALL_DIR - the directory where dbus interfaces are 
# installed
set (DBUS_INTERFACES_INSTALL_DIR "${CMAKE_INSTALL_DATADIR}/dbus-1/interfaces" CACHE STRING
     "The dbus interfaces install dir")
mark_as_advanced (DBUS_INTERFACES_INSTALL_DIR) 

# DBUS_SERVICES_INSTALL_DIR  - the directory where dbus services are installed
set (DBUS_SERVICES_INSTALL_DIR "${CMAKE_INSTALL_DATADIR}/dbus-1/services" CACHE STRING
     "The dbus services install dir")
mark_as_advanced (DBUS_SERVICES_INSTALL_DIR) 

# DBUS_SYSTEM_SERVICES_INSTALL_DIR - the directory where dbus system services 
# are installed 
set (DBUS_SYSTEM_SERVICES_INSTALL_DIR "${CMAKE_INSTALL_DATADIR}/dbus-1/system-services"
     CACHE STRING "The dbus system services install dir")
mark_as_advanced (DBUS_SYSTEM_SERVICES_INSTALL_DIR) 
