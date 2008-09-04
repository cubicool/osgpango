#include <osg/Texture2D>
#include <osg/MatrixTransform>
#include <osgGA/StateSetManipulator>
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

osg::Camera* createOrthoCamera(unsigned int width, unsigned int height) {
	osg::Camera* camera = new osg::Camera();

	camera->getOrCreateStateSet()->setMode(
		GL_LIGHTING,
		osg::StateAttribute::PROTECTED | osg::StateAttribute::OFF
	);
	
	camera->setProjectionMatrix(osg::Matrix::ortho2D(0, width, 0, height));
	camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	camera->setViewMatrix(osg::Matrix::identity());
	camera->setClearMask(GL_DEPTH_BUFFER_BIT);
	camera->setRenderOrder(osg::Camera::POST_RENDER);
	
	return camera;
}

int main(int argc, char** argv) {
	osgPango::Font::init();

	const std::string font("Sans 28");

	osgPango::Font::create(font, 512, 64);

	osgPango::Text* t = new osgPango::Text(font);

	t->setText(LOREM_IPSUM);
	
	osgPango::GlyphCache* gc = osgPango::Font::getFont(font)->getGlyphCache();
	
	gc->writeImagesAsFiles("foo_");

	osgViewer::Viewer viewer;

	osg::Camera* camera = createOrthoCamera(1280, 1024);
	osg::Geode*  cairo  = new osg::Geode();
	
	osg::MatrixTransform* mt = new osg::MatrixTransform(
		osg::Matrix::translate(10.0f, 800.0f, 0.0f)
	);

	cairo->addDrawable(createGeometry(gc->getImage(0)));

	mt->addChild(t);

        viewer.addEventHandler(new osgViewer::StatsHandler());
        viewer.addEventHandler(new osgViewer::WindowSizeHandler());
        viewer.addEventHandler(new osgGA::StateSetManipulator(
                viewer.getCamera()->getOrCreateStateSet()
        ));

	if(camera && cairo) {
		camera->addChild(cairo);
		camera->addChild(mt);

		viewer.setSceneData(camera);

		viewer.run();
	}

	osgPango::Font::cleanup();
	osgPango::Text::cleanup();

	return 0;
}
