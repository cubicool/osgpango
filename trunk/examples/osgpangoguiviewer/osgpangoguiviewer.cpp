#include <osgDB/ReadFile>
#include <osgWidget/WindowManager>
#include <osgWidget/Util>
#include <osgWidget/Frame>
#include <osgPango/Label>

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

class ButtonTheme: public osgCairo::Image {
	void _finalizeCorner(
		float x,
		float y,
		const osgWidget::Color& lc, 
		const osgWidget::Color& fc
	) {
		setSourceRGBA(lc);
		strokePreserve();
		setSourceRGBA(fc);
		lineTo(x, y);
		fill();
	}

	void _drawBorder(
		float x1,
		float y1,
		float x2,
		float y2,
		float xoff,
		float size,
		const osgWidget::Color& lc, 
		const osgWidget::Color& fc
	) {
		setSourceRGBA(lc);
		moveTo(x1 + xoff, 0.0f);
		lineTo(x1 + xoff, size);
		stroke();
		rectangle(x1, y1, x2, y2);
		setSourceRGBA(fc);
		fill();
	}

public:
	ButtonTheme():
	osgCairo::Image(CAIRO_FORMAT_ARGB32) {
	}

	bool renderTheme(
		float                   s  = 8.0f,
		float                   lw = 2.0f,
		const osgWidget::Color& lc = osgWidget::Color(0.0f, 0.0f, 0.0f, 1.0f),
		const osgWidget::Color& fc = osgWidget::Color(1.0f, 1.0f, 1.0f, 1.0f)
	) {
		if(!allocateSurface(s * 8.0f, s)) return false;

		if(!createContext()) return false;

		// Some variables to make this easier to work with...
		float r      = s * 0.75f;
		float top    = osg::PI + osg::PI_2;
		float bottom = osg::PI_2;
		float left   = osg::PI;
		float right  = 0.0f;
	
		setLineWidth(lw * 2.0f);

		// Create the upper-left region.
		arc(s, s, r, left, top);

		_finalizeCorner(s, s, lc, fc);

		// Create the top border region.
		_drawBorder(s + (s - r), 0.0f, r, s, 0.0f, s, lc, fc);

		// Create the upper-right region.
		arc(s * 2.0f, s, r, top, right);
		_finalizeCorner(s * 2.0f, s, lc, fc);

		// Create the left border.
		_drawBorder((s * 3.0f) + (s - r), 0.0f, r, s, 0.0f, s, lc, fc);

		// Create the right border.
		_drawBorder((s * 4.0f), 0.0f, r, s, r, s, lc, fc);
		fill();

		// Create the bottom-left region.
		arc(s * 6.0f, 0.0f, r, bottom, left);
		_finalizeCorner(s * 6.0f, 0.0f, lc, fc);

		// Create the bottom border region.
		_drawBorder((s * 6.0f), 0.0f, r, s, r, s, lc, fc);

		// Create the bottom-right region.
		arc(s * 7.0f, 0.0f, r, right, bottom);
		_finalizeCorner(s * 7.0f, 0.0f, lc, fc);

		flipVertical();
	
		return true;
	}
};

class ButtonArrow: public osgCairo::Image {
public:
	ButtonArrow():
	osgCairo::Image(CAIRO_FORMAT_ARGB32) {
	}

	bool renderArrow(
		float                   s  = 8.0f,
		float                   lw = 2.0f,
		const osgWidget::Color& lc = osgWidget::Color(0.0f, 0.0f, 0.0f, 1.0f),
		const osgWidget::Color& fc = osgWidget::Color(1.0f, 1.0f, 1.0f, 1.0f)
	) {
		if(!allocateSurface(s * 2.0f, s)) return false;

		if(!createContext()) return false;

		// Fill the background.
		setSourceRGBA(fc);
		rectangle(0.0f, 0.0f, s * 2.0f, s);
		fill();

		setLineWidth(lw);
		setSourceRGBA(lc);

		// Draw our "vertical line."
		moveTo(s / 4.0f, 0.0f);
		lineTo(s / 4.0f, s);
		stroke();

		// Draw the arrow.
		setLineWidth(0.0f);

		moveTo(s, 0.0f);
		lineTo(s, s);
		lineTo(s + (s / 2.0f), s / 2.0f);
		lineTo(s, 0.0f);
		fill();

		writeToPNG("arrow.png");

		return true;
	}
};

class ListBoxBase: public osgWidget::Frame {
protected:
	osg::Vec3 _lineColor;
	osg::Vec3 _fillColor;
	
