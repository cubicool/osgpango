#include <osgDB/ReadFile>
#include <osgWidget/WindowManager>
#include <osgWidget/Util>
#include <osgWidget/Frame>
#include <osgPango/Text>

const unsigned int SCREEN_WIDTH  = 1280;
const unsigned int SCREEN_HEIGHT = 1024;

const std::string LOREM_IPSUM(
	"Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod "
	"tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, "
	"quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. "
	"Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu "
	"fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in "
	"culpa qui officia deserunt mollit anim id est laborum."
);

class ListBox: public osgWidget::Frame {
public:
	ListBox():
	osgWidget::Frame("listbox") {
		createSimpleFrameWithSingleTexture(
			osgDB::readImageFile("osgWidget/frame-theme.png"),
			200.0f,
			15.0f
		);

		setFlags(osgWidget::Frame::FRAME_TEXTURE);

		getBackground()->setColor(0.0f, 0.0f, 0.0f, 0.0f);

		osgWidget::Box*   drop = new osgWidget::Box("drop", osgWidget::Box::HORIZONTAL);
		osgWidget::Label* l    = new osgWidget::Label("test1", "Test");

		l->setFontColor(0.0f, 0.0f, 0.0, 1.0f);
		l->setFont("fonts/Vera.ttf");
		l->setFontSize(12);

		// drop->addWidget(l);
		drop->getBackground()->setColor(1.0f, 1.0f, 1.0f, 1.0f);
		drop->addWidget(osg::clone(l, "test2", osg::CopyOp::DEEP_COPY_ALL));
		drop->addWidget(l);
		drop->resize();

		setWindow(drop);
		resizeFrame(drop->getWidth(), drop->getHeight());
		// getEmbeddedWindow()->setSize(drop->getWidth(), drop->getHeight());
		// getEmbeddedWindow()->setColor(1.0f, 1.0f, 1.0f, 1.0f);
		// setAnchorVertical(osgWidget::Window::VA_TOP);
		// setAnchorHorizontal(osgWidget::Window::HA_RIGHT);
	}
};

int main(int argc, char** argv) {
	osgViewer::Viewer viewer;

	/*
	std::string font("Sans 10");
	std::string text(LOREM_IPSUM);

	osgPango::Font::init();

	osgPango::GlyphCache* c = 0;

	osgPango::Font* f = new osgPango::Font(font, c);
	osgPango::Text* t = new osgPango::Text(f);

	t->setText(text);
	*/

	osgWidget::WindowManager* wm = new osgWidget::WindowManager(
		&viewer,
		SCREEN_WIDTH,
		SCREEN_HEIGHT,
		0x01234,
		osgWidget::WindowManager::WM_PICK_DEBUG
	);

	wm->addChild(new ListBox());

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

	/*
	osg::MatrixTransform* mt = new osg::MatrixTransform(
		osg::Matrix::translate(osg::Vec3(t->getOriginTranslated(), 0.0f))
	);

	mt->addChild(t);
	*/

	// osgPango::Font::cleanup();
	// osgPango::Text::cleanup();
	
	return osgWidget::createExample(viewer, wm);
}
