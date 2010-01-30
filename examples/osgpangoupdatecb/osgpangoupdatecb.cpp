#include <sstream>
#include <osg/Node>
#include <osg/Timer>
#include <osg/ShapeDrawable>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/StateSetManipulator>
#include <osgPango/Context>

const unsigned int WINDOW_WIDTH  = 800;
const unsigned int WINDOW_HEIGHT = 600;

class UpdateCallback: public osg::NodeCallback {
public:
	typedef std::pair<osgPango::TextTransform::PositionAlignment, const std::string>
		PositionAlignmentPair
	;
	
	typedef std::list<PositionAlignmentPair> PositionAlignmentList;

	UpdateCallback() :
	_time(new osg::Timer) {
		_elapsed = _time->time_s();

		/*
		_list.push_back(PositionAlignmentPair(
			osgPango::TextTransform::POS_ALIGN_BOTTOM_RIGHT,
			"POS_ALIGN_BOTTOM_RIGHT"
		));

		_list.push_back(PositionAlignmentPair(
			osgPango::TextTransform::POS_ALIGN_RIGHT,
			"POS_ALIGN_RIGHT"
		));

		_list.push_back(PositionAlignmentPair(
			osgPango::TextTransform::POS_ALIGN_TOP_RIGHT,
			"POS_ALIGN_TOP_RIGHT"
		));

		_list.push_back(PositionAlignmentPair(
			osgPango::TextTransform::POS_ALIGN_BOTTOM_LEFT, 
			"POS_ALIGN_BOTTOM_LEFT"
		));

		_list.push_back(PositionAlignmentPair(
			osgPango::TextTransform::POS_ALIGN_BOTTOM,
			"POS_ALIGN_BOTTOM"
		));
		
		_list.push_back(PositionAlignmentPair(
			osgPango::TextTransform::POS_ALIGN_TOP,
			"POS_ALIGN_TOP"
		));

		_list.push_back(PositionAlignmentPair(
			osgPango::TextTransform::POS_ALIGN_TOP_LEFT,
			"POS_ALIGN_TOP_LEFT"
		));

		_list.push_back(PositionAlignmentPair(
			osgPango::TextTransform::POS_ALIGN_LEFT,
			"POS_ALIGN_LEFT"
		));
		*/

		_list.push_back(PositionAlignmentPair(
			osgPango::TextTransform::POS_ALIGN_TOP_LEFT,
			"abcefghijklmnopqrstuvwxyz"
		));

		_list.push_back(PositionAlignmentPair(
			osgPango::TextTransform::POS_ALIGN_LEFT,
			"zyxwvutsrqponmlkjihgfecba"
		));

		_list.push_back(PositionAlignmentPair(
			osgPango::TextTransform::POS_ALIGN_CENTER,
			"POS_ALIGN_CENTER"
		));
	}

	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv) {
		osg::Camera* camera = dynamic_cast<osg::Camera*>(node);
		
		if(!camera) return;

		else if(_time->time_s() - _elapsed > 2.0f) {
			_elapsed = _time->time_s();

			osgPango::TextTransform* oldTransform = dynamic_cast<osgPango::TextTransform*>(
				camera->getChild(0)
			);

			if(!oldTransform) return;

			std::ostringstream os;
			
			PositionAlignmentPair& pal = _getNextPositionAlignment();

			os << "<span font='Georgia 15'>" << pal.second << "</span>";

			oldTransform->clear();
			oldTransform->addText(os.str().c_str(), 0, 0);
			oldTransform->setAlignment(pal.first);
			oldTransform->finalize();
		}

		traverse(node, nv);
	}

private:
	osg::Timer*           _time;
	double                _elapsed;
	PositionAlignmentList _list;
		
	PositionAlignmentPair& _getNextPositionAlignment() {
		static PositionAlignmentList::iterator i = _list.begin();

		if(i == _list.end()) i = _list.begin();

		return *(i++);
	}
};

int main(int ac, char **av) {
	osgViewer::Viewer viewer;

	viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()));
	viewer.addEventHandler(new osgViewer::StatsHandler());
	viewer.addEventHandler(new osgViewer::WindowSizeHandler);
	viewer.setUpViewInWindow(50, 50, WINDOW_WIDTH, WINDOW_HEIGHT);

	osg::ref_ptr<osg::Camera> camera = new osg::Camera();
	
	camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	camera->setViewMatrix(osg::Matrix::identity());
	camera->setRenderOrder(osg::Camera::POST_RENDER);
	camera->setClearMask(GL_DEPTH_BUFFER_BIT);
	camera->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	camera->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
	camera->setProjectionMatrixAsOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);

	viewer.setSceneData(camera);

	osgPango::Context::instance().init();

	osgPango::TextTransform* textTransform = new osgPango::TextTransform();
	osg::Geode*              geode         = new osg::Geode();
	
	osg::Vec3 pos(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, 0.0f);

	textTransform->setPosition(pos);
	textTransform->setAlignment(osgPango::TextTransform::POS_ALIGN_CENTER);
	textTransform->addText("<span font='Georgia 15'>POS_ALIGN_CENTER</span>", 0, 0);
	textTransform->finalize();

	geode->addDrawable(new osg::ShapeDrawable(new osg::Sphere(pos, 5.0f)));

	camera->addChild(textTransform);
	camera->addChild(geode);
	camera->setUpdateCallback(new UpdateCallback());

	viewer.run();

	// TODO: You'd uncomment the following line to see the intermediate textures
	// used internally; can be really helpful sometimes.
	osgPango::Context::instance().writeCachesToPNGFiles("osgpangoupdatecb");

	return 0;
}
