Introduction

Welcome to the osgPango development page. In order to get started, you'll need to compile osgCairo first, which is another project hosted here.

osgPango is a nodekit for OpenSceneGraph? that drastically improves the font rendering flexibility (and quality) available for use in your OSG applications. However, it does this by using two very powerful libraries (cairo and pango), so these dependencies must be available on your operating system. osgPango is HEAVILY tested on Linux, and only less so on Windows. I have not, unfortunately, had anyone with access to Mac test it out. :(
Technical

osgPango renders fonts using the following setup:

- A variable number of "layers" (textures) are allocated, per-font, that will eventually ALL be combined on a textured quad to form the final resultant glyph. You could think of this as a kind of osgPPU pipeline, if you're familiar with that fantastic nodekit. :)
- Each layer is drawn to using a polymorphic API and the Cairo library. The overridden method in your subclass can do virtually ANYTHING with ANY layer, and for demonstration some standard font effects are provided: shadows, outlines, bitmap fills, etc.
- Each layer is packed into an array that is fed to the current state, along with a global alpha value and a color value (per-layer) that can either be ignored--if your layers include color data--or instead blended with the alpha value provided in the texture; in fact, this is the most common osgPango usage: the GL_ALPHA texture drawn by Cairo is modulated by a color value specified in Pango markup to produce the final color.
- Finally, a GLSL shader comes along and does the final compositing. Code is provided inside of osgPango to help you generate these shaders (or, preferably, just use the ones we provide!), but you're welcome to "roll your own." 

=Videos=

And finally, we have a (somewhat older) video of osgPango + osgAnimation demonstrating the VERY IMPORTANT fact that each glyph and it's associated quad is readily available to the user to either manipulate directly or to be modified in derived classes.

<wiki:video url="http://www.youtube.com/watch?v=Q-kvTtlpbLA"/>

=Building=

OSG, osgCairo, and you'll also need one of these (though you already needed it to build osgCairo, so...)

[http://www.gtk.org/download/win64.php 64bit GTK+ Libs]

[http://www.gtk.org/download/win32.php 32bit GTK+ Libs]
