PROJECT(osgPango)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/etc/")

# Use a debug postfix to distinguish build products. Mostly important on
# Windows, because linking a debug library into a release executable (or
# vice-versa, or just mixing C++ runtime versions) on Windows will lead
# to crashes if the libraries use the C++ runtime.
SET(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "Add a postfix, usually d on windows")

# On GCC, we need to set these compiler flags.
IF(NOT WIN32)
	SET(CMAKE_CXX_FLAGS "-W -Wall -Wno-unused")
ENDIF(NOT WIN32)

IF(NOT WIN32)
	INCLUDE(FindPkgConfig)

	PKG_CHECK_MODULES(CAIRO REQUIRED cairo)
	PKG_CHECK_MODULES(PANGO REQUIRED pango)
	PKG_CHECK_MODULES(PANGOCAIRO REQUIRED pangocairo)
ELSE(NOT WIN32)
	FIND_PACKAGE(Cairo)
	FIND_PACKAGE(Pango)
	FIND_PACKAGE(PangoCairo)
	FIND_PACKAGE(GLib)
ENDIF(NOT WIN32)

SET(SRC_FILES
	src/Context.cpp
	src/Glyph.cpp
	src/GlyphLayerInterface.cpp
	src/GlyphLayer.cpp
	src/GlyphRenderer.cpp
	src/ShaderGenerator.cpp
	src/ShaderManager.cpp
	src/Text.cpp
	src/Util.cpp
)

SET(SRC_LAYER_FILES
	src/layers/Bitmap.cpp
	src/layers/Bevel.cpp
	src/layers/Outline.cpp
	src/layers/ShadowBlur.cpp
	src/layers/ShadowInset.cpp
	src/layers/Shadow.cpp
)

SET(SRC_RENDERER_FILES
	src/renderers/Default.cpp
	src/renderers/Outline.cpp
	src/renderers/ShadowBlur.cpp
	src/renderers/ShadowInset.cpp
	src/renderers/Shadow.cpp
)

SET(HEADER_FILES
	${osgPango_SOURCE_DIR}/include/osgPango/Context
	${osgPango_SOURCE_DIR}/include/osgPango/Export
	${osgPango_SOURCE_DIR}/include/osgPango/Glyph
	${osgPango_SOURCE_DIR}/include/osgPango/GlyphLayerInterface
	${osgPango_SOURCE_DIR}/include/osgPango/GlyphLayer
	${osgPango_SOURCE_DIR}/include/osgPango/GlyphRenderer
	${osgPango_SOURCE_DIR}/include/osgPango/ShaderGenerator
	${osgPango_SOURCE_DIR}/include/osgPango/ShaderManager
	${osgPango_SOURCE_DIR}/include/osgPango/String
	${osgPango_SOURCE_DIR}/include/osgPango/Text
	${osgPango_SOURCE_DIR}/include/osgPango/Util
)

SOURCE_GROUP("Header Files" FILES ${HEADER_FILES})
SOURCE_GROUP("Source Files" FILES ${SRC_FILES})
SOURCE_GROUP("Source Files\\Layers" FILES ${SRC_LAYER_FILES})
SOURCE_GROUP("Source Files\\Renderers" FILES ${SRC_RENDERER_FILES})

FIND_PACKAGE(OpenSceneGraph)
FIND_PACKAGE(OsgCairo)

# Modify these so that osgCairo headers and libs are found.
IF(NOT OSGCAIRO_INCLUDE_DIR AND NOT OSGCAIRO_LIB_DIR)
	SET(OSGCAIRO_INCLUDE_DIR ../osgCairo/include CACHE STRING "Modify the path if needed")
	SET(OSGCAIRO_LIB_DIR     ../osgCairo/lib     CACHE STRING "Modify the path if needed")
ENDIF(NOT OSGCAIRO_INCLUDE_DIR AND NOT OSGCAIRO_LIB_DIR)

INCLUDE_DIRECTORIES(include
	${OPENSCENEGRAPH_INCLUDE_DIR}
	${CAIRO_INCLUDE_DIRS}
	${PANGO_INCLUDE_DIRS}
	${PANGOCAIRO_INCLUDE_DIRS}
	${GLIB_INCLUDE_DIRS}
	${OSGCAIRO_INCLUDE_DIR}
)

