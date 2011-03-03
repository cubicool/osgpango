// -*-c++-*- Copyright (C) 2010 osgPango Development Team
// $Id$

#include <osg/Math>
#include <osgDB/FileUtils>
#include <osgPango/Util>

namespace osgPango {

//! This function will return a vector of font files on disk matching the FONTCONFIG string
//! @descr. Please note that this routine (for now) uses a Fontconfig description, not a Pango
//! font description. The syntax is similar, but it's unfortunate we cannot use Pango's format
//! instead. Future versions of Pango may allow us to covert a Pango string description into
//! a Fontconfig string description, though only the OPPOSITE is currently available.
//!
//! Please see: http://www.freedesktop.org/software/fontconfig/fontconfig-user.html
//!
//! ...for more information on the format of a Fontconfig matching string, particularly the
//! section called "Font Properties." I've included some samples below.
//!
//! 'arial' => matches ALL arial font files.
//! 'arial:bold' => matches any arial file that includes bold.
//! 'arial:style=normal' => matches on the normal arial font.
//! 'arial:weight=bold:slant=roman'
//!
//! TODO: Continue to keep an eye on Pango for when this feature will be possible using it's
//! API instead of Fontconfig directly. :(
bool getFontPaths(const std::string& descr, FontPaths& paths) {
	// Below is how you would use this code...
	/* -----------------------------------------------------------------
	osgPango::FontPaths paths;

	if(osgPango::getFontPaths($STRING, paths)) {
		for(unsigned int i = 0; i < paths.size(); i++) {
			osg::notify(osg::NOTICE) << paths[i] << std::endl;
		}
	}
	----------------------------------------------------------------- */

	const unsigned char* d   = reinterpret_cast<const unsigned char*>(descr.c_str());
	FcPattern*           pat = FcNameParse(d);
	FcFontSet*           fs  = FcFontList(0, pat, 0);

	if(pat) FcPatternDestroy(pat);

	for(int j = 0; j < fs->nfont; j++) {
		unsigned char* file;

		if(FcPatternGetString(fs->fonts[j], FC_FILE, 0, &file) == FcResultMatch) {
			paths.push_back(file);
		}
	}

	if(fs) FcFontSetDestroy(fs);

	return paths.size() != 0;
}

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
