// -*-c++-*- Copyright (C) 2010 osgPango Development Team
// $Id$

#include <osgDB/FileUtils>
#include <osgPango/Util>

namespace osgPango {

std::string getFilePath(const std::string& filename) {
	osgDB::FilePathList path;

	char* fp = getenv("OSGPANGO_FILE_PATH");
    
	osgDB::convertStringPathIntoFilePathList(fp ? fp : ".", path);

	return osgDB::findFileInPath(filename, path);
}

}
