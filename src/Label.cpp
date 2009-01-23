// -*-c++-*- osgPango - Copyright (C) 2008 Jeremy Moles

#include <osg/io_utils>
#include <osgPango/Label>

namespace osgPango {

Label::Label(
	const std::string& name,
	Font*              font,
	GlyphEffectsMethod gem
):
osgWidget::Widget (name, 0, 0),
_textIndex        (0) {
	_text = new Text(font, gem);
}

Label::Label(const Label& label, const osg::CopyOp& co) {
}

void Label::parented(osgWidget::Window* parent) {
	_textIndex = parent->addChildAndGetIndex(_text.get());

	/*
	osg::Geode* geode = parent->getGeode();

	Text* text = dynamic_cast<Text*>(geode->getDrawable(_textIndex));

	if(text) parent->getGeode()->setDrawable(_textIndex, _text.get());

	else _textIndex = parent->addDrawableAndGetIndex(_text.get());
	*/
}

void Label::unparented(osgWidget::Window* parent) {
	// _removeDrawables();
}

void Label::positioned() {
	osgWidget::XYCoord    size   = _text->getSize();
	osgWidget::XYCoord    origin = _text->getOriginTranslated();
	osgWidget::point_type x      = osg::round(((getWidth() - size.x()) / 2.0f) + getX());
	osgWidget::point_type y      = osg::round(((getHeight() - size.y()) / 2.0f) + getY());
	osgWidget::point_type z      = _calculateZ(getLayer() + 1);

	/*
	const WindowManager* wm = _getWindowManager();

	if(wm && wm->isUsingRenderBins()) {
		_text->getOrCreateStateSet()->setRenderBinDetails(
				static_cast<int>(z * OSGWIDGET_RENDERBIN_MOD),
				"RenderBin"
				);

		z = 0.0f;
	}
	*/

	_text->setPosition(osg::Vec3(osgWidget::XYCoord(x, y) + origin, z));
}

void Label::textUpdated() {
	osgWidget::XYCoord size = _text->getSize();
	
	osgWidget::warn() << "size: " << size << std::endl;

	if(size.x() > getWidth()) setWidth(size.x());
	
	if(size.y() > getHeight()) setHeight(size.y());

	positioned();
}

}
