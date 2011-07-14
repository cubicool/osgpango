// -*-c++-*- Copyright (C) 2011 osgPango Development Team
// $Id$

#include <osg/Math>
#include <osgDB/FileUtils>
#include <osgPango/Util>

namespace osgPango {

std::string getFilePath(const std::string& filename) {
	osgDB::FilePathList  path;
	osgDB::FilePathList& paths = osgDB::getDataFilePathList();
	
	char* fp = getenv("OSGPANGO_FILE_PATH");
    
	osgDB::convertStringPathIntoFilePathList(fp ? fp : ".", path);

	for(osgDB::FilePathList::iterator i = path.begin(); i != path.end(); i++) paths.push_back(*i);

	return osgDB::findDataFile(filename);
}

void roundVec3(osg::Vec3& vec) {
	vec[0] = osg::round(vec[0]);
	vec[1] = osg::round(vec[1]);
	vec[2] = osg::round(vec[2]);
}

}
