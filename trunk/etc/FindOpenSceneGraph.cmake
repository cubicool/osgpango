# Copyright (C) 2008 Cedric Pinson
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#
# Authors:
#  Cedric Pinson <mornifle@plopbyte.net

IF (WIN32)
  SET(LIBOSG osg${OSG_DEBUG_POSTFIX}.lib)
ELSE (WIN32)
  SET(LIBOSG libosg${OSG_DEBUG_POSTFIX}.so)
ENDIF (WIN32)

FIND_PATH(OPENSCENEGRAPH_LIB_DIR ${LIBOSG}
  $ENV{OSGDIR}/lib
  ${OSGDIR}/lib
  /usr/lib 
  /usr/local/lib
  )

SET(OPENSCENEGRAPH_INCLUDE_DIR ${OPENSCENEGRAPH_LIB_DIR}/../include CACHE STRING "Modify the path if needed")

IF (NOT OPENSCENEGRAPH_LIB_DIR STREQUAL OPENSCENEGRAPH_LIB_DIR-NOTFOUND)

  SET(LIBOSGDB_OSG osgdb_osg)
  IF (WIN32)
    SET(LIBOSGDB_OSG "${LIBOSGDB_OSG}.lib")
  ELSE (WIN32)
    SET(LIBOSGDB_OSG "${LIBOSGDB_OSG}.so")
  ENDIF (WIN32)


  FIND_PATH(OPENSCENEGRAPH_PLUGIN_DIR ${LIBOSGDB_OSG}
    "${OPENSCENEGRAPH_LIB_DIR}/osgPlugins-2.8.0/"
    "${OPENSCENEGRAPH_LIB_DIR}/osgPlugins-2.6.0/"
    "${OPENSCENEGRAPH_LIB_DIR}/osgPlugins-2.4.0/"
    "${OPENSCENEGRAPH_LIB_DIR}/osgPlugins-2.2.0/"
    "${OPENSCENEGRAPH_LIB_DIR}/osgPlugins-2.0.0/"
    )
  

  MESSAGE(STATUS "Use OpenSceneGraph include dir: " ${OPENSCENEGRAPH_INCLUDE_DIR})
  MESSAGE(STATUS "Use OpenSceneGraph lib dir: "${OPENSCENEGRAPH_LIB_DIR})
  MESSAGE(STATUS "Use OpenSceneGraph lib plugin dir: "${OPENSCENEGRAPH_PLUGIN_DIR})
  SET(OPENSCENEGRAPH_FOUND TRUE)
ELSE (NOT OPENSCENEGRAPH_LIB_DIR STREQUAL OPENSCENEGRAPH_LIB_DIR-NOTFOUND)
  MESSAGE(STATUS "Could not find OpenSceneGraph with " ${LIBOSG} " , dont forget to set OSGDIR variable, or configure the path throw ccmake")
ENDIF (NOT OPENSCENEGRAPH_LIB_DIR STREQUAL OPENSCENEGRAPH_LIB_DIR-NOTFOUND)


