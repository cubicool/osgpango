// -*-c++-*- osgPango - Copyright (C) 2008 Jeremy Moles

#include <osgPango/Label>

namespace osgPango {

Label::Label(
	const std::string& name,
	const std::string& label,
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
	/*
	XYCoord    size = getTextSize();
	point_type x    = osg::round(((getWidth() - size.x()) / 2.0f) + getX());
	point_type y    = osg::round(((getHeight() - size.y()) / 2.0f) + getY());
	point_type z    = _calculateZ(getLayer() + 1);

	const WindowManager* wm = _getWindowManager();

	if(wm && wm->isUsingRenderBins()) {
		_text->getOrCreateStateSet()->setRenderBinDetails(
				static_cast<int>(z * OSGWIDGET_RENDERBIN_MOD),
				"RenderBin"
				);

		z = 0.0f;
	}

	_text->setPosition(osg::Vec3(x, y, z));
	*/
}

/*
void Label::_removeDrawables() {
	if(_indexes.size()) {
		for(
			std::list<unsigned int>::iterator i = _indexes.begin();
			i != _indexes.end();
			i++
		) _parent->getGeode()->removeDrawables(*i);
	}
}

void Label::_setDrawables() {
	_removeDrawables();

	_indexes.clear();

	for(GlyphGeometryVector::iterator i = _ggv.begin(); i != _ggv.end(); i++)
		_indexes.push_back(_parent->addDrawableAndGetIndex(*i));
	;
}
*/

}