LINK_DIRECTORIES(
	${OPENSCENEGRAPH_LIB_DIR}
	${CAIRO_LIBRARY_DIRS}
	${PANGO_LIBRARY_DIRS}
	${PANGOCAIRO_LIBRARY_DIRS}
	${GLIB_LIBRARY_DIRS}
	${OSGCAIRO_LIB_DIR}
)

ADD_LIBRARY(osgPango SHARED ${SRC_FILES} ${SRC_LAYER_FILES} ${SRC_RENDERER_FILES} ${HEADER_FILES})

# Add debug postfix to OSG libraries so we link to the right ones in debug.
# Cairo, pango, pangocairo and the gtk libs are C-only libraries so the same
# one (release) can be linked to both debug and release without problems.
TARGET_LINK_LIBRARIES(osgPango
	debug     OpenThreads${CMAKE_DEBUG_POSTFIX}
	optimized OpenThreads
	debug     osg${CMAKE_DEBUG_POSTFIX}
	optimized osg
	debug     osgUtil${CMAKE_DEBUG_POSTFIX}
	optimized osgUtil
	debug     osgText${CMAKE_DEBUG_POSTFIX}
	optimized osgText
	debug     osgViewer${CMAKE_DEBUG_POSTFIX}
	optimized osgViewer
	debug     osgDB${CMAKE_DEBUG_POSTFIX}
	optimized osgDB
	debug     osgCairo${CMAKE_DEBUG_POSTFIX}
	optimized osgCairo
        debug     cairo
        optimized cairo
	debug     pango-1.0
	optimized pango-1.0
	debug     pangocairo-1.0
	optimized pangocairo-1.0
	debug     gobject-2.0
	optimized gobject-2.0
	debug     glib-2.0
	optimized glib-2.0
)

# The archive/runtime part is so that the lib/dll pair goes into lib and bin
# respectively on Windows.
INSTALL(TARGETS osgPango
	ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
	LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
	RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
)

FOREACH(INCLUDEFILE ${HEADER_FILES})
	FILE(RELATIVE_PATH REL_INCLUDEFILE ${osgPango_SOURCE_DIR}/include/osgPango ${INCLUDEFILE})
	GET_FILENAME_COMPONENT(REL_INCLUDE_PATH ${REL_INCLUDEFILE} PATH)
	INSTALL(
		FILES ${INCLUDEFILE}
		DESTINATION ${CMAKE_INSTALL_PREFIX}/include/osgPango/${REL_INCLUDE_PATH}
	)
ENDFOREACH(INCLUDEFILE)

ADD_SUBDIRECTORY(examples/osgpangoanimation)
ADD_SUBDIRECTORY(examples/osgpangocomplexrenderer)
ADD_SUBDIRECTORY(examples/osgpangocustomrenderer)
ADD_SUBDIRECTORY(examples/osgpangofonts)
#ADD_SUBDIRECTORY(examples/osgpangoguiviewer)
ADD_SUBDIRECTORY(examples/osgpangotest)
#ADD_SUBDIRECTORY(examples/osgpangothreaded)
ADD_SUBDIRECTORY(examples/osgpangoupdatecb)
ADD_SUBDIRECTORY(examples/osgpangoviewer)


CONFIGURE_FILE(
	"${CMAKE_CURRENT_SOURCE_DIR}/etc/uninstall.cmake"
	"${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake"
	IMMEDIATE @ONLY
)

ADD_CUSTOM_TARGET(uninstall
	"${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake"
)

# Packaging information.
SET(CPACK_GENERATOR TGZ)
SET(CPACK_SOURCE_GENERATOR TGZ)
SET(CPACK_SOURCE_IGNORE_FILES
	"~$"
	".*.svn"
	".*build/"
	".*debug/"
	".*release/"
)

SET(OSGPANGO_VERSION "0.0.1")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "osgPango-${OSGPANGO_VERSION}")

INCLUDE(CPack)
