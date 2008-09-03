PROJECT(osgpangofonts)

LINK_LIBRARIES(osgPango)

ADD_EXECUTABLE(osgpangofonts osgpangofonts.cpp)

INSTALL(TARGETS osgpangofonts DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
