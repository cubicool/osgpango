#include <iostream>
#include <osg/io_utils>
#include <osg/Texture2D>
#include <osg/MatrixTransform>
#include <osgGA/StateSetManipulator>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgWidget/Widget>
#include <osgPango/Text>

const std::string LOREM_IPSUM(
	"Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod "
	"tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
	"quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. "
	"Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu "
	"fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in "
	"culpa qui officia deserunt mollit anim id est laborum."
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

int main(int argc, char** argv) {
	osgPango::Font::init();

	// const std::string font("Aurulent Sans Mono Bold 50");
	// const std::string font("Osaka-Sans Serif Bold 120");
	const std::string font("Sans Bold 120");

	// osgPango::GlyphCache* cache = new osgPango::GlyphCacheShadowOffset(1024, 128, 2);
	// osgPango::GlyphCache* cache = new osgPango::GlyphCacheShadowGaussian(1024, 128, 10);
	osgPango::GlyphCache* cache = new osgPango::GlyphCacheOutline(1024, 128, 10);
	// osgPango::GlyphCache* cache = 0;

	osgPango::Font* f = new osgPango::Font(font, cache);
	osgPango::Text* t = new osgPango::Text(f);

	t->setColor(osg::Vec3(1.0f, 1.0f, 1.0f));
	t->setEffectsColor(osg::Vec3(0.0f, 0.0f, 0.0f));
	t->setAlpha(0.5f);
	t->setAlignment(osgPango::Text::ALIGN_LEFT);
	t->setWidth(1100);
	//t->setText(LOREM_IPSUM);
	t->setText("this is a TEST");

	// cache->getImage(0, true)->gaussianBlur(10);

	// ----------------------------------------------------------------------------------------
	const osg::Vec2& size   = t->getSize();
	const osg::Vec2& origin = t->getOrigin();

	std::cout << "size: "   << size << std::endl;
	std::cout << "origin: " << origin << std::endl;

	/*
	osgWidget::Widget* wi = new osgWidget::Widget("", size.x(), size.y());
	
	wi->setColor(1.0f, 1.0f, 1.0f, 1.0f);

	float x = origin.x();
	float y = -(size.y() + origin.y());
	float w = size.x();
	float h = size.y();
	float z = -1.0f;

	osg::Vec3Array* v = dynamic_cast<osg::Vec3Array*>(wi->getVertexArray());

	(*v)[0].set(x,     y,     z);
	(*v)[1].set(x + w, y,     z);
	(*v)[2].set(x + w, y + h, z);
	(*v)[3].set(x,     y + h, z);

	t->addDrawable(wi);
	// ----------------------------------------------------------------------------------------
	*/

	f->getGlyphCache()->writeImagesAsFiles("foo_");

	osgViewer::Viewer viewer;

	osg::Group*  group  = new osg::Group();
	osg::Camera* camera = createOrthoCamera(1280, 1024);
	osg::Node*   node   = osgDB::readNodeFile("cow.osg");
	
	osg::MatrixTransform* mt = new osg::MatrixTransform(
		osg::Matrix::translate(osg::Vec3(t->getOriginTranslated(), 0.0f))
	);

	mt->addChild(t);

        viewer.addEventHandler(new osgViewer::StatsHandler());
        viewer.addEventHandler(new osgViewer::WindowSizeHandler());
        viewer.addEventHandler(new osgGA::StateSetManipulator(
                viewer.getCamera()->getOrCreateStateSet()
        ));

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
