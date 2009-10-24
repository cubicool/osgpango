#include <sstream>
#include <osg/Node>
#include <osg/Timer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/StateSetManipulator>
#include <osgPango/Context>

class UpdateCallback: public osg::NodeCallback {
public:
	UpdateCallback() :
	_time(new osg::Timer),
	_done(false) {
		_start = _time->time_s();
	}

	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv) {
		osg::Camera* camera = dynamic_cast<osg::Camera*>(node);
		
		if(!camera) return;

		if(_start > 5) _done = true;

		else if(_done == false && _time->time_s() - _start > 1.0f) {
			_start = _time->time_s();

			osgPango::TextTransform* oldTransform = dynamic_cast<osgPango::TextTransform*>(camera->getChild(0));

			if(!oldTransform) return;

			std::ostringstream os;
				
			unsigned int size = static_cast<unsigned int>((_start + 2.0f) * 20.0f);

			os << "<span font='sans " << size << "'>Sans " << size << "</span>";

			oldTransform->addText(os.str().c_str(), 0, 0);
			oldTransform->setPosition(oldTransform->getOriginTranslated());
			oldTransform->finalize();
		}

		traverse(node, nv);
	}

private:
	osg::Timer* _time;
	double      _start;
	bool        _done;
};

int main(int ac, char **av) {
	osgViewer::Viewer viewer;

	viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()));
	viewer.addEventHandler(new osgViewer::StatsHandler());
	viewer.addEventHandler(new osgViewer::WindowSizeHandler);
	viewer.setUpViewInWindow(100, 100, 800, 600);

	osg::ref_ptr<osg::Camera> camera = new osg::Camera();
	
	camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	camera->setViewMatrix(osg::Matrix::identity());
	camera->setRenderOrder(osg::Camera::POST_RENDER);
	camera->setClearMask(GL_DEPTH_BUFFER_BIT);
	camera->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	camera->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
	camera->setProjectionMatrixAsOrtho2D(0, viewer.getCamera()->getViewport()->width(), 0, viewer.getCamera()->getViewport()->height());

	viewer.setSceneData(camera);

	osgPango::Context::instance().init();

	osgPango::TextTransform* textTransform = new osgPango::TextTransform();

	textTransform->addText("<span font='Sans 20'>Sans 20</span>", 0, 0);
	textTransform->finalize();
	textTransform->setPosition(textTransform->getOriginTranslated());

	camera->addChild(textTransform);
	camera->setUpdateCallback(new UpdateCallback());

	viewer.run();

	// TODO: You'd uncomment the following line to see the intermediate textures
	// used internally; can be really helpful sometimes.
	// osgPango::Context::instance().writeCachesToPNGFiles("example");

	return 0;
}
