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
	osgPango::DistanceFieldText* asText(osg::Node* potentialText) {
		return dynamic_cast<osgPango::DistanceFieldText*>(potentialText);
	}

	osgPango::DistanceFieldText* getDistanceFieldText(osgGA::GUIActionAdapter& aa) {
		osgPango::DistanceFieldText* text = 0;

		osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);

		if(!view) return 0;
		
		osg::Camera* camera = dynamic_cast<osg::Camera*>(view->getSceneData());

		// If the camera isn't our toplevel object, maybe it's the text itself.
		if(!camera) text = asText(view->getSceneData());
		
		else text = dynamic_cast<osgPango::DistanceFieldText*>(camera->getChild(0));

		return text;
	}

	virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) {
		if(
			ea.getEventType() != osgGA::GUIEventAdapter::KEYDOWN ||
			(
				ea.getKey() != osgGA::GUIEventAdapter::KEY_Up &&
				ea.getKey() != osgGA::GUIEventAdapter::KEY_Down
			)
		) return false;

		osgPango::DistanceFieldText* text = getDistanceFieldText(aa);
		
		if(!text) return false;

		osg::Vec3::value_type scale = text->getScale();

		if(ea.getKey() == osgGA::GUIEventAdapter::KEY_Up) scale += SCALE_STEP;

		else if(ea.getKey() == osgGA::GUIEventAdapter::KEY_Down) scale -= SCALE_STEP;
		
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

	// These arguments are VERY IMPORTANT in the generation of distance field text.
	context.addGlyphRenderer("distancefield", new osgPango::GlyphRendererDistanceField(
		// The first argument is the "scan size", which determines the range of the 
		// distance field. A larger value means that smoothing can occur at larger scales,
		// but will usually require more padding.
		100,
		// The second argument is the "block size", which is essentially the scale of the
		// glyph. Internally, the glyph surface will be scaled up by this value and then
		// passed to the distance field generation algorighthm. Higher values produce
		// larger--but higher quality--text.
		32,
		// This is the padding value each glyph recieves. It will determine largely on the
		// two previous values, in addition to the size of the actual font being rendered.
		3.0f,
		// Finally, this value represents the scale denominator that will be used in the
		// shader's smoothstepping min and max values. In 2D orthographic projections,
		// there is a relationship between the block_size and this value; in 3D
		// projections (currently unsupported), the relationship will be derived form the
		// distance of the viewer to the DistanceFieldText object.
		2.0f
	));

	osgPango::DistanceFieldText* t = new osgPango::DistanceFieldText();

	t->setGlyphRenderer("distancefield");
	t->setText("<span font='sans 64px'>Up/Down Arrow Keys</span>");
	t->setCoordinateAlign(osgPango::TextTransform::COORDINATE_ALIGN_NONE);
	t->setMatrix(osg::Matrix::translate(20.0f, 20.0f, 0.0f));
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

	// context.writeCachesToPNGFiles("osgpangodistancefield");

	return r;
}

