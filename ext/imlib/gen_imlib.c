#include "php_gtk.h"
#if HAVE_PHP_GTK

#include <gdk_imlib.h>
#include "php_imlib.h"
#include "ext/gtk+/php_gtk+.h"

void php_gdkimlib_register_constants(int module_number TSRMLS_DC)
{


}


PHP_FUNCTION(gdk_imlib_init)
{
	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	gdk_imlib_init();
	RETURN_NULL();
}


PHP_FUNCTION(dk_imlib_init_params)
{
	long rend_type;

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &rend_type))
		return;

	dk_imlib_init_params((gint)rend_type);
	RETURN_NULL();
}


PHP_FUNCTION(gdk_imlib_get_render_type)
{
	long php_retval;

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	php_retval = gdk_imlib_get_render_type();
	RETURN_LONG(php_retval);
}


PHP_FUNCTION(gdk_imlib_set_render_type)
{
	long rend_type;

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &rend_type))
		return;

	gdk_imlib_set_render_type((gint)rend_type);
	RETURN_NULL();
}


PHP_FUNCTION(gdk_imlib_load_colors)
{
	char *file;
	long php_retval;

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "s", &file))
		return;

	php_retval = gdk_imlib_load_colors(file);
	RETURN_LONG(php_retval);
}


PHP_FUNCTION(gdk_imlib_best_color_get)
{
	zval *c;

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "O", &c, gdk_color_ce))
		return;

	gdk_imlib_best_color_get(PHP_GDK_COLOR_GET(c));
	RETURN_NULL();
}


PHP_FUNCTION(gdk_imlib_render)
{
	zval *image;
	long width, height, php_retval;

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "Oii", &image, gdk_imlib_image_ce, &width, &height))
		return;

	php_retval = gdk_imlib_render(PHP_GDK_IMLIB_IMAGE_GET(image), (gint)width, (gint)height);
	RETURN_LONG(php_retval);
}


PHP_FUNCTION(gdk_imlib_destroy_image)
{
	zval *image;

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "O", &image, gdk_imlib_image_ce))
		return;

	gdk_imlib_destroy_image(PHP_GDK_IMLIB_IMAGE_GET(image));
	RETURN_NULL();
}


PHP_FUNCTION(gdk_imlib_kill_image)
{
	zval *image;

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "O", &image, gdk_imlib_image_ce))
		return;

	gdk_imlib_kill_image(PHP_GDK_IMLIB_IMAGE_GET(image));
	RETURN_NULL();
}


PHP_FUNCTION(gdk_imlib_free_colors)
{
	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	gdk_imlib_free_colors();
	RETURN_NULL();
}


PHP_FUNCTION(gdk_imlib_free_pixmap)
{
	zval *pixmap;

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "O", &pixmap, gdk_pixmap_ce))
		return;

	gdk_imlib_free_pixmap(PHP_GDK_PIXMAP_GET(pixmap));
	RETURN_NULL();
}


PHP_FUNCTION(gdk_imlib_free_bitmap)
{
	zval *bitmap;

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "O", &bitmap, gdk_bitmap_ce))
		return;

	gdk_imlib_free_bitmap(PHP_GDK_BITMAP_GET(bitmap));
	RETURN_NULL();
}

PHP_FUNCTION(gdk_imlib_get_image_border)
{
	zval *image;
	GdkImlibBorder border;
	

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "O", &image, gdk_imlib_image_ce))
		return;
	
	gdk_imlib_get_image_border(PHP_GDK_IMLIB_IMAGE_GET(image),  &border);
	
	
	array_init(return_value);
	add_next_index_long(return_value, (long)border.left);
	
	add_next_index_long(return_value, (long)border.right);
	add_next_index_long(return_value, (long)border.top);
	add_next_index_long(return_value, (long)border.bottom);
	
	 
}

PHP_FUNCTION(gdk_imlib_set_image_border)
{
	zval *image;
	GdkImlibBorder border;
	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "Oiiii", &image, gdk_imlib_image_ce,&border.left,&border.right,&border.top,&border.bottom ))
		return;

	gdk_imlib_set_image_border(PHP_GDK_IMLIB_IMAGE_GET(image), &border);
	RETURN_NULL();
}

