// -*-c++-*- osgPango - Copyright (C) 2009 Jeremy Moles

#include <iostream>
#include <sstream>
#include <algorithm>
#include <osgPango/Context>

namespace osgPango {

bool TextOptions::setupPangoLayout(PangoLayout* layout) const {
	if(alignment != ALIGN_JUSTIFY) {
		PangoAlignment pa = PANGO_ALIGN_LEFT;

		if(alignment == ALIGN_CENTER) pa = PANGO_ALIGN_CENTER;

		else if(alignment == ALIGN_RIGHT) pa = PANGO_ALIGN_RIGHT;
	
		pango_layout_set_alignment(layout, pa);
	}

	else pango_layout_set_justify(layout, true);

	if(width > 0) pango_layout_set_width(layout, width * PANGO_SCALE);

	if(height > 0) pango_layout_set_height(layout, height * PANGO_SCALE);

	if(indent > 0) pango_layout_set_indent(layout, indent * PANGO_SCALE);

	if(spacing > 0) pango_layout_set_spacing(layout, spacing * PANGO_SCALE);

	return true;
}

Text::Text():
osg::MatrixTransform(),
_lastX    (0),
_lastY    (0),
_baseline (0),
_alpha    (1.0f) {
	addChild(new osg::Geode());
}

void Text::drawGlyphs(PangoFont* font, PangoGlyphString* glyphs, int x, int y) {
	// Get the GlyphCache from a key, which may or may not be set via PangoAttr if I
	// can it to work properly. :)
	GlyphCache* gc = Context::instance().getGlyphCache(font, _glyphRenderer);

	if(!gc) return;

	osg::Vec2::value_type currentY = -(y / PANGO_SCALE);

	osg::Vec2 layoutPos(x / PANGO_SCALE, currentY);

	if(static_cast<int>(currentY) == _lastY) layoutPos[0] += _lastX;

	else _lastX = 0.0f;

	// TODO: Enabling optional honoring of extents...
	osg::Vec4 extents = osg::Vec4(); //gc->getExtraGlyphExtents();

	const osg::Vec3& color = Context::instance().getCurrentColor();
		
	for(int i = 0; i < glyphs->num_glyphs; i++) {
		PangoGlyphInfo* gi = glyphs->glyphs + i;

		if((gi->glyph & PANGO_GLYPH_UNKNOWN_FLAG)) {
			PangoFontMetrics* metrics = pango_font_get_metrics(font, 0);

			pango_font_metrics_unref(metrics);

			continue;
		}

		const CachedGlyph* cg = gc->getCachedGlyph(gi->glyph);

		if(!cg) cg = gc->createCachedGlyph(font, gi);

		if(!cg) continue;

		GlyphGeometryVector& ggv = _ggMap[GlyphGeometryMapKey(gc, color)];
	
		if(cg->size.x() > 0.0f && cg->size.y() > 0.0f) {
			osg::Vec2 pos(
				(gi->geometry.x_offset / PANGO_SCALE) + extents[0],
				(gi->geometry.y_offset / PANGO_SCALE) + extents[1]
			);

			bool hasEffects = gc->getGlyphRenderer()->hasEffects();

			for(unsigned int j = 0; j < cg->img; j++) {
				if(j >= ggv.size()) ggv.push_back(0);
			}

			// TODO: This whole block of code is VILE and ATROCIOUS.
			if(cg->img >= ggv.size()) ggv.push_back(new GlyphGeometry(hasEffects));

			else {
				if(!ggv[cg->img]) ggv[cg->img] = new GlyphGeometry(hasEffects);
			}

			ggv[cg->img]->pushCachedGlyphAt(
				cg,
				pos + layoutPos,
				hasEffects,
				GLYPH_EFFECTS_METHOD_DEFAULT
			);
		}
		
		layoutPos += osg::Vec2((gi->geometry.width / PANGO_SCALE) + extents[0], 0.0f);
		_lastX    += extents[0];
	}

	_lastY    = currentY;
	_baseline = y / PANGO_SCALE;
}

bool Text::finalize() {
	osg::Geode* geode = dynamic_cast<osg::Geode*>(getChild(0));

	if(!geode) return false;

	geode->removeDrawables(0, geode->getNumDrawables());

	for(GlyphGeometryMap::iterator g = _ggMap.begin(); g != _ggMap.end(); g++) {
		GlyphGeometryVector& ggv = g->second;

		for(unsigned int i = 0; i < ggv.size(); i++) {
			// They can be set to null, so ignore them if that's the case.
			if(!ggv[i]) {
				osg::notify(osg::NOTICE) << "Skipping nul..." << std::endl;

				continue;
			}

			else osg::notify(osg::NOTICE)
				<< i << " has "
				<< ggv[i]->getVertexArray()->getNumElements() / 4 << " elements."
				<< std::endl
			;

			GlyphCache* gc = g->first.first;

			if(!ggv[i]->finalize(GlyphGeometryState(
				gc->getTexture(i),
				gc->getTexture(i, true),
				g->first.second,
				osg::Vec3(0.0f, 0.0f, 0.0f),
				_alpha
			))) continue;

			geode->addDrawable(ggv[i]);
		}
	}

	return true;
}

void Text::addText(const std::string& str, int x, int y, const TextOptions& to) {
	String       text;
	PangoLayout* layout = pango_layout_new(Context::instance().getPangoContext());

	if(str.size()) {
		text.set(str, osgText::String::ENCODING_UTF8);

		std::string utf8 = text.createUTF8EncodedString();

		pango_layout_set_markup(layout, utf8.c_str(), -1);
	}

	to.setupPangoLayout(layout);

	Context::instance().drawLayout(this, layout, x, y);

	/*
	// Get text dimensions and whatnot...
	PangoRectangle rect;

	pango_layout_get_pixel_extents(layout, &rect, 0);

	GlyphCache* gc = _font->getGlyphCache();

	const osg::Vec4& extents = gc->getExtraGlyphExtents();

	_origin.set(rect.x + extents[0], rect.y - extents[1] - extents[3]);

	_effectsSize.y() = extents[3];

	_size.set(rect.width, rect.height);

	_size += _effectsSize;
	*/

	g_object_unref(layout);
}

void Text::setPosition(const osg::Vec3& pos) {
	setMatrix(osg::Matrix::translate(pos));
}

void Text::setAlpha(double alpha) {
	_alpha = alpha;
}

void Text::setGlyphRenderer(const std::string& gr) {
	_glyphRenderer = gr;
}

osg::Vec3 Text::getPosition() const {
	return getMatrix().getTrans();
}

osg::Vec2 Text::getOriginBaseline() const {
	return osg::Vec2(-_origin.x(), _baseline);
}

osg::Vec2 Text::getOriginTranslated() const {
	return osg::Vec2(-_origin.x(), _size.y() + _origin.y());
}

}
