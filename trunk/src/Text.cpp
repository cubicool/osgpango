// -*-c++-*- osgPango - Copyright (C) 2010 Jeremy Moles

#include <sstream>
#include <algorithm>
#include <osg/io_utils>
#include <osg/Math>
#include <osg/Image>
#include <osgPango/Context>

namespace osgPango {

const char* VERTEX_SHADER =
"#version 120\n"
"varying vec4 pangoTexCoord;"
"void main() {"
"	pangoTexCoord = gl_MultiTexCoord0;"
"	gl_Position = ftransform();"
"}"
;

const char* FRAGMENT_SHADER =
"varying vec4 pangoTexCoord;"
"uniform vec3 pangoColor[8];"
"uniform sampler2D pangoTex;"
"void main() {"
"	float alpha = texture2D(pangoTex, pangoTexCoord.st).a;"
"	gl_FragColor = vec4(pangoColor[0], alpha);"
"}"
;

bool TextOptions::setupPangoLayout(PangoLayout* layout) const {
	if(alignment != TEXT_ALIGN_JUSTIFY) {
		PangoAlignment pa = PANGO_ALIGN_LEFT;

		if(alignment == TEXT_ALIGN_CENTER) pa = PANGO_ALIGN_CENTER;

		else if(alignment == TEXT_ALIGN_RIGHT) pa = PANGO_ALIGN_RIGHT;
	
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
_alpha(1.0f) {
	clear();
}

void Text::clear() {
	for(GlyphGeometryMap::iterator g = _ggMap.begin(); g != _ggMap.end(); g++) {
		GlyphGeometryIndex& ggi = g->second;

		ggi.clear();
	}

	_size      = osg::Vec2();
	_origin    = osg::Vec2();
	_lastX     = 0;
	_lastY     = 0;
	_baseline  = 0;
	_init      = false;
	_newGlyphs = false;
	_finalized = false;
}

GlyphGeometry* createGlyphGeometry(bool hasEffects) {
	static unsigned int ggId = 0;

	std::ostringstream ss;

	ss << "GlyphGeometry_" << ggId;

	ggId++;

	GlyphGeometry* gg = new GlyphGeometry(hasEffects);

	gg->setName(ss.str());

	return gg;
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
	// osg::Vec4 extents = gc->getGlyphRenderer()->getExtraGlyphExtents();
	osg::Vec4 extents = osg::Vec4();

	const ColorPair& color = Context::instance().getColorPair();
	
	for(int i = 0; i < glyphs->num_glyphs; i++) {
		PangoGlyphInfo* gi = glyphs->glyphs + i;

		if((gi->glyph & PANGO_GLYPH_UNKNOWN_FLAG)) {
			PangoFontMetrics* metrics = pango_font_get_metrics(font, 0);

			pango_font_metrics_unref(metrics);

			continue;
		}

		const CachedGlyph* cg = gc->getCachedGlyph(gi->glyph);

		if(!cg) {
			cg = gc->createCachedGlyph(font, gi);

			_newGlyphs = true;
		}

		if(!cg) continue;

		GlyphGeometryIndex& ggi = _ggMap[GlyphGeometryMapKey(gc, color)];

		if(cg->size.x() > 0.0f && cg->size.y() > 0.0f) {
			osg::Vec2 pos(
				(gi->geometry.x_offset / PANGO_SCALE) + extents[0],
				(gi->geometry.y_offset / PANGO_SCALE) + extents[1]
			);

			bool hasEffects = gc->getGlyphRenderer()->hasEffects();

			if(!ggi[cg->img]) ggi[cg->img] = createGlyphGeometry(hasEffects);

			ggi[cg->img]->pushCachedGlyphAt(
				cg,
				pos + layoutPos,
				hasEffects
			);
		}

		layoutPos += osg::Vec2((gi->geometry.width / PANGO_SCALE) + extents[0], 0.0f);
		_lastX    += extents[0];
	}

	_lastY = currentY;

	// Use the lowest baseline of all the texts that are added.
	int baseline = y / PANGO_SCALE;

	if(!_init || baseline > _baseline) _baseline = baseline;
}

void Text::addText(const std::string& str, int x, int y, const TextOptions& to) {
	String       text;
	PangoLayout* layout = pango_layout_new(Context::instance().getPangoContext());

	if(str.size()) {
		text.set(str, OSGPANGO_ENCODING);

		std::string utf8 = text.createUTF8EncodedString();

		pango_layout_set_markup(layout, utf8.c_str(), -1);
	}

	to.setupPangoLayout(layout);

	_glyphRenderer = to.renderer;

	Context::instance().drawLayout(this, layout, x, y);

	// Get text dimensions and whatnot; we'll accumulate this data after each rendering
	// to keep it accurate.
	PangoRectangle rect;

	pango_layout_get_pixel_extents(layout, &rect, 0);

	g_object_unref(layout);

	const GlyphRenderer* gr = Context::instance().getGlyphRenderer(to.renderer);

	if(!gr) return;

	const osg::Vec4& extents = gr->getExtraGlyphExtents();

	osg::Vec2::value_type ox = -(x + rect.x); //- extents[0];
	osg::Vec2::value_type oy = y + rect.y - extents[3];
	osg::Vec2::value_type sw = rect.width;
	osg::Vec2::value_type sh = rect.height + extents[3];

	if(!_init) {
		_origin.set(ox, oy);
		_size.set(sw, sh);
	}
	
	else {
		if(ox > _origin[0]) _origin[0] = ox;

		if(oy > _origin[1]) _origin[1] = oy;

		if(sw > _size[0]) _size[0] = sw;

		if(sh > _size[1]) _size[1] = sh;
	}

	// We've run ONCE, so we're initialized to some state. Everything else from
	// here is based on this position, greater or lower.
	_init = true;
}

void Text::setAlpha(double alpha) {
	if(!_finalized) {
		_alpha = alpha;

		return;
	}

	for(GlyphGeometryMap::iterator g = _ggMap.begin(); g != _ggMap.end(); g++) {
		GlyphGeometryIndex& ggi = g->second;

		for(GlyphGeometryIndex::iterator i = ggi.begin(); i != ggi.end(); i++) {
			if(!i->second->setAlpha(alpha)) osg::notify(osg::WARN)
				<< "Failed to set new alpha value for GlyphGeometryIndex "
				<< i->first
				<< std::endl
			;
		}
	}
}

osg::Vec2 Text::getOriginBaseline() const {
	return osg::Vec2(_origin.x(), _baseline);
}

osg::Vec2 Text::getOriginTranslated() const {
	return osg::Vec2(_origin.x(), _size.y() + _origin.y());
}

void Text::_finalizeGeometry(GeometryList& drawables) {
	for(GlyphGeometryMap::iterator g = _ggMap.begin(); g != _ggMap.end(); g++) {
		GlyphGeometryIndex& ggi = g->second;

		for(GlyphGeometryIndex::iterator i = ggi.begin(); i != ggi.end(); i++) {
			GlyphCache* gc      = g->first.first;
			ColorPair   color   = g->first.second;
			osg::Image* texture = gc->getImage(i->first);
			osg::Image* effects = gc->getImage(i->first, true);

			if(_newGlyphs) {
				if(texture) texture->dirty();
				
				if(effects) effects->dirty();
			}

			if(!i->second->finalize(GlyphGeometryState(
				gc->getTexture(i->first),
				gc->getTexture(i->first, true),
				color.first,
				color.second,
				_alpha
			))) continue;

			drawables.push_back(i->second);
		}
	}

	_finalized = true;
}

TextTransform::TextTransform():
_alignment (POS_ALIGN_TOP_RIGHT),
_position  (osg::Vec3(0.0f, 0.0f, 0.0f)) {
}

bool TextTransform::finalize() {
	if(!_finalizeGeode()) return false;

	_calculatePosition();

	// TODO: Move this elsewhere and make it optional...
	osg::Program* program = new osg::Program();
	osg::Shader*  vert    = new osg::Shader(osg::Shader::VERTEX, VERTEX_SHADER);
	osg::Shader*  frag    = new osg::Shader(osg::Shader::FRAGMENT, FRAGMENT_SHADER);

	vert->setName("pangoRendererVert");
	frag->setName("pangoRendererFrag");
	program->setName("pangoRenderer");

	program->addShader(vert);
	program->addShader(frag);

	getGeode()->getOrCreateStateSet()->setAttributeAndModes(program);

	return true;
}

void TextTransform::setPosition(const osg::Vec3& position, bool recalculate) {
	_position = position;

	if(recalculate) _calculatePosition();
}

void TextTransform::setAlignment(PositionAlignment alignment, bool recalculate) {
	_alignment = alignment;

	if(recalculate) _calculatePosition();
}

void TextTransform::_calculatePosition() {
	osg::Vec3 origin(getOriginTranslated(), 0.0f);
	osg::Vec3 size(_size, 0.0f);

	if(_alignment == POS_ALIGN_TOP) 
		origin.x() -= osg::round(size.x() / 2.0f)
	;
	
	else if(_alignment == POS_ALIGN_TOP_LEFT)
		origin.x() -= osg::round(size.x())
	;
	
	else if(_alignment == POS_ALIGN_LEFT) origin -= osg::Vec3(
		osg::round(size.x()),
		osg::round(size.y() / 2.0f),
		0.0f
	);
	
	else if(_alignment == POS_ALIGN_BOTTOM_LEFT)
		origin -= size
	;
	
	else if(_alignment == POS_ALIGN_BOTTOM) origin -= osg::Vec3(
		osg::round(size.x() / 2.0f),
		size.y(),
		0.0f
	);
	
	else if(_alignment == POS_ALIGN_BOTTOM_RIGHT)
		origin.y() -= size.y()
	;
	
	else if(_alignment == POS_ALIGN_RIGHT)
		origin.y() -= osg::round(size.y() / 2.0f)
	;
	
	else if(_alignment == POS_ALIGN_CENTER) origin += osg::Vec3(
		osg::round(-size.x() / 2.0f),
		osg::round(-size.y() / 2.0f),
		0.0f
	);

	setMatrix(osg::Matrix::translate(origin + _position));
}

bool TextAutoTransform::finalize() {
	return _finalizeGeode();
}

}