	float        _lineWidth;
	unsigned int _fontSize;

	osg::observer_ptr<osgPango::Font> _font;

public:
	ListBoxBase(const std::string& name):
	osgWidget::Frame(name),
	_lineColor (0.3f, 0.2f, 0.3f),
	_fillColor (0.8f, 0.9f, 0.8f),
	_lineWidth (2.0f),
	_fontSize  (12) {
		std::ostringstream font;
		
		font << "Sans Bold " << _fontSize;

		_font = new osgPango::Font(font.str().c_str());

		ButtonTheme* bt = new ButtonTheme();

		bt->renderTheme(
			8.0f,
			_lineWidth,
			osgWidget::Color(_lineColor, 1.0f),
			osgWidget::Color(_fillColor, 1.0f)
		);

		createSimpleFrameWithSingleTexture(bt, 200.0f, 15.0f);
		setFlags(osgWidget::Frame::FRAME_TEXTURE);
		getBackground()->setColor(0.0f, 0.0f, 0.0f, 0.0f);
	}
};

class ListBoxPopup: public ListBoxBase {
public:
	ListBoxPopup():
	ListBoxBase("listboxPopup") {
		osgPango::Font::FontList fl;

		osgPango::Font::getFontList(fl, false);

		osgWidget::Box* fonts = new osgWidget::Box("fonts", osgWidget::Box::VERTICAL);

		for(osgPango::Font::FontList::iterator i = fl.begin(); i != fl.end(); i++) {
			osgPango::Label* label = new osgPango::Label(*i + "label", _font.get());
			
			label->getText()->setColor(_lineColor);
			label->getText()->setText(*i);
			label->setColor(osg::Vec4(_fillColor, 1.0f));
			label->setPadding(3.0f);
			label->textUpdated();

			fonts->addWidget(label);
		}

		fonts->getBackground()->setColor(0.8f, 0.9f, 0.8f, 1.0f);

		setWindow(fonts);
		resizeFrame(fonts->getWidth(), fonts->getHeight());
	}
};

class ListBox: public ListBoxBase {
	osg::observer_ptr<ListBoxPopup> _lbp;
	
public:
	ListBox():
	ListBoxBase("listbox"),
	_lbp(new ListBoxPopup()) {
		ButtonArrow* ba = new ButtonArrow();

		ba->renderArrow(
			_fontSize,
			_lineWidth,
			osgWidget::Color(_lineColor, 1.0f),
			osgWidget::Color(_fillColor, 1.0f)
		);

		osgWidget::Box*    drop  = new osgWidget::Box("drop", osgWidget::Box::HORIZONTAL);
		osgWidget::Widget* arrow = new osgWidget::Widget("arror", _fontSize * 2.0f, _fontSize);
		osgPango::Label*   label = new osgPango::Label("testing", _font.get());

		label->getText()->setColor(_lineColor);
		label->getText()->setText("Choose a font...");
		label->setColor(osgWidget::Color(_fillColor, 1.0f));
		label->textUpdated();

		arrow->setImage(ba, true, true);
		arrow->setPadLeft(30.0f);

		drop->getBackground()->setColor(osgWidget::Color(_fillColor, 1.0f));
		drop->addWidget(label);
		drop->addWidget(arrow);
		drop->resize();

		setWindow(drop);
		resizeFrame(drop->getWidth(), drop->getHeight());
	}

	virtual void managed(osgWidget::WindowManager* wm) {
		ListBoxBase::managed(wm);

		wm->addChild(_lbp.get());

		_lbp->setOrigin(getOrigin());
		_lbp->hide();
	}

	virtual bool mousePush(double, double, osgWidget::WindowManager*) {
		osgWidget::warn() << "Here..." << std::endl;

		_lbp->show();
		_lbp->grabFocus();

		return true;
	}
};

int main(int argc, char** argv) {
	osgViewer::Viewer viewer;

	osgPango::Font::init();

	/*
	std::string font("Sans 10");
	std::string text(LOREM_IPSUM);

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

	float x = 10.0f;
	float y = 10.0f;

	for(unsigned int i = 0; i < 7; i++) {
		ListBox* lb = new ListBox();

		lb->setOrigin(x, y);

		wm->addChild(lb);

		x += 210.0f;
	}

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

	osgPango::Font::cleanup();
	osgPango::Text::cleanup();
	
	viewer.getCamera()->setClearColor(osg::Vec4(0.7f, 0.7f, 0.7f, 1.0f));

	return osgWidget::createExample(viewer, wm);
}
