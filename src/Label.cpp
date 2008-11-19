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

/*
    osg::Geode* geode = parent->getGeode();

    // If we've been cloned, use the index of the old text Drawable if it's already there.
    // However, we have a problem here: imagine a Label gets cloned AFTER being added to
    // a Window; it'll have a _textIndex, but that _textIndex won't apply to the
    // currently cloned object. In this case, we'll need to check to be SURE.
    osgText::Text* text = dynamic_cast<osgText::Text*>(geode->getDrawable(_textIndex));
    
    if(text) parent->getGeode()->setDrawable(_textIndex, _text.get());

    // Otherwise, add it as new.
    else _textIndex = parent->addDrawableAndGetIndex(_text.get());
}

void Label::unparented(Window* parent) {
    if(_textIndex) parent->getGeode()->removeDrawable(_text.get());

    _textIndex = 0;
}
*/

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
