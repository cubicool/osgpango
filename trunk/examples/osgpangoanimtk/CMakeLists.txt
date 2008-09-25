PROJECT(osgpangoanimtk)

LINK_LIBRARIES(osgPango)

ADD_EXECUTABLE(osgpangoanimtk osgpangoanimtk.cpp)

INSTALL(TARGETS osgpangoanimtk DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
