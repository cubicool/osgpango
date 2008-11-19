// -*-c++-*- osgPango - Copyright (C) 2008 Jeremy Moles

#include <osgPango/Text>

namespace osgPango {

Renderer* Text::_renderer = 0;

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

	renderer_class->draw_glyphs    = &Text::drawGlyphs;
	renderer_class->draw_rectangle = &Text::drawRectangle;
	renderer_class->draw_trapezoid = &Text::drawTrapezoid;
}

Text::Text(Font* font, GlyphEffectsMethod gem):
_font         (font ? font : new Font()),
_text         ("", osgText::String::ENCODING_UTF8),
_layout       (pango_layout_new(Font::getPangoContext())),
_gem          (gem),
_color        (1.0f, 1.0f, 1.0f),
_effectsColor (0.0f, 0.0f, 0.0f),
_alpha        (1.0f),
_baseline     (0) {
	if(!_renderer) _renderer = static_cast<Renderer*>(g_object_new(TYPE_RENDERER, 0));

	pango_layout_set_font_description(_layout, _font->getDescription());

	// Add the child Geode that we use throughout the API.
	addChild(new osg::Geode());
}

void Text::setText(const std::string& str) {
	if(str.size()) {
		_text.set(str, osgText::String::ENCODING_UTF8);

		std::string utf8 = _text.createUTF8EncodedString();

		_pos.clear();

		pango_layout_set_text(_layout, utf8.c_str(), -1);
	}

	_renderer->text = const_cast<Text*>(this);

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
	GlyphGeometryVector ggv(gc->getNumImages(), 0);

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

	osg::Geode* geode = getGeode();

	if(!geode) return;

	geode->removeDrawables(0, geode->getNumDrawables());

	for(unsigned int i = 0; i < ggv.size(); i++) {
		if(!ggv[i]->finalize(
			gc->getImage(i),
			gc->getImage(i, true),
			_color,
			_effectsColor,
			_alpha
		)) continue;

		geode->addDrawable(ggv[i]);
	}
}

void Text::setAlignment(Text::Alignment align) {
	if(align != ALIGN_JUSTIFY) {
		PangoAlignment pa = PANGO_ALIGN_LEFT;

		if(align == ALIGN_CENTER) pa = PANGO_ALIGN_CENTER;

		else if(align == ALIGN_RIGHT) pa = PANGO_ALIGN_RIGHT;
	
		pango_layout_set_alignment(_layout, pa);
	}

	else pango_layout_set_justify(_layout, true);
}

void Text::setWidth(unsigned int width) {
	pango_layout_set_width(_layout, width * PANGO_SCALE);
}

void Text::setColor(const osg::Vec3& color) {
	_color = color;
}

void Text::setEffectsColor(const osg::Vec3& color) {
	_effectsColor = color;
}

void Text::setAlpha(double alpha) {
	_alpha = alpha;
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

void Text::drawGlyphs(
	PangoRenderer*    renderer,
	PangoFont*        font,
	PangoGlyphString* glyphs,
	int               x,
	int               y
) {
	Renderer* r = RENDERER(renderer);

	if(!r || !r->text) return;

	Text*       t  = r->text;
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

void Text::drawRectangle(PangoRenderer*, PangoRenderPart, int, int, int, int) {
}

void Text::drawTrapezoid(
	PangoRenderer*,
	PangoRenderPart,
	double,
	double,
	double,
	double,
	double,
	double
) {
}

}
