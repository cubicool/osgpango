// -*-c++-*- Copyright (C) 2010 osgPango Development Team
// $Id$

#include <sstream>
#include <osg/Node>
#include <osg/Timer>
#include <osg/ShapeDrawable>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/StateSetManipulator>
#include <osgDB/WriteFile>
#include <osgPango/Text>

const unsigned int WINDOW_WIDTH  = 800;
const unsigned int WINDOW_HEIGHT = 600;

#define FONT "<span font='Verdana Bold 27' color='black' bgcolor='white'>"

class UpdateCallback: public osg::NodeCallback {
public:
	typedef std::pair<osgPango::TextTransform::PositionAlignment, const std::string>
		PositionAlignmentPair
	;
	
	typedef std::list<PositionAlignmentPair> PositionAlignmentList;

	UpdateCallback() {
		_elapsed = _time.time_s();

		_list.push_back(PositionAlignmentPair(
			osgPango::TextTransform::POS_ALIGN_LEFT_BOTTOM, 
			"POS_ALIGN_LEFT_BOTTOM"
		));

		_list.push_back(PositionAlignmentPair(
			osgPango::TextTransform::POS_ALIGN_CENTER_BOTTOM,
			"POS_ALIGN_CENTER_BOTTOM"
		));

		_list.push_back(PositionAlignmentPair(
			osgPango::TextTransform::POS_ALIGN_RIGHT_BOTTOM,
			"POS_ALIGN_RIGHT_BOTTOM"
		));

		_list.push_back(PositionAlignmentPair(
			osgPango::TextTransform::POS_ALIGN_RIGHT_CENTER,
			"POS_ALIGN_RIGHT_CENTER"
		));

		_list.push_back(PositionAlignmentPair(
			osgPango::TextTransform::POS_ALIGN_RIGHT_TOP,
			"POS_ALIGN_RIGHT_TOP"
		));
		
		_list.push_back(PositionAlignmentPair(
			osgPango::TextTransform::POS_ALIGN_CENTER_TOP,
			"POS_ALIGN_CENTER_TOP"
		));

		_list.push_back(PositionAlignmentPair(
			osgPango::TextTransform::POS_ALIGN_LEFT_TOP,
			"POS_ALIGN_LEFT_TOP"
		));

		_list.push_back(PositionAlignmentPair(
			osgPango::TextTransform::POS_ALIGN_LEFT_CENTER,
			"POS_ALIGN_LEFT_CENTER"
		));

		_list.push_back(PositionAlignmentPair(
			osgPango::TextTransform::POS_ALIGN_CENTER_CENTER,
			"POS_ALIGN_CENTER_CENTER"
		));
	}

	virtual void operator()(osg::Node* node, osg::NodeVisitor* nv) {
		osg::Camera* camera = dynamic_cast<osg::Camera*>(node);
		
		if(!camera) return;

		else if(_time.time_s() - _elapsed > 2.0f) {
			_elapsed = _time.time_s();

			osgPango::TextTransform* oldTransform = dynamic_cast<osgPango::TextTransform*>(
				camera->getChild(0)
			);

			if(!oldTransform) return;

			std::ostringstream os;
			
			PositionAlignmentPair& pal = _getNextPositionAlignment();

			os << FONT << pal.second << "</span>";

			oldTransform->clear();
			oldTransform->addText(os.str().c_str());
			oldTransform->setPositionAlignment(pal.first, false);
			oldTransform->finalize();
		}

		traverse(node, nv);
	}

private:
	osg::Timer            _time;
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

	osg::Camera* camera = new osg::Camera();
	
	camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	camera->setViewMatrix(osg::Matrix::identity());
	camera->setRenderOrder(osg::Camera::POST_RENDER);
	camera->setClearMask(GL_DEPTH_BUFFER_BIT);
	camera->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	camera->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
	camera->setProjectionMatrixAsOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);

	viewer.setSceneData(camera);

	osgPango::Context::instance().init();
	osgPango::Context::instance().addGlyphRenderer(
		"outline",
		new osgPango::GlyphRendererOutline(1)
	);

	osgPango::TextTransform* textTransform = new osgPango::TextTransform();
	osg::Geode*              geode         = new osg::Geode();
	
	osg::Vec3 pos(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, 0.0f);

	textTransform->setMatrix(osg::Matrixd::translate(pos));
	textTransform->setPositionAlignment(osgPango::TextTransform::POS_ALIGN_CENTER_CENTER);
	textTransform->setGlyphRenderer("outline");
	textTransform->addText(FONT "POS_ALIGN_CENTER_CENTER</span>");
	textTransform->finalize();

	geode->addDrawable(new osg::ShapeDrawable(new osg::Sphere(pos, 4.0f)));

	camera->addChild(textTransform);
	camera->addChild(geode);
	camera->setUpdateCallback(new UpdateCallback());

	viewer.run();

	// TODO: You'd uncomment the following line to see the intermediate textures
	// used internally; can be really helpful sometimes.
	// osgPango::Context::instance().writeCachesToPNGFiles("osgpangoupdatecb");
	// osgDB::writeNodeFile(*camera, "osgpangoupdatecb.osg");

	return 0;
}
