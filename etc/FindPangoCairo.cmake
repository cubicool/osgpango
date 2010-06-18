# Locate pangocairo
# This module defines
# PANGOCAIRO_LIBRARY_DIRS
# PANGOCAIRO_INCLUDE_DIRS, where to find the headers
#


FIND_PATH(PANGOCAIRO_INCLUDE_DIRS pango/pangocairo.h
    $ENV{PANGOCAIRODIR}/include
    $ENV{PANGOCAIRODIR}
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/include
    /usr/include
    /sw/include # Fink
    /opt/local/include # DarwinPorts
    /opt/csw/include # Blastwave
    /opt/include
)

IF (WIN32)
  SET(LIBPANGOCAIRO "pangocairo-1.0.lib")
ELSE (WIN32)
  SET(LIBPANGOCAIRO "libpangocairo-1.0.so")
ENDIF (WIN32)

FIND_PATH(PANGOCAIRO_LIBRARY_DIRS ${LIBPANGOCAIRO}
    $ENV{PANGOCAIRODIR}/lib
    $ENV{PANGOCAIRODIR}
    ~/Library/Frameworks
    /Library/Frameworks
    /usr/local/lib
    /usr/lib
    /sw/lib
    /opt/local/lib
    /opt/csw/lib
    /opt/lib
    /usr/freeware/lib64
)

SET(PANGOCAIRO_FOUND "NO")
IF(PANGOCAIRO_LIBRARY_DIRS AND PANGOCAIRO_INCLUDE_DIRS)
    SET(PANGOCAIRO_FOUND "YES")
ENDIF(PANGOCAIRO_LIBRARY_DIRS AND PANGOCAIRO_INCLUDE_DIRS)
