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

	// osgPango::GlyphCache* cache = new osgPango::GlyphCacheShadowOffset(1024, 128, 2);
	osgPango::GlyphCache* cache1 = new osgPango::GlyphCacheShadowGaussian(1024, 128, 10);
	osgPango::GlyphCache* cache2 = new osgPango::GlyphCacheOutline(1024, 128, 4);

	osgPango::Font* f1 = new osgPango::Font("Sans Bold 102", cache1);
	osgPango::Font* f2 = new osgPango::Font("Osaka-Sans Serif 62", cache2);
	osgPango::Text* t1 = new osgPango::Text(f1);
	osgPango::Text* t2 = new osgPango::Text(f2);

	t1->setColor(osg::Vec3(1.0f, 1.0f, 1.0f));
	t1->setEffectsColor(osg::Vec3(0.0f, 0.0f, 0.0f));
	t1->setAlpha(0.7f);
	t1->setAlignment(osgPango::Text::ALIGN_LEFT);
	t1->setWidth(1100);
	t1->setText("this is a TEST");

	t2->setColor(osg::Vec3(0.7f, 0.8f, 1.0f));
	t2->setEffectsColor(osg::Vec3(0.0f, 0.0f, 0.0f));
	t2->setAlpha(0.7f);
	t2->setAlignment(osgPango::Text::ALIGN_RIGHT);
	t2->setWidth(800);
	t2->setText("this is a TEST, please do not adjust your dial.");

	cache1->getImage(0, true)->gaussianBlur(10);

	/*
	// ----------------------------------------------------------------------------------------
	const osg::Vec2& size   = t1->getSize();
	const osg::Vec2& origin = t1->getOrigin();

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

	// f1->getGlyphCache()->writeImagesAsFiles("foo_");

	osgViewer::Viewer viewer;

	osg::Group*  group  = new osg::Group();
	osg::Camera* camera = createOrthoCamera(1280, 1024);
	osg::Node*   node   = osgDB::readNodeFile("dumptruck.osg");
	
	osg::MatrixTransform* mt1 = new osg::MatrixTransform(
		osg::Matrix::translate(osg::Vec3(t1->getOriginTranslated(), 0.0f))
	);

	osg::MatrixTransform* mt2 = new osg::MatrixTransform(
		osg::Matrix::translate(osg::Vec3(t2->getOriginTranslated(), -0.1f))
	);

	mt1->addChild(t1);
	mt2->addChild(t2);

        viewer.addEventHandler(new osgViewer::StatsHandler());
        viewer.addEventHandler(new osgViewer::WindowSizeHandler());
        viewer.addEventHandler(new osgGA::StateSetManipulator(
                viewer.getCamera()->getOrCreateStateSet()
        ));

	camera->addChild(mt1);
	camera->addChild(mt2);

	group->addChild(node);
	group->addChild(camera);

	viewer.setSceneData(group);
	viewer.getCamera()->setClearColor(osg::Vec4(0.3f, 0.3f, 0.3f, 1.0f));

	viewer.run();

	osgPango::Font::cleanup();
	osgPango::Text::cleanup();

	return 0;
}
