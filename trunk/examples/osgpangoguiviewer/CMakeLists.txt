PROJECT(osgpangoguiviewer)

LINK_LIBRARIES(osgPango)

ADD_EXECUTABLE(osgpangoguiviewer osgpangoguiviewer.cpp)

INSTALL(TARGETS osgpangoguiviewer DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
