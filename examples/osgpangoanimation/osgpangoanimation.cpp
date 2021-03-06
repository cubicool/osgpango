// -*-c++-*- Copyright (C) 2010 osgPango Development Team
// $Id$

#include <iostream>
#include <osg/io_utils>
#include <osg/Texture2D>
#include <osg/MatrixTransform>
#include <osgGA/StateSetManipulator>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgAnimation/EaseMotion>
#include <osgPango/TextTransform>

#include <stdlib.h>

const unsigned int WINDOW_WIDTH  = 720;
const unsigned int WINDOW_HEIGHT = 480;

// A lot of this code is by:
// 
// 	Cedric Pinson <mornifle@plopbyte.net>
//
// ...who is also the author of osgAnimation. Many thanks, Ced. :)
struct GlyphSampler: public osg::Drawable::UpdateCallback {
	// typedef osgAnimation::OutBounceMotion MyMotion;
	typedef osgAnimation::OutCubicMotion MyMotion;

	float _previous;

	osg::ref_ptr<osg::Vec3Array> _originals;
	std::vector<MyMotion>        _motions;

	GlyphSampler():
	_previous(0) {
	}

	void update(osg::NodeVisitor* nv , osg::Drawable* drawable) {
		static float mod = 8.0f;

		if(nv->getVisitorType() != osg::NodeVisitor::UPDATE_VISITOR) return;

		osg::Geometry* geom = dynamic_cast<osg::Geometry*>(drawable);
		
		if(!geom) return;
		
		osg::Vec3Array* verts = dynamic_cast<osg::Vec3Array*>(geom->getVertexArray());
		
		if(!verts) return;

		if(!_originals.valid()) {
			_originals = new osg::Vec3Array(*verts);

			_motions.resize(_originals->size() / 4);
			
			for (unsigned int i = 0; i < _motions.size(); i++) {
				float duration = 1;
				
				_motions[i] = MyMotion(0, duration, 3.14f, osgAnimation::Motion::LOOP);
				
				float offset = (rand() * 1.0f / (1.0f * RAND_MAX)) * duration;
				
				_motions[i].setTime(offset);
			}
		}

		const osg::FrameStamp* f = nv->getFrameStamp();

		double t  = f->getSimulationTime();
		float  dt = t - _previous;

		_previous = t;

		for(unsigned int g = 0; g < verts->size(); g += 4) {
			_motions[g / 4].update(dt);

			float val = _motions[g / 4].getValue();
		
			(*verts)[g    ].y() = (*_originals)[g + 0].y() - mod * sin(val);
			(*verts)[g + 1].y() = (*_originals)[g + 1].y() - mod * sin(val);
			(*verts)[g + 2].y() = (*_originals)[g + 2].y() + mod * sin(val);
			(*verts)[g + 3].y() = (*_originals)[g + 3].y() + mod * sin(val);

			(*verts)[g    ].x() = (*_originals)[g + 0].x() - mod * sin(val);
			(*verts)[g + 1].x() = (*_originals)[g + 1].x() + mod * sin(val);
			(*verts)[g + 2].x() = (*_originals)[g + 2].x() + mod * sin(val);
			(*verts)[g + 3].x() = (*_originals)[g + 3].x() - mod * sin(val);
		}
	}
};

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

int main(int argc, char** argv) {
	osgPango::Context& context = osgPango::Context::instance();

	context.init();
	context.addGlyphRenderer("outline", new osgPango::GlyphRendererOutline(2));

	osgPango::TextTransform* t = new osgPango::TextTransform();

	t->setGlyphRenderer("outline");
	t->setText(
		"<span font='Georgia Bold 50' color='black' bgcolor='white'>"
		"osgPango\nand\nosgAnimation"
		"</span>"
	);

	t->finalize();
	
	// TODO: OMG, this is horrible. :)
	osg::Group* group = dynamic_cast<osg::Group*>(t->getChild(0));
	
	if(group) {
		osg::Geode* geode = dynamic_cast<osg::Geode*>(group->getChild(0)); 

	
		if(geode) geode->getDrawable(0)->setUpdateCallback(new GlyphSampler());
	}

	osgViewer::Viewer viewer;

	osg::Camera* camera = createOrthoCamera(WINDOW_WIDTH, WINDOW_HEIGHT);
	
	const osg::Vec2& size = t->getSize();

	// t->setPosition(osg::Vec3(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, 0.0f));
	// t->setAlignment(osgPango::TextTransform::POS_ALIGN_CENTER);

	viewer.addEventHandler(new osgViewer::StatsHandler());
	viewer.addEventHandler(new osgViewer::WindowSizeHandler());
	viewer.addEventHandler(new osgGA::StateSetManipulator(
		viewer.getCamera()->getOrCreateStateSet()
	));

	camera->addChild(t);

	viewer.setUpViewInWindow(50, 50, WINDOW_WIDTH, WINDOW_HEIGHT);
	viewer.setSceneData(camera);
	viewer.getCamera()->setClearColor(osg::Vec4(0.2f, 0.2f, 0.2f, 1.0f));

	viewer.run();

	return 0;
}
