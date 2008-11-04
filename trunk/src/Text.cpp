// -*-c++-*- osgPango - Copyright (C) 2008 Jeremy Moles

#include <osgPango/Text>

namespace osgPango {

bool Text::textUpdated(GlyphGeometryVector& ggv) {
	removeDrawables(0, getNumDrawables());

	for(unsigned int i = 0; i < ggv.size(); i++) addDrawable(ggv[i]);

	return true;
}

}
