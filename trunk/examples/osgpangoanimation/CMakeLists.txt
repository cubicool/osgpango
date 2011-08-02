PROJECT(osgpangoanimation)

LINK_LIBRARIES(osgPango
	debug     osgViewer${CMAKE_DEBUG_POSTFIX}
	optimized osgViewer
	debug     osgGA${CMAKE_DEBUG_POSTFIX}
	optimized osgGA
)

ADD_EXECUTABLE(osgpangoanimation osgpangoanimation.cpp)

INSTALL(TARGETS osgpangoanimation DESTINATION ${CMAKE_INSTALL_PREFIX}/share/OpenSceneGraph/bin)
SET_TARGET_PROPERTIES(osgpangoanimation PROPERTIES DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}")
