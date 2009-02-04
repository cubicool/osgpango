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
	"Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod "
	"tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
	"quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat."
	"Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu "
	"fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in "
	"culpa qui officia deserunt mollit anim id est laborum."
);

osg::Matrix createInvertedYOrthoProjectionMatrix(float width, float height) {
	osg::Matrix m = osg::Matrix::ortho2D(0.0f, width, 0.0f, height);
	osg::Matrix s = osg::Matrix::scale(1.0f, -1.0f, 1.0f);
	osg::Matrix t = osg::Matrix::translate(0.0f, -height, 0.0f);

	return t * s * m;
}

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

osg::Camera* createInvertedYOrthoCamera(float width, float height) {
	osg::Camera* camera = createOrthoCamera(width, height);

	camera->setProjectionMatrix(createInvertedYOrthoProjectionMatrix(width, height));

	return camera;
}

void setupArguments(osg::ArgumentParser& args) {
	args.getApplicationUsage()->setDescription(
		args.getApplicationName() + " is a quick font viewer application for OSG."
	);

	args.getApplicationUsage()->setCommandLineUsage(
		args.getApplicationName() + " [options] text..."
	);

	args.getApplicationUsage()->addCommandLineOption(
		"--renderer <string> <int>",
		"The GlyphRenderer object to use (outline, shadowOffset) and size."
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

	osgPango::Text* t = new osgPango::Text();

	osgPango::TextOptions to;

	while(args.read("--renderer", renderer, rendererSize)) {
		int s = std::atoi(rendererSize.c_str());

		if(renderer == "outline") context.addGlyphRenderer(
			"outline",
			new osgPango::GlyphRendererOutline(s)
		);

		else if(renderer == "shadowOffset") context.addGlyphRenderer(
			"shadowOffset",
			new osgPango::GlyphRendererShadowOffset(s, s)
		);

		else continue;

		osg::notify(osg::NOTICE) << "settng renderer: " << renderer << std::endl;

		t->setGlyphRenderer(renderer);
	}

	while(args.read("--alpha", alpha)) {
		float a = std::atof(alpha.c_str());

		t->setAlpha(a);
	}

	while(args.read("--width", width)) to.width = std::atoi(width.c_str());

	while(args.read("--alignment", alignment)) {
		if(alignment == "center") to.alignment = osgPango::TextOptions::ALIGN_CENTER;
		
		else if(alignment == "right") to.alignment = osgPango::TextOptions::ALIGN_RIGHT;
		
		else if(alignment == "justify") to.alignment = osgPango::TextOptions::ALIGN_JUSTIFY;
	}

	if(args.argc() >= 2) {
		text = "";

		for(int i = 1; i < args.argc(); i++) text += std::string(args[i]) + " ";
	}

	t->addText(text, 300, 300, to);
	t->setMatrix(osg::Matrix::translate(0, 0, 0)); //osg::Vec3(t->getOriginTranslated(), 0.0f)));

	if(!t->finalize()) return 1;

	osg::Group*  group  = new osg::Group();
	osg::Camera* camera = createOrthoCamera(1280, 1024);

	/*
	osg::MatrixTransform* mt = new osg::MatrixTransform(
		osg::Matrix::translate(osg::Vec3(t->getOriginTranslated(), 0.0f))
	);

	mt->addChild(t);
	*/

	viewer.addEventHandler(new osgViewer::StatsHandler());
	viewer.addEventHandler(new osgViewer::WindowSizeHandler());
	viewer.addEventHandler(new osgGA::StateSetManipulator(
		viewer.getCamera()->getOrCreateStateSet()
	));

	camera->addChild(t);

	group->addChild(camera);

	viewer.setSceneData(group);

	viewer.run();

	osgPango::Context::instance().writeCachesToPNGFiles("osgpangoviewer");
	
	return 0;
}
