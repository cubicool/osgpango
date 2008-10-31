// -*-c++-*- osgPango - Copyright (C) 2008 Jeremy Moles

#include <iostream>
#include <cmath>
#include <osg/io_utils>
#include <osgPango/Layout>

namespace osgPango {

Renderer* Layout::_renderer = 0;

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

	renderer_class->draw_glyphs    = &Layout::drawGlyphs;
	renderer_class->draw_rectangle = &Layout::drawRectangle;
	renderer_class->draw_trapezoid = &Layout::drawTrapezoid;
}

Layout::Layout(Font* font, GlyphEffectsMethod gem):
_font         (font ? font : new Font()),
_text         ("", osgLayout::String::ENCODING_UTF8),
_layout       (pango_layout_new(Font::getPangoContext())),
_gem          (gem),
_color        (1.0f, 1.0f, 1.0f),
_effectsColor (0.0f, 0.0f, 0.0f),
_alpha        (1.0f),
_baseline     (0) {
	if(!_renderer) _renderer = static_cast<Renderer*>(g_object_new(TYPE_RENDERER, 0));

	pango_layout_set_font_description(_layout, _font->getDescription());
}

void Layout::setLayout(const std::string& str) {
	if(str.size()) {
		_text.set(str, osgLayout::String::ENCODING_UTF8);

		std::string utf8 = _text.createUTF8EncodedString();

		_pos.clear();

		pango_layout_set_text(_layout, utf8.c_str(), -1);
	}

	_renderer->text  = const_cast<Layout*>(this);
	_renderer->count = 0;

	pango_renderer_draw_layout(PANGO_RENDERER(_renderer), _layout, 0, 0);

	// Here we calculate the size of the text we just rendered. This may be possible to do
	// WHILE we render it in drawGlyphs later, but for the time being it's simply too
	// difficult.
	PangoRectangle rect;

	pango_layout_get_pixel_extents(_layout, &rect, 0);

	GlyphCache* gc = _font->getGlyphCache();

	const osg::Vec4& extents = gc->getExtraGlyphExtents();

	_origin.set(rect.x + extents[0], rect.y - extents[1] - extents[3]);

	_effectsSize.y() = extents[3];

	_size.set(rect.width, rect.height);

	_size += _effectsSize;

	// Now we set the texture values, etc...
	_ggv.resize(gc->getNumImages(), 0);

	for(unsigned int i = 0; i < ggv.size(); i++) ggv[i] = new GlyphGeometry(gc->hasEffects());

	/*
	typedef std::pair<const CachedGlyph*, const osg::Vec2&> ReversedListItem;
	typedef std::list<ReversedListItem>                     ReversedList;

	ReversedList rlist;

	for(GlyphPositionList::iterator i = _pos.begin(); i != _pos.end(); i++) rlist.push_front(
		ReversedListItem(gc->getCachedGlyph(i->first), i->second)
	);

	for(ReversedList::const_iterator i = rlist.begin(); i != rlist.end(); i++) {
		const CachedGlyph* cg = i->first;
	
		std::cout << i->second << std::endl;

		ggv[cg->img]->pushCachedGlyphAt(cg, i->second, gc->hasEffects(), _gem);	
	}
	*/

	for(GlyphPositionList::iterator i = _pos.begin(); i != _pos.end(); i++) {
		const CachedGlyph* cg = gc->getCachedGlyph(i->first);

		ggv[cg->img]->pushCachedGlyphAt(cg, i->second, gc->hasEffects(), _gem);
	}

	// removeDrawables(0, getNumDrawables());

	for(unsigned int i = 0; i < ggv.size(); i++) {
		if(!ggv[i]->finalize(
			gc->getImage(i),
			gc->getImage(i, true),
			_color,
			_effectsColor,
			_alpha
		)) continue;

		// addDrawable(ggv[i]);
	}

	textUpdated(_ggv);
}

void Layout::setAlignment(Layout::Alignment align) {
	if(align != ALIGN_JUSTIFY) {
		PangoAlignment pa = PANGO_ALIGN_LEFT;

		if(align == ALIGN_CENTER) pa = PANGO_ALIGN_CENTER;

		else if(align == ALIGN_RIGHT) pa = PANGO_ALIGN_RIGHT;
	
		pango_layout_set_alignment(_layout, pa);
	}

	else pango_layout_set_justify(_layout, true);
}

void Layout::setWidth(unsigned int width) {
	pango_layout_set_width(_layout, width * PANGO_SCALE);
}

void Layout::setColor(const osg::Vec3& color) {
	_color = color;
}

void Layout::setEffectsColor(const osg::Vec3& color) {
	_effectsColor = color;
}

void Layout::setAlpha(double alpha) {
	_alpha = alpha;
}

osg::Vec2 Layout::getOriginBaseline() const {
	return osg::Vec2(-_origin.x(), _baseline);
}

osg::Vec2 Layout::getOriginTranslated() const {
	return osg::Vec2(-_origin.x(), _size.y() + _origin.y());
}

void Layout::drawGlyphs(
	PangoRenderer*    renderer,
	PangoFont*        font,
	PangoGlyphString* glyphs,
	int               x,
	int               y
) {
	Renderer* r = RENDERER(renderer);

	if(!r || !r->text) return;

	Layout*       t  = r->text;
	GlyphCache* gc = t->getFont()->getGlyphCache();

	if(!gc) return;

	osg::Vec2 layoutPos(x / PANGO_SCALE, -(y / PANGO_SCALE));

	osg::Vec4 extents = gc->getExtraGlyphExtents();

	unsigned int effectsWidth = 0;

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

		if(cg->size.x() > 0.0f && cg->size.y() > 0.0f) {
			osg::Vec2 pos(
				(gi->geometry.x_offset / PANGO_SCALE) + extents[0],
				(gi->geometry.y_offset / PANGO_SCALE) + extents[1]
			);
	
			t->_pos.push_back(GlyphPositionPair(
				gi->glyph,
				pos + layoutPos
			));
		}
		
		layoutPos += osg::Vec2((gi->geometry.width / PANGO_SCALE) + extents[0], 0.0f);

		effectsWidth += extents[0];
	}

	t->_baseline = y / PANGO_SCALE;

	if(t->_effectsSize.x() < effectsWidth) t->_effectsSize.x() = effectsWidth;
}

void Layout::drawRectangle(PangoRenderer*, PangoRenderPart, int, int, int, int) {
	std::cout << "Layout::drawRectangle" << std::endl;
}

void Layout::drawTrapezoid(
	PangoRenderer*,
	PangoRenderPart,
	double,
	double,
	double,
	double,
	double,
	double
) {
	std::cout << "Layout::drawTrapezoid" << std::endl;
}

}
