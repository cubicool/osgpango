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
	"<span color='white' font='monospace 6'>"
	"Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod\n"
	"tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam,\n"
	"quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.\n"
	"Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu\n"
	"<b>fugiat</b> nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in\n"
	"culpa qui officia deserunt mollit anim id est laborum."
	"</span>"
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
		"--font '<string>'",
		"A proper Pango font description; 'Sans 20', 'Monospace Bold 10', etc."
	);

	args.getApplicationUsage()->addCommandLineOption(
		"--cache <string> <int>",
		"The GlyphCache object to use (outline, shadowOffset) and size."
	);

	args.getApplicationUsage()->addCommandLineOption(
		"--color <float> <float> <float>",
		"The RGB color of the font."
	);

	args.getApplicationUsage()->addCommandLineOption(
		"--effectsColor <float> <float> <float>",
		"The RGB color of the effect (if present)."
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

int mainTest(int, char**) {
	osgPango::Context& context = osgPango::Context::instance();
	
	context.init();
	context.addGlyphRenderer("shadowOffset-1", new osgPango::GlyphRendererShadowOffset(1, 1));
	context.addGlyphRenderer("shadowOffset-2", new osgPango::GlyphRendererShadowOffset(2, 2));
	context.addGlyphRenderer("outline-1", new osgPango::GlyphRendererOutline(1));
	context.addGlyphRenderer("outline-2", new osgPango::GlyphRendererOutline(2));
	
	return 0;
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

	std::string font("Sans 10");
	std::string text(LOREM_IPSUM);

	// All of our temporary variables.
	std::string cache, cacheSize, red, green, blue, alpha, alignment, width;

	osgPango::Context::instance().init(132);

	/*
	osgPango::GlyphCache* c = 0;

	while(args.read("--font", font)) {};

	while(args.read("--cache", cache, cacheSize)) {
		int s = std::atoi(cacheSize.c_str());

		if(cache == "outline") c = new osgPango::GlyphCacheOutline(1024, 128, s);
		
		else if(cache == "shadowOffset") c = new osgPango::GlyphCacheShadowOffset(
			1024,
			128,
			s,
			s
		);
	}
	*/

	osgPango::Text* t = new osgPango::Text();

	/*
	while(args.read("--color", red, green, blue)) {
		float r = std::atof(red.c_str());
		float g = std::atof(green.c_str());
		float b = std::atof(blue.c_str());

		t->setColor(osg::Vec3(r, g, b));
	}

	while(args.read("--effectsColor", red, green, blue)) {
		float r = std::atof(red.c_str());
		float g = std::atof(green.c_str());
		float b = std::atof(blue.c_str());

		t->setEffectsColor(osg::Vec3(r, g, b));
	}

	while(args.read("--alpha", alpha)) {
		float a = std::atof(alpha.c_str());

		t->setAlpha(a);
	}

	while(args.read("--width", width)) {
		int w = std::atoi(width.c_str());

		t->setWidth(w);
	}

	while(args.read("--alignment", alignment)) {
		if(alignment == "center") t->setAlignment(osgPango::Text::ALIGN_CENTER);
		
		else if(alignment == "right") t->setAlignment(osgPango::Text::ALIGN_RIGHT);
		
		else if(alignment == "justify") t->setAlignment(osgPango::Text::ALIGN_JUSTIFY);
	}

	if(args.argc() >= 2) {
		text = "";

		for(int i = 1; i < args.argc(); i++) text += std::string(args[i]) + " ";
	}

	t->setText(text);

	f->getGlyphCache()->writeImagesAsFiles("osgpangoviewer");
	*/

	t->addText(text, 300, 300);
	t->setMatrix(osg::Matrix::translate(0, 0, 0)); //osg::Vec3(t->getOriginTranslated(), 0.0f)));

	if(!t->finalize()) return 1;

	osg::Group*  group  = new osg::Group();
	osg::Camera* camera = createOrthoCamera(1280, 1024);
	osg::Node*   node   = osgDB::readNodeFile("cow.osg");

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

	group->addChild(node);
	group->addChild(camera);

	viewer.setSceneData(group);

	viewer.run();

	osgPango::Context::instance().writeCachesToPNGFiles("osgpangoviewer");
	
	return 0;
}
