PROJECT(osgpangovertical)

LINK_LIBRARIES(osgPango
	debug     osgViewer${CMAKE_DEBUG_POSTFIX}
	optimized osgViewer
	debug     osgGA${CMAKE_DEBUG_POSTFIX}
	optimized osgGA
)

ADD_EXECUTABLE(osgpangovertical osgpangovertical.cpp)

INSTALL(TARGETS osgpangovertical DESTINATION ${CMAKE_INSTALL_PREFIX}/share/OpenSceneGraph/bin)
SET_TARGET_PROPERTIES(osgpangovertical PROPERTIES DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}")
