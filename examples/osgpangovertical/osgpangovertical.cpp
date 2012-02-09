// -*-c++-*- Copyright (C) 2010 osgPango Development Team
// $Id$

#include <osgGA/StateSetManipulator>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgPango/VerticalText>

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

	osgPango::VerticalText* t = new osgPango::VerticalText();
	// osgPango::TextTransform* t = new osgPango::TextTransform();

	t->setText("<span font='Sans Bold 30'>VERTICAL!!!</span>");
	t->finalize();
	t->setMatrix(osg::Matrix::translate(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f, 0.0f));
	t->setPositionAlignment(osgPango::TextTransform::POS_ALIGN_CENTER_CENTER);

	osgViewer::Viewer viewer;

	osg::Camera* camera = createOrthoCamera(WINDOW_WIDTH, WINDOW_HEIGHT);

	camera->addChild(t);

	viewer.addEventHandler(new osgViewer::StatsHandler());
	viewer.addEventHandler(new osgViewer::WindowSizeHandler());
	viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()));
	viewer.setSceneData(camera);
	viewer.getCamera()->setClearColor(osg::Vec4(0.5f, 0.5f, 0.5f, 1.0f));
	viewer.setUpViewInWindow(50, 50, WINDOW_WIDTH, WINDOW_HEIGHT);
	viewer.run();

	return 0;
}

