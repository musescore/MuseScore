# Several directory names used by FluidSynth to install files
# the variable names are similar to the KDE4 build system

# DEFAULT_SOUNDFONT - automatically loaded in some use cases
if ( WIN32 )
  set (DEFAULT_SOUNDFONT "C:\\\\soundfonts\\\\default.sf2" CACHE STRING
       "Default soundfont file")
else ( WIN32 )
  set (DEFAULT_SOUNDFONT "${CMAKE_INSTALL_PREFIX}/share/soundfonts/default.sf2" CACHE STRING
       "Default soundfont file")
endif ( WIN32 )
mark_as_advanced (DEFAULT_SOUNDFONT)

# BUNDLE_INSTALL_DIR - Mac only: the directory for application bundles 
set (BUNDLE_INSTALL_DIR "/Applications" CACHE STRING 
     "The install dir for application bundles")
mark_as_advanced (BUNDLE_INSTALL_DIR)
     
# FRAMEWORK_INSTALL_DIR - Mac only: the directory for framework bundles
set (FRAMEWORK_INSTALL_DIR "/Library/Frameworks" CACHE STRING 
     "The install dir for framework bundles")
mark_as_advanced (FRAMEWORK_INSTALL_DIR) 

# BIN_INSTALL_DIR - the directory where executables will be installed
set (BIN_INSTALL_DIR "bin" CACHE STRING "The install dir for executables")
mark_as_advanced (BIN_INSTALL_DIR) 

# SBIN_INSTALL_DIR - the directory where system executables will be installed
set (SBIN_INSTALL_DIR "sbin" CACHE STRING 
     "The install dir for system executables")
mark_as_advanced (SBIN_INSTALL_DIR) 

# LIB_INSTALL_DIR - the directory where libraries will be installed
set (LIB_INSTALL_DIR "lib${LIB_SUFFIX}" CACHE STRING "The install dir for libraries")
mark_as_advanced (LIB_INSTALL_DIR) 

# INCLUDE_INSTALL_DIR - the install dir for header files
set (INCLUDE_INSTALL_DIR "include" CACHE STRING "The install dir for headers")
mark_as_advanced (INCLUDE_INSTALL_DIR) 

# DATA_INSTALL_DIR - the base install directory for data files
set (DATA_INSTALL_DIR "share" CACHE STRING 
     "The base install dir for data files")
mark_as_advanced (DATA_INSTALL_DIR) 

# DOC_INSTALL_DIR - the install dir for documentation
set (DOC_INSTALL_DIR "share/doc" CACHE STRING 
     "The install dir for documentation")
mark_as_advanced (DOC_INSTALL_DIR) 

# INFO_INSTALL_DIR - the info install dir
set (INFO_INSTALL_DIR "share/info" CACHE STRING "The info install dir")
mark_as_advanced (INFO_INSTALL_DIR) 

# MAN_INSTALL_DIR - the man pages install dir
if ( CMAKE_SYSTEM_NAME MATCHES "FreeBSD|DragonFly")
  set (MAN_INSTALL_DIR "man/man1" CACHE STRING "The man pages install dir")
else()
  set (MAN_INSTALL_DIR "share/man/man1" CACHE STRING "The man pages install dir")
endif()
mark_as_advanced (MAN_INSTALL_DIR) 

# SYSCONF_INSTALL_DIR - the config file install dir
set (SYSCONF_INSTALL_DIR "/etc" CACHE PATH 
     "The sysconfig install dir")
mark_as_advanced (SYSCONF_INSTALL_DIR) 

# XDG_APPS_INSTALL_DIR - the XDG apps dir, where .desktop files are installed
set (XDG_APPS_INSTALL_DIR "share/applications" CACHE STRING "The XDG apps dir")
mark_as_advanced (XDG_APPS_INSTALL_DIR) 

# XDG_MIME_INSTALL_DIR - the XDG mimetypes install dir
set (XDG_MIME_INSTALL_DIR "share/mime/packages" CACHE STRING 
     "The install dir for the xdg mimetypes")
mark_as_advanced (XDG_MIME_INSTALL_DIR)
 
# DBUS_INTERFACES_INSTALL_DIR - the directory where dbus interfaces are 
# installed
set (DBUS_INTERFACES_INSTALL_DIR "share/dbus-1/interfaces" CACHE STRING 
     "The dbus interfaces install dir")
mark_as_advanced (DBUS_INTERFACES_INSTALL_DIR) 

# DBUS_SERVICES_INSTALL_DIR  - the directory where dbus services are installed
set (DBUS_SERVICES_INSTALL_DIR "share/dbus-1/services" CACHE STRING 
     "The dbus services install dir")
mark_as_advanced (DBUS_SERVICES_INSTALL_DIR) 

# DBUS_SYSTEM_SERVICES_INSTALL_DIR - the directory where dbus system services 
# are installed 
set (DBUS_SYSTEM_SERVICES_INSTALL_DIR "share/dbus-1/system-services" 
     CACHE STRING "The dbus system services install dir")
mark_as_advanced (DBUS_SYSTEM_SERVICES_INSTALL_DIR) 
