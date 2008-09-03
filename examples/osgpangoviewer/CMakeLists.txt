PROJECT(osgpangoviewer)

LINK_LIBRARIES(osgPango)

ADD_EXECUTABLE(osgpangoviewer osgpangoviewer.cpp)

INSTALL(TARGETS osgpangoviewer DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
