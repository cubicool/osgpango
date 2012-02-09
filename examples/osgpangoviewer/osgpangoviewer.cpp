// -*-c++-*- Copyright (C) 2010 osgPango Development Team
// $Id$

#include <iostream>
#include <sstream>
#include <osg/io_utils>
#include <osg/ArgumentParser>
#include <osg/Texture2D>
#include <osg/MatrixTransform>
#include <osgGA/StateSetManipulator>
#include <osgDB/FileUtils>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgPango/TextTransform>
#include <osgPango/ShaderManager>

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
		"--renderer <string> <int,int,int,int>",
		"The GlyphRenderer object to use (outline, shadow, shadowBlur, shadowInset) and sizes. "
		"Note that not all renderers need--or require--four arguments."
	);

	args.getApplicationUsage()->addCommandLineOption(
		"--bitmap <image>",
		"Use the specified image to fill the glyph face."
	);

	args.getApplicationUsage()->addCommandLineOption(
		"--bevel",
		"Apply a simple bevel (TESTING!)"
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
		"--alignment <alignment>",
		"The Pango alignment; one of left, right, center, or justify."
	);

	args.getApplicationUsage()->addCommandLineOption(
		"--perspective",
		"Put the text into a 3D scene, instead of a 2D Ortho 'HUD' camera."
	);

	args.getApplicationUsage()->addCommandLineOption(
		"--ati",
		"DEBUG: Use the custom ATI shaders; this is TEMPORARY!"
	);

	args.getApplicationUsage()->addCommandLineOption(
		"--dump-textures",
		"Dump all of the GlyphLayer textures used for every font created."
	);
}

std::string boringStringParsing(osg::ArgumentParser& args) {
	std::string str(LOREM_IPSUM);

	// Construct our string to display.
	if(args.argc() >= 2) {
		str = "";

		for(int i = 1; i < args.argc(); i++) {
			str += std::string(args[i]);
			
			if(i != args.argc()) str += " ";
		}
	}

	size_t             pos = 0;
	std::ostringstream os;

	while(true) {
		size_t p = str.find("\\n", pos);

		if(p == std::string::npos) {
			os << str.substr(pos, std::string::npos);

			break;
		}

		os << str.substr(pos, p - pos) << std::endl;

		pos = p + 2;
	}

	return os.str();
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

	// All of our temporary variables.
	std::string renderer, rendererSize, alpha, alignment, width, image;

	bool perspective  = false;
	bool dumpTextures = false;
	bool bevel        = false;

	osgPango::Context& context = osgPango::Context::instance();

	context.init();

	osgPango::TextTransform* t = new osgPango::TextTransform();

	osgPango::TextOptions to;

	while(args.read("--renderer", renderer, rendererSize)) {
		int          arg1, arg2 = 0;
		unsigned int arg3, arg4 = 0;

		std::sscanf(rendererSize.c_str(), "%i,%i,%u,%u", &arg1, &arg2, &arg3, &arg4);

		osgPango::GlyphRenderer* r = 0;
		
		if(renderer == "outline") r = new osgPango::GlyphRendererOutline(arg1);

		else if(renderer == "shadow") r = new osgPango::GlyphRendererShadow(arg1, arg2);

		else if(renderer == "shadowBlur") r = new osgPango::GlyphRendererShadowBlur(
			arg1,
			arg2,
			arg3,
			arg4
		);

		else if(renderer == "shadowInset") r = new osgPango::GlyphRendererShadowInset(
			arg1,
			arg2,
			arg3,
			arg4
		);

		else {
			OSG_NOTICE << "Bad renderer: " << renderer << std::endl;

			continue;
		}

		if(r) context.addGlyphRenderer(renderer, r);
	}

	while(args.read("--bitmap", image)) {}
	
	while(args.read("--bevel")) bevel = true;

	while(args.read("--perspective")) perspective = true;
	
	while(args.read("--dump-textures")) dumpTextures = true;

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

	while(args.read("--ati")) {
		osgDB::FilePathList& paths = osgDB::getDataFilePathList();
	
		paths.push_back("../examples/osgpangoviewer/");
		paths.push_back("examples/osgpangoviewer/");
		paths.push_back(".");
		
		osgPango::ShaderManager& sm = osgPango::ShaderManager::instance();

		// Overwrite the NVIDIA-centric shaders with shaders that are customized
		// for ATI cards. THIS IS TEMPORARY! A dual-card generated shader is going
		// to be required...
		sm.addShaderFile("osgPango-frag1", osg::Shader::FRAGMENT, "osgPango-frag1-ATI.glsl");
		sm.addShaderFile("osgPango-frag2", osg::Shader::FRAGMENT, "osgPango-frag2-ATI.glsl");
	}

	std::string text = boringStringParsing(args);

	if(!image.empty() || bevel) {
		osgPango::GlyphRenderer* r = context.getGlyphRenderer(renderer);
		osgPango::GlyphLayer*    l = 0;

		if(!bevel) l = new osgPango::GlyphLayerBitmap(image);

		else l = new osgPango::GlyphLayerBevel(
			15.0f,
			1.0f,
			osg::DegreesToRadians(35.0f),
			osg::DegreesToRadians(35.0f),
			90.0f,
			0.1f,
			0.9f
		);

		if(r) r->replaceLayer(0, l);

		text = "<span font='Sans Bold 150px'>Bevel\nExample</span>";
	}

	// The user didn't set a width, so use our screen size.
	if(to.width <= 0) to.width = WINDOW_WIDTH;

	t->setGlyphRenderer(renderer);
	t->setText(text, to);
	
	// TODO: Continue working on this API. :)
	// t->setScale(2);

	if(!t->finalize()) return 1;

	viewer.addEventHandler(new osgViewer::StatsHandler());
	viewer.addEventHandler(new osgViewer::WindowSizeHandler());
	viewer.addEventHandler(new osgGA::StateSetManipulator(
		viewer.getCamera()->getOrCreateStateSet()
	));

	if(!perspective) {
		osg::Camera* camera = createOrthoCamera(WINDOW_WIDTH, WINDOW_HEIGHT);

		camera->addChild(t);

		viewer.setSceneData(camera);
	}

	else {
		t->setAxisAlignment(osgPango::TextTransform::AXIS_ALIGN_XZ_PLANE);

		viewer.setSceneData(t);
	}

	viewer.setUpViewInWindow(50, 50, WINDOW_WIDTH, WINDOW_HEIGHT);

	// Set the window name.
	osgViewer::Viewer::Windows windows;
	
	viewer.getWindows(windows);

	windows[0]->setWindowName("osgpangoviewer");

	// Run the viewer until ESC is pressed.
	viewer.run();

	// if(dumpTextures) context.writeCachesToPNGFiles("osgpangoviewer");

	return 0;
}
