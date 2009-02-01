#include <osg/Texture2D>
#include <osg/MatrixTransform>
#include <osgGA/StateSetManipulator>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgPango/MultiText>

const std::string LOREM_IPSUM(
	"<span font='Calibri 8'>"
	"Lorem ipsum dolor sit amet, conse<span color='#fa0'>ctetur adipisicing</span> elit, sed do eiusmod "
	"</span>"
	"tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
	"quis <b>nostrud exercitation ullamco</b> laboris nisi ut aliquip ex ea commodo consequat. "
	"Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu "
	"fugiat nulla pariatur. Excepteur sint <i>occaecat cupidatat non proident</i>, sunt in "
	"<span font='Georgia 18'><b>"
	"culpa qui</b> officia deserunt mollit anim id est laborum."
	"</span>"
	"tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
	"quis <b>nostrud exercitation ullamco</b> laboris nisi ut aliquip ex ea commodo consequat. "
	"<span color='#acf' size='larger'>Duis aute irure dolor in reprehenderit</span> in voluptate velit esse cillum dolore eu "
);

const std::string LOREM_IPSUM_ALT(
	"<span font='Courier New 8'>"
	"Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod "
	"tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
	"quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. "
	"Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu "
	"fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in "
	"culpa qui officia deserunt mollit anim id est laborum."
	"tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
	"quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. "
	"Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu"
	"</span>"
);

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

	osgPango::Font::init();

	osgPango::MultiText* t = new osgPango::MultiText();

	// for(unsigned int i = 0; i < 20; i++) t->addText(LOREM_IPSUM, i * 50, i * 50);
	t->addText(LOREM_IPSUM, 300, 300);
	
	t->setMatrix(osg::Matrix::translate(0.0f, 0.0f, 0.0f));

	if(!t->finalize()) return 1;

	osg::Camera* camera = createOrthoCamera(1440, 900);
	
        viewer.addEventHandler(new osgViewer::StatsHandler());
        viewer.addEventHandler(new osgViewer::WindowSizeHandler());
        viewer.addEventHandler(new osgGA::StateSetManipulator(
                viewer.getCamera()->getOrCreateStateSet()
        ));

	camera->addChild(t);

	viewer.setSceneData(camera);
	viewer.setUpViewInWindow(50, 50, 1440, 900);
	viewer.getCamera()->setClearColor(osg::Vec4(0.3f, 0.3f, 0.3f, 1.0f));

	viewer.run();

	t->writeAllImages("osgpangomultitext");

	osgPango::Font::cleanup();
	osgPango::Text::cleanup();

	return 0;
}
