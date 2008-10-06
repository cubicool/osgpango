#include <osg/Texture2D>
#include <osg/MatrixTransform>
#include <osgGA/StateSetManipulator>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgPango/Text>

const std::string LOREM_IPSUM(
	"Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod "
	"tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
	"quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. "
	"Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu "
	"fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in "
	"culpa qui officia deserunt mollit anim id est laborum."
);

class GlyphCacheGradient: public osgPango::GlyphCache {
	virtual bool renderGlyph(
		osgCairo::SurfaceImage* si,
		const osgCairo::Glyph&  g,
		unsigned int            w,
		unsigned int            h
	) {
		si->setLineWidth(1.5f);
		si->glyphPath(g);
		
		osgCairo::LinearPattern lp(w / 2.0f, 0.0f, w / 2.0f, h);

		lp.addColorStopRGBA(0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
		lp.addColorStopRGBA(0.8f, 0.0f, 0.0f, 0.0f, 0.0f);

		si->setSource(&lp);
		si->fill();

		return true;
	}
};

osg::Geometry* createGeometry(osg::Image* image) {
	static osg::Vec3 pos(50.0f, 50.0f, -0.8f);

	osg::Texture2D* texture = new osg::Texture2D();
	osg::Geometry*  geom    = osg::createTexturedQuadGeometry(
		pos,
		osg::Vec3(image->s(), 0.0f, 0.0f),
		osg::Vec3(0.0f, image->t(), 0.0f),
		0.0f,
		0.0f, 
		1.0f,
		1.0f
	);

	texture->setImage(image);
	texture->setDataVariance(osg::Object::DYNAMIC);

	osg::StateSet* state = geom->getOrCreateStateSet();

	state->setTextureAttributeAndModes(
		0,
		texture,
		osg::StateAttribute::ON
	);

	state->setMode(GL_BLEND, osg::StateAttribute::ON);
	state->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

	pos += osg::Vec3(image->s() + 50.0f, 0.0f, 0.1f);

	return geom;
}

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

	const std::string font("iNked God 45");

	osgPango::GlyphCache* cache = new GlyphCacheGradient();

	osgPango::Font* f = new osgPango::Font(font, cache);
	osgPango::Text* t = new osgPango::Text(f);

	t->setColor(osg::Vec3(0.0f, 0.0f, 0.2f));
	t->setWidth(900);
	t->setText(LOREM_IPSUM);

	osgPango::GlyphCache* gc = f->getGlyphCache();
	
	gc->writeImagesAsFiles("osgpangocustomrenderer");

	osgViewer::Viewer viewer;

	osg::Group*  group  = new osg::Group();
	osg::Camera* camera = createOrthoCamera(1280, 1024);
	osg::Node*   node   = osgDB::readNodeFile("cow.osg");
	osg::Geode*  cairo  = new osg::Geode();
	
	osg::MatrixTransform* mt = new osg::MatrixTransform(
		osg::Matrix::translate(100.0f, 1000.0f, 0.0f)
	);

	cairo->addDrawable(createGeometry(gc->getImage(0)));

	mt->addChild(t);

        viewer.addEventHandler(new osgViewer::StatsHandler());
        viewer.addEventHandler(new osgViewer::WindowSizeHandler());
        viewer.addEventHandler(new osgGA::StateSetManipulator(
                viewer.getCamera()->getOrCreateStateSet()
        ));

	camera->addChild(cairo);
	camera->addChild(mt);

	group->addChild(node);
	group->addChild(camera);

	viewer.setSceneData(group);
	viewer.getCamera()->setClearColor(osg::Vec4(0.3f, 0.3f, 0.3f, 1.0f));

	viewer.run();

	osgPango::Font::cleanup();
	osgPango::Text::cleanup();

	return 0;
}
