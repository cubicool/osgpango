// -*-c++-*- Copyright (C) 2011 osgPango Development Team
// $Id$

#include <osg/Geode>
#include <osgPango/DistanceFieldText>

namespace osgPango {

class ApplyScaleStateVisitor: public osg::NodeVisitor {
public:	
	ApplyScaleStateVisitor(osg::Vec3::value_type scale, GlyphRendererDistanceField* renderer):
	NodeVisitor(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN),
	_scale    (scale),
	_renderer (renderer) {
		setVisitorType(osg::NodeVisitor::UPDATE_VISITOR);
	}

	void apply(osg::Geode& geode) {
		if(!_renderer.valid()) return;
		
		_renderer->updateScaleState(_scale, geode.getStateSet());
	}

private:
	osg::Vec3::value_type                         _scale;
	osg::observer_ptr<GlyphRendererDistanceField> _renderer;
};

void DistanceFieldText::calculatePosition() {
	TextTransform::calculatePosition();

	GlyphRendererDistanceField* gr = dynamic_cast<GlyphRendererDistanceField*>(
		Context::instance().getGlyphRenderer(_glyphRenderer)
	);

	if(!gr) {
		osg::notify(osg::WARN)
			<< "Could not retrieve a proper GlyphRendererDistanceField from the Context. "
			<< "DistanceFieldText scale will not be updated."
			<< std::endl
		;

		return;
	}

	ApplyScaleStateVisitor ssv(_scale, gr);
		
	this->accept(ssv);
}

}

