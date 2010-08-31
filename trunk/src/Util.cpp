// -*-c++-*- Copyright (C) 2010 osgPango Development Team
// $Id$

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

}
