// -*-c++-*- Copyright (C) 2010 osgPango Development Team
// $Id$

#include <osgDB/WriteFile>
#include <osgPango/Context>

void setupArguments(osg::ArgumentParser& args) {
	args.getApplicationUsage()->setDescription(
		args.getApplicationName() + " is used to generate GlyphRenderer cache OSG2 files."
	);

	args.getApplicationUsage()->setCommandLineUsage(
		args.getApplicationName() + " [options] text..."
	);

	/*
	args.getApplicationUsage()->addCommandLineOption(
		"--renderer <string> <int,int,int,int>",
		"The GlyphRenderer object to use (outline, shadow, shadowBlur, shadowInset) and sizes. "
		"Note that not all renderers need--or require--four arguments."
	);
	*/
}

int main(int argc, char** argv) {
	osg::ArgumentParser args(&argc, argv);

	setupArguments(args);

	osgPango::Context::instance().init();

	osgPango::GlyphRenderer* renderer = new osgPango::GlyphRendererDefault();

	if(!osgDB::writeObjectFile(*renderer, "renderer.osgt")) OSG_NOTICE << "Failed." << std::endl;

	return 0;
}

