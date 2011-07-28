// -*-c++-*- Copyright (C) 2011 osgPango Development Team
// $Id$

#include <osgPango/DistanceFieldText>

namespace osgPango {

void DistanceFieldText::addText(
	String::Encoding   encoding,
	const std::string& str,
	const std::string& descr,
	int                x,
	int                y,
	const TextOptions& to
) {
	if(descr.empty()) {
		osg::notify(osg::WARN)
			<< "DistanceFieldText does NOT support Pango Markup; please specify a "
			<< "(preferably non-sized) font description."
			<< std::endl
		;

		return;
	}

	GlyphRendererDistanceField* gr = dynamic_cast<GlyphRendererDistanceField*>(
		Context::instance().getGlyphRenderer(_glyphRenderer)
	);

	if(!gr) {
		osg::notify(osg::WARN)
			<< "Could not retrieve a proper GlyphRendererDistanceField from the Context. "
			<< "DistanceFieldText will fail."
			<< std::endl
		;

		return;
	}

	String       text;
	PangoLayout* layout = pango_layout_new(Context::instance().getPangoContext());

	if(str.size()) {
		text.set(str, encoding);

		std::string utf8 = text.createUTF8EncodedString();

		// Get the user's description; will will only honor a subset of their requests.
		PangoFontDescription* userDescription = pango_font_description_from_string(descr.c_str());

		pango_layout_set_font_description(
			layout,
			pango_font_description_from_string(descr.c_str())
		);

		pango_layout_set_text(layout, utf8.c_str(), -1);
	}

	to.setupPangoLayout(layout);
	
	Context::instance().drawLayout(this, layout, x, y);

	_updateOriginAndSize(x, y, layout, gr->getExtraGlyphExtents());

	g_object_unref(layout);
}

}

