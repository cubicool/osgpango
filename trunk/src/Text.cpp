// -*-c++-*- osgPango - Copyright (C) 2009 Jeremy Moles

#include <iostream>
#include <sstream>
#include <algorithm>
#include <osgPango/Text>

namespace osgPango {

Renderer*   TextRenderer::_renderer = 0;
std::string TextRenderer::_gcr;
osg::Vec3   TextRenderer::_fg;

G_DEFINE_TYPE(Renderer, renderer, PANGO_TYPE_RENDERER);

#define TYPE_RENDERER (renderer_get_type())
#define RENDERER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_RENDERER, Renderer))

static GObjectClass* _pangoClass = 0;

void renderer_finalize(GObject* object) {
	Renderer* priv = RENDERER(object);

	G_OBJECT_CLASS(_pangoClass)->finalize(object);
}

void renderer_init(Renderer* priv) {
}

void renderer_class_init(RendererClass* klass) {
	GObjectClass*       object_class   = G_OBJECT_CLASS(klass);
	PangoRendererClass* renderer_class = PANGO_RENDERER_CLASS(klass);

	_pangoClass = static_cast<GObjectClass*>(g_type_class_peek_parent(klass));

	object_class->finalize = renderer_finalize;

	renderer_class->draw_glyphs = &TextRenderer::_drawGlyphs;
}

TextRenderer::TextRenderer() {
	if(!_renderer) {
		_renderer = static_cast<Renderer*>(g_object_new(TYPE_RENDERER, 0));

		_renderer->renderer = 0;
		// _mutex              = new OpenThreads::Mutex();
	}
}

void TextRenderer::_drawGlyphs(
	PangoRenderer*    renderer,
	PangoFont*        font,
	PangoGlyphString* glyphs,
	int               x,
	int               y
) {
	PangoColor* fg = pango_renderer_get_color(renderer, PANGO_RENDER_PART_FOREGROUND);

	if(fg) _fg.set(
		fg->red / 65535.0f,
		fg->green / 65535.0f,
		fg->blue / 65535.0f
	);

	else _fg.set(-1.0f, -1.0, -1.0f);

	RENDERER(renderer)->renderer->drawGlyphs(font, glyphs, x, y);
}

const osg::Vec3* TextRenderer::_getRequestedPangoColor() const {
	if(_fg[0] != -1.0f) return &_fg;

	else return 0;
}

const std::string& TextRenderer::_getRequestedGlyphCacheRenderer() const {
	return _gcr;
}

void TextRenderer::drawLayout(PangoLayout* layout, unsigned int x, unsigned int y) {
	// OpenThreads::ScopedLock<OpenThreads::Mutex> lock(*_mutex);

	_renderer->renderer = const_cast<TextRenderer*>(this);

	pango_renderer_draw_layout(
		PANGO_RENDERER(_renderer),
		layout,
		x * PANGO_SCALE,
		-(y * PANGO_SCALE)
	);
}

TextOptions::TextOptions(const std::string& d, Alignment a, int w, int h, int i, int s):
alignment    (a),
width        (w),
height       (h),
indent       (i),
spacing      (s),
_description (0) {
	setFontDescription(d);
}

TextOptions::~TextOptions() {
	// _unrefFontDescription();
}

void TextOptions::setFontDescription(const std::string& descr) {
	_unrefFontDescription();

	_description = pango_font_description_from_string(descr.c_str());
}

void TextOptions::setFontFamily(const std::string& family) {
	if(_description) pango_font_description_set_family(_description, family.c_str());
}

void TextOptions::setFontStyle(PangoStyle style) {
	if(_description) pango_font_description_set_style(_description, style);
}

void TextOptions::setFontVariant(PangoVariant variant) {
	if(_description) pango_font_description_set_variant(_description, variant);
}

void TextOptions::setFontWeight(PangoWeight weight) {
	if(_description) pango_font_description_set_weight(_description, weight);
}

void TextOptions::setFontSize(int size) {
	if(_description) pango_font_description_set_size(_description, size * PANGO_SCALE);
}

bool TextOptions::setupPangoLayout(PangoLayout* layout) const {
	if(alignment != ALIGN_JUSTIFY) {
		PangoAlignment pa = PANGO_ALIGN_LEFT;

		if(alignment == ALIGN_CENTER) pa = PANGO_ALIGN_CENTER;

		else if(alignment == ALIGN_RIGHT) pa = PANGO_ALIGN_RIGHT;
	
		pango_layout_set_alignment(layout, pa);
	}

	else pango_layout_set_justify(layout, true);

	if(_description) pango_layout_set_font_description(layout, _description);

	if(width > 0) pango_layout_set_width(layout, width * PANGO_SCALE);

	if(height > 0) pango_layout_set_height(layout, height * PANGO_SCALE);

	if(indent > 0) pango_layout_set_indent(layout, indent * PANGO_SCALE);

	if(spacing > 0) pango_layout_set_spacing(layout, spacing * PANGO_SCALE);

	return true;
}

void TextOptions::_unrefFontDescription() {
	if(_description) g_object_unref(_description);

	_description = 0;
}

Text::Text():
TextRenderer(),
osg::MatrixTransform(),
_lastX    (0),
_lastY    (0),
_baseline (0) {
	addChild(new osg::Geode());
}

void Text::drawGlyphs(PangoFont* font, PangoGlyphString* glyphs, int x, int y) {
	// Get the GlyphCache from a key, which may or may not be set via PangoAttr if I
	// can it to work properly. :)
	GlyphCache* gc = Context::instance().getGlyphCache(
		font,
		_getRequestedGlyphCacheRenderer()
	);

	osg::Vec2::value_type currentY = -(y / PANGO_SCALE);

	osg::Vec2 layoutPos(x / PANGO_SCALE, currentY);

	if(static_cast<int>(currentY) == _lastY) layoutPos[0] += _lastX;

	else _lastX = 0.0f;

	// TODO: Enabling optional honoring of extents...
	osg::Vec4 extents = osg::Vec4(); //gc->getExtraGlyphExtents();

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

		osg::Vec3 color(1.0f, 1.0f, 1.0f);

		const osg::Vec3* pangoColor = _getRequestedPangoColor();

		if(pangoColor) color = *pangoColor;

		GlyphGeometryVector& ggv = _ggMap[GlyphGeometryMapKey(font, color)];
	
		if(cg->size.x() > 0.0f && cg->size.y() > 0.0f) {
			osg::Vec2 pos(
				(gi->geometry.x_offset / PANGO_SCALE) + extents[0],
				(gi->geometry.y_offset / PANGO_SCALE) + extents[1]
			);

			if(cg->img >= ggv.size()) ggv.push_back(
				new GlyphGeometry(gc->hasEffects())
			);
	
			ggv[cg->img]->pushCachedGlyphAt(
				cg,
				pos + layoutPos,
				gc->hasEffects(),
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
			GlyphCache* gc = Context::instance().getGlyphCache(
				g->first.first,
				_getRequestedGlyphCacheRenderer()
			);

			if(!ggv[i]->finalize(GlyphTexEnvCombineState(
				gc->getTexture(i),
				gc->getTexture(i, true),
				g->first.second,
				osg::Vec3(0.0f, 0.0f, 0.0f),
				1.0f
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

	drawLayout(layout, x, y);

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

/*
void Text::writeAllImages(const std::string& path) {
	for(FontMap::iterator i = _fontMap.begin(); i != _fontMap.end(); i++) {
		PangoFontDescription* d  = pango_font_describe(i->first);
		GlyphCache*           gc = i->second.get();

		std::ostringstream os;

		std::string family(pango_font_description_get_family(d));

		std::replace(family.begin(), family.end(), ' ', '_');

		os << path << "_" << family << "_" << pango_font_description_get_size(d);

		gc->writeImagesAsFiles(os.str());

		pango_font_description_free(d);
	}
}
*/

void Text::setPosition(const osg::Vec3& pos) {
	setMatrix(osg::Matrix::translate(pos));
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
