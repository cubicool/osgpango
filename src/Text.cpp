// -*-c++-*- osgPango - Copyright (C) 2009 Jeremy Moles

#include <iostream>
#include <sstream>
#include <algorithm>
#include <osgPango/Text>

namespace osgPango {

Renderer* TextRenderer::_renderer = 0;

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
		_renderer->mutex    = new OpenThreads::Mutex();
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

	if(fg) _renderer->fg.set(
		fg->red / 65535.0f,
		fg->green / 65535.0f,
		fg->blue / 65535.0f
	);

	else _renderer->fg.set(-1.0f, -1.0, -1.0f);

	RENDERER(renderer)->renderer->drawGlyphs(font, glyphs, x, y);
}

const osg::Vec3* TextRenderer::_getRequestedPangoColor() const {
	if(_renderer->fg[0] != -1.0f) return &_renderer->fg;

	else return 0;
}

void TextRenderer::drawLayout(PangoLayout* layout, unsigned int x, unsigned int y) {
	OpenThreads::ScopedLock<OpenThreads::Mutex> lock(*_renderer->mutex);

	_renderer->renderer = const_cast<TextRenderer*>(this);

	pango_renderer_draw_layout(
		PANGO_RENDERER(_renderer),
		layout,
		x * PANGO_SCALE,
		-(y * PANGO_SCALE)
	);
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
	GlyphCache* gc = _fontMap[font].get();

	if(!gc) {
		gc = new GlyphCache(); //Outline(512, 512, 1);

		_fontMap[font] = gc;
	}

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
			if(!ggv[i]->finalize(GlyphTexEnvCombineState(
				_fontMap[g->first.first]->getTexture(i),
				_fontMap[g->first.first]->getTexture(i, true),
				g->first.second,
				osg::Vec3(0.0f, 0.0f, 0.0f),
				1.0f
			))) continue;

			geode->addDrawable(ggv[i]);
		}
	}

	return true;
}

void Text::addText(const std::string& str, int x, int y) {
	String       text;
	PangoLayout* layout = pango_layout_new(Font::getPangoContext());

	if(str.size()) {
		text.set(str, osgText::String::ENCODING_UTF8);

		std::string utf8 = text.createUTF8EncodedString();

		pango_layout_set_width(layout, 500 * PANGO_SCALE);
		pango_layout_set_justify(layout, true);
		pango_layout_set_markup(layout, utf8.c_str(), -1);
	}

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