PHP_FUNCTION(gdk_imlib_save_image_to_eim)
{
	zval *image;
	char *file;
	long php_retval;

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "Os", &image, gdk_imlib_image_ce, &file))
		return;

	php_retval = gdk_imlib_save_image_to_eim(PHP_GDK_IMLIB_IMAGE_GET(image), file);
	RETURN_LONG(php_retval);
}


PHP_FUNCTION(gdk_imlib_add_image_to_eim)
{
	zval *image;
	char *file;
	long php_retval;

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "Os", &image, gdk_imlib_image_ce, &file))
		return;

	php_retval = gdk_imlib_add_image_to_eim(PHP_GDK_IMLIB_IMAGE_GET(image), file);
	RETURN_LONG(php_retval);
}


PHP_FUNCTION(gdk_imlib_save_image_to_ppm)
{
	zval *image;
	char *file;
	long php_retval;

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "Os", &image, gdk_imlib_image_ce, &file))
		return;

	php_retval = gdk_imlib_save_image_to_ppm(PHP_GDK_IMLIB_IMAGE_GET(image), file);
	RETURN_LONG(php_retval);
}


PHP_FUNCTION(gdk_imlib_apply_modifiers_to_rgb)
{
	zval *im;

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "O", &im, gdk_imlib_image_ce))
		return;

	gdk_imlib_apply_modifiers_to_rgb(PHP_GDK_IMLIB_IMAGE_GET(im));
	RETURN_NULL();
}


PHP_FUNCTION(gdk_imlib_changed_image)
{
	zval *im;

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "O", &im, gdk_imlib_image_ce))
		return;

	gdk_imlib_changed_image(PHP_GDK_IMLIB_IMAGE_GET(im));
	RETURN_NULL();
}


PHP_FUNCTION(gdk_imlib_apply_image)
{
	zval *im, *p;

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "OO", &im, gdk_imlib_image_ce, &p, gdk_window_ce))
		return;

	gdk_imlib_apply_image(PHP_GDK_IMLIB_IMAGE_GET(im), PHP_GDK_WINDOW_GET(p));
	RETURN_NULL();
}


PHP_FUNCTION(gdk_imlib_paste_image)
{
	zval *im, *p;
	long x, y, w, h;

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "OOiiii", &im, gdk_imlib_image_ce, &p, gdk_window_ce, &x, &y, &w, &h))
		return;

	gdk_imlib_paste_image(PHP_GDK_IMLIB_IMAGE_GET(im), PHP_GDK_WINDOW_GET(p), (gint)x, (gint)y, (gint)w, (gint)h);
	RETURN_NULL();
}


PHP_FUNCTION(gdk_imlib_paste_image_border)
{
	zval *im, *p;
	long x, y, w, h;

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "OOiiii", &im, gdk_imlib_image_ce, &p, gdk_window_ce, &x, &y, &w, &h))
		return;

	gdk_imlib_paste_image_border(PHP_GDK_IMLIB_IMAGE_GET(im), PHP_GDK_WINDOW_GET(p), (gint)x, (gint)y, (gint)w, (gint)h);
	RETURN_NULL();
}


PHP_FUNCTION(gdk_imlib_flip_image_horizontal)
{
	zval *im;

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "O", &im, gdk_imlib_image_ce))
		return;

	gdk_imlib_flip_image_horizontal(PHP_GDK_IMLIB_IMAGE_GET(im));
	RETURN_NULL();
}


PHP_FUNCTION(gdk_imlib_flip_image_vertical)
{
	zval *im;

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "O", &im, gdk_imlib_image_ce))
		return;

	gdk_imlib_flip_image_vertical(PHP_GDK_IMLIB_IMAGE_GET(im));
	RETURN_NULL();
}


PHP_FUNCTION(gdk_imlib_rotate_image)
{
	zval *im;
	long d;

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "Oi", &im, gdk_imlib_image_ce, &d))
		return;

	gdk_imlib_rotate_image(PHP_GDK_IMLIB_IMAGE_GET(im), (gint)d);
	RETURN_NULL();
}


PHP_FUNCTION(gdk_imlib_get_fallback)
{
	long php_retval;

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	php_retval = gdk_imlib_get_fallback();
	RETURN_LONG(php_retval);
}


