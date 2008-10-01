#include <iostream>
#include <osg/Texture2D>
#include <osg/MatrixTransform>
#include <osgGA/StateSetManipulator>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgATK/EaseMotion>
#include <osgPango/Text>

struct GlyphSampler: public osg::Drawable::UpdateCallback {
	osgATK::LinearMotion _motion;

	float _direction;
	float _previous;
	
	GlyphSampler():
	_direction (1.0f),
	_previous  (0) {
	}

	void update(osg::NodeVisitor* nv , osg::Drawable* drawable) {
		const osg::FrameStamp* f = nv->getFrameStamp();

		float dt = f->getSimulationTime() - _previous;

		_previous = f->getSimulationTime();
		
		osg::Geometry* geom = dynamic_cast<osg::Geometry*>(drawable);

		if(!geom) return;
	
		_motion.update(dt * _direction * 4.0f);
		
		float val = _motion.getValue();

		if(val == 1.0f) _direction = -1.0f;

		if(val == 0.0f) _direction = 1.0f;

		osg::Vec3Array* verts = dynamic_cast<osg::Vec3Array*>(geom->getVertexArray());

		if(!verts) return;

		// We don't have any value to interpolate too, so we just use some function
		// of the framerate. :) ACK!!!
		float mod = (val * _direction) * 0.9f;

		for(unsigned int g = 0; g < verts->size(); g += 4) {
			(*verts)[g    ].x() += mod;
			(*verts)[g + 1].y() += mod;
			(*verts)[g + 2].x() += mod;
			(*verts)[g + 3].y() += mod;
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
	osgPango::Font::init();

	const std::string font("Jellyka Castle's Queen 100");

	osgPango::GlyphCache* cache = new osgPango::GlyphCacheShadowOffset(512, 512, 2);

	osgPango::Font* f = new osgPango::Font(font, cache);
	osgPango::Text* t = new osgPango::Text(f);

	t->setColor(osg::Vec3(0.9f, 0.1f, 0.1f));
	t->setEffectsColor(osg::Vec3(1.0f, 1.0f, 1.0f));
	t->setText("ripley");
	t->getDrawable(0)->setUpdateCallback(new GlyphSampler());

	osgViewer::Viewer viewer;

	osg::Camera* camera = createOrthoCamera(720, 480);
	
	osg::MatrixTransform* mt = new osg::MatrixTransform(
		osg::Matrix::translate(200.0f, 300.0f, 0.0f)
	);

	mt->addChild(t);

        viewer.addEventHandler(new osgViewer::StatsHandler());
        viewer.addEventHandler(new osgViewer::WindowSizeHandler());
        viewer.addEventHandler(new osgGA::StateSetManipulator(
                viewer.getCamera()->getOrCreateStateSet()
        ));

	camera->addChild(mt);

	viewer.setUpViewInWindow(0, 0, 720, 480);
	viewer.setSceneData(camera);
	viewer.getCamera()->setClearColor(osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f));

	viewer.run();

	osgPango::Font::cleanup();
	osgPango::Text::cleanup();

	return 0;
}
