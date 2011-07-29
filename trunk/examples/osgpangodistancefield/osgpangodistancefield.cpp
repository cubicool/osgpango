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
#include <osgPango/DistanceFieldText>
#include <osgPango/ShaderManager>

const unsigned int          WINDOW_WIDTH  = 800;
const unsigned int          WINDOW_HEIGHT = 600;
const osg::Vec3::value_type SCALE_STEP    = 0.05f;

class ScaleSetHandler: public osgGA::GUIEventHandler {
public:
	osgPango::DistanceFieldText* getDistanceFieldText(osgGA::GUIActionAdapter& aa) {
		osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);

		if(!view) return 0;
		
		osg::Camera* camera = dynamic_cast<osg::Camera*>(view->getSceneData());

		if(!camera) return 0;
		
		osgPango::DistanceFieldText* text = dynamic_cast<osgPango::DistanceFieldText*>(
			camera->getChild(0)
		);

		if(!text) return 0;

		return text;
	}

	virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) {
		if(
			ea.getEventType() != osgGA::GUIEventAdapter::KEYDOWN ||
			(
				ea.getKey() != '=' &&
				ea.getKey() != '-'
			)
		) return false;

		osgPango::DistanceFieldText* text = getDistanceFieldText(aa);
		
		if(!text) return false;

		osg::Vec3::value_type scale = text->getScale();

		if(ea.getKey() == '=') scale += SCALE_STEP;

		else if(ea.getKey() == '-') scale -= SCALE_STEP;
		
		OSG_NOTICE << "Scale: " << scale << std::endl;

		text->setScale(scale);

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

int main(int argc, char** argv) {
	osgViewer::Viewer viewer;

	osgPango::Context& context = osgPango::Context::instance();

	context.init();

	osgPango::GlyphRenderer* dfgr = new osgPango::GlyphRendererDistanceField(
		osgPango::GlyphRendererDistanceField::MODE_LARGE
	);

	context.addGlyphRenderer("distancefield", dfgr);

	osgPango::DistanceFieldText* t = new osgPango::DistanceFieldText();

	t->setGlyphRenderer("distancefield");
	t->addText("<span font='sans 128px'>osg</span>");
	t->setCoordinateAlign(osgPango::TextTransform::COORDINATE_ALIGN_NONE);
	t->finalize();

	bool perspective = false;

	if(!perspective) {
		osg::Camera* camera = createOrthoCamera(WINDOW_WIDTH, WINDOW_HEIGHT);

		camera->addChild(t);

		viewer.setSceneData(camera);
	}

	else {
		t->setAxisAlignment(osgPango::DistanceFieldText::AXIS_ALIGN_XZ_PLANE);

		viewer.setSceneData(t);
	}

	viewer.setUpViewInWindow(50, 50, WINDOW_WIDTH, WINDOW_HEIGHT);
	viewer.addEventHandler(new ScaleSetHandler());
	viewer.addEventHandler(new osgViewer::StatsHandler());
	viewer.addEventHandler(new osgViewer::WindowSizeHandler());
	viewer.addEventHandler(new osgGA::StateSetManipulator(
		viewer.getCamera()->getOrCreateStateSet()
	));

	int r = viewer.run();

	context.writeCachesToPNGFiles("osgpangoviewer");

	return r;
}

