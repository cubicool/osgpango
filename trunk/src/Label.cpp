// -*-c++-*- osgPango - Copyright (C) 2008 Jeremy Moles

#include <osgPango/Label>

namespace osgPango {

Label(
	const std::string& name,
	const std::string& label,
	Font*              font,
	GlyphEffectsMethod gem
):
Layout            (font, gem),
osgWidget::Widget (name, 0, 0),
_textUpdated      (false) {
}

Label::Label(const Label& label, const osg::CopyOp& co) {
}

virtual bool Label::textUpdated(GlyphGeometryVector& ggv) {
	// TODO: Get our parent and call allDrawable(), just like text. Get
	// the index for future use.
	if(!_parent) {
		_textUpdated = true;

		return false;
	}

	else _setDrawables();

	return true;
}

void Label::parented(Window* parent) {
	if(_textUpdated) {
		_textUpdated = false;

		_setDrawables(getGlyphGeometryVector());
	}
}

void Label::unparented(Window* parent) {
	_removeDrawables();
}

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

void Label::_removeDrawables() {
	if(_indexes.size()) {
		for(
			std::list<unsigned int>::iterator i = _indexes.begin();
			i != _indexes.end();
			i++
		) _parent->removeDrawable(*i);
	}
}

void Label::_setDrawables() {
	_removeDrawables();

	_indexes.clear();

	for(GlyphGeometryVector::iterator i = _ggv.begin(); i != _ggv.end(); i++)
		_indexes.push_back(_parent->addDrawableAndGetIndex(*i));
	;
}

}
