// -*-c++-*- Copyright (C) 2010 osgPango Development Team
// $Id$

#include <iostream>
#include <osg/io_utils>
#include <osg/ArgumentParser>
#include <osg/Texture2D>
#include <osg/MatrixTransform>
#include <osgGA/StateSetManipulator>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgPango/Context>

const std::string LOREM_IPSUM(
	"<span font='Verdana 20'>"
	"<span color='red'><b>Lorem ipsum dolor sit amet</b>, consectetur adipisicing elit, sed do eiusmod</span> "
	"<span color='orange'>tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam,</span> "	
	"<span font='Verdana 15'><i>"
	"<span color='yellow'>quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.</span> "
	"</i></span>"
	"<span color='green'>Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu</span> "
	"<span color='white'>fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in</span> "
	"<span color='black'>culpa qui officia deserunt mollit anim id est laborum.</span>"
	"</span>"
);

const unsigned int WINDOW_WIDTH  = 800;
const unsigned int WINDOW_HEIGHT = 600;

osg::Camera* createOrthoCamera(float width, float height) {
	osg::Camera* camera = new osg::Camera();

	camera->getOrCreateStateSet()->setMode(
		GL_LIGHTING,
		osg::StateAttribute::PROTECTED | osg::StateAttribute::OFF
	);

	camera->setProjectionMatrix(osg::Matrix::ortho2D(0.0, width, 0.0f, height));
	camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	camera->setViewMatrix(osg::Matrix::identity());
	camera->setClearMask(GL_DEPTH_BUFFER_BIT);
	camera->setRenderOrder(osg::Camera::POST_RENDER);

	return camera;
}

void setupArguments(osg::ArgumentParser& args) {
	args.getApplicationUsage()->setDescription(
		args.getApplicationName() + " is a quick font viewer application for osgPango."
	);

	args.getApplicationUsage()->setCommandLineUsage(
		args.getApplicationName() + " [options] text..."
	);

	args.getApplicationUsage()->addCommandLineOption(
		"--renderer <string> <int>",
		"The GlyphRenderer object to use (outline, shadow, shadowBlur) and size."
	);

	args.getApplicationUsage()->addCommandLineOption(
		"--alpha <float>",
		"The composite alpha for both the font and the effects."
	);

	args.getApplicationUsage()->addCommandLineOption(
		"--width <int>",
		"The width that the Pango layout is constrained to (useful for alignment)."
	);

	args.getApplicationUsage()->addCommandLineOption(
		"--alignment",
		"The Pango alignment; one of left, right, center, or justify."
	);
}

int main(int argc, char** argv) {
	osg::ArgumentParser args(&argc, argv);

	setupArguments(args);

	osgViewer::Viewer viewer(args);

	while(args.read("--help")) {
		args.getApplicationUsage()->write(
			std::cout,
			osg::ApplicationUsage::COMMAND_LINE_OPTION
		);

		return 0;
	}

	std::string text(LOREM_IPSUM);

	// All of our temporary variables.
	std::string renderer, rendererSize, alpha, alignment, width;

	osgPango::Context& context = osgPango::Context::instance();

	context.init();

	osgPango::TextTransform* t = new osgPango::TextTransform();

	osgPango::TextOptions to;

	while(args.read("--renderer", renderer, rendererSize)) {
		int s = std::atoi(rendererSize.c_str());

		if(renderer == "outline") context.addGlyphRenderer(
			"outline",
			new osgPango::GlyphRendererOutline(s)
		);

		else if(renderer == "shadow") context.addGlyphRenderer(
			"shadow",
			new osgPango::GlyphRendererShadow(s, s)
		);

		else if(renderer == "shadowBlur") context.addGlyphRenderer(
			"shadowBlur",
			new osgPango::GlyphRendererShadowBlur(s)
		);

		else continue;

		to.renderer = renderer;
	}

	while(args.read("--alpha", alpha)) {
		float a = std::atof(alpha.c_str());

		t->setAlpha(a);
	}

	while(args.read("--width", width)) to.width = std::atoi(width.c_str());

	while(args.read("--alignment", alignment)) {
		if(alignment == "center") to.alignment = osgPango::TextOptions::TEXT_ALIGN_CENTER;
		
		else if(alignment == "right") to.alignment = osgPango::TextOptions::TEXT_ALIGN_RIGHT;
		
		else if(alignment == "justify") to.alignment = osgPango::TextOptions::TEXT_ALIGN_JUSTIFY;
	}


	if(args.argc() >= 2) {
		text = "";

		for(int i = 1; i < args.argc(); i++) text += std::string(args[i]) + " ";
	}

	// The user didn't set a width, so use our screen size.
	if(to.width <= 0) to.width = WINDOW_WIDTH;

	t->addText(text, 0, 0, to);

	if(!t->finalize()) return 1;

	osg::Camera* camera = createOrthoCamera(WINDOW_WIDTH, WINDOW_HEIGHT);

	viewer.addEventHandler(new osgViewer::StatsHandler());
	viewer.addEventHandler(new osgViewer::WindowSizeHandler());
	viewer.addEventHandler(new osgGA::StateSetManipulator(
		viewer.getCamera()->getOrCreateStateSet()
	));

	camera->addChild(t);

	viewer.setSceneData(camera);
	viewer.setUpViewInWindow(50, 50, WINDOW_WIDTH, WINDOW_HEIGHT);

	viewer.run();

	// TODO: Uncomment to see all the intermediate textures created internally.
	osgPango::Context::instance().writeCachesToPNGFiles("osgpangoviewer");
	
	return 0;
}
