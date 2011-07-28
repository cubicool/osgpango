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
#include <osgPango/Text>
#include <osgPango/ShaderManager>

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

int main(int argc, char** argv) {
	osgViewer::Viewer viewer;

	osgPango::Context& context = osgPango::Context::instance();

	context.init();
	context.addGlyphRenderer("distancefield", new osgPango::GlyphRendererDistanceField(
		osgPango::GlyphRendererDistanceField::MODE_LARGE
	));

	osgPango::TextTransform* t = new osgPango::TextTransform();

	t->setGlyphRenderer("distancefield");
	t->addText("<span font='sans 128px'>Jeremy</span>");
	t->finalize();

	viewer.addEventHandler(new osgViewer::StatsHandler());
	viewer.addEventHandler(new osgViewer::WindowSizeHandler());
	viewer.addEventHandler(new osgGA::StateSetManipulator(
		viewer.getCamera()->getOrCreateStateSet()
	));

	bool perspective = false;

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
	
	return viewer.run();

	return 0;
}

