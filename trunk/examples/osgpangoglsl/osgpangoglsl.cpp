// -*-c++-*- Copyright (C) 2010 osgPango Development Team
// $Id$

#include <osgGA/StateSetManipulator>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgPango/TextTransform>
#include <osgPango/ShaderGenerator>
#include <osgPango/ShaderManager>

struct GlyphRendererMultiOutline: public osgPango::GlyphRenderer {
	GlyphRendererMultiOutline(bool useCustomLayer = false) {
		addLayer(new osgPango::GlyphLayer());
		addLayer(new osgPango::GlyphLayerOutline(1.0f));
		addLayer(new osgPango::GlyphLayerOutline(3.0f));
		addLayer(new osgPango::GlyphLayerOutline(4.0f));

		osgPango::ShaderManager::instance().addShaderFile(
			"my-shader",
			osg::Shader::FRAGMENT,
			"../examples/osgpangoglsl/osgpangoglsl.glsl"
		);
	}

	bool updateOrCreateState(int pass, osg::Geode* geode) const {
		if(!GlyphRenderer::updateOrCreateState(pass, geode)) return false;

		osg::StateSet* state = geode->getOrCreateStateSet();
		
		osg::Program* program = dynamic_cast<osg::Program*>(
			state->getAttribute(osg::StateAttribute::PROGRAM)
		);

		if(!program) return false;
		
		state->getOrCreateUniform("pangoNumLayers", osg::Uniform::INT)->set(4);

		program->addShader(osgPango::ShaderManager::instance().getShader("my-shader"));

		return true;
	}
};

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

const int WINDOW_WIDTH  = 800;
const int WINDOW_HEIGHT = 600;

int main(int argc, char** argv) {
	osgPango::Context& context = osgPango::Context::instance();

	context.init();
	context.addGlyphRenderer("multioutline", new GlyphRendererMultiOutline());

	osgPango::TextTransform* t = new osgPango::TextTransform(osgPango::Text::COLOR_MODE_PALETTE_ONLY);

	osgPango::ColorPalette cp;

	cp.push_back(osg::Vec3(1.0f, 1.0f, 0.0f));
	cp.push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
	cp.push_back(osg::Vec3(1.0f, 1.0f, 1.0f));
	cp.push_back(osg::Vec3(0.0f, 0.0f, 0.0f));

	t->setAlpha(1.0f);
	t->setColorPalette(cp);
	t->setGlyphRenderer("multioutline");
	t->addText(
		"<span font='Cheri Liney 70'>I've got\na lovely bunch\nof coconuts!!!</span>"
		// "<span font='Sans 70'>I've got\na lovely bunch\nof coconuts!!!</span>",
	);

	t->finalize();
	t->setMatrix(osg::Matrixd::translate(osg::Vec3(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, 0.0f)));
	t->setPositionAlignment(osgPango::TextTransform::POS_ALIGN_CENTER_CENTER);
	// t->setCoordinateAlign(osgPango::TextTransform::COORDINATE_ALIGN_ALWAYS);

	osgViewer::Viewer viewer;

	osg::Group*  group  = new osg::Group();
	osg::Camera* camera = createOrthoCamera(WINDOW_WIDTH, WINDOW_HEIGHT);
	osg::Node*   node   = osgDB::readNodeFile("cow.osg");
	
        viewer.addEventHandler(new osgViewer::StatsHandler());
        viewer.addEventHandler(new osgViewer::WindowSizeHandler());
        viewer.addEventHandler(new osgGA::StateSetManipulator(
                viewer.getCamera()->getOrCreateStateSet()
        ));

	camera->addChild(t);

	group->addChild(node);
	group->addChild(camera);

	viewer.setSceneData(group);
	viewer.getCamera()->setClearColor(osg::Vec4(0.5f, 0.5f, 0.5f, 1.0f));
	viewer.setUpViewInWindow(50, 50, WINDOW_WIDTH, WINDOW_HEIGHT);

	viewer.run();

	// osgPango::Context::instance().writeCachesToPNGFiles("osgpangotest");
	
	return 0;
}
