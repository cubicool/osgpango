# This file adds support for the GTK+ Win32 Development bundle. It is REALLY
# unlikely that anyone would EVER try and build all this by default.
# You can get the bundle from:
#
# http://ftp.gnome.org/pub/gnome/binaries/win32/gtk+/2.22/gtk+-bundle_2.22.0-20101016_win32.zip
#
# This bundle was tested against on 12/2/2010 by Jeremy Moles.

SET(WIN32_GTK_BUNDLE_DIR "" CACHE PATH "Location of the GTK+ Win32 bundle.")

FIND_PATH(CAIRO_INCLUDE_DIR cairo.h ${WIN32_GTK_BUNDLE_DIR}/include/cairo)
FIND_PATH(PANGOCAIRO_INCLUDE_DIR pango/pangocairo.h ${WIN32_GTK_BUNDLE_DIR}/include/pango-1.0)
FIND_PATH(GLIB_INCLUDE_DIR glib/glib.h ${WIN32_GTK_BUNDLE_DIR}/include/glib-2.0)

FIND_PATH(PANGOCAIRO_LIBRARY_DIR pangocairo.lib ${WIN32_GTK_BUNDLE_DIR}/lib)

