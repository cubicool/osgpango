############
Introduction
############

Welcome to the osgPango development page. In order to get started, you'll need
to compile `osgCairo <http://github.com/cubicool/osgcairo>`_, which is another
project hosted here.

osgPango is a nodekit for OpenSceneGraph that drastically improves the font
rendering flexibility (and quality) available for use in your OSG applications.
However, it does this by using two very powerful libraries (cairo and pango), so
these dependencies must be available on your operating system. osgPango is
HEAVILY tested on Linux, but woefully less so on Windows.

=========
Technical
=========

osgPango renders fonts using the following setup:

- A variable number of "layers" (textures) are allocated, per-font, that will
  eventually ALL be combined on a textured quad to form the final resultant
  glyph. You could think of this as a kind of osgPPU pipeline, if you're
  familiar with that fantastic nodekit. :)

- Each layer is drawn to using a polymorphic API and the Cairo library. The
  overridden method in your subclass can do virtually ANYTHING with ANY layer,
  and for demonstration some standard font effects are provided: shadows,
  outlines, bitmap fills, etc.

- Each layer is packed into an array that is fed to the current state, along
  with a global alpha value and a color value (per-layer) that can either be
  ignored--if your layers include color data--or instead blended with the alpha
  value provided in the texture; in fact, this is the most common osgPango
  usage: the GL_ALPHA texture drawn by Cairo is modulated by a color value
  specified in Pango markup to produce the final color.

- Finally, a GLSL shader comes along and does the final compositing. Code is
  provided inside of osgPango to help you generate these shaders (or,
  preferably, just use the ones we provide!), but you're welcome to "*roll your
  own*" as well.

Videos
======

And finally, we have a (somewhat older) video of osgPango + osgAnimation
demonstrating the VERY IMPORTANT fact that each glyph and its associated quad
are readily available to the user for direct, immediate manipulation.

`VIDEO-001 (YouTube) <http://www.youtube.com/watch?v=Q-kvTtlpbLA>`_

Building
========

OSG, osgCairo, and you'll also need one of these (though you already needed it
to build osgCairo, so...)

`64bit GTK+ Libs <http://www.gtk.org/download/win64.php 64bit GTK+ Libs>`_

`32bit GTK+ Libs <http://www.gtk.org/download/win32.php>`_

=======
Authors
=======

+-----------------+----------------------------------+-------------------------+
| **Author**      | **Contact**                      | *Contributions...*      |
+=================+==================================+=========================+
| Jeremy Moles    | cubicool@gmail.com               | Lead hacker/developer   |
+-----------------+----------------------------------+-------------------------+
| Jaromir Vitek   | jaromir.vitek@gmail.com          | Professional hacking    |
+-----------------+----------------------------------+-------------------------+
| Cedric Pinson   | cedric.pinson@plopbyte.net       | library exercise and    |
|                 |                                  | the creation of the     |
|                 |                                  | osgAnimation video      |
+-----------------+----------------------------------+-------------------------+
| Serge Lages     | serge.lages@tharsis-software.com | Bug fixes               |
+-----------------+----------------------------------+-------------------------+
| Sergey Kurdakov | sergey.forum@gmail.com           | Bug fixes               |
+-----------------+----------------------------------+-------------------------+

====
TODO
====

- Add Glyph loading from images/metadata; ESPECIALLY DISTANCE FIELD TEXT!

- Add the ability to dump the caches in one large PNG, instead of a
  bunch of pngs. This should be a "developer debug" image.

- Add a wrapper around having to use <span> all the time; perhaps
  a kind of ostream<< clone.
