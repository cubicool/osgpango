// -*-c++-*- Copyright (C) 2011 osgPango Development Team
// $Id$

#include <osgPango/VerticalText>

namespace osgPango {

void VerticalText::_setText(
	String::Encoding   encoding,
	const std::string& str,
	const std::string& descr,
	const TextOptions& to
) {
	Context&         context = Context::instance();
	PangoGravity     gravity = context.getGravity();
	PangoGravityHint hint    = context.getGravityHint();

	context.setGravity(PANGO_GRAVITY_EAST);
	context.setGravityHint(PANGO_GRAVITY_HINT_STRONG);

	TextTransform::_setText(encoding, str, descr, to);

	context.setGravity(gravity);
	context.setGravityHint(hint);
}	

osg::Matrix VerticalText::getAlignmentTransform() const {
	osg::Matrix             m  = TextTransform::getAlignmentTransform();
	osg::Matrix::value_type vr = -90.0f;

	if(_rotation == ROTATE_CCW) vr = 90.0f;

	return m * osg::Matrix::rotate(osg::DegreesToRadians(vr), 0.0f, 0.0f, 1.0f);
}

}