PHP_FUNCTION(gdk_imlib_set_fallback)
{
	long fallback;

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &fallback))
		return;

	gdk_imlib_set_fallback((gint)fallback);
	RETURN_NULL();
}


PHP_FUNCTION(gdk_imlib_crop_image)
{
	zval *im;
	long x, y, w, h;

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "Oiiii", &im, gdk_imlib_image_ce, &x, &y, &w, &h))
		return;

	gdk_imlib_crop_image(PHP_GDK_IMLIB_IMAGE_GET(im), (gint)x, (gint)y, (gint)w, (gint)h);
	RETURN_NULL();
}


PHP_FUNCTION(gdk_imlib_set_cache_info)
{
	long cache_pixmaps, cache_images;

	if (!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &cache_pixmaps, &cache_images))
		return;

	gdk_imlib_set_cache_info((int)cache_pixmaps, (int)cache_images);
	RETURN_NULL();
}

PHP_GTK_EXPORT_CE(gdkimlib_ce);

static function_entry php_gdkimlib_functions[] = {
	{"_init",	PHP_FN(gdk_imlib_init),	NULL},
	{"gdk_imlib_init_params",	PHP_FN(dk_imlib_init_params),	NULL},
	{"_get_render_type",	PHP_FN(gdk_imlib_get_render_type),	NULL},
	{"_set_render_type",	PHP_FN(gdk_imlib_set_render_type),	NULL},
	{"_load_colors",	PHP_FN(gdk_imlib_load_colors),	NULL},
	{"_best_color_get",	PHP_FN(gdk_imlib_best_color_get),	NULL},
	{"_render",	PHP_FN(gdk_imlib_render),	NULL},
	{"_destroy_image",	PHP_FN(gdk_imlib_destroy_image),	NULL},
	{"_kill_image",	PHP_FN(gdk_imlib_kill_image),	NULL},
	{"_free_colors",	PHP_FN(gdk_imlib_free_colors),	NULL},
	{"_free_pixmap",	PHP_FN(gdk_imlib_free_pixmap),	NULL},
	{"_free_bitmap",	PHP_FN(gdk_imlib_free_bitmap),	NULL},
	{"_get_image_border",	PHP_FN(gdk_imlib_get_image_border),	NULL},
	{"_set_image_border",	PHP_FN(gdk_imlib_set_image_border),	NULL},
	{"_save_image_to_eim",	PHP_FN(gdk_imlib_save_image_to_eim),	NULL},
	{"_add_image_to_eim",	PHP_FN(gdk_imlib_add_image_to_eim),	NULL},
	{"_save_image_to_ppm",	PHP_FN(gdk_imlib_save_image_to_ppm),	NULL},
	{"_apply_modifiers_to_rgb",	PHP_FN(gdk_imlib_apply_modifiers_to_rgb),	NULL},
	{"_changed_image",	PHP_FN(gdk_imlib_changed_image),	NULL},
	{"_apply_image",	PHP_FN(gdk_imlib_apply_image),	NULL},
	{"_paste_image",	PHP_FN(gdk_imlib_paste_image),	NULL},
	{"_paste_image_border",	PHP_FN(gdk_imlib_paste_image_border),	NULL},
	{"_flip_image_horizontal",	PHP_FN(gdk_imlib_flip_image_horizontal),	NULL},
	{"_flip_image_vertical",	PHP_FN(gdk_imlib_flip_image_vertical),	NULL},
	{"_rotate_image",	PHP_FN(gdk_imlib_rotate_image),	NULL},
	{"_get_fallback",	PHP_FN(gdk_imlib_get_fallback),	NULL},
	{"_set_fallback",	PHP_FN(gdk_imlib_set_fallback),	NULL},
	{"_crop_image",	PHP_FN(gdk_imlib_crop_image),	NULL},
	{"_set_cache_info",	PHP_FN(gdk_imlib_set_cache_info),	NULL},
	{NULL, NULL, NULL}
};


void php_gdkimlib_register_classes(void)
{
	zend_class_entry ce;
	TSRMLS_FETCH();

	INIT_OVERLOADED_CLASS_ENTRY(ce, "GdkImlib", php_gdkimlib_functions, NULL, NULL, php_gtk_set_property);
	gdkimlib_ce = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);


}

#endif /* HAVE_PHP_GTK */
