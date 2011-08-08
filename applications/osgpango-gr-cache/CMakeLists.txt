PROJECT(osgpango-gr-cache)

LINK_LIBRARIES(osgPango
	debug     osgDB${CMAKE_DEBUG_POSTFIX}
	optimized osgDB
)

ADD_EXECUTABLE(osgpango-gr-cache osgpango-gr-cache.cpp)

INSTALL(TARGETS osgpango-gr-cache DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
SET_TARGET_PROPERTIES(osgpango-gr-cache PROPERTIES DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}")

