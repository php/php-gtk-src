#include "php_gtk.h"
#include "php_gtk_api.h"

#if HAVE_SCINTILLA
#include "gtkscintilla.h"
#include "ext/gtk+/php_gtk+.h"

PHP_GTK_EXPORT_CE(scintilla_ce);

#ifdef DOUBLE_CLICK
#undef DOUBLE_CLICK
#endif
enum {
	STYLE_NEEDED,
	CHAR_ADDED,
	SAVE_POINT_REACHED,
	SAVE_POINT_LEFT,
	MODIFY_ATTEMPT_RO,
	KEY,
	DOUBLE_CLICK,
	UPDATE_UI,
	MODIFIED,
	MACRO_RECORD,
	MARGIN_CLICK,
	NEED_SHOWN,
	PAINTED,
	USER_LIST_SELECTION,
	URI_DROPPED,
	DWELL_START,
	DWELL_END,
	ZOOM,
	LAST_SIGNAL
};

static void gtk_scintilla_class_init (GtkScintillaClass *klass);
static void gtk_scintilla_init       (GtkScintilla      *sci);
static void gtk_scintilla_destroy    (GtkObject         *object);
static void notify_cb                (GtkWidget         *w,
																		gint               param,
																		gpointer           notif,
																		gpointer           data);
static void pass_throug_key          (GtkScintilla      *sci,
																		gint               ch,
																		gint               modifiers);

static gpointer parent_class;
static guint signals[LAST_SIGNAL] = { 0 };

GType gtk_scintilla_get_type(void) {
	static GType our_type = 0;
	
	if(!our_type) {
			static const GTypeInfo our_info =
			{
					sizeof(GtkScintillaClass),
					NULL,               /* base_init */
					NULL,               /* base_finalize */
					(GClassInitFunc) gtk_scintilla_class_init,
					NULL,               /* class_finalize */
					NULL,               /* class_data */
					sizeof(GtkScintilla),
					0,                  /* n_preallocs */
					(GInstanceInitFunc) gtk_scintilla_init,
			};

			our_type = g_type_register_static(GTK_TYPE_FRAME, "GtkScintilla",
																				 &our_info, 0);
	}
	
	return our_type;
}

static void gtk_scintilla_class_init(GtkScintillaClass *klass) {
	GtkObjectClass *object_class;
	
	object_class = (GtkObjectClass *) klass;
	parent_class = g_type_class_peek_parent(klass);

	object_class->destroy = gtk_scintilla_destroy;
	
	signals[STYLE_NEEDED] =
			g_signal_new("style_needed",
										G_OBJECT_CLASS_TYPE(object_class),
										G_SIGNAL_RUN_FIRST,
										G_STRUCT_OFFSET(GtkScintillaClass, style_needed),
										NULL, NULL,
										g_cclosure_marshal_VOID__VOID,
										G_TYPE_NONE, 1,
										G_TYPE_INT);

	signals[CHAR_ADDED] =
			g_signal_new("char_added",
										G_OBJECT_CLASS_TYPE(object_class),
										G_SIGNAL_RUN_FIRST,
										G_STRUCT_OFFSET(GtkScintillaClass, char_added),
										NULL, NULL,
										g_cclosure_marshal_VOID__UINT,
										G_TYPE_NONE, 1,
										G_TYPE_INT);

	signals[SAVE_POINT_REACHED] =
			g_signal_new("save_point_reached",
										G_OBJECT_CLASS_TYPE(object_class),
										G_SIGNAL_RUN_FIRST,
										G_STRUCT_OFFSET(GtkScintillaClass, save_point_reached),
										NULL, NULL,
										g_cclosure_marshal_VOID__VOID,
										G_TYPE_NONE, 0);

	signals[SAVE_POINT_LEFT] =
			g_signal_new("save_point_left",
										G_OBJECT_CLASS_TYPE(object_class),
										G_SIGNAL_RUN_FIRST,
										G_STRUCT_OFFSET(GtkScintillaClass, save_point_left),
										NULL, NULL,
										g_cclosure_marshal_VOID__VOID,
										G_TYPE_NONE, 0);

	signals[MODIFY_ATTEMPT_RO] =
			g_signal_new("modify_attempt_ro",
										G_OBJECT_CLASS_TYPE(object_class),
										G_SIGNAL_RUN_FIRST,
										G_STRUCT_OFFSET(GtkScintillaClass, modify_attempt_ro),
										NULL, NULL,
										g_cclosure_marshal_VOID__VOID,
										G_TYPE_NONE, 0);

	signals[KEY] =
			g_signal_new("key",
										G_OBJECT_CLASS_TYPE(object_class),
										G_SIGNAL_RUN_FIRST,
										G_STRUCT_OFFSET(GtkScintillaClass, key),
										NULL, NULL,
										g_cclosure_marshal_VOID__VOID,
										G_TYPE_NONE, 2,
										G_TYPE_INT, G_TYPE_INT);

	signals[DOUBLE_CLICK] =
			g_signal_new("double_click",
										G_OBJECT_CLASS_TYPE(object_class),
										G_SIGNAL_RUN_FIRST,
										G_STRUCT_OFFSET(GtkScintillaClass, double_click),
										NULL, NULL,
										g_cclosure_marshal_VOID__VOID,
										G_TYPE_NONE, 0);

	signals[UPDATE_UI] =
			g_signal_new("update_ui",
										G_OBJECT_CLASS_TYPE(object_class),
										G_SIGNAL_RUN_FIRST,
										G_STRUCT_OFFSET(GtkScintillaClass, update_ui),
										NULL, NULL,
										g_cclosure_marshal_VOID__VOID,
										G_TYPE_NONE, 0);

	signals[MODIFIED] =
			g_signal_new("modified",
										G_OBJECT_CLASS_TYPE(object_class),
										G_SIGNAL_RUN_FIRST,
										G_STRUCT_OFFSET(GtkScintillaClass, modified),
										NULL, NULL,
										g_cclosure_marshal_VOID__VOID,
										G_TYPE_NONE, 8,
										G_TYPE_INT, G_TYPE_INT, G_TYPE_STRING,
										G_TYPE_INT, G_TYPE_INT, G_TYPE_INT,
										G_TYPE_INT, G_TYPE_INT);

	signals[MACRO_RECORD] =
			g_signal_new("macro_record",
										G_OBJECT_CLASS_TYPE(object_class),
										G_SIGNAL_RUN_FIRST,
										G_STRUCT_OFFSET(GtkScintillaClass, macro_record),
										NULL, NULL,
										g_cclosure_marshal_VOID__VOID,
										G_TYPE_NONE, 3,
										G_TYPE_INT, G_TYPE_ULONG, G_TYPE_LONG);

	signals[MARGIN_CLICK] =
			g_signal_new("margin_click",
										G_OBJECT_CLASS_TYPE(object_class),
										G_SIGNAL_RUN_FIRST,
										G_STRUCT_OFFSET(GtkScintillaClass, margin_click),
										NULL, NULL,
										g_cclosure_marshal_VOID__VOID,
										G_TYPE_NONE, 3,
										G_TYPE_INT, G_TYPE_INT, G_TYPE_INT);

	signals[NEED_SHOWN] =
			g_signal_new("need_shown",
										G_OBJECT_CLASS_TYPE(object_class),
										G_SIGNAL_RUN_FIRST,
										G_STRUCT_OFFSET(GtkScintillaClass, need_shown),
										NULL, NULL,
										g_cclosure_marshal_VOID__VOID,
										G_TYPE_NONE, 2,
										G_TYPE_INT, G_TYPE_INT);

	signals[PAINTED] =
			g_signal_new("painted",
										G_OBJECT_CLASS_TYPE(object_class),
										G_SIGNAL_RUN_FIRST,
										G_STRUCT_OFFSET(GtkScintillaClass, painted),
										NULL, NULL,
										g_cclosure_marshal_VOID__VOID,
										G_TYPE_NONE, 0);

	signals[USER_LIST_SELECTION] =
			g_signal_new("user_list_selection",
										G_OBJECT_CLASS_TYPE(object_class),
										G_SIGNAL_RUN_FIRST,
										G_STRUCT_OFFSET(GtkScintillaClass, user_list_selection),
										NULL, NULL,
										g_cclosure_marshal_VOID__VOID,
										G_TYPE_NONE, 2,
										G_TYPE_INT, G_TYPE_STRING);

	signals[URI_DROPPED] =
			g_signal_new("uri_dropped",
										G_OBJECT_CLASS_TYPE(object_class),
										G_SIGNAL_RUN_FIRST,
										G_STRUCT_OFFSET(GtkScintillaClass, uri_dropped),
										NULL, NULL,
										g_cclosure_marshal_VOID__VOID,
										G_TYPE_NONE, 1,
										G_TYPE_STRING);

	signals[DWELL_START] =
			g_signal_new("dwell_start",
										G_OBJECT_CLASS_TYPE(object_class),
										G_SIGNAL_RUN_FIRST,
										G_STRUCT_OFFSET(GtkScintillaClass, dwell_start),
										NULL, NULL,
										g_cclosure_marshal_VOID__VOID,
										G_TYPE_NONE, 1,
										G_TYPE_INT);

	signals[DWELL_END] =
			g_signal_new("dwell_end",
										G_OBJECT_CLASS_TYPE(object_class),
										G_SIGNAL_RUN_FIRST,
										G_STRUCT_OFFSET(GtkScintillaClass, dwell_end),
										NULL, NULL,
										g_cclosure_marshal_VOID__VOID,
										G_TYPE_NONE, 1,
										G_TYPE_INT);

	signals[ZOOM] =
			g_signal_new("zoom",
										G_OBJECT_CLASS_TYPE(object_class),
										G_SIGNAL_RUN_FIRST,
										G_STRUCT_OFFSET(GtkScintillaClass, zoom),
										NULL, NULL,
										g_cclosure_marshal_VOID__VOID,
										G_TYPE_NONE, 0);
}

static void gtk_scintilla_init(GtkScintilla *sci) {
	sci->scintilla = GTK_WIDGET(scintilla_new());
	g_signal_connect(G_OBJECT(sci->scintilla), "sci-notify",
										G_CALLBACK(notify_cb), GTK_WIDGET(sci));
	
	gtk_container_add(GTK_CONTAINER(sci), sci->scintilla);
	
	gtk_widget_set(GTK_WIDGET(sci->scintilla),
									"visible", TRUE, NULL);
	
	gtk_widget_show(GTK_WIDGET(sci->scintilla));
	gtk_widget_show_all(GTK_WIDGET(sci));
	sci->pSciMsg = (SciFnDirect) scintilla_send_message(SCINTILLA (sci->scintilla), SCI_GETDIRECTFUNCTION, 0, 0);
	sci->pSciWndData = (sptr_t) scintilla_send_message(SCINTILLA (sci->scintilla), SCI_GETDIRECTPOINTER, 0, 0);
	
}

static void gtk_scintilla_destroy(GtkObject *object) {
	g_return_if_fail(object != NULL);
	g_return_if_fail(GTK_IS_SCINTILLA(object));
}

GtkWidget *gtk_scintilla_new(void) {
	GtkScintilla *scintilla;
	
	scintilla = (GtkScintilla *) gtk_type_new(gtk_scintilla_get_type());
	
	return GTK_WIDGET(scintilla);
}

static void notify_cb(GtkWidget *w, gint param, gpointer notif, gpointer data) {
	struct SCNotification *notification =(struct SCNotification *) notif;
	
	switch(notification->nmhdr.code) {
			case SCN_STYLENEEDED:
					g_signal_emit(G_OBJECT(data),
												 signals[STYLE_NEEDED], 0,
												(gint) notification->position);
					break;
			case SCN_UPDATEUI:
					g_signal_emit(G_OBJECT(data),
												 signals[UPDATE_UI], 0);
					break;
			case SCN_CHARADDED:
					g_signal_emit(G_OBJECT(data),
												 signals[CHAR_ADDED], 0,
												(gint) notification->ch);
					break;
			case SCN_SAVEPOINTREACHED:
					g_signal_emit(G_OBJECT(data),
												 signals[SAVE_POINT_REACHED], 0);
					break;
			case SCN_SAVEPOINTLEFT:
					g_signal_emit(G_OBJECT(data),
												 signals[SAVE_POINT_LEFT], 0);
					break;
			case SCN_MODIFYATTEMPTRO:
					g_signal_emit(G_OBJECT(data),
												 signals[MODIFY_ATTEMPT_RO], 0);
					break;
			case SCN_KEY:
					pass_throug_key(GTK_SCINTILLA(data),
													(gint) notification->ch,
													(gint) notification->modifiers);
					g_signal_emit(G_OBJECT(data),
												 signals[KEY], 0,
												(gint) notification->ch,
												(gint) notification->modifiers);
					break;
			case SCN_DOUBLECLICK:
					g_signal_emit(G_OBJECT(data),
												 signals[DOUBLE_CLICK], 0);
					break;
			case SCN_MODIFIED:
					g_signal_emit(G_OBJECT(data),
												 signals[MODIFIED], 0,
												(gint) notification->position,
												(gint) notification->modificationType,
												(gchar *)notification->text,
												(gint) notification->length,
												(gint) notification->linesAdded,
												(gint) notification->line,
												(gint) notification->foldLevelNow,
												(gint) notification->foldLevelPrev);
					break;
			case SCN_MACRORECORD:
					g_signal_emit(G_OBJECT(data),
												 signals[MACRO_RECORD], 0,
												(gint) notification->message,
												(gulong) notification->wParam,
												(glong) notification->lParam);
					break;
			case SCN_MARGINCLICK:
					g_signal_emit(G_OBJECT(data),
												 signals[MARGIN_CLICK], 0,
												(gint) notification->modifiers,
												(gint) notification->position,
												(gint) notification->margin);
					break;
			case SCN_NEEDSHOWN:
					g_signal_emit(G_OBJECT(data),
												 signals[NEED_SHOWN], 0,
												(gint) notification->position,
												(gint) notification->length);
					break;
			case SCN_PAINTED:
					g_signal_emit(G_OBJECT(data),
												 signals[PAINTED], 0);
					break;
			case SCN_USERLISTSELECTION:
					g_signal_emit(G_OBJECT(data),
												 signals[USER_LIST_SELECTION], 0,
												(gint) notification->listType,
												(gchar *) notification->text);
					break;
			case SCN_URIDROPPED:
					g_signal_emit(G_OBJECT(data),
												 signals[URI_DROPPED], 0,
												(gchar *) notification->text);
					break;
			case SCN_DWELLSTART:
					g_signal_emit(G_OBJECT(data),
												 signals[DWELL_START], 0,
												(gint) notification->position);
					break;
			case SCN_DWELLEND:
					g_signal_emit(G_OBJECT(data),
												 signals[DWELL_END], 0,
												(gint) notification->position);
					break;
			case SCN_ZOOM:
					g_signal_emit(G_OBJECT(data),
												 signals[ZOOM], 0);
					break;
			default:
					//g_warning("GtkScintilla2: Notification code %d not handled!\n",
					//          (gint) notification->nmhdr.code);
					break;
	}
}

void pass_throug_key(GtkScintilla *sci, gint ch, gint modifiers) {
	
	gint mods = 0;
	
	if(modifiers & SCMOD_SHIFT)
			mods |= GDK_SHIFT_MASK;
	if(modifiers & SCMOD_CTRL)
			mods |= GDK_CONTROL_MASK;
	if(modifiers & SCMOD_ALT)
			mods |= GDK_MOD1_MASK;
	
	if(sci->accel_group) {
			gtk_accel_groups_activate(G_OBJECT(sci->accel_group),
																 ch,(GdkModifierType) mods);
	}
}

//	Start of PHP functions

static PHP_METHOD(Scintilla, __construct) {

	GObject *wrapped_obj;
  
	GType gtype = phpg_gtype_from_zval(this_ptr); 
	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "")) {
    php_error(E_ERROR, "php-gtk-scintilla: Wrong args");  
        //PHPG_THROW_CONSTRUCT_EXCEPTION(Scintilla);
	}
	
	wrapped_obj =(GObject *) gtk_scintilla_new();

	if(!wrapped_obj) {
    php_error(E_ERROR, "php-gtk-scintilla: Could not wrap object");  
        //PHPG_THROW_CONSTRUCT_EXCEPTION(Scintilla);
	}
	phpg_gobject_set_wrapper(this_ptr, wrapped_obj TSRMLS_CC);
	
}
/*Start of generated code*/

static
PHP_METHOD(Scintilla, add_text)
{
	long doc_len, length=0;
	char *text;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "s|i", &text, &length))
		return;

	doc_len = strlen(text);
	if(!length || length > doc_len)
		length = doc_len;

	SCINTILLA_MESSAGE(SCI_ADDTEXT, (int) length, (sptr_t) text);
}

static
PHP_METHOD(Scintilla, add_styled_text)
{
	long length;
	char *c;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "is", &length, &c))
		return;

	SCINTILLA_MESSAGE(SCI_ADDSTYLEDTEXT, (int) length, (sptr_t) c);
}

static
PHP_METHOD(Scintilla, insert_text)
{
	long pos;
	char *text;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "is", &pos, &text))
		return;

	SCINTILLA_MESSAGE(SCI_INSERTTEXT, (int) pos, (sptr_t) text);
}

static
PHP_METHOD(Scintilla, clear_all)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_CLEARALL, 0, 0);
}

static
PHP_METHOD(Scintilla, clear_document_style)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_CLEARDOCUMENTSTYLE, 0, 0);
}

static
PHP_METHOD(Scintilla, get_length)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETLENGTH, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, get_char_at)
{
	long pos;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &pos))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETCHARAT, (int) pos, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, get_current_pos)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETCURRENTPOS, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, get_anchor)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETANCHOR, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, get_style_at)
{
	long pos;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &pos))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETSTYLEAT, (int) pos, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, redo)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_REDO, 0, 0);
}

static
PHP_METHOD(Scintilla, set_undo_collection)
{
	long collectUndo;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &collectUndo))
		return;

	SCINTILLA_MESSAGE(SCI_SETUNDOCOLLECTION, (int) collectUndo, 0);
}

static
PHP_METHOD(Scintilla, select_all)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_SELECTALL, 0, 0);
}

static
PHP_METHOD(Scintilla, set_save_point)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_SETSAVEPOINT, 0, 0);
}

static
PHP_METHOD(Scintilla, get_styled_text)
{
	long start, end, length;
	char *text;
	struct  TextRange tr = {{0, 0}, 0};

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &start, &end))
		return;

	length = ((end - start) * 2) + 2;
	if(length > 1) {
		text = emalloc(length);
		tr.chrg.cpMin = start;
		tr.chrg.cpMax = end;
		tr.lpstrText = text;

		SCINTILLA_MESSAGE(SCI_GETSTYLEDTEXT, 0, (sptr_t) &tr);

		RETVAL_STRINGL(text, length-2, 1);
		efree(text);
	} else {
		RETURN_EMPTY_STRING();
	}
}

static
PHP_METHOD(Scintilla, can_redo)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_CANREDO, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, marker_line_from_handle)
{
	long handle;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &handle))
		return;

	retval = SCINTILLA_MESSAGE(SCI_MARKERLINEFROMHANDLE, (int) handle, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, marker_delete_handle)
{
	long handle;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &handle))
		return;

	SCINTILLA_MESSAGE(SCI_MARKERDELETEHANDLE, (int) handle, 0);
}

static
PHP_METHOD(Scintilla, get_undo_collection)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETUNDOCOLLECTION, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, get_view_ws)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETVIEWWS, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_view_ws)
{
	long viewWS;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &viewWS))
		return;

	SCINTILLA_MESSAGE(SCI_SETVIEWWS, (int) viewWS, 0);
}

static
PHP_METHOD(Scintilla, position_from_point)
{
	long x, y;

	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &x, &y))
		return;

	retval = SCINTILLA_MESSAGE(SCI_POSITIONFROMPOINT, (int) x, (int) y);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, position_from_point_close)
{
	long x, y;

	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &x, &y))
		return;

	retval = SCINTILLA_MESSAGE(SCI_POSITIONFROMPOINTCLOSE, (int) x, (int) y);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, goto_line)
{
	long line;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &line))
		return;

	SCINTILLA_MESSAGE(SCI_GOTOLINE, (int) line, 0);
}

static
PHP_METHOD(Scintilla, goto_pos)
{
	long pos;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &pos))
		return;

	SCINTILLA_MESSAGE(SCI_GOTOPOS, (int) pos, 0);
}

static
PHP_METHOD(Scintilla, set_anchor)
{
	long posAnchor;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &posAnchor))
		return;

	SCINTILLA_MESSAGE(SCI_SETANCHOR, (int) posAnchor, 0);
}

static
PHP_METHOD(Scintilla, get_cur_line)
{
	long length=0;
	char *text;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	length = SCINTILLA_MESSAGE(SCI_GETCURLINE, 0, 0);
	if(length > 1) {
		text = emalloc(length);

		SCINTILLA_MESSAGE(SCI_GETCURLINE, (int) length, (sptr_t) text);

		RETVAL_STRINGL(text, length-1, 1);
		efree(text);
	} else {
		RETURN_EMPTY_STRING();
	}
}

static
PHP_METHOD(Scintilla, get_end_styled)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETENDSTYLED, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, convert_eols)
{
	long eolMode;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &eolMode))
		return;

	SCINTILLA_MESSAGE(SCI_CONVERTEOLS, (int) eolMode, 0);
}

static
PHP_METHOD(Scintilla, get_eol_mode)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETEOLMODE, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_eol_mode)
{
	long eolMode;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &eolMode))
		return;

	SCINTILLA_MESSAGE(SCI_SETEOLMODE, (int) eolMode, 0);
}

static
PHP_METHOD(Scintilla, start_styling)
{
	long pos, mask;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &pos, &mask))
		return;

	SCINTILLA_MESSAGE(SCI_STARTSTYLING, (int) pos, (int) mask);
}

static
PHP_METHOD(Scintilla, set_styling)
{
	long length, style;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &length, &style))
		return;

	SCINTILLA_MESSAGE(SCI_SETSTYLING, (int) length, (int) style);
}

static
PHP_METHOD(Scintilla, get_buffered_draw)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETBUFFEREDDRAW, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_buffered_draw)
{
	long buffered;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &buffered))
		return;

	SCINTILLA_MESSAGE(SCI_SETBUFFEREDDRAW, (int) buffered, 0);
}

static
PHP_METHOD(Scintilla, set_tab_width)
{
	long tabWidth;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &tabWidth))
		return;

	SCINTILLA_MESSAGE(SCI_SETTABWIDTH, (int) tabWidth, 0);
}

static
PHP_METHOD(Scintilla, get_tab_width)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETTABWIDTH, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_code_page)
{
	long codePage;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &codePage))
		return;

	SCINTILLA_MESSAGE(SCI_SETCODEPAGE, (int) codePage, 0);
}

static
PHP_METHOD(Scintilla, set_use_palette)
{
	long usePalette;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &usePalette))
		return;

	SCINTILLA_MESSAGE(SCI_SETUSEPALETTE, (int) usePalette, 0);
}

static
PHP_METHOD(Scintilla, marker_define)
{
	long markerNumber, markerSymbol;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &markerNumber, &markerSymbol))
		return;

	SCINTILLA_MESSAGE(SCI_MARKERDEFINE, (int) markerNumber, (int) markerSymbol);
}

static
PHP_METHOD(Scintilla, marker_set_fore)
{
	long markerNumber, fore;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &markerNumber, &fore))
		return;

	SCINTILLA_MESSAGE(SCI_MARKERSETFORE, (int) markerNumber, (int) fore);
}

static
PHP_METHOD(Scintilla, marker_set_back)
{
	long markerNumber, back;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &markerNumber, &back))
		return;

	SCINTILLA_MESSAGE(SCI_MARKERSETBACK, (int) markerNumber, (int) back);
}

static
PHP_METHOD(Scintilla, marker_add)
{
	long line, markerNumber;

	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &line, &markerNumber))
		return;

	retval = SCINTILLA_MESSAGE(SCI_MARKERADD, (int) line, (int) markerNumber);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, marker_delete)
{
	long line, markerNumber;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &line, &markerNumber))
		return;

	SCINTILLA_MESSAGE(SCI_MARKERDELETE, (int) line, (int) markerNumber);
}

static
PHP_METHOD(Scintilla, marker_delete_all)
{
	long markerNumber;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &markerNumber))
		return;

	SCINTILLA_MESSAGE(SCI_MARKERDELETEALL, (int) markerNumber, 0);
}

static
PHP_METHOD(Scintilla, marker_get)
{
	long line;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &line))
		return;

	retval = SCINTILLA_MESSAGE(SCI_MARKERGET, (int) line, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, marker_next)
{
	long lineStart, markerMask;

	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &lineStart, &markerMask))
		return;

	retval = SCINTILLA_MESSAGE(SCI_MARKERNEXT, (int) lineStart, (int) markerMask);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, marker_previous)
{
	long lineStart, markerMask;

	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &lineStart, &markerMask))
		return;

	retval = SCINTILLA_MESSAGE(SCI_MARKERPREVIOUS, (int) lineStart, (int) markerMask);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, marker_define_pixmap)
{
	long markerNumber;
	char *pixmap;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "is", &markerNumber, &pixmap))
		return;

	SCINTILLA_MESSAGE(SCI_MARKERDEFINEPIXMAP, (int) markerNumber, (sptr_t) pixmap);
}

static
PHP_METHOD(Scintilla, marker_add_set)
{
	long line, set;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &line, &set))
		return;

	SCINTILLA_MESSAGE(SCI_MARKERADDSET, (int) line, (int) set);
}

static
PHP_METHOD(Scintilla, marker_set_alpha)
{
	long markerNumber, alpha;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &markerNumber, &alpha))
		return;

	SCINTILLA_MESSAGE(SCI_MARKERSETALPHA, (int) markerNumber, (int) alpha);
}

static
PHP_METHOD(Scintilla, set_margin_type_n)
{
	long margin, marginType;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &margin, &marginType))
		return;

	SCINTILLA_MESSAGE(SCI_SETMARGINTYPEN, (int) margin, (int) marginType);
}

static
PHP_METHOD(Scintilla, get_margin_type_n)
{
	long margin;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &margin))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETMARGINTYPEN, (int) margin, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_margin_width_n)
{
	long margin, pixelWidth;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &margin, &pixelWidth))
		return;

	SCINTILLA_MESSAGE(SCI_SETMARGINWIDTHN, (int) margin, (int) pixelWidth);
}

static
PHP_METHOD(Scintilla, get_margin_width_n)
{
	long margin;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &margin))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETMARGINWIDTHN, (int) margin, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_margin_mask_n)
{
	long margin, mask;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &margin, &mask))
		return;

	SCINTILLA_MESSAGE(SCI_SETMARGINMASKN, (int) margin, (int) mask);
}

static
PHP_METHOD(Scintilla, get_margin_mask_n)
{
	long margin;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &margin))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETMARGINMASKN, (int) margin, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_margin_sensitive_n)
{
	long margin, sensitive;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &margin, &sensitive))
		return;

	SCINTILLA_MESSAGE(SCI_SETMARGINSENSITIVEN, (int) margin, (int) sensitive);
}

static
PHP_METHOD(Scintilla, get_margin_sensitive_n)
{
	long margin;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &margin))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETMARGINSENSITIVEN, (int) margin, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, style_clear_all)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_STYLECLEARALL, 0, 0);
}

static
PHP_METHOD(Scintilla, style_set_fore)
{
	long style, fore;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &style, &fore))
		return;

	SCINTILLA_MESSAGE(SCI_STYLESETFORE, (int) style, (int) fore);
}

static
PHP_METHOD(Scintilla, style_set_back)
{
	long style, back;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &style, &back))
		return;

	SCINTILLA_MESSAGE(SCI_STYLESETBACK, (int) style, (int) back);
}

static
PHP_METHOD(Scintilla, style_set_bold)
{
	long style, bold;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &style, &bold))
		return;

	SCINTILLA_MESSAGE(SCI_STYLESETBOLD, (int) style, (int) bold);
}

static
PHP_METHOD(Scintilla, style_set_italic)
{
	long style, italic;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &style, &italic))
		return;

	SCINTILLA_MESSAGE(SCI_STYLESETITALIC, (int) style, (int) italic);
}

static
PHP_METHOD(Scintilla, style_set_size)
{
	long style, sizePoints;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &style, &sizePoints))
		return;

	SCINTILLA_MESSAGE(SCI_STYLESETSIZE, (int) style, (int) sizePoints);
}

static
PHP_METHOD(Scintilla, style_set_font)
{
	long style;
	char *fontName;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "is", &style, &fontName))
		return;

	SCINTILLA_MESSAGE(SCI_STYLESETFONT, (int) style, (sptr_t) fontName);
}

static
PHP_METHOD(Scintilla, style_set_eol_filled)
{
	long style, filled;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &style, &filled))
		return;

	SCINTILLA_MESSAGE(SCI_STYLESETEOLFILLED, (int) style, (int) filled);
}

static
PHP_METHOD(Scintilla, style_reset_default)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_STYLERESETDEFAULT, 0, 0);
}

static
PHP_METHOD(Scintilla, style_set_underline)
{
	long style, underline;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &style, &underline))
		return;

	SCINTILLA_MESSAGE(SCI_STYLESETUNDERLINE, (int) style, (int) underline);
}

static
PHP_METHOD(Scintilla, style_get_fore)
{
	long style;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &style))
		return;

	retval = SCINTILLA_MESSAGE(SCI_STYLEGETFORE, (int) style, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, style_get_back)
{
	long style;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &style))
		return;

	retval = SCINTILLA_MESSAGE(SCI_STYLEGETBACK, (int) style, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, style_get_bold)
{
	long style;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &style))
		return;

	retval = SCINTILLA_MESSAGE(SCI_STYLEGETBOLD, (int) style, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, style_get_italic)
{
	long style;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &style))
		return;

	retval = SCINTILLA_MESSAGE(SCI_STYLEGETITALIC, (int) style, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, style_get_size)
{
	long style;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &style))
		return;

	retval = SCINTILLA_MESSAGE(SCI_STYLEGETSIZE, (int) style, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, style_get_font)
{
	long style;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &style))
		return;

	retval = SCINTILLA_MESSAGE(SCI_STYLEGETFONT, (int) style, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, style_get_eol_filled)
{
	long style;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &style))
		return;

	retval = SCINTILLA_MESSAGE(SCI_STYLEGETEOLFILLED, (int) style, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, style_get_underline)
{
	long style;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &style))
		return;

	retval = SCINTILLA_MESSAGE(SCI_STYLEGETUNDERLINE, (int) style, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, style_get_case)
{
	long style;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &style))
		return;

	retval = SCINTILLA_MESSAGE(SCI_STYLEGETCASE, (int) style, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, style_get_character_set)
{
	long style;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &style))
		return;

	retval = SCINTILLA_MESSAGE(SCI_STYLEGETCHARACTERSET, (int) style, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, style_get_visible)
{
	long style;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &style))
		return;

	retval = SCINTILLA_MESSAGE(SCI_STYLEGETVISIBLE, (int) style, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, style_get_changeable)
{
	long style;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &style))
		return;

	retval = SCINTILLA_MESSAGE(SCI_STYLEGETCHANGEABLE, (int) style, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, style_get_hot_spot)
{
	long style;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &style))
		return;

	retval = SCINTILLA_MESSAGE(SCI_STYLEGETHOTSPOT, (int) style, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, style_set_case)
{
	long style, caseForce;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &style, &caseForce))
		return;

	SCINTILLA_MESSAGE(SCI_STYLESETCASE, (int) style, (int) caseForce);
}

static
PHP_METHOD(Scintilla, style_set_character_set)
{
	long style, characterSet;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &style, &characterSet))
		return;

	SCINTILLA_MESSAGE(SCI_STYLESETCHARACTERSET, (int) style, (int) characterSet);
}

static
PHP_METHOD(Scintilla, style_set_hot_spot)
{
	long style, hotspot;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &style, &hotspot))
		return;

	SCINTILLA_MESSAGE(SCI_STYLESETHOTSPOT, (int) style, (int) hotspot);
}

static
PHP_METHOD(Scintilla, set_sel_fore)
{
	long useSetting, fore;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &useSetting, &fore))
		return;

	SCINTILLA_MESSAGE(SCI_SETSELFORE, (int) useSetting, (int) fore);
}

static
PHP_METHOD(Scintilla, set_sel_back)
{
	long useSetting, back;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &useSetting, &back))
		return;

	SCINTILLA_MESSAGE(SCI_SETSELBACK, (int) useSetting, (int) back);
}

static
PHP_METHOD(Scintilla, get_sel_alpha)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETSELALPHA, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_sel_alpha)
{
	long alpha;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &alpha))
		return;

	SCINTILLA_MESSAGE(SCI_SETSELALPHA, (int) alpha, 0);
}

static
PHP_METHOD(Scintilla, get_sel_eol_filled)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETSELEOLFILLED, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_sel_eol_filled)
{
	long filled;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &filled))
		return;

	SCINTILLA_MESSAGE(SCI_SETSELEOLFILLED, (int) filled, 0);
}

static
PHP_METHOD(Scintilla, set_caret_fore)
{
	long fore;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &fore))
		return;

	SCINTILLA_MESSAGE(SCI_SETCARETFORE, (int) fore, 0);
}

static
PHP_METHOD(Scintilla, assign_cmd_key)
{
	long km, msg;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &km, &msg))
		return;

	SCINTILLA_MESSAGE(SCI_ASSIGNCMDKEY, (int) km, (int) msg);
}

static
PHP_METHOD(Scintilla, clear_cmd_key)
{
	long km;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &km))
		return;

	SCINTILLA_MESSAGE(SCI_CLEARCMDKEY, (int) km, 0);
}

static
PHP_METHOD(Scintilla, clear_all_cmd_keys)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_CLEARALLCMDKEYS, 0, 0);
}

static
PHP_METHOD(Scintilla, set_styling_ex)
{
	long length;
	char *styles;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "is", &length, &styles))
		return;

	SCINTILLA_MESSAGE(SCI_SETSTYLINGEX, (int) length, (sptr_t) styles);
}

static
PHP_METHOD(Scintilla, style_set_visible)
{
	long style, visible;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &style, &visible))
		return;

	SCINTILLA_MESSAGE(SCI_STYLESETVISIBLE, (int) style, (int) visible);
}

static
PHP_METHOD(Scintilla, get_caret_period)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETCARETPERIOD, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_caret_period)
{
	long periodMilliseconds;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &periodMilliseconds))
		return;

	SCINTILLA_MESSAGE(SCI_SETCARETPERIOD, (int) periodMilliseconds, 0);
}

static
PHP_METHOD(Scintilla, set_word_chars)
{
	char *characters;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "s", &characters))
		return;

	SCINTILLA_MESSAGE(SCI_SETWORDCHARS, 0, (sptr_t) characters);
}

static
PHP_METHOD(Scintilla, begin_undo_action)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_BEGINUNDOACTION, 0, 0);
}

static
PHP_METHOD(Scintilla, end_undo_action)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_ENDUNDOACTION, 0, 0);
}

static
PHP_METHOD(Scintilla, indic_set_style)
{
	long indic, style;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &indic, &style))
		return;

	SCINTILLA_MESSAGE(SCI_INDICSETSTYLE, (int) indic, (int) style);
}

static
PHP_METHOD(Scintilla, indic_get_style)
{
	long indic;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &indic))
		return;

	retval = SCINTILLA_MESSAGE(SCI_INDICGETSTYLE, (int) indic, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, indic_set_fore)
{
	long indic, fore;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &indic, &fore))
		return;

	SCINTILLA_MESSAGE(SCI_INDICSETFORE, (int) indic, (int) fore);
}

static
PHP_METHOD(Scintilla, indic_get_fore)
{
	long indic;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &indic))
		return;

	retval = SCINTILLA_MESSAGE(SCI_INDICGETFORE, (int) indic, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_whitespace_fore)
{
	long useSetting, fore;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &useSetting, &fore))
		return;

	SCINTILLA_MESSAGE(SCI_SETWHITESPACEFORE, (int) useSetting, (int) fore);
}

static
PHP_METHOD(Scintilla, set_whitespace_back)
{
	long useSetting, back;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &useSetting, &back))
		return;

	SCINTILLA_MESSAGE(SCI_SETWHITESPACEBACK, (int) useSetting, (int) back);
}

static
PHP_METHOD(Scintilla, set_style_bits)
{
	long bits;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &bits))
		return;

	SCINTILLA_MESSAGE(SCI_SETSTYLEBITS, (int) bits, 0);
}

static
PHP_METHOD(Scintilla, get_style_bits)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETSTYLEBITS, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_line_state)
{
	long line, state;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &line, &state))
		return;

	SCINTILLA_MESSAGE(SCI_SETLINESTATE, (int) line, (int) state);
}

static
PHP_METHOD(Scintilla, get_line_state)
{
	long line;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &line))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETLINESTATE, (int) line, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, get_max_line_state)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETMAXLINESTATE, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, get_caret_line_visible)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETCARETLINEVISIBLE, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_caret_line_visible)
{
	long show;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &show))
		return;

	SCINTILLA_MESSAGE(SCI_SETCARETLINEVISIBLE, (int) show, 0);
}

static
PHP_METHOD(Scintilla, get_caret_line_back)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETCARETLINEBACK, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_caret_line_back)
{
	long back;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &back))
		return;

	SCINTILLA_MESSAGE(SCI_SETCARETLINEBACK, (int) back, 0);
}

static
PHP_METHOD(Scintilla, style_set_changeable)
{
	long style, changeable;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &style, &changeable))
		return;

	SCINTILLA_MESSAGE(SCI_STYLESETCHANGEABLE, (int) style, (int) changeable);
}

static
PHP_METHOD(Scintilla, auto_cshow)
{
	long lenEntered;
	char *itemList;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "is", &lenEntered, &itemList))
		return;

	SCINTILLA_MESSAGE(SCI_AUTOCSHOW, (int) lenEntered, (sptr_t) itemList);
}

static
PHP_METHOD(Scintilla, auto_ccancel)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_AUTOCCANCEL, 0, 0);
}

static
PHP_METHOD(Scintilla, auto_cactive)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_AUTOCACTIVE, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, auto_cpos_start)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_AUTOCPOSSTART, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, auto_ccomplete)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_AUTOCCOMPLETE, 0, 0);
}

static
PHP_METHOD(Scintilla, auto_cstops)
{
	char *characterSet;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "s", &characterSet))
		return;

	SCINTILLA_MESSAGE(SCI_AUTOCSTOPS, 0, (sptr_t) characterSet);
}

static
PHP_METHOD(Scintilla, auto_cset_separator)
{
	long separatorCharacter;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &separatorCharacter))
		return;

	SCINTILLA_MESSAGE(SCI_AUTOCSETSEPARATOR, (int) separatorCharacter, 0);
}

static
PHP_METHOD(Scintilla, auto_cget_separator)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_AUTOCGETSEPARATOR, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, auto_cselect)
{
	char *text;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "s", &text))
		return;

	SCINTILLA_MESSAGE(SCI_AUTOCSELECT, 0, (sptr_t) text);
}

static
PHP_METHOD(Scintilla, auto_cset_cancel_at_start)
{
	long cancel;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &cancel))
		return;

	SCINTILLA_MESSAGE(SCI_AUTOCSETCANCELATSTART, (int) cancel, 0);
}

static
PHP_METHOD(Scintilla, auto_cget_cancel_at_start)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_AUTOCGETCANCELATSTART, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, auto_cset_fill_ups)
{
	char *characterSet;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "s", &characterSet))
		return;

	SCINTILLA_MESSAGE(SCI_AUTOCSETFILLUPS, 0, (sptr_t) characterSet);
}

static
PHP_METHOD(Scintilla, auto_cset_choose_single)
{
	long chooseSingle;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &chooseSingle))
		return;

	SCINTILLA_MESSAGE(SCI_AUTOCSETCHOOSESINGLE, (int) chooseSingle, 0);
}

static
PHP_METHOD(Scintilla, auto_cget_choose_single)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_AUTOCGETCHOOSESINGLE, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, auto_cset_ignore_case)
{
	long ignoreCase;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &ignoreCase))
		return;

	SCINTILLA_MESSAGE(SCI_AUTOCSETIGNORECASE, (int) ignoreCase, 0);
}

static
PHP_METHOD(Scintilla, auto_cget_ignore_case)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_AUTOCGETIGNORECASE, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, user_list_show)
{
	long listType;
	char *itemList;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "is", &listType, &itemList))
		return;

	SCINTILLA_MESSAGE(SCI_USERLISTSHOW, (int) listType, (sptr_t) itemList);
}

static
PHP_METHOD(Scintilla, auto_cset_auto_hide)
{
	long autoHide;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &autoHide))
		return;

	SCINTILLA_MESSAGE(SCI_AUTOCSETAUTOHIDE, (int) autoHide, 0);
}

static
PHP_METHOD(Scintilla, auto_cget_auto_hide)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_AUTOCGETAUTOHIDE, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, auto_cset_drop_rest_of_word)
{
	long dropRestOfWord;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &dropRestOfWord))
		return;

	SCINTILLA_MESSAGE(SCI_AUTOCSETDROPRESTOFWORD, (int) dropRestOfWord, 0);
}

static
PHP_METHOD(Scintilla, auto_cget_drop_rest_of_word)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_AUTOCGETDROPRESTOFWORD, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, register_image)
{
	long type;
	char *xpmData;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "is", &type, &xpmData))
		return;

	SCINTILLA_MESSAGE(SCI_REGISTERIMAGE, (int) type, (sptr_t) xpmData);
}

static
PHP_METHOD(Scintilla, clear_registered_images)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_CLEARREGISTEREDIMAGES, 0, 0);
}

static
PHP_METHOD(Scintilla, auto_cget_type_separator)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_AUTOCGETTYPESEPARATOR, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, auto_cset_type_separator)
{
	long separatorCharacter;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &separatorCharacter))
		return;

	SCINTILLA_MESSAGE(SCI_AUTOCSETTYPESEPARATOR, (int) separatorCharacter, 0);
}

static
PHP_METHOD(Scintilla, auto_cset_max_width)
{
	long characterCount;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &characterCount))
		return;

	SCINTILLA_MESSAGE(SCI_AUTOCSETMAXWIDTH, (int) characterCount, 0);
}

static
PHP_METHOD(Scintilla, auto_cget_max_width)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_AUTOCGETMAXWIDTH, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, auto_cset_max_height)
{
	long rowCount;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &rowCount))
		return;

	SCINTILLA_MESSAGE(SCI_AUTOCSETMAXHEIGHT, (int) rowCount, 0);
}

static
PHP_METHOD(Scintilla, auto_cget_max_height)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_AUTOCGETMAXHEIGHT, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_indent)
{
	long indentSize;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &indentSize))
		return;

	SCINTILLA_MESSAGE(SCI_SETINDENT, (int) indentSize, 0);
}

static
PHP_METHOD(Scintilla, get_indent)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETINDENT, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_use_tabs)
{
	long useTabs;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &useTabs))
		return;

	SCINTILLA_MESSAGE(SCI_SETUSETABS, (int) useTabs, 0);
}

static
PHP_METHOD(Scintilla, get_use_tabs)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETUSETABS, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_line_indentation)
{
	long line, indentSize;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &line, &indentSize))
		return;

	SCINTILLA_MESSAGE(SCI_SETLINEINDENTATION, (int) line, (int) indentSize);
}

static
PHP_METHOD(Scintilla, get_line_indentation)
{
	long line;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &line))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETLINEINDENTATION, (int) line, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, get_line_indent_position)
{
	long line;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &line))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETLINEINDENTPOSITION, (int) line, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, get_column)
{
	long pos;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &pos))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETCOLUMN, (int) pos, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_hscroll_bar)
{
	long show;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &show))
		return;

	SCINTILLA_MESSAGE(SCI_SETHSCROLLBAR, (int) show, 0);
}

static
PHP_METHOD(Scintilla, get_hscroll_bar)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETHSCROLLBAR, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_indentation_guides)
{
	long show;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &show))
		return;

	SCINTILLA_MESSAGE(SCI_SETINDENTATIONGUIDES, (int) show, 0);
}

static
PHP_METHOD(Scintilla, get_indentation_guides)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETINDENTATIONGUIDES, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_highlight_guide)
{
	long column;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &column))
		return;

	SCINTILLA_MESSAGE(SCI_SETHIGHLIGHTGUIDE, (int) column, 0);
}

static
PHP_METHOD(Scintilla, get_highlight_guide)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETHIGHLIGHTGUIDE, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, get_line_end_position)
{
	long line;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &line))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETLINEENDPOSITION, (int) line, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, get_code_page)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETCODEPAGE, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, get_caret_fore)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETCARETFORE, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, get_use_palette)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETUSEPALETTE, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, get_read_only)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETREADONLY, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_current_pos)
{
	long pos;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &pos))
		return;

	SCINTILLA_MESSAGE(SCI_SETCURRENTPOS, (int) pos, 0);
}

static
PHP_METHOD(Scintilla, set_selection_start)
{
	long pos;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &pos))
		return;

	SCINTILLA_MESSAGE(SCI_SETSELECTIONSTART, (int) pos, 0);
}

static
PHP_METHOD(Scintilla, get_selection_start)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETSELECTIONSTART, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_selection_end)
{
	long pos;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &pos))
		return;

	SCINTILLA_MESSAGE(SCI_SETSELECTIONEND, (int) pos, 0);
}

static
PHP_METHOD(Scintilla, get_selection_end)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETSELECTIONEND, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_print_magnification)
{
	long magnification;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &magnification))
		return;

	SCINTILLA_MESSAGE(SCI_SETPRINTMAGNIFICATION, (int) magnification, 0);
}

static
PHP_METHOD(Scintilla, get_print_magnification)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETPRINTMAGNIFICATION, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_print_colour_mode)
{
	long mode;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &mode))
		return;

	SCINTILLA_MESSAGE(SCI_SETPRINTCOLOURMODE, (int) mode, 0);
}

static
PHP_METHOD(Scintilla, get_print_colour_mode)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETPRINTCOLOURMODE, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, find_text)
{
	long flags, chrg_min=0, chrg_max=0, retval;
	char *text;
	struct TextToFind ft = {{0, 0}, 0, {0, 0}};

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "is|ii", &flags, &text, &chrg_min, &chrg_max))
		return;

	if(!chrg_min && !chrg_max)
		chrg_max = SCINTILLA_MESSAGE(SCI_GETLENGTH, 0, 0);
	ft.chrg.cpMin = chrg_min;
	ft.chrg.cpMax = chrg_max;
	ft.lpstrText = text;

	retval = SCINTILLA_MESSAGE(SCI_FINDTEXT, (int) flags, (sptr_t) &ft);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, format_range)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	
}

static
PHP_METHOD(Scintilla, get_first_visible_line)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETFIRSTVISIBLELINE, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, get_line)
{
	long line, length=0;
	char *text;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &line))
		return;

	length = SCINTILLA_MESSAGE(SCI_GETLINE, (int) line, 0);
	text = emalloc(length);

	SCINTILLA_MESSAGE(SCI_GETLINE, (int) line, (sptr_t) text);

	RETVAL_STRINGL(text, length, 1);
	efree(text);
}

static
PHP_METHOD(Scintilla, get_line_count)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETLINECOUNT, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_margin_left)
{
	long pixelWidth;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &pixelWidth))
		return;

	SCINTILLA_MESSAGE(SCI_SETMARGINLEFT, 0, (int) pixelWidth);
}

static
PHP_METHOD(Scintilla, get_margin_left)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETMARGINLEFT, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_margin_right)
{
	long pixelWidth;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &pixelWidth))
		return;

	SCINTILLA_MESSAGE(SCI_SETMARGINRIGHT, 0, (int) pixelWidth);
}

static
PHP_METHOD(Scintilla, get_margin_right)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETMARGINRIGHT, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, get_modify)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETMODIFY, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_sel)
{
	long start, end;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &start, &end))
		return;

	SCINTILLA_MESSAGE(SCI_SETSEL, (int) start, (int) end);
}

static
PHP_METHOD(Scintilla, get_sel_text)
{
	long length=0;
	char *text;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	length = SCINTILLA_MESSAGE(SCI_GETSELTEXT, 0, 0);
	if(length > 1) {
		text = emalloc(length);

		SCINTILLA_MESSAGE(SCI_GETSELTEXT, (int) length, (sptr_t) text);

		RETVAL_STRINGL(text, length-1, 1);
		efree(text);
	} else {
		RETURN_EMPTY_STRING();
	}
}

static
PHP_METHOD(Scintilla, get_text_range)
{
	long start, end, length;
	char *text;
	struct  TextRange tr = {{0, 0}, 0};

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &start, &end))
		return;

	length = end - start + 1;
	if(length > 1) {
		text = emalloc(length);
		tr.chrg.cpMin = start;
		tr.chrg.cpMax = end;
		tr.lpstrText = text;

		SCINTILLA_MESSAGE(SCI_GETTEXTRANGE, 0, (sptr_t) &tr);

		RETVAL_STRINGL(text, length-1, 1);
		efree(text);
	} else {
		RETURN_EMPTY_STRING();
	}
}

static
PHP_METHOD(Scintilla, hide_selection)
{
	long normal;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &normal))
		return;

	SCINTILLA_MESSAGE(SCI_HIDESELECTION, (int) normal, 0);
}

static
PHP_METHOD(Scintilla, point_xfrom_position)
{
	long pos;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &pos))
		return;

	retval = SCINTILLA_MESSAGE(SCI_POINTXFROMPOSITION, 0, (int) pos);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, point_yfrom_position)
{
	long pos;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &pos))
		return;

	retval = SCINTILLA_MESSAGE(SCI_POINTYFROMPOSITION, 0, (int) pos);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, line_from_position)
{
	long pos;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &pos))
		return;

	retval = SCINTILLA_MESSAGE(SCI_LINEFROMPOSITION, (int) pos, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, position_from_line)
{
	long line;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &line))
		return;

	retval = SCINTILLA_MESSAGE(SCI_POSITIONFROMLINE, (int) line, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, line_scroll)
{
	long columns, lines;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &columns, &lines))
		return;

	SCINTILLA_MESSAGE(SCI_LINESCROLL, (int) columns, (int) lines);
}

static
PHP_METHOD(Scintilla, scroll_caret)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_SCROLLCARET, 0, 0);
}

static
PHP_METHOD(Scintilla, replace_sel)
{
	char *text;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "s", &text))
		return;

	SCINTILLA_MESSAGE(SCI_REPLACESEL, 0, (sptr_t) text);
}

static
PHP_METHOD(Scintilla, set_read_only)
{
	long readOnly;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &readOnly))
		return;

	SCINTILLA_MESSAGE(SCI_SETREADONLY, (int) readOnly, 0);
}

static
PHP_METHOD(Scintilla, null)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_NULL, 0, 0);
}

static
PHP_METHOD(Scintilla, can_paste)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_CANPASTE, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, can_undo)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_CANUNDO, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, empty_undo_buffer)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_EMPTYUNDOBUFFER, 0, 0);
}

static
PHP_METHOD(Scintilla, undo)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_UNDO, 0, 0);
}

static
PHP_METHOD(Scintilla, cut)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_CUT, 0, 0);
}

static
PHP_METHOD(Scintilla, copy)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_COPY, 0, 0);
}

static
PHP_METHOD(Scintilla, paste)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_PASTE, 0, 0);
}

static
PHP_METHOD(Scintilla, clear)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_CLEAR, 0, 0);
}

static
PHP_METHOD(Scintilla, set_text)
{
	char *text;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "s", &text))
		return;

	SCINTILLA_MESSAGE(SCI_SETTEXT, 0, (sptr_t) text);
}

static
PHP_METHOD(Scintilla, get_text)
{
	long doc_len, length=0;
	char *text;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "|i", &length))
		return;

	doc_len = SCINTILLA_MESSAGE(SCI_GETLENGTH, 0, 0);
	if(!length || length > doc_len)
		 length = doc_len;
	length ++;
	text = emalloc(length);

	SCINTILLA_MESSAGE(SCI_GETTEXT, (int) length, (sptr_t) text);

	RETVAL_STRINGL(text, length-1, 1);
	efree(text);
}

static
PHP_METHOD(Scintilla, get_text_length)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETTEXTLENGTH, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, get_direct_function)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETDIRECTFUNCTION, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, get_direct_pointer)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETDIRECTPOINTER, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_overtype)
{
	long overtype;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &overtype))
		return;

	SCINTILLA_MESSAGE(SCI_SETOVERTYPE, (int) overtype, 0);
}

static
PHP_METHOD(Scintilla, get_overtype)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETOVERTYPE, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_caret_width)
{
	long pixelWidth;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &pixelWidth))
		return;

	SCINTILLA_MESSAGE(SCI_SETCARETWIDTH, (int) pixelWidth, 0);
}

static
PHP_METHOD(Scintilla, get_caret_width)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETCARETWIDTH, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_target_start)
{
	long pos;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &pos))
		return;

	SCINTILLA_MESSAGE(SCI_SETTARGETSTART, (int) pos, 0);
}

static
PHP_METHOD(Scintilla, get_target_start)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETTARGETSTART, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_target_end)
{
	long pos;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &pos))
		return;

	SCINTILLA_MESSAGE(SCI_SETTARGETEND, (int) pos, 0);
}

static
PHP_METHOD(Scintilla, get_target_end)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETTARGETEND, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, replace_target)
{
	long length;
	char *text;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "is", &length, &text))
		return;

	retval = SCINTILLA_MESSAGE(SCI_REPLACETARGET, (int) length, (sptr_t) text);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, replace_target_re)
{
	long length;
	char *text;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "is", &length, &text))
		return;

	retval = SCINTILLA_MESSAGE(SCI_REPLACETARGETRE, (int) length, (sptr_t) text);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, search_in_target)
{
	long length;
	char *text;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "is", &length, &text))
		return;

	retval = SCINTILLA_MESSAGE(SCI_SEARCHINTARGET, (int) length, (sptr_t) text);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_search_flags)
{
	long flags;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &flags))
		return;

	SCINTILLA_MESSAGE(SCI_SETSEARCHFLAGS, (int) flags, 0);
}

static
PHP_METHOD(Scintilla, get_search_flags)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETSEARCHFLAGS, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, call_tip_show)
{
	long pos;
	char *definition;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "is", &pos, &definition))
		return;

	SCINTILLA_MESSAGE(SCI_CALLTIPSHOW, (int) pos, (sptr_t) definition);
}

static
PHP_METHOD(Scintilla, call_tip_cancel)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_CALLTIPCANCEL, 0, 0);
}

static
PHP_METHOD(Scintilla, call_tip_active)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_CALLTIPACTIVE, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, call_tip_pos_start)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_CALLTIPPOSSTART, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, call_tip_set_hlt)
{
	long start, end;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &start, &end))
		return;

	SCINTILLA_MESSAGE(SCI_CALLTIPSETHLT, (int) start, (int) end);
}

static
PHP_METHOD(Scintilla, call_tip_set_back)
{
	long back;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &back))
		return;

	SCINTILLA_MESSAGE(SCI_CALLTIPSETBACK, (int) back, 0);
}

static
PHP_METHOD(Scintilla, call_tip_set_fore)
{
	long fore;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &fore))
		return;

	SCINTILLA_MESSAGE(SCI_CALLTIPSETFORE, (int) fore, 0);
}

static
PHP_METHOD(Scintilla, call_tip_set_fore_hlt)
{
	long fore;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &fore))
		return;

	SCINTILLA_MESSAGE(SCI_CALLTIPSETFOREHLT, (int) fore, 0);
}

static
PHP_METHOD(Scintilla, call_tip_use_style)
{
	long tabSize;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &tabSize))
		return;

	SCINTILLA_MESSAGE(SCI_CALLTIPUSESTYLE, (int) tabSize, 0);
}

static
PHP_METHOD(Scintilla, visible_from_doc_line)
{
	long line;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &line))
		return;

	retval = SCINTILLA_MESSAGE(SCI_VISIBLEFROMDOCLINE, (int) line, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, doc_line_from_visible)
{
	long lineDisplay;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &lineDisplay))
		return;

	retval = SCINTILLA_MESSAGE(SCI_DOCLINEFROMVISIBLE, (int) lineDisplay, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, wrap_count)
{
	long line;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &line))
		return;

	retval = SCINTILLA_MESSAGE(SCI_WRAPCOUNT, (int) line, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_fold_level)
{
	long line, level;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &line, &level))
		return;

	SCINTILLA_MESSAGE(SCI_SETFOLDLEVEL, (int) line, (int) level);
}

static
PHP_METHOD(Scintilla, get_fold_level)
{
	long line;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &line))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETFOLDLEVEL, (int) line, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, get_last_child)
{
	long line, level;

	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &line, &level))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETLASTCHILD, (int) line, (int) level);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, get_fold_parent)
{
	long line;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &line))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETFOLDPARENT, (int) line, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, show_lines)
{
	long lineStart, lineEnd;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &lineStart, &lineEnd))
		return;

	SCINTILLA_MESSAGE(SCI_SHOWLINES, (int) lineStart, (int) lineEnd);
}

static
PHP_METHOD(Scintilla, hide_lines)
{
	long lineStart, lineEnd;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &lineStart, &lineEnd))
		return;

	SCINTILLA_MESSAGE(SCI_HIDELINES, (int) lineStart, (int) lineEnd);
}

static
PHP_METHOD(Scintilla, get_line_visible)
{
	long line;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &line))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETLINEVISIBLE, (int) line, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_fold_expanded)
{
	long line, expanded;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &line, &expanded))
		return;

	SCINTILLA_MESSAGE(SCI_SETFOLDEXPANDED, (int) line, (int) expanded);
}

static
PHP_METHOD(Scintilla, get_fold_expanded)
{
	long line;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &line))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETFOLDEXPANDED, (int) line, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, toggle_fold)
{
	long line;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &line))
		return;

	SCINTILLA_MESSAGE(SCI_TOGGLEFOLD, (int) line, 0);
}

static
PHP_METHOD(Scintilla, ensure_visible)
{
	long line;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &line))
		return;

	SCINTILLA_MESSAGE(SCI_ENSUREVISIBLE, (int) line, 0);
}

static
PHP_METHOD(Scintilla, set_fold_flags)
{
	long flags;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &flags))
		return;

	SCINTILLA_MESSAGE(SCI_SETFOLDFLAGS, (int) flags, 0);
}

static
PHP_METHOD(Scintilla, ensure_visible_enforce_policy)
{
	long line;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &line))
		return;

	SCINTILLA_MESSAGE(SCI_ENSUREVISIBLEENFORCEPOLICY, (int) line, 0);
}

static
PHP_METHOD(Scintilla, set_tab_indents)
{
	long tabIndents;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &tabIndents))
		return;

	SCINTILLA_MESSAGE(SCI_SETTABINDENTS, (int) tabIndents, 0);
}

static
PHP_METHOD(Scintilla, get_tab_indents)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETTABINDENTS, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_backspace_unindents)
{
	long bsUnIndents;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &bsUnIndents))
		return;

	SCINTILLA_MESSAGE(SCI_SETBACKSPACEUNINDENTS, (int) bsUnIndents, 0);
}

static
PHP_METHOD(Scintilla, get_backspace_unindents)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETBACKSPACEUNINDENTS, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_mouse_dwell_time)
{
	long periodMilliseconds;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &periodMilliseconds))
		return;

	SCINTILLA_MESSAGE(SCI_SETMOUSEDWELLTIME, (int) periodMilliseconds, 0);
}

static
PHP_METHOD(Scintilla, get_mouse_dwell_time)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETMOUSEDWELLTIME, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, word_start_position)
{
	long pos, onlyWordCharacters;

	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &pos, &onlyWordCharacters))
		return;

	retval = SCINTILLA_MESSAGE(SCI_WORDSTARTPOSITION, (int) pos, (int) onlyWordCharacters);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, word_end_position)
{
	long pos, onlyWordCharacters;

	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &pos, &onlyWordCharacters))
		return;

	retval = SCINTILLA_MESSAGE(SCI_WORDENDPOSITION, (int) pos, (int) onlyWordCharacters);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_wrap_mode)
{
	long mode;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &mode))
		return;

	SCINTILLA_MESSAGE(SCI_SETWRAPMODE, (int) mode, 0);
}

static
PHP_METHOD(Scintilla, get_wrap_mode)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETWRAPMODE, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_wrap_visual_flags)
{
	long wrapVisualFlags;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &wrapVisualFlags))
		return;

	SCINTILLA_MESSAGE(SCI_SETWRAPVISUALFLAGS, (int) wrapVisualFlags, 0);
}

static
PHP_METHOD(Scintilla, get_wrap_visual_flags)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETWRAPVISUALFLAGS, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_wrap_visual_flags_location)
{
	long wrapVisualFlagsLocation;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &wrapVisualFlagsLocation))
		return;

	SCINTILLA_MESSAGE(SCI_SETWRAPVISUALFLAGSLOCATION, (int) wrapVisualFlagsLocation, 0);
}

static
PHP_METHOD(Scintilla, get_wrap_visual_flags_location)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETWRAPVISUALFLAGSLOCATION, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_wrap_start_indent)
{
	long indent;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &indent))
		return;

	SCINTILLA_MESSAGE(SCI_SETWRAPSTARTINDENT, (int) indent, 0);
}

static
PHP_METHOD(Scintilla, get_wrap_start_indent)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETWRAPSTARTINDENT, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_layout_cache)
{
	long mode;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &mode))
		return;

	SCINTILLA_MESSAGE(SCI_SETLAYOUTCACHE, (int) mode, 0);
}

static
PHP_METHOD(Scintilla, get_layout_cache)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETLAYOUTCACHE, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_scroll_width)
{
	long pixelWidth;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &pixelWidth))
		return;

	SCINTILLA_MESSAGE(SCI_SETSCROLLWIDTH, (int) pixelWidth, 0);
}

static
PHP_METHOD(Scintilla, get_scroll_width)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETSCROLLWIDTH, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, text_width)
{
	long style;
	char *text;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "is", &style, &text))
		return;

	retval = SCINTILLA_MESSAGE(SCI_TEXTWIDTH, (int) style, (sptr_t) text);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_end_at_last_line)
{
	long endAtLastLine;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &endAtLastLine))
		return;

	SCINTILLA_MESSAGE(SCI_SETENDATLASTLINE, (int) endAtLastLine, 0);
}

static
PHP_METHOD(Scintilla, get_end_at_last_line)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETENDATLASTLINE, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, text_height)
{
	long line;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &line))
		return;

	retval = SCINTILLA_MESSAGE(SCI_TEXTHEIGHT, (int) line, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_vscroll_bar)
{
	long show;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &show))
		return;

	SCINTILLA_MESSAGE(SCI_SETVSCROLLBAR, (int) show, 0);
}

static
PHP_METHOD(Scintilla, get_vscroll_bar)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETVSCROLLBAR, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, append_text)
{
	long length;
	char *text;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "is", &length, &text))
		return;

	SCINTILLA_MESSAGE(SCI_APPENDTEXT, (int) length, (sptr_t) text);
}

static
PHP_METHOD(Scintilla, get_two_phase_draw)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETTWOPHASEDRAW, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_two_phase_draw)
{
	long twoPhase;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &twoPhase))
		return;

	SCINTILLA_MESSAGE(SCI_SETTWOPHASEDRAW, (int) twoPhase, 0);
}

static
PHP_METHOD(Scintilla, target_from_selection)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_TARGETFROMSELECTION, 0, 0);
}

static
PHP_METHOD(Scintilla, lines_join)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_LINESJOIN, 0, 0);
}

static
PHP_METHOD(Scintilla, lines_split)
{
	long pixelWidth;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &pixelWidth))
		return;

	SCINTILLA_MESSAGE(SCI_LINESSPLIT, (int) pixelWidth, 0);
}

static
PHP_METHOD(Scintilla, set_fold_margin_colour)
{
	long useSetting, back;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &useSetting, &back))
		return;

	SCINTILLA_MESSAGE(SCI_SETFOLDMARGINCOLOUR, (int) useSetting, (int) back);
}

static
PHP_METHOD(Scintilla, set_fold_margin_hi_colour)
{
	long useSetting, fore;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &useSetting, &fore))
		return;

	SCINTILLA_MESSAGE(SCI_SETFOLDMARGINHICOLOUR, (int) useSetting, (int) fore);
}

static
PHP_METHOD(Scintilla, line_down)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_LINEDOWN, 0, 0);
}

static
PHP_METHOD(Scintilla, line_down_extend)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_LINEDOWNEXTEND, 0, 0);
}

static
PHP_METHOD(Scintilla, line_up)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_LINEUP, 0, 0);
}

static
PHP_METHOD(Scintilla, line_up_extend)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_LINEUPEXTEND, 0, 0);
}

static
PHP_METHOD(Scintilla, char_left)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_CHARLEFT, 0, 0);
}

static
PHP_METHOD(Scintilla, char_left_extend)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_CHARLEFTEXTEND, 0, 0);
}

static
PHP_METHOD(Scintilla, char_right)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_CHARRIGHT, 0, 0);
}

static
PHP_METHOD(Scintilla, char_right_extend)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_CHARRIGHTEXTEND, 0, 0);
}

static
PHP_METHOD(Scintilla, word_left)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_WORDLEFT, 0, 0);
}

static
PHP_METHOD(Scintilla, word_left_extend)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_WORDLEFTEXTEND, 0, 0);
}

static
PHP_METHOD(Scintilla, word_right)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_WORDRIGHT, 0, 0);
}

static
PHP_METHOD(Scintilla, word_right_extend)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_WORDRIGHTEXTEND, 0, 0);
}

static
PHP_METHOD(Scintilla, home)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_HOME, 0, 0);
}

static
PHP_METHOD(Scintilla, home_extend)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_HOMEEXTEND, 0, 0);
}

static
PHP_METHOD(Scintilla, line_end)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_LINEEND, 0, 0);
}

static
PHP_METHOD(Scintilla, line_end_extend)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_LINEENDEXTEND, 0, 0);
}

static
PHP_METHOD(Scintilla, document_start)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_DOCUMENTSTART, 0, 0);
}

static
PHP_METHOD(Scintilla, document_start_extend)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_DOCUMENTSTARTEXTEND, 0, 0);
}

static
PHP_METHOD(Scintilla, document_end)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_DOCUMENTEND, 0, 0);
}

static
PHP_METHOD(Scintilla, document_end_extend)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_DOCUMENTENDEXTEND, 0, 0);
}

static
PHP_METHOD(Scintilla, page_up)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_PAGEUP, 0, 0);
}

static
PHP_METHOD(Scintilla, page_up_extend)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_PAGEUPEXTEND, 0, 0);
}

static
PHP_METHOD(Scintilla, page_down)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_PAGEDOWN, 0, 0);
}

static
PHP_METHOD(Scintilla, page_down_extend)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_PAGEDOWNEXTEND, 0, 0);
}

static
PHP_METHOD(Scintilla, edit_toggle_overtype)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_EDITTOGGLEOVERTYPE, 0, 0);
}

static
PHP_METHOD(Scintilla, cancel)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_CANCEL, 0, 0);
}

static
PHP_METHOD(Scintilla, delete_back)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_DELETEBACK, 0, 0);
}

static
PHP_METHOD(Scintilla, tab)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_TAB, 0, 0);
}

static
PHP_METHOD(Scintilla, back_tab)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_BACKTAB, 0, 0);
}

static
PHP_METHOD(Scintilla, new_line)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_NEWLINE, 0, 0);
}

static
PHP_METHOD(Scintilla, form_feed)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_FORMFEED, 0, 0);
}

static
PHP_METHOD(Scintilla, vchome)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_VCHOME, 0, 0);
}

static
PHP_METHOD(Scintilla, vchome_extend)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_VCHOMEEXTEND, 0, 0);
}

static
PHP_METHOD(Scintilla, zoom_in)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_ZOOMIN, 0, 0);
}

static
PHP_METHOD(Scintilla, zoom_out)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_ZOOMOUT, 0, 0);
}

static
PHP_METHOD(Scintilla, del_word_left)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_DELWORDLEFT, 0, 0);
}

static
PHP_METHOD(Scintilla, del_word_right)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_DELWORDRIGHT, 0, 0);
}

static
PHP_METHOD(Scintilla, line_cut)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_LINECUT, 0, 0);
}

static
PHP_METHOD(Scintilla, line_delete)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_LINEDELETE, 0, 0);
}

static
PHP_METHOD(Scintilla, line_transpose)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_LINETRANSPOSE, 0, 0);
}

static
PHP_METHOD(Scintilla, line_duplicate)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_LINEDUPLICATE, 0, 0);
}

static
PHP_METHOD(Scintilla, lower_case)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_LOWERCASE, 0, 0);
}

static
PHP_METHOD(Scintilla, upper_case)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_UPPERCASE, 0, 0);
}

static
PHP_METHOD(Scintilla, line_scroll_down)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_LINESCROLLDOWN, 0, 0);
}

static
PHP_METHOD(Scintilla, line_scroll_up)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_LINESCROLLUP, 0, 0);
}

static
PHP_METHOD(Scintilla, delete_back_not_line)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_DELETEBACKNOTLINE, 0, 0);
}

static
PHP_METHOD(Scintilla, home_display)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_HOMEDISPLAY, 0, 0);
}

static
PHP_METHOD(Scintilla, home_display_extend)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_HOMEDISPLAYEXTEND, 0, 0);
}

static
PHP_METHOD(Scintilla, line_end_display)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_LINEENDDISPLAY, 0, 0);
}

static
PHP_METHOD(Scintilla, line_end_display_extend)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_LINEENDDISPLAYEXTEND, 0, 0);
}

static
PHP_METHOD(Scintilla, home_wrap)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_HOMEWRAP, 0, 0);
}

static
PHP_METHOD(Scintilla, home_wrap_extend)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_HOMEWRAPEXTEND, 0, 0);
}

static
PHP_METHOD(Scintilla, line_end_wrap)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_LINEENDWRAP, 0, 0);
}

static
PHP_METHOD(Scintilla, line_end_wrap_extend)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_LINEENDWRAPEXTEND, 0, 0);
}

static
PHP_METHOD(Scintilla, vchome_wrap)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_VCHOMEWRAP, 0, 0);
}

static
PHP_METHOD(Scintilla, vchome_wrap_extend)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_VCHOMEWRAPEXTEND, 0, 0);
}

static
PHP_METHOD(Scintilla, line_copy)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_LINECOPY, 0, 0);
}

static
PHP_METHOD(Scintilla, move_caret_inside_view)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_MOVECARETINSIDEVIEW, 0, 0);
}

static
PHP_METHOD(Scintilla, line_length)
{
	long line;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &line))
		return;

	retval = SCINTILLA_MESSAGE(SCI_LINELENGTH, (int) line, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, brace_highlight)
{
	long pos1, pos2;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &pos1, &pos2))
		return;

	SCINTILLA_MESSAGE(SCI_BRACEHIGHLIGHT, (int) pos1, (int) pos2);
}

static
PHP_METHOD(Scintilla, brace_bad_light)
{
	long pos;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &pos))
		return;

	SCINTILLA_MESSAGE(SCI_BRACEBADLIGHT, (int) pos, 0);
}

static
PHP_METHOD(Scintilla, brace_match)
{
	long pos;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &pos))
		return;

	retval = SCINTILLA_MESSAGE(SCI_BRACEMATCH, (int) pos, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, get_view_eol)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETVIEWEOL, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_view_eol)
{
	long visible;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &visible))
		return;

	SCINTILLA_MESSAGE(SCI_SETVIEWEOL, (int) visible, 0);
}

static
PHP_METHOD(Scintilla, get_doc_pointer)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETDOCPOINTER, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_doc_pointer)
{
	long pointer;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &pointer))
		return;

	SCINTILLA_MESSAGE(SCI_SETDOCPOINTER, 0, (int) pointer);
}

static
PHP_METHOD(Scintilla, set_mod_event_mask)
{
	long mask;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &mask))
		return;

	SCINTILLA_MESSAGE(SCI_SETMODEVENTMASK, (int) mask, 0);
}

static
PHP_METHOD(Scintilla, get_edge_column)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETEDGECOLUMN, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_edge_column)
{
	long column;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &column))
		return;

	SCINTILLA_MESSAGE(SCI_SETEDGECOLUMN, (int) column, 0);
}

static
PHP_METHOD(Scintilla, get_edge_mode)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETEDGEMODE, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_edge_mode)
{
	long mode;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &mode))
		return;

	SCINTILLA_MESSAGE(SCI_SETEDGEMODE, (int) mode, 0);
}

static
PHP_METHOD(Scintilla, get_edge_colour)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETEDGECOLOUR, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_edge_colour)
{
	long edgeColour;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &edgeColour))
		return;

	SCINTILLA_MESSAGE(SCI_SETEDGECOLOUR, (int) edgeColour, 0);
}

static
PHP_METHOD(Scintilla, search_anchor)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_SEARCHANCHOR, 0, 0);
}

static
PHP_METHOD(Scintilla, search_next)
{
	long flags;
	char *text;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "is", &flags, &text))
		return;

	retval = SCINTILLA_MESSAGE(SCI_SEARCHNEXT, (int) flags, (sptr_t) text);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, search_prev)
{
	long flags;
	char *text;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "is", &flags, &text))
		return;

	retval = SCINTILLA_MESSAGE(SCI_SEARCHPREV, (int) flags, (sptr_t) text);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, lines_on_screen)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_LINESONSCREEN, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, use_pop_up)
{
	long allowPopUp;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &allowPopUp))
		return;

	SCINTILLA_MESSAGE(SCI_USEPOPUP, (int) allowPopUp, 0);
}

static
PHP_METHOD(Scintilla, selection_is_rectangle)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_SELECTIONISRECTANGLE, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_zoom)
{
	long zoom;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &zoom))
		return;

	SCINTILLA_MESSAGE(SCI_SETZOOM, (int) zoom, 0);
}

static
PHP_METHOD(Scintilla, get_zoom)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETZOOM, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, create_document)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_CREATEDOCUMENT, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, add_ref_document)
{
	long doc;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &doc))
		return;

	SCINTILLA_MESSAGE(SCI_ADDREFDOCUMENT, 0, (int) doc);
}

static
PHP_METHOD(Scintilla, release_document)
{
	long doc;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &doc))
		return;

	SCINTILLA_MESSAGE(SCI_RELEASEDOCUMENT, 0, (int) doc);
}

static
PHP_METHOD(Scintilla, get_mod_event_mask)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETMODEVENTMASK, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_focus)
{
	long focus;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &focus))
		return;

	SCINTILLA_MESSAGE(SCI_SETFOCUS, (int) focus, 0);
}

static
PHP_METHOD(Scintilla, get_focus)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETFOCUS, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_status)
{
	long statusCode;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &statusCode))
		return;

	SCINTILLA_MESSAGE(SCI_SETSTATUS, (int) statusCode, 0);
}

static
PHP_METHOD(Scintilla, get_status)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETSTATUS, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_mouse_down_captures)
{
	long captures;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &captures))
		return;

	SCINTILLA_MESSAGE(SCI_SETMOUSEDOWNCAPTURES, (int) captures, 0);
}

static
PHP_METHOD(Scintilla, get_mouse_down_captures)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETMOUSEDOWNCAPTURES, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_cursor)
{
	long cursorType;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &cursorType))
		return;

	SCINTILLA_MESSAGE(SCI_SETCURSOR, (int) cursorType, 0);
}

static
PHP_METHOD(Scintilla, get_cursor)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETCURSOR, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_control_char_symbol)
{
	long symbol;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &symbol))
		return;

	SCINTILLA_MESSAGE(SCI_SETCONTROLCHARSYMBOL, (int) symbol, 0);
}

static
PHP_METHOD(Scintilla, get_control_char_symbol)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETCONTROLCHARSYMBOL, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, word_part_left)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_WORDPARTLEFT, 0, 0);
}

static
PHP_METHOD(Scintilla, word_part_left_extend)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_WORDPARTLEFTEXTEND, 0, 0);
}

static
PHP_METHOD(Scintilla, word_part_right)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_WORDPARTRIGHT, 0, 0);
}

static
PHP_METHOD(Scintilla, word_part_right_extend)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_WORDPARTRIGHTEXTEND, 0, 0);
}

static
PHP_METHOD(Scintilla, set_visible_policy)
{
	long visiblePolicy, visibleSlop;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &visiblePolicy, &visibleSlop))
		return;

	SCINTILLA_MESSAGE(SCI_SETVISIBLEPOLICY, (int) visiblePolicy, (int) visibleSlop);
}

static
PHP_METHOD(Scintilla, del_line_left)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_DELLINELEFT, 0, 0);
}

static
PHP_METHOD(Scintilla, del_line_right)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_DELLINERIGHT, 0, 0);
}

static
PHP_METHOD(Scintilla, set_xoffset)
{
	long newOffset;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &newOffset))
		return;

	SCINTILLA_MESSAGE(SCI_SETXOFFSET, (int) newOffset, 0);
}

static
PHP_METHOD(Scintilla, get_xoffset)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETXOFFSET, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, choose_caret_x)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_CHOOSECARETX, 0, 0);
}

static
PHP_METHOD(Scintilla, grab_focus)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_GRABFOCUS, 0, 0);
}

static
PHP_METHOD(Scintilla, set_xcaret_policy)
{
	long caretPolicy, caretSlop;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &caretPolicy, &caretSlop))
		return;

	SCINTILLA_MESSAGE(SCI_SETXCARETPOLICY, (int) caretPolicy, (int) caretSlop);
}

static
PHP_METHOD(Scintilla, set_ycaret_policy)
{
	long caretPolicy, caretSlop;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &caretPolicy, &caretSlop))
		return;

	SCINTILLA_MESSAGE(SCI_SETYCARETPOLICY, (int) caretPolicy, (int) caretSlop);
}

static
PHP_METHOD(Scintilla, set_print_wrap_mode)
{
	long mode;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &mode))
		return;

	SCINTILLA_MESSAGE(SCI_SETPRINTWRAPMODE, (int) mode, 0);
}

static
PHP_METHOD(Scintilla, get_print_wrap_mode)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETPRINTWRAPMODE, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_hotspot_active_fore)
{
	long useSetting, fore;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &useSetting, &fore))
		return;

	SCINTILLA_MESSAGE(SCI_SETHOTSPOTACTIVEFORE, (int) useSetting, (int) fore);
}

static
PHP_METHOD(Scintilla, get_hotspot_active_fore)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETHOTSPOTACTIVEFORE, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_hotspot_active_back)
{
	long useSetting, back;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &useSetting, &back))
		return;

	SCINTILLA_MESSAGE(SCI_SETHOTSPOTACTIVEBACK, (int) useSetting, (int) back);
}

static
PHP_METHOD(Scintilla, get_hotspot_active_back)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETHOTSPOTACTIVEBACK, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_hotspot_active_underline)
{
	long underline;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &underline))
		return;

	SCINTILLA_MESSAGE(SCI_SETHOTSPOTACTIVEUNDERLINE, (int) underline, 0);
}

static
PHP_METHOD(Scintilla, get_hotspot_active_underline)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETHOTSPOTACTIVEUNDERLINE, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_hotspot_single_line)
{
	long singleLine;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &singleLine))
		return;

	SCINTILLA_MESSAGE(SCI_SETHOTSPOTSINGLELINE, (int) singleLine, 0);
}

static
PHP_METHOD(Scintilla, get_hotspot_single_line)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETHOTSPOTSINGLELINE, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, para_down)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_PARADOWN, 0, 0);
}

static
PHP_METHOD(Scintilla, para_down_extend)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_PARADOWNEXTEND, 0, 0);
}

static
PHP_METHOD(Scintilla, para_up)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_PARAUP, 0, 0);
}

static
PHP_METHOD(Scintilla, para_up_extend)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_PARAUPEXTEND, 0, 0);
}

static
PHP_METHOD(Scintilla, position_before)
{
	long pos;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &pos))
		return;

	retval = SCINTILLA_MESSAGE(SCI_POSITIONBEFORE, (int) pos, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, position_after)
{
	long pos;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &pos))
		return;

	retval = SCINTILLA_MESSAGE(SCI_POSITIONAFTER, (int) pos, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, copy_range)
{
	long start, end;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &start, &end))
		return;

	SCINTILLA_MESSAGE(SCI_COPYRANGE, (int) start, (int) end);
}

static
PHP_METHOD(Scintilla, copy_text)
{
	long length;
	char *text;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "is", &length, &text))
		return;

	SCINTILLA_MESSAGE(SCI_COPYTEXT, (int) length, (sptr_t) text);
}

static
PHP_METHOD(Scintilla, set_selection_mode)
{
	long mode;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &mode))
		return;

	SCINTILLA_MESSAGE(SCI_SETSELECTIONMODE, (int) mode, 0);
}

static
PHP_METHOD(Scintilla, get_selection_mode)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETSELECTIONMODE, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, get_line_sel_start_position)
{
	long line;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &line))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETLINESELSTARTPOSITION, (int) line, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, get_line_sel_end_position)
{
	long line;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &line))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETLINESELENDPOSITION, (int) line, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, line_down_rect_extend)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_LINEDOWNRECTEXTEND, 0, 0);
}

static
PHP_METHOD(Scintilla, line_up_rect_extend)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_LINEUPRECTEXTEND, 0, 0);
}

static
PHP_METHOD(Scintilla, char_left_rect_extend)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_CHARLEFTRECTEXTEND, 0, 0);
}

static
PHP_METHOD(Scintilla, char_right_rect_extend)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_CHARRIGHTRECTEXTEND, 0, 0);
}

static
PHP_METHOD(Scintilla, home_rect_extend)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_HOMERECTEXTEND, 0, 0);
}

static
PHP_METHOD(Scintilla, vchome_rect_extend)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_VCHOMERECTEXTEND, 0, 0);
}

static
PHP_METHOD(Scintilla, line_end_rect_extend)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_LINEENDRECTEXTEND, 0, 0);
}

static
PHP_METHOD(Scintilla, page_up_rect_extend)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_PAGEUPRECTEXTEND, 0, 0);
}

static
PHP_METHOD(Scintilla, page_down_rect_extend)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_PAGEDOWNRECTEXTEND, 0, 0);
}

static
PHP_METHOD(Scintilla, stuttered_page_up)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_STUTTEREDPAGEUP, 0, 0);
}

static
PHP_METHOD(Scintilla, stuttered_page_up_extend)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_STUTTEREDPAGEUPEXTEND, 0, 0);
}

static
PHP_METHOD(Scintilla, stuttered_page_down)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_STUTTEREDPAGEDOWN, 0, 0);
}

static
PHP_METHOD(Scintilla, stuttered_page_down_extend)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_STUTTEREDPAGEDOWNEXTEND, 0, 0);
}

static
PHP_METHOD(Scintilla, word_left_end)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_WORDLEFTEND, 0, 0);
}

static
PHP_METHOD(Scintilla, word_left_end_extend)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_WORDLEFTENDEXTEND, 0, 0);
}

static
PHP_METHOD(Scintilla, word_right_end)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_WORDRIGHTEND, 0, 0);
}

static
PHP_METHOD(Scintilla, word_right_end_extend)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_WORDRIGHTENDEXTEND, 0, 0);
}

static
PHP_METHOD(Scintilla, set_whitespace_chars)
{
	char *characters;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "s", &characters))
		return;

	SCINTILLA_MESSAGE(SCI_SETWHITESPACECHARS, 0, (sptr_t) characters);
}

static
PHP_METHOD(Scintilla, set_chars_default)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_SETCHARSDEFAULT, 0, 0);
}

static
PHP_METHOD(Scintilla, auto_cget_current)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_AUTOCGETCURRENT, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, allocate)
{
	long bytes;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &bytes))
		return;

	SCINTILLA_MESSAGE(SCI_ALLOCATE, (int) bytes, 0);
}

static
PHP_METHOD(Scintilla, target_as_utf8)
{
	long length=0;
	char *text;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	length = SCINTILLA_MESSAGE(SCI_TARGETASUTF8, 0, 0);
	if(length) {
		text = emalloc(length);

		SCINTILLA_MESSAGE(SCI_TARGETASUTF8, 0, (sptr_t) text);

		RETVAL_STRINGL(text, length, 1);
		efree(text);
	} else {
		RETURN_EMPTY_STRING();
	}
}

static
PHP_METHOD(Scintilla, set_length_for_encode)
{
	long bytes;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &bytes))
		return;

	SCINTILLA_MESSAGE(SCI_SETLENGTHFORENCODE, (int) bytes, 0);
}

static
PHP_METHOD(Scintilla, encoded_from_utf8)
{
	long length=0;
	char *text_in, *text_out;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "s", &text_in))
		return;

	length = SCINTILLA_MESSAGE(SCI_ENCODEDFROMUTF8, (sptr_t) text_in, 0);
	if(length) {
		text_out = emalloc(length);

		SCINTILLA_MESSAGE(SCI_ENCODEDFROMUTF8, (sptr_t) text_in, (sptr_t) text_out);

		RETVAL_STRINGL(text_out, length, 1);
		efree(text_out);
	} else {
		RETURN_EMPTY_STRING();
	}
}

static
PHP_METHOD(Scintilla, find_column)
{
	long line, column;

	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &line, &column))
		return;

	retval = SCINTILLA_MESSAGE(SCI_FINDCOLUMN, (int) line, (int) column);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, get_caret_sticky)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETCARETSTICKY, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, set_caret_sticky)
{
	long useCaretStickyBehaviour;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &useCaretStickyBehaviour))
		return;

	SCINTILLA_MESSAGE(SCI_SETCARETSTICKY, (int) useCaretStickyBehaviour, 0);
}

static
PHP_METHOD(Scintilla, toggle_caret_sticky)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_TOGGLECARETSTICKY, 0, 0);
}

static
PHP_METHOD(Scintilla, set_paste_convert_endings)
{
	long convert;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &convert))
		return;

	SCINTILLA_MESSAGE(SCI_SETPASTECONVERTENDINGS, (int) convert, 0);
}

static
PHP_METHOD(Scintilla, get_paste_convert_endings)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETPASTECONVERTENDINGS, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, selection_duplicate)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_SELECTIONDUPLICATE, 0, 0);
}

static
PHP_METHOD(Scintilla, set_caret_line_back_alpha)
{
	long alpha;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &alpha))
		return;

	SCINTILLA_MESSAGE(SCI_SETCARETLINEBACKALPHA, (int) alpha, 0);
}

static
PHP_METHOD(Scintilla, get_caret_line_back_alpha)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETCARETLINEBACKALPHA, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, start_record)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_STARTRECORD, 0, 0);
}

static
PHP_METHOD(Scintilla, stop_record)
{

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	SCINTILLA_MESSAGE(SCI_STOPRECORD, 0, 0);
}

static
PHP_METHOD(Scintilla, set_lexer)
{
	long lexer;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "i", &lexer))
		return;

	SCINTILLA_MESSAGE(SCI_SETLEXER, (int) lexer, 0);
}

static
PHP_METHOD(Scintilla, get_lexer)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETLEXER, 0, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, colourise)
{
	long start, end;


	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ii", &start, &end))
		return;

	SCINTILLA_MESSAGE(SCI_COLOURISE, (int) start, (int) end);
}

static
PHP_METHOD(Scintilla, set_property)
{
	char *key, *value;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "ss", &key, &value))
		return;

	SCINTILLA_MESSAGE(SCI_SETPROPERTY, (sptr_t) key, (sptr_t) value);
}

static
PHP_METHOD(Scintilla, set_keywords)
{
	long keywordSet;
	char *keyWords;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "is", &keywordSet, &keyWords))
		return;

	SCINTILLA_MESSAGE(SCI_SETKEYWORDS, (int) keywordSet, (sptr_t) keyWords);
}

static
PHP_METHOD(Scintilla, set_lexer_language)
{
	char *language;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "s", &language))
		return;

	SCINTILLA_MESSAGE(SCI_SETLEXERLANGUAGE, 0, (sptr_t) language);
}

static
PHP_METHOD(Scintilla, load_lexer_library)
{
	char *path;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "s", &path))
		return;

	SCINTILLA_MESSAGE(SCI_LOADLEXERLIBRARY, 0, (sptr_t) path);
}

static
PHP_METHOD(Scintilla, get_property)
{
	long length=0;
	char *key, *value;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "s", &key))
		return;

	length = SCINTILLA_MESSAGE(SCI_GETPROPERTY, (sptr_t) key, 0);
	value = emalloc(length);

	length = SCINTILLA_MESSAGE(SCI_GETPROPERTY, (sptr_t) key, (sptr_t) value);

	RETVAL_STRINGL(value, length, 1);
	efree(value);
}

static
PHP_METHOD(Scintilla, get_property_expanded)
{
	long length=0;
	char *key, *value;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "s", &key))
		return;

	length = SCINTILLA_MESSAGE(SCI_GETPROPERTYEXPANDED, (sptr_t) key, 0);
	value = emalloc(length);

	length = SCINTILLA_MESSAGE(SCI_GETPROPERTYEXPANDED, (sptr_t) key, (sptr_t) value);

	RETVAL_STRINGL(value, length, 1);
	efree(value);
}

static
PHP_METHOD(Scintilla, get_property_int)
{
	char *key;
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), "s", &key))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETPROPERTYINT, (sptr_t) key, 0);

	RETURN_LONG(retval);
}

static
PHP_METHOD(Scintilla, get_style_bits_needed)
{
	long retval;

	NOT_STATIC_METHOD();

	if(!php_gtk_parse_args(ZEND_NUM_ARGS(), ""))
		return;

	retval = SCINTILLA_MESSAGE(SCI_GETSTYLEBITSNEEDED, 0, 0);

	RETURN_LONG(retval);
}


static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_add_text, 0)
	ZEND_ARG_INFO(0, text)
	ZEND_ARG_INFO(0, length)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_add_styled_text, 0)
	ZEND_ARG_INFO(0, length)
	ZEND_ARG_INFO(0, c)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_insert_text, 0)
	ZEND_ARG_INFO(0, pos)
	ZEND_ARG_INFO(0, text)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_get_char_at, 0)
	ZEND_ARG_INFO(0, pos)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_get_style_at, 0)
	ZEND_ARG_INFO(0, pos)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_undo_collection, 0)
	ZEND_ARG_INFO(0, collectUndo)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_get_styled_text, 0)
	ZEND_ARG_INFO(0, start)
	ZEND_ARG_INFO(0, end)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_marker_line_from_handle, 0)
	ZEND_ARG_INFO(0, handle)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_marker_delete_handle, 0)
	ZEND_ARG_INFO(0, handle)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_view_ws, 0)
	ZEND_ARG_INFO(0, viewWS)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_position_from_point, 0)
	ZEND_ARG_INFO(0, x)
	ZEND_ARG_INFO(0, y)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_position_from_point_close, 0)
	ZEND_ARG_INFO(0, x)
	ZEND_ARG_INFO(0, y)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_goto_line, 0)
	ZEND_ARG_INFO(0, line)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_goto_pos, 0)
	ZEND_ARG_INFO(0, pos)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_anchor, 0)
	ZEND_ARG_INFO(0, posAnchor)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_convert_eols, 0)
	ZEND_ARG_INFO(0, eolMode)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_eol_mode, 0)
	ZEND_ARG_INFO(0, eolMode)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_start_styling, 0)
	ZEND_ARG_INFO(0, pos)
	ZEND_ARG_INFO(0, mask)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_styling, 0)
	ZEND_ARG_INFO(0, length)
	ZEND_ARG_INFO(0, style)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_buffered_draw, 0)
	ZEND_ARG_INFO(0, buffered)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_tab_width, 0)
	ZEND_ARG_INFO(0, tabWidth)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_code_page, 0)
	ZEND_ARG_INFO(0, codePage)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_use_palette, 0)
	ZEND_ARG_INFO(0, usePalette)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_marker_define, 0)
	ZEND_ARG_INFO(0, markerNumber)
	ZEND_ARG_INFO(0, markerSymbol)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_marker_set_fore, 0)
	ZEND_ARG_INFO(0, markerNumber)
	ZEND_ARG_INFO(0, fore)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_marker_set_back, 0)
	ZEND_ARG_INFO(0, markerNumber)
	ZEND_ARG_INFO(0, back)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_marker_add, 0)
	ZEND_ARG_INFO(0, line)
	ZEND_ARG_INFO(0, markerNumber)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_marker_delete, 0)
	ZEND_ARG_INFO(0, line)
	ZEND_ARG_INFO(0, markerNumber)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_marker_delete_all, 0)
	ZEND_ARG_INFO(0, markerNumber)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_marker_get, 0)
	ZEND_ARG_INFO(0, line)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_marker_next, 0)
	ZEND_ARG_INFO(0, lineStart)
	ZEND_ARG_INFO(0, markerMask)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_marker_previous, 0)
	ZEND_ARG_INFO(0, lineStart)
	ZEND_ARG_INFO(0, markerMask)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_marker_define_pixmap, 0)
	ZEND_ARG_INFO(0, markerNumber)
	ZEND_ARG_INFO(0, pixmap)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_marker_add_set, 0)
	ZEND_ARG_INFO(0, line)
	ZEND_ARG_INFO(0, set)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_marker_set_alpha, 0)
	ZEND_ARG_INFO(0, markerNumber)
	ZEND_ARG_INFO(0, alpha)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_margin_type_n, 0)
	ZEND_ARG_INFO(0, margin)
	ZEND_ARG_INFO(0, marginType)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_get_margin_type_n, 0)
	ZEND_ARG_INFO(0, margin)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_margin_width_n, 0)
	ZEND_ARG_INFO(0, margin)
	ZEND_ARG_INFO(0, pixelWidth)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_get_margin_width_n, 0)
	ZEND_ARG_INFO(0, margin)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_margin_mask_n, 0)
	ZEND_ARG_INFO(0, margin)
	ZEND_ARG_INFO(0, mask)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_get_margin_mask_n, 0)
	ZEND_ARG_INFO(0, margin)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_margin_sensitive_n, 0)
	ZEND_ARG_INFO(0, margin)
	ZEND_ARG_INFO(0, sensitive)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_get_margin_sensitive_n, 0)
	ZEND_ARG_INFO(0, margin)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_style_set_fore, 0)
	ZEND_ARG_INFO(0, style)
	ZEND_ARG_INFO(0, fore)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_style_set_back, 0)
	ZEND_ARG_INFO(0, style)
	ZEND_ARG_INFO(0, back)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_style_set_bold, 0)
	ZEND_ARG_INFO(0, style)
	ZEND_ARG_INFO(0, bold)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_style_set_italic, 0)
	ZEND_ARG_INFO(0, style)
	ZEND_ARG_INFO(0, italic)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_style_set_size, 0)
	ZEND_ARG_INFO(0, style)
	ZEND_ARG_INFO(0, sizePoints)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_style_set_font, 0)
	ZEND_ARG_INFO(0, style)
	ZEND_ARG_INFO(0, fontName)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_style_set_eol_filled, 0)
	ZEND_ARG_INFO(0, style)
	ZEND_ARG_INFO(0, filled)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_style_set_underline, 0)
	ZEND_ARG_INFO(0, style)
	ZEND_ARG_INFO(0, underline)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_style_get_fore, 0)
	ZEND_ARG_INFO(0, style)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_style_get_back, 0)
	ZEND_ARG_INFO(0, style)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_style_get_bold, 0)
	ZEND_ARG_INFO(0, style)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_style_get_italic, 0)
	ZEND_ARG_INFO(0, style)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_style_get_size, 0)
	ZEND_ARG_INFO(0, style)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_style_get_font, 0)
	ZEND_ARG_INFO(0, style)
	ZEND_ARG_INFO(0, fontName)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_style_get_eol_filled, 0)
	ZEND_ARG_INFO(0, style)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_style_get_underline, 0)
	ZEND_ARG_INFO(0, style)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_style_get_case, 0)
	ZEND_ARG_INFO(0, style)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_style_get_character_set, 0)
	ZEND_ARG_INFO(0, style)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_style_get_visible, 0)
	ZEND_ARG_INFO(0, style)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_style_get_changeable, 0)
	ZEND_ARG_INFO(0, style)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_style_get_hot_spot, 0)
	ZEND_ARG_INFO(0, style)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_style_set_case, 0)
	ZEND_ARG_INFO(0, style)
	ZEND_ARG_INFO(0, caseForce)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_style_set_character_set, 0)
	ZEND_ARG_INFO(0, style)
	ZEND_ARG_INFO(0, characterSet)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_style_set_hot_spot, 0)
	ZEND_ARG_INFO(0, style)
	ZEND_ARG_INFO(0, hotspot)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_sel_fore, 0)
	ZEND_ARG_INFO(0, useSetting)
	ZEND_ARG_INFO(0, fore)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_sel_back, 0)
	ZEND_ARG_INFO(0, useSetting)
	ZEND_ARG_INFO(0, back)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_sel_alpha, 0)
	ZEND_ARG_INFO(0, alpha)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_sel_eol_filled, 0)
	ZEND_ARG_INFO(0, filled)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_caret_fore, 0)
	ZEND_ARG_INFO(0, fore)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_assign_cmd_key, 0)
	ZEND_ARG_INFO(0, km)
	ZEND_ARG_INFO(0, msg)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_clear_cmd_key, 0)
	ZEND_ARG_INFO(0, km)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_styling_ex, 0)
	ZEND_ARG_INFO(0, length)
	ZEND_ARG_INFO(0, styles)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_style_set_visible, 0)
	ZEND_ARG_INFO(0, style)
	ZEND_ARG_INFO(0, visible)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_caret_period, 0)
	ZEND_ARG_INFO(0, periodMilliseconds)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_word_chars, 0)
	ZEND_ARG_INFO(0, characters)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_indic_set_style, 0)
	ZEND_ARG_INFO(0, indic)
	ZEND_ARG_INFO(0, style)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_indic_get_style, 0)
	ZEND_ARG_INFO(0, indic)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_indic_set_fore, 0)
	ZEND_ARG_INFO(0, indic)
	ZEND_ARG_INFO(0, fore)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_indic_get_fore, 0)
	ZEND_ARG_INFO(0, indic)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_whitespace_fore, 0)
	ZEND_ARG_INFO(0, useSetting)
	ZEND_ARG_INFO(0, fore)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_whitespace_back, 0)
	ZEND_ARG_INFO(0, useSetting)
	ZEND_ARG_INFO(0, back)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_style_bits, 0)
	ZEND_ARG_INFO(0, bits)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_line_state, 0)
	ZEND_ARG_INFO(0, line)
	ZEND_ARG_INFO(0, state)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_get_line_state, 0)
	ZEND_ARG_INFO(0, line)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_caret_line_visible, 0)
	ZEND_ARG_INFO(0, show)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_caret_line_back, 0)
	ZEND_ARG_INFO(0, back)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_style_set_changeable, 0)
	ZEND_ARG_INFO(0, style)
	ZEND_ARG_INFO(0, changeable)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_auto_cshow, 0)
	ZEND_ARG_INFO(0, lenEntered)
	ZEND_ARG_INFO(0, itemList)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_auto_cstops, 0)
	ZEND_ARG_INFO(0, characterSet)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_auto_cset_separator, 0)
	ZEND_ARG_INFO(0, separatorCharacter)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_auto_cselect, 0)
	ZEND_ARG_INFO(0, text)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_auto_cset_cancel_at_start, 0)
	ZEND_ARG_INFO(0, cancel)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_auto_cset_fill_ups, 0)
	ZEND_ARG_INFO(0, characterSet)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_auto_cset_choose_single, 0)
	ZEND_ARG_INFO(0, chooseSingle)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_auto_cset_ignore_case, 0)
	ZEND_ARG_INFO(0, ignoreCase)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_user_list_show, 0)
	ZEND_ARG_INFO(0, listType)
	ZEND_ARG_INFO(0, itemList)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_auto_cset_auto_hide, 0)
	ZEND_ARG_INFO(0, autoHide)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_auto_cset_drop_rest_of_word, 0)
	ZEND_ARG_INFO(0, dropRestOfWord)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_register_image, 0)
	ZEND_ARG_INFO(0, type)
	ZEND_ARG_INFO(0, xpmData)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_auto_cset_type_separator, 0)
	ZEND_ARG_INFO(0, separatorCharacter)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_auto_cset_max_width, 0)
	ZEND_ARG_INFO(0, characterCount)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_auto_cset_max_height, 0)
	ZEND_ARG_INFO(0, rowCount)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_indent, 0)
	ZEND_ARG_INFO(0, indentSize)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_use_tabs, 0)
	ZEND_ARG_INFO(0, useTabs)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_line_indentation, 0)
	ZEND_ARG_INFO(0, line)
	ZEND_ARG_INFO(0, indentSize)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_get_line_indentation, 0)
	ZEND_ARG_INFO(0, line)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_get_line_indent_position, 0)
	ZEND_ARG_INFO(0, line)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_get_column, 0)
	ZEND_ARG_INFO(0, pos)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_hscroll_bar, 0)
	ZEND_ARG_INFO(0, show)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_indentation_guides, 0)
	ZEND_ARG_INFO(0, show)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_highlight_guide, 0)
	ZEND_ARG_INFO(0, column)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_get_line_end_position, 0)
	ZEND_ARG_INFO(0, line)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_current_pos, 0)
	ZEND_ARG_INFO(0, pos)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_selection_start, 0)
	ZEND_ARG_INFO(0, pos)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_selection_end, 0)
	ZEND_ARG_INFO(0, pos)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_print_magnification, 0)
	ZEND_ARG_INFO(0, magnification)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_print_colour_mode, 0)
	ZEND_ARG_INFO(0, mode)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_find_text, 0)
	ZEND_ARG_INFO(0, flags)
	ZEND_ARG_INFO(0, text)
	ZEND_ARG_INFO(0, chrg_min)
	ZEND_ARG_INFO(0, chrg_max)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_get_line, 0)
	ZEND_ARG_INFO(0, line)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_margin_left, 0)
	ZEND_ARG_INFO(0, pixelWidth)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_margin_right, 0)
	ZEND_ARG_INFO(0, pixelWidth)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_sel, 0)
	ZEND_ARG_INFO(0, start)
	ZEND_ARG_INFO(0, end)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_get_text_range, 0)
	ZEND_ARG_INFO(0, start)
	ZEND_ARG_INFO(0, end)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_hide_selection, 0)
	ZEND_ARG_INFO(0, normal)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_point_xfrom_position, 0)
	ZEND_ARG_INFO(0, pos)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_point_yfrom_position, 0)
	ZEND_ARG_INFO(0, pos)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_line_from_position, 0)
	ZEND_ARG_INFO(0, pos)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_position_from_line, 0)
	ZEND_ARG_INFO(0, line)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_line_scroll, 0)
	ZEND_ARG_INFO(0, columns)
	ZEND_ARG_INFO(0, lines)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_replace_sel, 0)
	ZEND_ARG_INFO(0, text)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_read_only, 0)
	ZEND_ARG_INFO(0, readOnly)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_text, 0)
	ZEND_ARG_INFO(0, text)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_get_text, 0)
	ZEND_ARG_INFO(0, length)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_overtype, 0)
	ZEND_ARG_INFO(0, overtype)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_caret_width, 0)
	ZEND_ARG_INFO(0, pixelWidth)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_target_start, 0)
	ZEND_ARG_INFO(0, pos)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_target_end, 0)
	ZEND_ARG_INFO(0, pos)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_replace_target, 0)
	ZEND_ARG_INFO(0, length)
	ZEND_ARG_INFO(0, text)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_replace_target_re, 0)
	ZEND_ARG_INFO(0, length)
	ZEND_ARG_INFO(0, text)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_search_in_target, 0)
	ZEND_ARG_INFO(0, length)
	ZEND_ARG_INFO(0, text)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_search_flags, 0)
	ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_call_tip_show, 0)
	ZEND_ARG_INFO(0, pos)
	ZEND_ARG_INFO(0, definition)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_call_tip_set_hlt, 0)
	ZEND_ARG_INFO(0, start)
	ZEND_ARG_INFO(0, end)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_call_tip_set_back, 0)
	ZEND_ARG_INFO(0, back)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_call_tip_set_fore, 0)
	ZEND_ARG_INFO(0, fore)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_call_tip_set_fore_hlt, 0)
	ZEND_ARG_INFO(0, fore)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_call_tip_use_style, 0)
	ZEND_ARG_INFO(0, tabSize)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_visible_from_doc_line, 0)
	ZEND_ARG_INFO(0, line)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_doc_line_from_visible, 0)
	ZEND_ARG_INFO(0, lineDisplay)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_wrap_count, 0)
	ZEND_ARG_INFO(0, line)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_fold_level, 0)
	ZEND_ARG_INFO(0, line)
	ZEND_ARG_INFO(0, level)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_get_fold_level, 0)
	ZEND_ARG_INFO(0, line)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_get_last_child, 0)
	ZEND_ARG_INFO(0, line)
	ZEND_ARG_INFO(0, level)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_get_fold_parent, 0)
	ZEND_ARG_INFO(0, line)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_show_lines, 0)
	ZEND_ARG_INFO(0, lineStart)
	ZEND_ARG_INFO(0, lineEnd)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_hide_lines, 0)
	ZEND_ARG_INFO(0, lineStart)
	ZEND_ARG_INFO(0, lineEnd)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_get_line_visible, 0)
	ZEND_ARG_INFO(0, line)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_fold_expanded, 0)
	ZEND_ARG_INFO(0, line)
	ZEND_ARG_INFO(0, expanded)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_get_fold_expanded, 0)
	ZEND_ARG_INFO(0, line)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_toggle_fold, 0)
	ZEND_ARG_INFO(0, line)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_ensure_visible, 0)
	ZEND_ARG_INFO(0, line)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_fold_flags, 0)
	ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_ensure_visible_enforce_policy, 0)
	ZEND_ARG_INFO(0, line)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_tab_indents, 0)
	ZEND_ARG_INFO(0, tabIndents)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_backspace_unindents, 0)
	ZEND_ARG_INFO(0, bsUnIndents)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_mouse_dwell_time, 0)
	ZEND_ARG_INFO(0, periodMilliseconds)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_word_start_position, 0)
	ZEND_ARG_INFO(0, pos)
	ZEND_ARG_INFO(0, onlyWordCharacters)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_word_end_position, 0)
	ZEND_ARG_INFO(0, pos)
	ZEND_ARG_INFO(0, onlyWordCharacters)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_wrap_mode, 0)
	ZEND_ARG_INFO(0, mode)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_wrap_visual_flags, 0)
	ZEND_ARG_INFO(0, wrapVisualFlags)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_wrap_visual_flags_location, 0)
	ZEND_ARG_INFO(0, wrapVisualFlagsLocation)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_wrap_start_indent, 0)
	ZEND_ARG_INFO(0, indent)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_layout_cache, 0)
	ZEND_ARG_INFO(0, mode)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_scroll_width, 0)
	ZEND_ARG_INFO(0, pixelWidth)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_text_width, 0)
	ZEND_ARG_INFO(0, style)
	ZEND_ARG_INFO(0, text)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_end_at_last_line, 0)
	ZEND_ARG_INFO(0, endAtLastLine)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_text_height, 0)
	ZEND_ARG_INFO(0, line)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_vscroll_bar, 0)
	ZEND_ARG_INFO(0, show)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_append_text, 0)
	ZEND_ARG_INFO(0, length)
	ZEND_ARG_INFO(0, text)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_two_phase_draw, 0)
	ZEND_ARG_INFO(0, twoPhase)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_lines_split, 0)
	ZEND_ARG_INFO(0, pixelWidth)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_fold_margin_colour, 0)
	ZEND_ARG_INFO(0, useSetting)
	ZEND_ARG_INFO(0, back)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_fold_margin_hi_colour, 0)
	ZEND_ARG_INFO(0, useSetting)
	ZEND_ARG_INFO(0, fore)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_line_length, 0)
	ZEND_ARG_INFO(0, line)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_brace_highlight, 0)
	ZEND_ARG_INFO(0, pos1)
	ZEND_ARG_INFO(0, pos2)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_brace_bad_light, 0)
	ZEND_ARG_INFO(0, pos)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_brace_match, 0)
	ZEND_ARG_INFO(0, pos)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_view_eol, 0)
	ZEND_ARG_INFO(0, visible)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_doc_pointer, 0)
	ZEND_ARG_INFO(0, pointer)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_mod_event_mask, 0)
	ZEND_ARG_INFO(0, mask)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_edge_column, 0)
	ZEND_ARG_INFO(0, column)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_edge_mode, 0)
	ZEND_ARG_INFO(0, mode)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_edge_colour, 0)
	ZEND_ARG_INFO(0, edgeColour)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_search_next, 0)
	ZEND_ARG_INFO(0, flags)
	ZEND_ARG_INFO(0, text)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_search_prev, 0)
	ZEND_ARG_INFO(0, flags)
	ZEND_ARG_INFO(0, text)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_use_pop_up, 0)
	ZEND_ARG_INFO(0, allowPopUp)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_zoom, 0)
	ZEND_ARG_INFO(0, zoom)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_add_ref_document, 0)
	ZEND_ARG_INFO(0, doc)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_release_document, 0)
	ZEND_ARG_INFO(0, doc)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_focus, 0)
	ZEND_ARG_INFO(0, focus)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_status, 0)
	ZEND_ARG_INFO(0, statusCode)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_mouse_down_captures, 0)
	ZEND_ARG_INFO(0, captures)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_cursor, 0)
	ZEND_ARG_INFO(0, cursorType)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_control_char_symbol, 0)
	ZEND_ARG_INFO(0, symbol)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_visible_policy, 0)
	ZEND_ARG_INFO(0, visiblePolicy)
	ZEND_ARG_INFO(0, visibleSlop)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_xoffset, 0)
	ZEND_ARG_INFO(0, newOffset)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_xcaret_policy, 0)
	ZEND_ARG_INFO(0, caretPolicy)
	ZEND_ARG_INFO(0, caretSlop)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_ycaret_policy, 0)
	ZEND_ARG_INFO(0, caretPolicy)
	ZEND_ARG_INFO(0, caretSlop)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_print_wrap_mode, 0)
	ZEND_ARG_INFO(0, mode)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_hotspot_active_fore, 0)
	ZEND_ARG_INFO(0, useSetting)
	ZEND_ARG_INFO(0, fore)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_hotspot_active_back, 0)
	ZEND_ARG_INFO(0, useSetting)
	ZEND_ARG_INFO(0, back)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_hotspot_active_underline, 0)
	ZEND_ARG_INFO(0, underline)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_hotspot_single_line, 0)
	ZEND_ARG_INFO(0, singleLine)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_position_before, 0)
	ZEND_ARG_INFO(0, pos)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_position_after, 0)
	ZEND_ARG_INFO(0, pos)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_copy_range, 0)
	ZEND_ARG_INFO(0, start)
	ZEND_ARG_INFO(0, end)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_copy_text, 0)
	ZEND_ARG_INFO(0, length)
	ZEND_ARG_INFO(0, text)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_selection_mode, 0)
	ZEND_ARG_INFO(0, mode)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_get_line_sel_start_position, 0)
	ZEND_ARG_INFO(0, line)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_get_line_sel_end_position, 0)
	ZEND_ARG_INFO(0, line)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_whitespace_chars, 0)
	ZEND_ARG_INFO(0, characters)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_allocate, 0)
	ZEND_ARG_INFO(0, bytes)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_length_for_encode, 0)
	ZEND_ARG_INFO(0, bytes)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_encoded_from_utf8, 0)
	ZEND_ARG_INFO(0, text_in)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_find_column, 0)
	ZEND_ARG_INFO(0, line)
	ZEND_ARG_INFO(0, column)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_caret_sticky, 0)
	ZEND_ARG_INFO(0, useCaretStickyBehaviour)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_paste_convert_endings, 0)
	ZEND_ARG_INFO(0, convert)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_caret_line_back_alpha, 0)
	ZEND_ARG_INFO(0, alpha)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_lexer, 0)
	ZEND_ARG_INFO(0, lexer)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_colourise, 0)
	ZEND_ARG_INFO(0, start)
	ZEND_ARG_INFO(0, end)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_property, 0)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_keywords, 0)
	ZEND_ARG_INFO(0, keywordSet)
	ZEND_ARG_INFO(0, keyWords)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_set_lexer_language, 0)
	ZEND_ARG_INFO(0, language)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_load_lexer_library, 0)
	ZEND_ARG_INFO(0, path)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_get_property, 0)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_scintilla_get_property_expanded, 0)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO();

static function_entry scintilla_methods[] = {
	PHP_ME(Scintilla, __construct, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, add_text,arginfo_scintilla_add_text, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, add_styled_text,arginfo_scintilla_add_styled_text, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, insert_text,arginfo_scintilla_insert_text, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, clear_all, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, clear_document_style, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_length, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_char_at,arginfo_scintilla_get_char_at, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_current_pos, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_anchor, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_style_at,arginfo_scintilla_get_style_at, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, redo, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_undo_collection,arginfo_scintilla_set_undo_collection, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, select_all, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_save_point, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_styled_text,arginfo_scintilla_get_styled_text, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, can_redo, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, marker_line_from_handle,arginfo_scintilla_marker_line_from_handle, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, marker_delete_handle,arginfo_scintilla_marker_delete_handle, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_undo_collection, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_view_ws, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_view_ws,arginfo_scintilla_set_view_ws, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, position_from_point,arginfo_scintilla_position_from_point, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, position_from_point_close,arginfo_scintilla_position_from_point_close, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, goto_line,arginfo_scintilla_goto_line, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, goto_pos,arginfo_scintilla_goto_pos, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_anchor,arginfo_scintilla_set_anchor, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_cur_line, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_end_styled, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, convert_eols,arginfo_scintilla_convert_eols, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_eol_mode, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_eol_mode,arginfo_scintilla_set_eol_mode, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, start_styling,arginfo_scintilla_start_styling, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_styling,arginfo_scintilla_set_styling, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_buffered_draw, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_buffered_draw,arginfo_scintilla_set_buffered_draw, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_tab_width,arginfo_scintilla_set_tab_width, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_tab_width, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_code_page,arginfo_scintilla_set_code_page, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_use_palette,arginfo_scintilla_set_use_palette, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, marker_define,arginfo_scintilla_marker_define, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, marker_set_fore,arginfo_scintilla_marker_set_fore, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, marker_set_back,arginfo_scintilla_marker_set_back, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, marker_add,arginfo_scintilla_marker_add, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, marker_delete,arginfo_scintilla_marker_delete, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, marker_delete_all,arginfo_scintilla_marker_delete_all, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, marker_get,arginfo_scintilla_marker_get, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, marker_next,arginfo_scintilla_marker_next, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, marker_previous,arginfo_scintilla_marker_previous, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, marker_define_pixmap,arginfo_scintilla_marker_define_pixmap, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, marker_add_set,arginfo_scintilla_marker_add_set, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, marker_set_alpha,arginfo_scintilla_marker_set_alpha, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_margin_type_n,arginfo_scintilla_set_margin_type_n, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_margin_type_n,arginfo_scintilla_get_margin_type_n, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_margin_width_n,arginfo_scintilla_set_margin_width_n, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_margin_width_n,arginfo_scintilla_get_margin_width_n, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_margin_mask_n,arginfo_scintilla_set_margin_mask_n, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_margin_mask_n,arginfo_scintilla_get_margin_mask_n, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_margin_sensitive_n,arginfo_scintilla_set_margin_sensitive_n, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_margin_sensitive_n,arginfo_scintilla_get_margin_sensitive_n, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, style_clear_all, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, style_set_fore,arginfo_scintilla_style_set_fore, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, style_set_back,arginfo_scintilla_style_set_back, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, style_set_bold,arginfo_scintilla_style_set_bold, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, style_set_italic,arginfo_scintilla_style_set_italic, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, style_set_size,arginfo_scintilla_style_set_size, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, style_set_font,arginfo_scintilla_style_set_font, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, style_set_eol_filled,arginfo_scintilla_style_set_eol_filled, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, style_reset_default, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, style_set_underline,arginfo_scintilla_style_set_underline, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, style_get_fore,arginfo_scintilla_style_get_fore, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, style_get_back,arginfo_scintilla_style_get_back, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, style_get_bold,arginfo_scintilla_style_get_bold, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, style_get_italic,arginfo_scintilla_style_get_italic, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, style_get_size,arginfo_scintilla_style_get_size, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, style_get_font,arginfo_scintilla_style_get_font, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, style_get_eol_filled,arginfo_scintilla_style_get_eol_filled, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, style_get_underline,arginfo_scintilla_style_get_underline, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, style_get_case,arginfo_scintilla_style_get_case, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, style_get_character_set,arginfo_scintilla_style_get_character_set, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, style_get_visible,arginfo_scintilla_style_get_visible, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, style_get_changeable,arginfo_scintilla_style_get_changeable, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, style_get_hot_spot,arginfo_scintilla_style_get_hot_spot, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, style_set_case,arginfo_scintilla_style_set_case, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, style_set_character_set,arginfo_scintilla_style_set_character_set, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, style_set_hot_spot,arginfo_scintilla_style_set_hot_spot, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_sel_fore,arginfo_scintilla_set_sel_fore, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_sel_back,arginfo_scintilla_set_sel_back, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_sel_alpha, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_sel_alpha,arginfo_scintilla_set_sel_alpha, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_sel_eol_filled, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_sel_eol_filled,arginfo_scintilla_set_sel_eol_filled, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_caret_fore,arginfo_scintilla_set_caret_fore, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, assign_cmd_key,arginfo_scintilla_assign_cmd_key, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, clear_cmd_key,arginfo_scintilla_clear_cmd_key, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, clear_all_cmd_keys, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_styling_ex,arginfo_scintilla_set_styling_ex, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, style_set_visible,arginfo_scintilla_style_set_visible, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_caret_period, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_caret_period,arginfo_scintilla_set_caret_period, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_word_chars,arginfo_scintilla_set_word_chars, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, begin_undo_action, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, end_undo_action, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, indic_set_style,arginfo_scintilla_indic_set_style, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, indic_get_style,arginfo_scintilla_indic_get_style, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, indic_set_fore,arginfo_scintilla_indic_set_fore, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, indic_get_fore,arginfo_scintilla_indic_get_fore, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_whitespace_fore,arginfo_scintilla_set_whitespace_fore, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_whitespace_back,arginfo_scintilla_set_whitespace_back, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_style_bits,arginfo_scintilla_set_style_bits, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_style_bits, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_line_state,arginfo_scintilla_set_line_state, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_line_state,arginfo_scintilla_get_line_state, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_max_line_state, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_caret_line_visible, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_caret_line_visible,arginfo_scintilla_set_caret_line_visible, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_caret_line_back, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_caret_line_back,arginfo_scintilla_set_caret_line_back, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, style_set_changeable,arginfo_scintilla_style_set_changeable, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, auto_cshow,arginfo_scintilla_auto_cshow, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, auto_ccancel, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, auto_cactive, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, auto_cpos_start, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, auto_ccomplete, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, auto_cstops,arginfo_scintilla_auto_cstops, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, auto_cset_separator,arginfo_scintilla_auto_cset_separator, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, auto_cget_separator, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, auto_cselect,arginfo_scintilla_auto_cselect, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, auto_cset_cancel_at_start,arginfo_scintilla_auto_cset_cancel_at_start, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, auto_cget_cancel_at_start, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, auto_cset_fill_ups,arginfo_scintilla_auto_cset_fill_ups, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, auto_cset_choose_single,arginfo_scintilla_auto_cset_choose_single, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, auto_cget_choose_single, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, auto_cset_ignore_case,arginfo_scintilla_auto_cset_ignore_case, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, auto_cget_ignore_case, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, user_list_show,arginfo_scintilla_user_list_show, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, auto_cset_auto_hide,arginfo_scintilla_auto_cset_auto_hide, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, auto_cget_auto_hide, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, auto_cset_drop_rest_of_word,arginfo_scintilla_auto_cset_drop_rest_of_word, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, auto_cget_drop_rest_of_word, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, register_image,arginfo_scintilla_register_image, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, clear_registered_images, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, auto_cget_type_separator, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, auto_cset_type_separator,arginfo_scintilla_auto_cset_type_separator, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, auto_cset_max_width,arginfo_scintilla_auto_cset_max_width, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, auto_cget_max_width, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, auto_cset_max_height,arginfo_scintilla_auto_cset_max_height, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, auto_cget_max_height, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_indent,arginfo_scintilla_set_indent, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_indent, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_use_tabs,arginfo_scintilla_set_use_tabs, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_use_tabs, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_line_indentation,arginfo_scintilla_set_line_indentation, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_line_indentation,arginfo_scintilla_get_line_indentation, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_line_indent_position,arginfo_scintilla_get_line_indent_position, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_column,arginfo_scintilla_get_column, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_hscroll_bar,arginfo_scintilla_set_hscroll_bar, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_hscroll_bar, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_indentation_guides,arginfo_scintilla_set_indentation_guides, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_indentation_guides, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_highlight_guide,arginfo_scintilla_set_highlight_guide, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_highlight_guide, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_line_end_position,arginfo_scintilla_get_line_end_position, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_code_page, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_caret_fore, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_use_palette, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_read_only, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_current_pos,arginfo_scintilla_set_current_pos, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_selection_start,arginfo_scintilla_set_selection_start, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_selection_start, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_selection_end,arginfo_scintilla_set_selection_end, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_selection_end, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_print_magnification,arginfo_scintilla_set_print_magnification, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_print_magnification, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_print_colour_mode,arginfo_scintilla_set_print_colour_mode, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_print_colour_mode, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, find_text,arginfo_scintilla_find_text, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, format_range, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_first_visible_line, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_line,arginfo_scintilla_get_line, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_line_count, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_margin_left,arginfo_scintilla_set_margin_left, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_margin_left, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_margin_right,arginfo_scintilla_set_margin_right, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_margin_right, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_modify, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_sel,arginfo_scintilla_set_sel, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_sel_text, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_text_range,arginfo_scintilla_get_text_range, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, hide_selection,arginfo_scintilla_hide_selection, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, point_xfrom_position,arginfo_scintilla_point_xfrom_position, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, point_yfrom_position,arginfo_scintilla_point_yfrom_position, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, line_from_position,arginfo_scintilla_line_from_position, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, position_from_line,arginfo_scintilla_position_from_line, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, line_scroll,arginfo_scintilla_line_scroll, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, scroll_caret, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, replace_sel,arginfo_scintilla_replace_sel, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_read_only,arginfo_scintilla_set_read_only, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, null, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, can_paste, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, can_undo, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, empty_undo_buffer, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, undo, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, cut, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, copy, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, paste, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, clear, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_text,arginfo_scintilla_set_text, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_text,arginfo_scintilla_get_text, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_text_length, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_direct_function, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_direct_pointer, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_overtype,arginfo_scintilla_set_overtype, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_overtype, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_caret_width,arginfo_scintilla_set_caret_width, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_caret_width, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_target_start,arginfo_scintilla_set_target_start, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_target_start, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_target_end,arginfo_scintilla_set_target_end, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_target_end, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, replace_target,arginfo_scintilla_replace_target, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, replace_target_re,arginfo_scintilla_replace_target_re, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, search_in_target,arginfo_scintilla_search_in_target, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_search_flags,arginfo_scintilla_set_search_flags, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_search_flags, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, call_tip_show,arginfo_scintilla_call_tip_show, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, call_tip_cancel, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, call_tip_active, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, call_tip_pos_start, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, call_tip_set_hlt,arginfo_scintilla_call_tip_set_hlt, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, call_tip_set_back,arginfo_scintilla_call_tip_set_back, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, call_tip_set_fore,arginfo_scintilla_call_tip_set_fore, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, call_tip_set_fore_hlt,arginfo_scintilla_call_tip_set_fore_hlt, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, call_tip_use_style,arginfo_scintilla_call_tip_use_style, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, visible_from_doc_line,arginfo_scintilla_visible_from_doc_line, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, doc_line_from_visible,arginfo_scintilla_doc_line_from_visible, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, wrap_count,arginfo_scintilla_wrap_count, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_fold_level,arginfo_scintilla_set_fold_level, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_fold_level,arginfo_scintilla_get_fold_level, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_last_child,arginfo_scintilla_get_last_child, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_fold_parent,arginfo_scintilla_get_fold_parent, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, show_lines,arginfo_scintilla_show_lines, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, hide_lines,arginfo_scintilla_hide_lines, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_line_visible,arginfo_scintilla_get_line_visible, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_fold_expanded,arginfo_scintilla_set_fold_expanded, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_fold_expanded,arginfo_scintilla_get_fold_expanded, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, toggle_fold,arginfo_scintilla_toggle_fold, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, ensure_visible,arginfo_scintilla_ensure_visible, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_fold_flags,arginfo_scintilla_set_fold_flags, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, ensure_visible_enforce_policy,arginfo_scintilla_ensure_visible_enforce_policy, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_tab_indents,arginfo_scintilla_set_tab_indents, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_tab_indents, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_backspace_unindents,arginfo_scintilla_set_backspace_unindents, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_backspace_unindents, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_mouse_dwell_time,arginfo_scintilla_set_mouse_dwell_time, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_mouse_dwell_time, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, word_start_position,arginfo_scintilla_word_start_position, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, word_end_position,arginfo_scintilla_word_end_position, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_wrap_mode,arginfo_scintilla_set_wrap_mode, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_wrap_mode, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_wrap_visual_flags,arginfo_scintilla_set_wrap_visual_flags, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_wrap_visual_flags, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_wrap_visual_flags_location,arginfo_scintilla_set_wrap_visual_flags_location, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_wrap_visual_flags_location, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_wrap_start_indent,arginfo_scintilla_set_wrap_start_indent, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_wrap_start_indent, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_layout_cache,arginfo_scintilla_set_layout_cache, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_layout_cache, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_scroll_width,arginfo_scintilla_set_scroll_width, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_scroll_width, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, text_width,arginfo_scintilla_text_width, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_end_at_last_line,arginfo_scintilla_set_end_at_last_line, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_end_at_last_line, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, text_height,arginfo_scintilla_text_height, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_vscroll_bar,arginfo_scintilla_set_vscroll_bar, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_vscroll_bar, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, append_text,arginfo_scintilla_append_text, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_two_phase_draw, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_two_phase_draw,arginfo_scintilla_set_two_phase_draw, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, target_from_selection, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, lines_join, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, lines_split,arginfo_scintilla_lines_split, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_fold_margin_colour,arginfo_scintilla_set_fold_margin_colour, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_fold_margin_hi_colour,arginfo_scintilla_set_fold_margin_hi_colour, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, line_down, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, line_down_extend, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, line_up, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, line_up_extend, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, char_left, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, char_left_extend, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, char_right, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, char_right_extend, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, word_left, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, word_left_extend, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, word_right, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, word_right_extend, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, home, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, home_extend, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, line_end, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, line_end_extend, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, document_start, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, document_start_extend, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, document_end, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, document_end_extend, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, page_up, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, page_up_extend, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, page_down, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, page_down_extend, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, edit_toggle_overtype, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, cancel, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, delete_back, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, tab, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, back_tab, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, new_line, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, form_feed, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, vchome, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, vchome_extend, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, zoom_in, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, zoom_out, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, del_word_left, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, del_word_right, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, line_cut, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, line_delete, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, line_transpose, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, line_duplicate, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, lower_case, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, upper_case, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, line_scroll_down, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, line_scroll_up, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, delete_back_not_line, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, home_display, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, home_display_extend, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, line_end_display, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, line_end_display_extend, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, home_wrap, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, home_wrap_extend, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, line_end_wrap, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, line_end_wrap_extend, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, vchome_wrap, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, vchome_wrap_extend, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, line_copy, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, move_caret_inside_view, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, line_length,arginfo_scintilla_line_length, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, brace_highlight,arginfo_scintilla_brace_highlight, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, brace_bad_light,arginfo_scintilla_brace_bad_light, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, brace_match,arginfo_scintilla_brace_match, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_view_eol, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_view_eol,arginfo_scintilla_set_view_eol, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_doc_pointer, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_doc_pointer,arginfo_scintilla_set_doc_pointer, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_mod_event_mask,arginfo_scintilla_set_mod_event_mask, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_edge_column, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_edge_column,arginfo_scintilla_set_edge_column, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_edge_mode, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_edge_mode,arginfo_scintilla_set_edge_mode, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_edge_colour, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_edge_colour,arginfo_scintilla_set_edge_colour, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, search_anchor, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, search_next,arginfo_scintilla_search_next, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, search_prev,arginfo_scintilla_search_prev, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, lines_on_screen, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, use_pop_up,arginfo_scintilla_use_pop_up, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, selection_is_rectangle, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_zoom,arginfo_scintilla_set_zoom, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_zoom, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, create_document, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, add_ref_document,arginfo_scintilla_add_ref_document, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, release_document,arginfo_scintilla_release_document, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_mod_event_mask, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_focus,arginfo_scintilla_set_focus, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_focus, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_status,arginfo_scintilla_set_status, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_status, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_mouse_down_captures,arginfo_scintilla_set_mouse_down_captures, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_mouse_down_captures, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_cursor,arginfo_scintilla_set_cursor, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_cursor, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_control_char_symbol,arginfo_scintilla_set_control_char_symbol, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_control_char_symbol, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, word_part_left, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, word_part_left_extend, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, word_part_right, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, word_part_right_extend, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_visible_policy,arginfo_scintilla_set_visible_policy, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, del_line_left, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, del_line_right, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_xoffset,arginfo_scintilla_set_xoffset, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_xoffset, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, choose_caret_x, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, grab_focus, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_xcaret_policy,arginfo_scintilla_set_xcaret_policy, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_ycaret_policy,arginfo_scintilla_set_ycaret_policy, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_print_wrap_mode,arginfo_scintilla_set_print_wrap_mode, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_print_wrap_mode, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_hotspot_active_fore,arginfo_scintilla_set_hotspot_active_fore, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_hotspot_active_fore, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_hotspot_active_back,arginfo_scintilla_set_hotspot_active_back, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_hotspot_active_back, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_hotspot_active_underline,arginfo_scintilla_set_hotspot_active_underline, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_hotspot_active_underline, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_hotspot_single_line,arginfo_scintilla_set_hotspot_single_line, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_hotspot_single_line, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, para_down, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, para_down_extend, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, para_up, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, para_up_extend, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, position_before,arginfo_scintilla_position_before, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, position_after,arginfo_scintilla_position_after, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, copy_range,arginfo_scintilla_copy_range, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, copy_text,arginfo_scintilla_copy_text, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_selection_mode,arginfo_scintilla_set_selection_mode, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_selection_mode, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_line_sel_start_position,arginfo_scintilla_get_line_sel_start_position, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_line_sel_end_position,arginfo_scintilla_get_line_sel_end_position, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, line_down_rect_extend, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, line_up_rect_extend, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, char_left_rect_extend, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, char_right_rect_extend, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, home_rect_extend, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, vchome_rect_extend, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, line_end_rect_extend, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, page_up_rect_extend, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, page_down_rect_extend, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, stuttered_page_up, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, stuttered_page_up_extend, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, stuttered_page_down, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, stuttered_page_down_extend, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, word_left_end, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, word_left_end_extend, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, word_right_end, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, word_right_end_extend, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_whitespace_chars,arginfo_scintilla_set_whitespace_chars, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_chars_default, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, auto_cget_current, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, allocate,arginfo_scintilla_allocate, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, target_as_utf8, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_length_for_encode,arginfo_scintilla_set_length_for_encode, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, encoded_from_utf8,arginfo_scintilla_encoded_from_utf8, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, find_column,arginfo_scintilla_find_column, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_caret_sticky, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_caret_sticky,arginfo_scintilla_set_caret_sticky, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, toggle_caret_sticky, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_paste_convert_endings,arginfo_scintilla_set_paste_convert_endings, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_paste_convert_endings, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, selection_duplicate, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_caret_line_back_alpha,arginfo_scintilla_set_caret_line_back_alpha, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_caret_line_back_alpha, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, start_record, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, stop_record, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_lexer,arginfo_scintilla_set_lexer, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_lexer, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, colourise,arginfo_scintilla_colourise, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_property,arginfo_scintilla_set_property, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_keywords,arginfo_scintilla_set_keywords, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, set_lexer_language,arginfo_scintilla_set_lexer_language, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, load_lexer_library,arginfo_scintilla_load_lexer_library, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_property,arginfo_scintilla_get_property, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_property_expanded,arginfo_scintilla_get_property_expanded, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_property_int, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(Scintilla, get_style_bits_needed, NULL, ZEND_ACC_PUBLIC)
	{ NULL, NULL, NULL }
};
void phpg_scintilla_register_classes(void)
{
	TSRMLS_FETCH();

	scintilla_ce = phpg_register_class("GtkScintilla", scintilla_methods, gtkcontainer_ce, 0, NULL, NULL, GTK_TYPE_SCINTILLA TSRMLS_CC);
}

void phpg_scintilla_register_constants(const char *strip_prefix)
{
	TSRMLS_FETCH();

	phpg_register_int_constant(scintilla_ce, "gtype", sizeof("gtype")-1, GTK_TYPE_FRAME);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_INVALID_POSITION", sizeof("SCINTILLA_INVALID_POSITION")-1, INVALID_POSITION);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SCI_START", sizeof("SCINTILLA_SCI_START")-1, SCI_START);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SCI_OPTIONAL_START", sizeof("SCINTILLA_SCI_OPTIONAL_START")-1, SCI_OPTIONAL_START);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SCI_LEXER_START", sizeof("SCINTILLA_SCI_LEXER_START")-1, SCI_LEXER_START);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_WS_INVISIBLE", sizeof("SCINTILLA_WS_INVISIBLE")-1, SCWS_INVISIBLE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_WS_VISIBLEALWAYS", sizeof("SCINTILLA_WS_VISIBLEALWAYS")-1, SCWS_VISIBLEALWAYS);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_WS_VISIBLEAFTERINDENT", sizeof("SCINTILLA_WS_VISIBLEAFTERINDENT")-1, SCWS_VISIBLEAFTERINDENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_EOL_CRLF", sizeof("SCINTILLA_EOL_CRLF")-1, SC_EOL_CRLF);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_EOL_CR", sizeof("SCINTILLA_EOL_CR")-1, SC_EOL_CR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_EOL_LF", sizeof("SCINTILLA_EOL_LF")-1, SC_EOL_LF);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CP_UTF8", sizeof("SCINTILLA_CP_UTF8")-1, SC_CP_UTF8);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CP_DBCS", sizeof("SCINTILLA_CP_DBCS")-1, SC_CP_DBCS);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MARKER_MAX", sizeof("SCINTILLA_MARKER_MAX")-1, MARKER_MAX);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MARK_CIRCLE", sizeof("SCINTILLA_MARK_CIRCLE")-1, SC_MARK_CIRCLE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MARK_ROUNDRECT", sizeof("SCINTILLA_MARK_ROUNDRECT")-1, SC_MARK_ROUNDRECT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MARK_ARROW", sizeof("SCINTILLA_MARK_ARROW")-1, SC_MARK_ARROW);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MARK_SMALLRECT", sizeof("SCINTILLA_MARK_SMALLRECT")-1, SC_MARK_SMALLRECT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MARK_SHORTARROW", sizeof("SCINTILLA_MARK_SHORTARROW")-1, SC_MARK_SHORTARROW);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MARK_EMPTY", sizeof("SCINTILLA_MARK_EMPTY")-1, SC_MARK_EMPTY);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MARK_ARROWDOWN", sizeof("SCINTILLA_MARK_ARROWDOWN")-1, SC_MARK_ARROWDOWN);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MARK_MINUS", sizeof("SCINTILLA_MARK_MINUS")-1, SC_MARK_MINUS);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MARK_PLUS", sizeof("SCINTILLA_MARK_PLUS")-1, SC_MARK_PLUS);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MARK_VLINE", sizeof("SCINTILLA_MARK_VLINE")-1, SC_MARK_VLINE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MARK_LCORNER", sizeof("SCINTILLA_MARK_LCORNER")-1, SC_MARK_LCORNER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MARK_TCORNER", sizeof("SCINTILLA_MARK_TCORNER")-1, SC_MARK_TCORNER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MARK_BOXPLUS", sizeof("SCINTILLA_MARK_BOXPLUS")-1, SC_MARK_BOXPLUS);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MARK_BOXPLUSCONNECTED", sizeof("SCINTILLA_MARK_BOXPLUSCONNECTED")-1, SC_MARK_BOXPLUSCONNECTED);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MARK_BOXMINUS", sizeof("SCINTILLA_MARK_BOXMINUS")-1, SC_MARK_BOXMINUS);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MARK_BOXMINUSCONNECTED", sizeof("SCINTILLA_MARK_BOXMINUSCONNECTED")-1, SC_MARK_BOXMINUSCONNECTED);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MARK_LCORNERCURVE", sizeof("SCINTILLA_MARK_LCORNERCURVE")-1, SC_MARK_LCORNERCURVE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MARK_TCORNERCURVE", sizeof("SCINTILLA_MARK_TCORNERCURVE")-1, SC_MARK_TCORNERCURVE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MARK_CIRCLEPLUS", sizeof("SCINTILLA_MARK_CIRCLEPLUS")-1, SC_MARK_CIRCLEPLUS);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MARK_CIRCLEPLUSCONNECTED", sizeof("SCINTILLA_MARK_CIRCLEPLUSCONNECTED")-1, SC_MARK_CIRCLEPLUSCONNECTED);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MARK_CIRCLEMINUS", sizeof("SCINTILLA_MARK_CIRCLEMINUS")-1, SC_MARK_CIRCLEMINUS);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MARK_CIRCLEMINUSCONNECTED", sizeof("SCINTILLA_MARK_CIRCLEMINUSCONNECTED")-1, SC_MARK_CIRCLEMINUSCONNECTED);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MARK_BACKGROUND", sizeof("SCINTILLA_MARK_BACKGROUND")-1, SC_MARK_BACKGROUND);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MARK_DOTDOTDOT", sizeof("SCINTILLA_MARK_DOTDOTDOT")-1, SC_MARK_DOTDOTDOT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MARK_ARROWS", sizeof("SCINTILLA_MARK_ARROWS")-1, SC_MARK_ARROWS);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MARK_PIXMAP", sizeof("SCINTILLA_MARK_PIXMAP")-1, SC_MARK_PIXMAP);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MARK_FULLRECT", sizeof("SCINTILLA_MARK_FULLRECT")-1, SC_MARK_FULLRECT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MARK_CHARACTER", sizeof("SCINTILLA_MARK_CHARACTER")-1, SC_MARK_CHARACTER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MARKNUM_FOLDEREND", sizeof("SCINTILLA_MARKNUM_FOLDEREND")-1, SC_MARKNUM_FOLDEREND);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MARKNUM_FOLDEROPENMID", sizeof("SCINTILLA_MARKNUM_FOLDEROPENMID")-1, SC_MARKNUM_FOLDEROPENMID);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MARKNUM_FOLDERMIDTAIL", sizeof("SCINTILLA_MARKNUM_FOLDERMIDTAIL")-1, SC_MARKNUM_FOLDERMIDTAIL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MARKNUM_FOLDERTAIL", sizeof("SCINTILLA_MARKNUM_FOLDERTAIL")-1, SC_MARKNUM_FOLDERTAIL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MARKNUM_FOLDERSUB", sizeof("SCINTILLA_MARKNUM_FOLDERSUB")-1, SC_MARKNUM_FOLDERSUB);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MARKNUM_FOLDER", sizeof("SCINTILLA_MARKNUM_FOLDER")-1, SC_MARKNUM_FOLDER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MARKNUM_FOLDEROPEN", sizeof("SCINTILLA_MARKNUM_FOLDEROPEN")-1, SC_MARKNUM_FOLDEROPEN);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MASK_FOLDERS", sizeof("SCINTILLA_MASK_FOLDERS")-1, SC_MASK_FOLDERS);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MARGIN_SYMBOL", sizeof("SCINTILLA_MARGIN_SYMBOL")-1, SC_MARGIN_SYMBOL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MARGIN_NUMBER", sizeof("SCINTILLA_MARGIN_NUMBER")-1, SC_MARGIN_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MARGIN_BACK", sizeof("SCINTILLA_MARGIN_BACK")-1, SC_MARGIN_BACK);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MARGIN_FORE", sizeof("SCINTILLA_MARGIN_FORE")-1, SC_MARGIN_FORE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_STYLE_DEFAULT", sizeof("SCINTILLA_STYLE_DEFAULT")-1, STYLE_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_STYLE_LINENUMBER", sizeof("SCINTILLA_STYLE_LINENUMBER")-1, STYLE_LINENUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_STYLE_BRACELIGHT", sizeof("SCINTILLA_STYLE_BRACELIGHT")-1, STYLE_BRACELIGHT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_STYLE_BRACEBAD", sizeof("SCINTILLA_STYLE_BRACEBAD")-1, STYLE_BRACEBAD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_STYLE_CONTROLCHAR", sizeof("SCINTILLA_STYLE_CONTROLCHAR")-1, STYLE_CONTROLCHAR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_STYLE_INDENTGUIDE", sizeof("SCINTILLA_STYLE_INDENTGUIDE")-1, STYLE_INDENTGUIDE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_STYLE_CALLTIP", sizeof("SCINTILLA_STYLE_CALLTIP")-1, STYLE_CALLTIP);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_STYLE_LASTPREDEFINED", sizeof("SCINTILLA_STYLE_LASTPREDEFINED")-1, STYLE_LASTPREDEFINED);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_STYLE_MAX", sizeof("SCINTILLA_STYLE_MAX")-1, STYLE_MAX);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CHARSET_ANSI", sizeof("SCINTILLA_CHARSET_ANSI")-1, SC_CHARSET_ANSI);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CHARSET_DEFAULT", sizeof("SCINTILLA_CHARSET_DEFAULT")-1, SC_CHARSET_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CHARSET_BALTIC", sizeof("SCINTILLA_CHARSET_BALTIC")-1, SC_CHARSET_BALTIC);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CHARSET_CHINESEBIG5", sizeof("SCINTILLA_CHARSET_CHINESEBIG5")-1, SC_CHARSET_CHINESEBIG5);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CHARSET_EASTEUROPE", sizeof("SCINTILLA_CHARSET_EASTEUROPE")-1, SC_CHARSET_EASTEUROPE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CHARSET_GB2312", sizeof("SCINTILLA_CHARSET_GB2312")-1, SC_CHARSET_GB2312);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CHARSET_GREEK", sizeof("SCINTILLA_CHARSET_GREEK")-1, SC_CHARSET_GREEK);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CHARSET_HANGUL", sizeof("SCINTILLA_CHARSET_HANGUL")-1, SC_CHARSET_HANGUL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CHARSET_MAC", sizeof("SCINTILLA_CHARSET_MAC")-1, SC_CHARSET_MAC);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CHARSET_OEM", sizeof("SCINTILLA_CHARSET_OEM")-1, SC_CHARSET_OEM);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CHARSET_RUSSIAN", sizeof("SCINTILLA_CHARSET_RUSSIAN")-1, SC_CHARSET_RUSSIAN);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CHARSET_CYRILLIC", sizeof("SCINTILLA_CHARSET_CYRILLIC")-1, SC_CHARSET_CYRILLIC);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CHARSET_SHIFTJIS", sizeof("SCINTILLA_CHARSET_SHIFTJIS")-1, SC_CHARSET_SHIFTJIS);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CHARSET_SYMBOL", sizeof("SCINTILLA_CHARSET_SYMBOL")-1, SC_CHARSET_SYMBOL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CHARSET_TURKISH", sizeof("SCINTILLA_CHARSET_TURKISH")-1, SC_CHARSET_TURKISH);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CHARSET_JOHAB", sizeof("SCINTILLA_CHARSET_JOHAB")-1, SC_CHARSET_JOHAB);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CHARSET_HEBREW", sizeof("SCINTILLA_CHARSET_HEBREW")-1, SC_CHARSET_HEBREW);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CHARSET_ARABIC", sizeof("SCINTILLA_CHARSET_ARABIC")-1, SC_CHARSET_ARABIC);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CHARSET_VIETNAMESE", sizeof("SCINTILLA_CHARSET_VIETNAMESE")-1, SC_CHARSET_VIETNAMESE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CHARSET_THAI", sizeof("SCINTILLA_CHARSET_THAI")-1, SC_CHARSET_THAI);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CHARSET_8859_15", sizeof("SCINTILLA_CHARSET_8859_15")-1, SC_CHARSET_8859_15);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CASE_MIXED", sizeof("SCINTILLA_CASE_MIXED")-1, SC_CASE_MIXED);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CASE_UPPER", sizeof("SCINTILLA_CASE_UPPER")-1, SC_CASE_UPPER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CASE_LOWER", sizeof("SCINTILLA_CASE_LOWER")-1, SC_CASE_LOWER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_INDIC_MAX", sizeof("SCINTILLA_INDIC_MAX")-1, INDIC_MAX);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_INDIC_PLAIN", sizeof("SCINTILLA_INDIC_PLAIN")-1, INDIC_PLAIN);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_INDIC_SQUIGGLE", sizeof("SCINTILLA_INDIC_SQUIGGLE")-1, INDIC_SQUIGGLE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_INDIC_TT", sizeof("SCINTILLA_INDIC_TT")-1, INDIC_TT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_INDIC_DIAGONAL", sizeof("SCINTILLA_INDIC_DIAGONAL")-1, INDIC_DIAGONAL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_INDIC_STRIKE", sizeof("SCINTILLA_INDIC_STRIKE")-1, INDIC_STRIKE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_INDIC_HIDDEN", sizeof("SCINTILLA_INDIC_HIDDEN")-1, INDIC_HIDDEN);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_INDIC_BOX", sizeof("SCINTILLA_INDIC_BOX")-1, INDIC_BOX);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_INDIC_ROUNDBOX", sizeof("SCINTILLA_INDIC_ROUNDBOX")-1, INDIC_ROUNDBOX);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_INDIC0_MASK", sizeof("SCINTILLA_INDIC0_MASK")-1, INDIC0_MASK);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_INDIC1_MASK", sizeof("SCINTILLA_INDIC1_MASK")-1, INDIC1_MASK);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_INDIC2_MASK", sizeof("SCINTILLA_INDIC2_MASK")-1, INDIC2_MASK);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_INDICS_MASK", sizeof("SCINTILLA_INDICS_MASK")-1, INDICS_MASK);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PRINT_NORMAL", sizeof("SCINTILLA_PRINT_NORMAL")-1, SC_PRINT_NORMAL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PRINT_INVERTLIGHT", sizeof("SCINTILLA_PRINT_INVERTLIGHT")-1, SC_PRINT_INVERTLIGHT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PRINT_BLACKONWHITE", sizeof("SCINTILLA_PRINT_BLACKONWHITE")-1, SC_PRINT_BLACKONWHITE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PRINT_COLOURONWHITE", sizeof("SCINTILLA_PRINT_COLOURONWHITE")-1, SC_PRINT_COLOURONWHITE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PRINT_COLOURONWHITEDEFAULTBG", sizeof("SCINTILLA_PRINT_COLOURONWHITEDEFAULTBG")-1, SC_PRINT_COLOURONWHITEDEFAULTBG);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FIND_WHOLEWORD", sizeof("SCINTILLA_FIND_WHOLEWORD")-1, SCFIND_WHOLEWORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FIND_MATCHCASE", sizeof("SCINTILLA_FIND_MATCHCASE")-1, SCFIND_MATCHCASE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FIND_WORDSTART", sizeof("SCINTILLA_FIND_WORDSTART")-1, SCFIND_WORDSTART);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FIND_REGEXP", sizeof("SCINTILLA_FIND_REGEXP")-1, SCFIND_REGEXP);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FIND_POSIX", sizeof("SCINTILLA_FIND_POSIX")-1, SCFIND_POSIX);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FOLDLEVELBASE", sizeof("SCINTILLA_FOLDLEVELBASE")-1, SC_FOLDLEVELBASE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FOLDLEVELWHITEFLAG", sizeof("SCINTILLA_FOLDLEVELWHITEFLAG")-1, SC_FOLDLEVELWHITEFLAG);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FOLDLEVELHEADERFLAG", sizeof("SCINTILLA_FOLDLEVELHEADERFLAG")-1, SC_FOLDLEVELHEADERFLAG);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FOLDLEVELBOXHEADERFLAG", sizeof("SCINTILLA_FOLDLEVELBOXHEADERFLAG")-1, SC_FOLDLEVELBOXHEADERFLAG);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FOLDLEVELBOXFOOTERFLAG", sizeof("SCINTILLA_FOLDLEVELBOXFOOTERFLAG")-1, SC_FOLDLEVELBOXFOOTERFLAG);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FOLDLEVELCONTRACTED", sizeof("SCINTILLA_FOLDLEVELCONTRACTED")-1, SC_FOLDLEVELCONTRACTED);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FOLDLEVELUNINDENT", sizeof("SCINTILLA_FOLDLEVELUNINDENT")-1, SC_FOLDLEVELUNINDENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FOLDLEVELNUMBERMASK", sizeof("SCINTILLA_FOLDLEVELNUMBERMASK")-1, SC_FOLDLEVELNUMBERMASK);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FOLDFLAG_LINEBEFORE_EXPANDED", sizeof("SCINTILLA_FOLDFLAG_LINEBEFORE_EXPANDED")-1, SC_FOLDFLAG_LINEBEFORE_EXPANDED);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FOLDFLAG_LINEBEFORE_CONTRACTED", sizeof("SCINTILLA_FOLDFLAG_LINEBEFORE_CONTRACTED")-1, SC_FOLDFLAG_LINEBEFORE_CONTRACTED);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FOLDFLAG_LINEAFTER_EXPANDED", sizeof("SCINTILLA_FOLDFLAG_LINEAFTER_EXPANDED")-1, SC_FOLDFLAG_LINEAFTER_EXPANDED);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FOLDFLAG_LINEAFTER_CONTRACTED", sizeof("SCINTILLA_FOLDFLAG_LINEAFTER_CONTRACTED")-1, SC_FOLDFLAG_LINEAFTER_CONTRACTED);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FOLDFLAG_LEVELNUMBERS", sizeof("SCINTILLA_FOLDFLAG_LEVELNUMBERS")-1, SC_FOLDFLAG_LEVELNUMBERS);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FOLDFLAG_BOX", sizeof("SCINTILLA_FOLDFLAG_BOX")-1, SC_FOLDFLAG_BOX);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_TIME_FOREVER", sizeof("SCINTILLA_TIME_FOREVER")-1, SC_TIME_FOREVER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_WRAP_NONE", sizeof("SCINTILLA_WRAP_NONE")-1, SC_WRAP_NONE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_WRAP_WORD", sizeof("SCINTILLA_WRAP_WORD")-1, SC_WRAP_WORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_WRAP_CHAR", sizeof("SCINTILLA_WRAP_CHAR")-1, SC_WRAP_CHAR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_WRAPVISUALFLAG_NONE", sizeof("SCINTILLA_WRAPVISUALFLAG_NONE")-1, SC_WRAPVISUALFLAG_NONE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_WRAPVISUALFLAG_END", sizeof("SCINTILLA_WRAPVISUALFLAG_END")-1, SC_WRAPVISUALFLAG_END);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_WRAPVISUALFLAG_START", sizeof("SCINTILLA_WRAPVISUALFLAG_START")-1, SC_WRAPVISUALFLAG_START);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_WRAPVISUALFLAGLOC_DEFAULT", sizeof("SCINTILLA_WRAPVISUALFLAGLOC_DEFAULT")-1, SC_WRAPVISUALFLAGLOC_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_WRAPVISUALFLAGLOC_END_BY_TEXT", sizeof("SCINTILLA_WRAPVISUALFLAGLOC_END_BY_TEXT")-1, SC_WRAPVISUALFLAGLOC_END_BY_TEXT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_WRAPVISUALFLAGLOC_START_BY_TEXT", sizeof("SCINTILLA_WRAPVISUALFLAGLOC_START_BY_TEXT")-1, SC_WRAPVISUALFLAGLOC_START_BY_TEXT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CACHE_NONE", sizeof("SCINTILLA_CACHE_NONE")-1, SC_CACHE_NONE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CACHE_CARET", sizeof("SCINTILLA_CACHE_CARET")-1, SC_CACHE_CARET);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CACHE_PAGE", sizeof("SCINTILLA_CACHE_PAGE")-1, SC_CACHE_PAGE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CACHE_DOCUMENT", sizeof("SCINTILLA_CACHE_DOCUMENT")-1, SC_CACHE_DOCUMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_EDGE_NONE", sizeof("SCINTILLA_EDGE_NONE")-1, EDGE_NONE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_EDGE_LINE", sizeof("SCINTILLA_EDGE_LINE")-1, EDGE_LINE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_EDGE_BACKGROUND", sizeof("SCINTILLA_EDGE_BACKGROUND")-1, EDGE_BACKGROUND);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CURSORNORMAL", sizeof("SCINTILLA_CURSORNORMAL")-1, SC_CURSORNORMAL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CURSORWAIT", sizeof("SCINTILLA_CURSORWAIT")-1, SC_CURSORWAIT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VISIBLE_SLOP", sizeof("SCINTILLA_VISIBLE_SLOP")-1, VISIBLE_SLOP);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VISIBLE_STRICT", sizeof("SCINTILLA_VISIBLE_STRICT")-1, VISIBLE_STRICT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CARET_SLOP", sizeof("SCINTILLA_CARET_SLOP")-1, CARET_SLOP);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CARET_STRICT", sizeof("SCINTILLA_CARET_STRICT")-1, CARET_STRICT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CARET_JUMPS", sizeof("SCINTILLA_CARET_JUMPS")-1, CARET_JUMPS);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CARET_EVEN", sizeof("SCINTILLA_CARET_EVEN")-1, CARET_EVEN);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SEL_STREAM", sizeof("SCINTILLA_SEL_STREAM")-1, SC_SEL_STREAM);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SEL_RECTANGLE", sizeof("SCINTILLA_SEL_RECTANGLE")-1, SC_SEL_RECTANGLE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SEL_LINES", sizeof("SCINTILLA_SEL_LINES")-1, SC_SEL_LINES);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ALPHA_TRANSPARENT", sizeof("SCINTILLA_ALPHA_TRANSPARENT")-1, SC_ALPHA_TRANSPARENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ALPHA_OPAQUE", sizeof("SCINTILLA_ALPHA_OPAQUE")-1, SC_ALPHA_OPAQUE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ALPHA_NOALPHA", sizeof("SCINTILLA_ALPHA_NOALPHA")-1, SC_ALPHA_NOALPHA);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_KEYWORDSET_MAX", sizeof("SCINTILLA_KEYWORDSET_MAX")-1, KEYWORDSET_MAX);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MOD_INSERTTEXT", sizeof("SCINTILLA_MOD_INSERTTEXT")-1, SC_MOD_INSERTTEXT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MOD_DELETETEXT", sizeof("SCINTILLA_MOD_DELETETEXT")-1, SC_MOD_DELETETEXT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MOD_CHANGESTYLE", sizeof("SCINTILLA_MOD_CHANGESTYLE")-1, SC_MOD_CHANGESTYLE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MOD_CHANGEFOLD", sizeof("SCINTILLA_MOD_CHANGEFOLD")-1, SC_MOD_CHANGEFOLD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PERFORMED_USER", sizeof("SCINTILLA_PERFORMED_USER")-1, SC_PERFORMED_USER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PERFORMED_UNDO", sizeof("SCINTILLA_PERFORMED_UNDO")-1, SC_PERFORMED_UNDO);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PERFORMED_REDO", sizeof("SCINTILLA_PERFORMED_REDO")-1, SC_PERFORMED_REDO);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MULTISTEPUNDOREDO", sizeof("SCINTILLA_MULTISTEPUNDOREDO")-1, SC_MULTISTEPUNDOREDO);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LASTSTEPINUNDOREDO", sizeof("SCINTILLA_LASTSTEPINUNDOREDO")-1, SC_LASTSTEPINUNDOREDO);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MOD_CHANGEMARKER", sizeof("SCINTILLA_MOD_CHANGEMARKER")-1, SC_MOD_CHANGEMARKER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MOD_BEFOREINSERT", sizeof("SCINTILLA_MOD_BEFOREINSERT")-1, SC_MOD_BEFOREINSERT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MOD_BEFOREDELETE", sizeof("SCINTILLA_MOD_BEFOREDELETE")-1, SC_MOD_BEFOREDELETE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MULTILINEUNDOREDO", sizeof("SCINTILLA_MULTILINEUNDOREDO")-1, SC_MULTILINEUNDOREDO);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_STARTACTION", sizeof("SCINTILLA_STARTACTION")-1, SC_STARTACTION);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MODEVENTMASKALL", sizeof("SCINTILLA_MODEVENTMASKALL")-1, SC_MODEVENTMASKALL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_EN_CHANGE", sizeof("SCINTILLA_EN_CHANGE")-1, SCEN_CHANGE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_EN_SETFOCUS", sizeof("SCINTILLA_EN_SETFOCUS")-1, SCEN_SETFOCUS);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_EN_KILLFOCUS", sizeof("SCINTILLA_EN_KILLFOCUS")-1, SCEN_KILLFOCUS);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_KEY_DOWN", sizeof("SCINTILLA_KEY_DOWN")-1, SCK_DOWN);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_KEY_UP", sizeof("SCINTILLA_KEY_UP")-1, SCK_UP);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_KEY_LEFT", sizeof("SCINTILLA_KEY_LEFT")-1, SCK_LEFT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_KEY_RIGHT", sizeof("SCINTILLA_KEY_RIGHT")-1, SCK_RIGHT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_KEY_HOME", sizeof("SCINTILLA_KEY_HOME")-1, SCK_HOME);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_KEY_END", sizeof("SCINTILLA_KEY_END")-1, SCK_END);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_KEY_PRIOR", sizeof("SCINTILLA_KEY_PRIOR")-1, SCK_PRIOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_KEY_NEXT", sizeof("SCINTILLA_KEY_NEXT")-1, SCK_NEXT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_KEY_DELETE", sizeof("SCINTILLA_KEY_DELETE")-1, SCK_DELETE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_KEY_INSERT", sizeof("SCINTILLA_KEY_INSERT")-1, SCK_INSERT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_KEY_ESCAPE", sizeof("SCINTILLA_KEY_ESCAPE")-1, SCK_ESCAPE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_KEY_BACK", sizeof("SCINTILLA_KEY_BACK")-1, SCK_BACK);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_KEY_TAB", sizeof("SCINTILLA_KEY_TAB")-1, SCK_TAB);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_KEY_RETURN", sizeof("SCINTILLA_KEY_RETURN")-1, SCK_RETURN);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_KEY_ADD", sizeof("SCINTILLA_KEY_ADD")-1, SCK_ADD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_KEY_SUBTRACT", sizeof("SCINTILLA_KEY_SUBTRACT")-1, SCK_SUBTRACT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_KEY_DIVIDE", sizeof("SCINTILLA_KEY_DIVIDE")-1, SCK_DIVIDE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_KEY_WIN", sizeof("SCINTILLA_KEY_WIN")-1, SCK_WIN);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_KEY_RWIN", sizeof("SCINTILLA_KEY_RWIN")-1, SCK_RWIN);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_KEY_MENU", sizeof("SCINTILLA_KEY_MENU")-1, SCK_MENU);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MOD_NORM", sizeof("SCINTILLA_MOD_NORM")-1, SCMOD_NORM);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MOD_SHIFT", sizeof("SCINTILLA_MOD_SHIFT")-1, SCMOD_SHIFT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MOD_CTRL", sizeof("SCINTILLA_MOD_CTRL")-1, SCMOD_CTRL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MOD_ALT", sizeof("SCINTILLA_MOD_ALT")-1, SCMOD_ALT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_CONTAINER", sizeof("SCINTILLA_LEX_CONTAINER")-1, SCLEX_CONTAINER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_NULL", sizeof("SCINTILLA_LEX_NULL")-1, SCLEX_NULL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_PYTHON", sizeof("SCINTILLA_LEX_PYTHON")-1, SCLEX_PYTHON);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_CPP", sizeof("SCINTILLA_LEX_CPP")-1, SCLEX_CPP);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_HTML", sizeof("SCINTILLA_LEX_HTML")-1, SCLEX_HTML);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_XML", sizeof("SCINTILLA_LEX_XML")-1, SCLEX_XML);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_PERL", sizeof("SCINTILLA_LEX_PERL")-1, SCLEX_PERL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_SQL", sizeof("SCINTILLA_LEX_SQL")-1, SCLEX_SQL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_VB", sizeof("SCINTILLA_LEX_VB")-1, SCLEX_VB);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_PROPERTIES", sizeof("SCINTILLA_LEX_PROPERTIES")-1, SCLEX_PROPERTIES);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_ERRORLIST", sizeof("SCINTILLA_LEX_ERRORLIST")-1, SCLEX_ERRORLIST);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_MAKEFILE", sizeof("SCINTILLA_LEX_MAKEFILE")-1, SCLEX_MAKEFILE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_BATCH", sizeof("SCINTILLA_LEX_BATCH")-1, SCLEX_BATCH);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_XCODE", sizeof("SCINTILLA_LEX_XCODE")-1, SCLEX_XCODE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_LATEX", sizeof("SCINTILLA_LEX_LATEX")-1, SCLEX_LATEX);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_LUA", sizeof("SCINTILLA_LEX_LUA")-1, SCLEX_LUA);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_DIFF", sizeof("SCINTILLA_LEX_DIFF")-1, SCLEX_DIFF);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_CONF", sizeof("SCINTILLA_LEX_CONF")-1, SCLEX_CONF);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_PASCAL", sizeof("SCINTILLA_LEX_PASCAL")-1, SCLEX_PASCAL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_AVE", sizeof("SCINTILLA_LEX_AVE")-1, SCLEX_AVE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_ADA", sizeof("SCINTILLA_LEX_ADA")-1, SCLEX_ADA);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_LISP", sizeof("SCINTILLA_LEX_LISP")-1, SCLEX_LISP);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_RUBY", sizeof("SCINTILLA_LEX_RUBY")-1, SCLEX_RUBY);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_EIFFEL", sizeof("SCINTILLA_LEX_EIFFEL")-1, SCLEX_EIFFEL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_EIFFELKW", sizeof("SCINTILLA_LEX_EIFFELKW")-1, SCLEX_EIFFELKW);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_TCL", sizeof("SCINTILLA_LEX_TCL")-1, SCLEX_TCL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_NNCRONTAB", sizeof("SCINTILLA_LEX_NNCRONTAB")-1, SCLEX_NNCRONTAB);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_BULLANT", sizeof("SCINTILLA_LEX_BULLANT")-1, SCLEX_BULLANT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_VBSCRIPT", sizeof("SCINTILLA_LEX_VBSCRIPT")-1, SCLEX_VBSCRIPT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_BAAN", sizeof("SCINTILLA_LEX_BAAN")-1, SCLEX_BAAN);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_MATLAB", sizeof("SCINTILLA_LEX_MATLAB")-1, SCLEX_MATLAB);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_SCRIPTOL", sizeof("SCINTILLA_LEX_SCRIPTOL")-1, SCLEX_SCRIPTOL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_ASM", sizeof("SCINTILLA_LEX_ASM")-1, SCLEX_ASM);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_CPPNOCASE", sizeof("SCINTILLA_LEX_CPPNOCASE")-1, SCLEX_CPPNOCASE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_FORTRAN", sizeof("SCINTILLA_LEX_FORTRAN")-1, SCLEX_FORTRAN);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_F77", sizeof("SCINTILLA_LEX_F77")-1, SCLEX_F77);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_CSS", sizeof("SCINTILLA_LEX_CSS")-1, SCLEX_CSS);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_POV", sizeof("SCINTILLA_LEX_POV")-1, SCLEX_POV);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_LOUT", sizeof("SCINTILLA_LEX_LOUT")-1, SCLEX_LOUT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_ESCRIPT", sizeof("SCINTILLA_LEX_ESCRIPT")-1, SCLEX_ESCRIPT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_PS", sizeof("SCINTILLA_LEX_PS")-1, SCLEX_PS);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_NSIS", sizeof("SCINTILLA_LEX_NSIS")-1, SCLEX_NSIS);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_MMIXAL", sizeof("SCINTILLA_LEX_MMIXAL")-1, SCLEX_MMIXAL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_CLW", sizeof("SCINTILLA_LEX_CLW")-1, SCLEX_CLW);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_CLWNOCASE", sizeof("SCINTILLA_LEX_CLWNOCASE")-1, SCLEX_CLWNOCASE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_LOT", sizeof("SCINTILLA_LEX_LOT")-1, SCLEX_LOT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_YAML", sizeof("SCINTILLA_LEX_YAML")-1, SCLEX_YAML);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_TEX", sizeof("SCINTILLA_LEX_TEX")-1, SCLEX_TEX);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_METAPOST", sizeof("SCINTILLA_LEX_METAPOST")-1, SCLEX_METAPOST);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_POWERBASIC", sizeof("SCINTILLA_LEX_POWERBASIC")-1, SCLEX_POWERBASIC);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_FORTH", sizeof("SCINTILLA_LEX_FORTH")-1, SCLEX_FORTH);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_ERLANG", sizeof("SCINTILLA_LEX_ERLANG")-1, SCLEX_ERLANG);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_OCTAVE", sizeof("SCINTILLA_LEX_OCTAVE")-1, SCLEX_OCTAVE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_MSSQL", sizeof("SCINTILLA_LEX_MSSQL")-1, SCLEX_MSSQL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_VERILOG", sizeof("SCINTILLA_LEX_VERILOG")-1, SCLEX_VERILOG);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_KIX", sizeof("SCINTILLA_LEX_KIX")-1, SCLEX_KIX);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_GUI4CLI", sizeof("SCINTILLA_LEX_GUI4CLI")-1, SCLEX_GUI4CLI);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_SPECMAN", sizeof("SCINTILLA_LEX_SPECMAN")-1, SCLEX_SPECMAN);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_AU3", sizeof("SCINTILLA_LEX_AU3")-1, SCLEX_AU3);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_APDL", sizeof("SCINTILLA_LEX_APDL")-1, SCLEX_APDL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_BASH", sizeof("SCINTILLA_LEX_BASH")-1, SCLEX_BASH);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_ASN1", sizeof("SCINTILLA_LEX_ASN1")-1, SCLEX_ASN1);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_VHDL", sizeof("SCINTILLA_LEX_VHDL")-1, SCLEX_VHDL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_CAML", sizeof("SCINTILLA_LEX_CAML")-1, SCLEX_CAML);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_BLITZBASIC", sizeof("SCINTILLA_LEX_BLITZBASIC")-1, SCLEX_BLITZBASIC);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_PUREBASIC", sizeof("SCINTILLA_LEX_PUREBASIC")-1, SCLEX_PUREBASIC);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_HASKELL", sizeof("SCINTILLA_LEX_HASKELL")-1, SCLEX_HASKELL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_PHPSCRIPT", sizeof("SCINTILLA_LEX_PHPSCRIPT")-1, SCLEX_PHPSCRIPT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_TADS3", sizeof("SCINTILLA_LEX_TADS3")-1, SCLEX_TADS3);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_REBOL", sizeof("SCINTILLA_LEX_REBOL")-1, SCLEX_REBOL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_SMALLTALK", sizeof("SCINTILLA_LEX_SMALLTALK")-1, SCLEX_SMALLTALK);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_FLAGSHIP", sizeof("SCINTILLA_LEX_FLAGSHIP")-1, SCLEX_FLAGSHIP);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_CSOUND", sizeof("SCINTILLA_LEX_CSOUND")-1, SCLEX_CSOUND);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_FREEBASIC", sizeof("SCINTILLA_LEX_FREEBASIC")-1, SCLEX_FREEBASIC);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_INNOSETUP", sizeof("SCINTILLA_LEX_INNOSETUP")-1, SCLEX_INNOSETUP);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_OPAL", sizeof("SCINTILLA_LEX_OPAL")-1, SCLEX_OPAL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_SPICE", sizeof("SCINTILLA_LEX_SPICE")-1, SCLEX_SPICE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_D", sizeof("SCINTILLA_LEX_D")-1, SCLEX_D);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_CMAKE", sizeof("SCINTILLA_LEX_CMAKE")-1, SCLEX_CMAKE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LEX_AUTOMATIC", sizeof("SCINTILLA_LEX_AUTOMATIC")-1, SCLEX_AUTOMATIC);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PY_DEFAULT", sizeof("SCINTILLA_PY_DEFAULT")-1, SCE_P_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PY_COMMENTLINE", sizeof("SCINTILLA_PY_COMMENTLINE")-1, SCE_P_COMMENTLINE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PY_NUMBER", sizeof("SCINTILLA_PY_NUMBER")-1, SCE_P_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PY_STRING", sizeof("SCINTILLA_PY_STRING")-1, SCE_P_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PY_CHARACTER", sizeof("SCINTILLA_PY_CHARACTER")-1, SCE_P_CHARACTER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PY_WORD", sizeof("SCINTILLA_PY_WORD")-1, SCE_P_WORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PY_TRIPLE", sizeof("SCINTILLA_PY_TRIPLE")-1, SCE_P_TRIPLE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PY_TRIPLEDOUBLE", sizeof("SCINTILLA_PY_TRIPLEDOUBLE")-1, SCE_P_TRIPLEDOUBLE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PY_CLASSNAME", sizeof("SCINTILLA_PY_CLASSNAME")-1, SCE_P_CLASSNAME);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PY_DEFNAME", sizeof("SCINTILLA_PY_DEFNAME")-1, SCE_P_DEFNAME);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PY_OPERATOR", sizeof("SCINTILLA_PY_OPERATOR")-1, SCE_P_OPERATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PY_IDENTIFIER", sizeof("SCINTILLA_PY_IDENTIFIER")-1, SCE_P_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PY_COMMENTBLOCK", sizeof("SCINTILLA_PY_COMMENTBLOCK")-1, SCE_P_COMMENTBLOCK);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PY_STRINGEOL", sizeof("SCINTILLA_PY_STRINGEOL")-1, SCE_P_STRINGEOL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PY_WORD2", sizeof("SCINTILLA_PY_WORD2")-1, SCE_P_WORD2);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PY_DECORATOR", sizeof("SCINTILLA_PY_DECORATOR")-1, SCE_P_DECORATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_C_DEFAULT", sizeof("SCINTILLA_C_DEFAULT")-1, SCE_C_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_C_COMMENT", sizeof("SCINTILLA_C_COMMENT")-1, SCE_C_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_C_COMMENTLINE", sizeof("SCINTILLA_C_COMMENTLINE")-1, SCE_C_COMMENTLINE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_C_COMMENTDOC", sizeof("SCINTILLA_C_COMMENTDOC")-1, SCE_C_COMMENTDOC);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_C_NUMBER", sizeof("SCINTILLA_C_NUMBER")-1, SCE_C_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_C_WORD", sizeof("SCINTILLA_C_WORD")-1, SCE_C_WORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_C_STRING", sizeof("SCINTILLA_C_STRING")-1, SCE_C_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_C_CHARACTER", sizeof("SCINTILLA_C_CHARACTER")-1, SCE_C_CHARACTER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_C_UUID", sizeof("SCINTILLA_C_UUID")-1, SCE_C_UUID);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_C_PREPROCESSOR", sizeof("SCINTILLA_C_PREPROCESSOR")-1, SCE_C_PREPROCESSOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_C_OPERATOR", sizeof("SCINTILLA_C_OPERATOR")-1, SCE_C_OPERATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_C_IDENTIFIER", sizeof("SCINTILLA_C_IDENTIFIER")-1, SCE_C_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_C_STRINGEOL", sizeof("SCINTILLA_C_STRINGEOL")-1, SCE_C_STRINGEOL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_C_VERBATIM", sizeof("SCINTILLA_C_VERBATIM")-1, SCE_C_VERBATIM);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_C_REGEX", sizeof("SCINTILLA_C_REGEX")-1, SCE_C_REGEX);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_C_COMMENTLINEDOC", sizeof("SCINTILLA_C_COMMENTLINEDOC")-1, SCE_C_COMMENTLINEDOC);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_C_WORD2", sizeof("SCINTILLA_C_WORD2")-1, SCE_C_WORD2);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_C_COMMENTDOCKEYWORD", sizeof("SCINTILLA_C_COMMENTDOCKEYWORD")-1, SCE_C_COMMENTDOCKEYWORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_C_COMMENTDOCKEYWORDERROR", sizeof("SCINTILLA_C_COMMENTDOCKEYWORDERROR")-1, SCE_C_COMMENTDOCKEYWORDERROR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_C_GLOBALCLASS", sizeof("SCINTILLA_C_GLOBALCLASS")-1, SCE_C_GLOBALCLASS);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_D_DEFAULT", sizeof("SCINTILLA_D_DEFAULT")-1, SCE_D_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_D_COMMENT", sizeof("SCINTILLA_D_COMMENT")-1, SCE_D_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_D_COMMENTLINE", sizeof("SCINTILLA_D_COMMENTLINE")-1, SCE_D_COMMENTLINE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_D_COMMENTDOC", sizeof("SCINTILLA_D_COMMENTDOC")-1, SCE_D_COMMENTDOC);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_D_COMMENTNESTED", sizeof("SCINTILLA_D_COMMENTNESTED")-1, SCE_D_COMMENTNESTED);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_D_NUMBER", sizeof("SCINTILLA_D_NUMBER")-1, SCE_D_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_D_WORD", sizeof("SCINTILLA_D_WORD")-1, SCE_D_WORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_D_WORD2", sizeof("SCINTILLA_D_WORD2")-1, SCE_D_WORD2);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_D_WORD3", sizeof("SCINTILLA_D_WORD3")-1, SCE_D_WORD3);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_D_TYPEDEF", sizeof("SCINTILLA_D_TYPEDEF")-1, SCE_D_TYPEDEF);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_D_STRING", sizeof("SCINTILLA_D_STRING")-1, SCE_D_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_D_STRINGEOL", sizeof("SCINTILLA_D_STRINGEOL")-1, SCE_D_STRINGEOL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_D_CHARACTER", sizeof("SCINTILLA_D_CHARACTER")-1, SCE_D_CHARACTER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_D_OPERATOR", sizeof("SCINTILLA_D_OPERATOR")-1, SCE_D_OPERATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_D_IDENTIFIER", sizeof("SCINTILLA_D_IDENTIFIER")-1, SCE_D_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_D_COMMENTLINEDOC", sizeof("SCINTILLA_D_COMMENTLINEDOC")-1, SCE_D_COMMENTLINEDOC);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_D_COMMENTDOCKEYWORD", sizeof("SCINTILLA_D_COMMENTDOCKEYWORD")-1, SCE_D_COMMENTDOCKEYWORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_D_COMMENTDOCKEYWORDERROR", sizeof("SCINTILLA_D_COMMENTDOCKEYWORDERROR")-1, SCE_D_COMMENTDOCKEYWORDERROR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_TCL_DEFAULT", sizeof("SCINTILLA_TCL_DEFAULT")-1, SCE_TCL_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_TCL_COMMENT", sizeof("SCINTILLA_TCL_COMMENT")-1, SCE_TCL_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_TCL_COMMENTLINE", sizeof("SCINTILLA_TCL_COMMENTLINE")-1, SCE_TCL_COMMENTLINE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_TCL_NUMBER", sizeof("SCINTILLA_TCL_NUMBER")-1, SCE_TCL_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_TCL_WORD_IN_QUOTE", sizeof("SCINTILLA_TCL_WORD_IN_QUOTE")-1, SCE_TCL_WORD_IN_QUOTE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_TCL_IN_QUOTE", sizeof("SCINTILLA_TCL_IN_QUOTE")-1, SCE_TCL_IN_QUOTE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_TCL_OPERATOR", sizeof("SCINTILLA_TCL_OPERATOR")-1, SCE_TCL_OPERATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_TCL_IDENTIFIER", sizeof("SCINTILLA_TCL_IDENTIFIER")-1, SCE_TCL_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_TCL_SUBSTITUTION", sizeof("SCINTILLA_TCL_SUBSTITUTION")-1, SCE_TCL_SUBSTITUTION);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_TCL_SUB_BRACE", sizeof("SCINTILLA_TCL_SUB_BRACE")-1, SCE_TCL_SUB_BRACE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_TCL_MODIFIER", sizeof("SCINTILLA_TCL_MODIFIER")-1, SCE_TCL_MODIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_TCL_EXPAND", sizeof("SCINTILLA_TCL_EXPAND")-1, SCE_TCL_EXPAND);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_TCL_WORD", sizeof("SCINTILLA_TCL_WORD")-1, SCE_TCL_WORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_TCL_WORD2", sizeof("SCINTILLA_TCL_WORD2")-1, SCE_TCL_WORD2);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_TCL_WORD3", sizeof("SCINTILLA_TCL_WORD3")-1, SCE_TCL_WORD3);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_TCL_WORD4", sizeof("SCINTILLA_TCL_WORD4")-1, SCE_TCL_WORD4);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_TCL_WORD5", sizeof("SCINTILLA_TCL_WORD5")-1, SCE_TCL_WORD5);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_TCL_WORD6", sizeof("SCINTILLA_TCL_WORD6")-1, SCE_TCL_WORD6);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_TCL_WORD7", sizeof("SCINTILLA_TCL_WORD7")-1, SCE_TCL_WORD7);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_TCL_WORD8", sizeof("SCINTILLA_TCL_WORD8")-1, SCE_TCL_WORD8);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_TCL_COMMENT_BOX", sizeof("SCINTILLA_TCL_COMMENT_BOX")-1, SCE_TCL_COMMENT_BOX);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_TCL_BLOCK_COMMENT", sizeof("SCINTILLA_TCL_BLOCK_COMMENT")-1, SCE_TCL_BLOCK_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_DEFAULT", sizeof("SCINTILLA_HTML_DEFAULT")-1, SCE_H_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_TAG", sizeof("SCINTILLA_HTML_TAG")-1, SCE_H_TAG);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_TAGUNKNOWN", sizeof("SCINTILLA_HTML_TAGUNKNOWN")-1, SCE_H_TAGUNKNOWN);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_ATTRIBUTE", sizeof("SCINTILLA_HTML_ATTRIBUTE")-1, SCE_H_ATTRIBUTE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_ATTRIBUTEUNKNOWN", sizeof("SCINTILLA_HTML_ATTRIBUTEUNKNOWN")-1, SCE_H_ATTRIBUTEUNKNOWN);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_NUMBER", sizeof("SCINTILLA_HTML_NUMBER")-1, SCE_H_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_DOUBLESTRING", sizeof("SCINTILLA_HTML_DOUBLESTRING")-1, SCE_H_DOUBLESTRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_SINGLESTRING", sizeof("SCINTILLA_HTML_SINGLESTRING")-1, SCE_H_SINGLESTRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_OTHER", sizeof("SCINTILLA_HTML_OTHER")-1, SCE_H_OTHER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_COMMENT", sizeof("SCINTILLA_HTML_COMMENT")-1, SCE_H_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_ENTITY", sizeof("SCINTILLA_HTML_ENTITY")-1, SCE_H_ENTITY);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_TAGEND", sizeof("SCINTILLA_HTML_TAGEND")-1, SCE_H_TAGEND);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_XMLSTART", sizeof("SCINTILLA_HTML_XMLSTART")-1, SCE_H_XMLSTART);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_XMLEND", sizeof("SCINTILLA_HTML_XMLEND")-1, SCE_H_XMLEND);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_SCRIPT", sizeof("SCINTILLA_HTML_SCRIPT")-1, SCE_H_SCRIPT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_ASP", sizeof("SCINTILLA_HTML_ASP")-1, SCE_H_ASP);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_ASPAT", sizeof("SCINTILLA_HTML_ASPAT")-1, SCE_H_ASPAT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_CDATA", sizeof("SCINTILLA_HTML_CDATA")-1, SCE_H_CDATA);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_QUESTION", sizeof("SCINTILLA_HTML_QUESTION")-1, SCE_H_QUESTION);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_VALUE", sizeof("SCINTILLA_HTML_VALUE")-1, SCE_H_VALUE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_XCCOMMENT", sizeof("SCINTILLA_HTML_XCCOMMENT")-1, SCE_H_XCCOMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_SGML_DEFAULT", sizeof("SCINTILLA_HTML_SGML_DEFAULT")-1, SCE_H_SGML_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_SGML_COMMAND", sizeof("SCINTILLA_HTML_SGML_COMMAND")-1, SCE_H_SGML_COMMAND);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_SGML_1ST_PARAM", sizeof("SCINTILLA_HTML_SGML_1ST_PARAM")-1, SCE_H_SGML_1ST_PARAM);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_SGML_DOUBLESTRING", sizeof("SCINTILLA_HTML_SGML_DOUBLESTRING")-1, SCE_H_SGML_DOUBLESTRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_SGML_SIMPLESTRING", sizeof("SCINTILLA_HTML_SGML_SIMPLESTRING")-1, SCE_H_SGML_SIMPLESTRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_SGML_ERROR", sizeof("SCINTILLA_HTML_SGML_ERROR")-1, SCE_H_SGML_ERROR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_SGML_SPECIAL", sizeof("SCINTILLA_HTML_SGML_SPECIAL")-1, SCE_H_SGML_SPECIAL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_SGML_ENTITY", sizeof("SCINTILLA_HTML_SGML_ENTITY")-1, SCE_H_SGML_ENTITY);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_SGML_COMMENT", sizeof("SCINTILLA_HTML_SGML_COMMENT")-1, SCE_H_SGML_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_SGML_1ST_PARAM_COMMENT", sizeof("SCINTILLA_HTML_SGML_1ST_PARAM_COMMENT")-1, SCE_H_SGML_1ST_PARAM_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_SGML_BLOCK_DEFAULT", sizeof("SCINTILLA_HTML_SGML_BLOCK_DEFAULT")-1, SCE_H_SGML_BLOCK_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_JS_START", sizeof("SCINTILLA_HTML_JS_START")-1, SCE_HJ_START);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_JS_DEFAULT", sizeof("SCINTILLA_HTML_JS_DEFAULT")-1, SCE_HJ_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_JS_COMMENT", sizeof("SCINTILLA_HTML_JS_COMMENT")-1, SCE_HJ_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_JS_COMMENTLINE", sizeof("SCINTILLA_HTML_JS_COMMENTLINE")-1, SCE_HJ_COMMENTLINE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_JS_COMMENTDOC", sizeof("SCINTILLA_HTML_JS_COMMENTDOC")-1, SCE_HJ_COMMENTDOC);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_JS_NUMBER", sizeof("SCINTILLA_HTML_JS_NUMBER")-1, SCE_HJ_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_JS_WORD", sizeof("SCINTILLA_HTML_JS_WORD")-1, SCE_HJ_WORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_JS_KEYWORD", sizeof("SCINTILLA_HTML_JS_KEYWORD")-1, SCE_HJ_KEYWORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_JS_DOUBLESTRING", sizeof("SCINTILLA_HTML_JS_DOUBLESTRING")-1, SCE_HJ_DOUBLESTRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_JS_SINGLESTRING", sizeof("SCINTILLA_HTML_JS_SINGLESTRING")-1, SCE_HJ_SINGLESTRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_JS_SYMBOLS", sizeof("SCINTILLA_HTML_JS_SYMBOLS")-1, SCE_HJ_SYMBOLS);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_JS_STRINGEOL", sizeof("SCINTILLA_HTML_JS_STRINGEOL")-1, SCE_HJ_STRINGEOL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_JS_REGEX", sizeof("SCINTILLA_HTML_JS_REGEX")-1, SCE_HJ_REGEX);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_JS_ASP_START", sizeof("SCINTILLA_HTML_JS_ASP_START")-1, SCE_HJA_START);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_JS_ASP_DEFAULT", sizeof("SCINTILLA_HTML_JS_ASP_DEFAULT")-1, SCE_HJA_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_JS_ASP_COMMENT", sizeof("SCINTILLA_HTML_JS_ASP_COMMENT")-1, SCE_HJA_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_JS_ASP_COMMENTLINE", sizeof("SCINTILLA_HTML_JS_ASP_COMMENTLINE")-1, SCE_HJA_COMMENTLINE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_JS_ASP_COMMENTDOC", sizeof("SCINTILLA_HTML_JS_ASP_COMMENTDOC")-1, SCE_HJA_COMMENTDOC);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_JS_ASP_NUMBER", sizeof("SCINTILLA_HTML_JS_ASP_NUMBER")-1, SCE_HJA_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_JS_ASP_WORD", sizeof("SCINTILLA_HTML_JS_ASP_WORD")-1, SCE_HJA_WORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_JS_ASP_KEYWORD", sizeof("SCINTILLA_HTML_JS_ASP_KEYWORD")-1, SCE_HJA_KEYWORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_JS_ASP_DOUBLESTRING", sizeof("SCINTILLA_HTML_JS_ASP_DOUBLESTRING")-1, SCE_HJA_DOUBLESTRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_JS_ASP_SINGLESTRING", sizeof("SCINTILLA_HTML_JS_ASP_SINGLESTRING")-1, SCE_HJA_SINGLESTRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_JS_ASP_SYMBOLS", sizeof("SCINTILLA_HTML_JS_ASP_SYMBOLS")-1, SCE_HJA_SYMBOLS);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_JS_ASP_STRINGEOL", sizeof("SCINTILLA_HTML_JS_ASP_STRINGEOL")-1, SCE_HJA_STRINGEOL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_JS_ASP_REGEX", sizeof("SCINTILLA_HTML_JS_ASP_REGEX")-1, SCE_HJA_REGEX);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_VB_START", sizeof("SCINTILLA_HTML_VB_START")-1, SCE_HB_START);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_VB_DEFAULT", sizeof("SCINTILLA_HTML_VB_DEFAULT")-1, SCE_HB_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_VB_COMMENTLINE", sizeof("SCINTILLA_HTML_VB_COMMENTLINE")-1, SCE_HB_COMMENTLINE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_VB_NUMBER", sizeof("SCINTILLA_HTML_VB_NUMBER")-1, SCE_HB_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_VB_WORD", sizeof("SCINTILLA_HTML_VB_WORD")-1, SCE_HB_WORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_VB_STRING", sizeof("SCINTILLA_HTML_VB_STRING")-1, SCE_HB_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_VB_IDENTIFIER", sizeof("SCINTILLA_HTML_VB_IDENTIFIER")-1, SCE_HB_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_VB_STRINGEOL", sizeof("SCINTILLA_HTML_VB_STRINGEOL")-1, SCE_HB_STRINGEOL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_VB_ASP_START", sizeof("SCINTILLA_HTML_VB_ASP_START")-1, SCE_HBA_START);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_VB_ASP_DEFAULT", sizeof("SCINTILLA_HTML_VB_ASP_DEFAULT")-1, SCE_HBA_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_VB_ASP_COMMENTLINE", sizeof("SCINTILLA_HTML_VB_ASP_COMMENTLINE")-1, SCE_HBA_COMMENTLINE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_VB_ASP_NUMBER", sizeof("SCINTILLA_HTML_VB_ASP_NUMBER")-1, SCE_HBA_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_VB_ASP_WORD", sizeof("SCINTILLA_HTML_VB_ASP_WORD")-1, SCE_HBA_WORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_VB_ASP_STRING", sizeof("SCINTILLA_HTML_VB_ASP_STRING")-1, SCE_HBA_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_VB_ASP_IDENTIFIER", sizeof("SCINTILLA_HTML_VB_ASP_IDENTIFIER")-1, SCE_HBA_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_VB_ASP_STRINGEOL", sizeof("SCINTILLA_HTML_VB_ASP_STRINGEOL")-1, SCE_HBA_STRINGEOL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_PY_START", sizeof("SCINTILLA_HTML_PY_START")-1, SCE_HP_START);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_PY_DEFAULT", sizeof("SCINTILLA_HTML_PY_DEFAULT")-1, SCE_HP_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_PY_COMMENTLINE", sizeof("SCINTILLA_HTML_PY_COMMENTLINE")-1, SCE_HP_COMMENTLINE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_PY_NUMBER", sizeof("SCINTILLA_HTML_PY_NUMBER")-1, SCE_HP_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_PY_STRING", sizeof("SCINTILLA_HTML_PY_STRING")-1, SCE_HP_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_PY_CHARACTER", sizeof("SCINTILLA_HTML_PY_CHARACTER")-1, SCE_HP_CHARACTER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_PY_WORD", sizeof("SCINTILLA_HTML_PY_WORD")-1, SCE_HP_WORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_PY_TRIPLE", sizeof("SCINTILLA_HTML_PY_TRIPLE")-1, SCE_HP_TRIPLE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_PY_TRIPLEDOUBLE", sizeof("SCINTILLA_HTML_PY_TRIPLEDOUBLE")-1, SCE_HP_TRIPLEDOUBLE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_PY_CLASSNAME", sizeof("SCINTILLA_HTML_PY_CLASSNAME")-1, SCE_HP_CLASSNAME);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_PY_DEFNAME", sizeof("SCINTILLA_HTML_PY_DEFNAME")-1, SCE_HP_DEFNAME);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_PY_OPERATOR", sizeof("SCINTILLA_HTML_PY_OPERATOR")-1, SCE_HP_OPERATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_PY_IDENTIFIER", sizeof("SCINTILLA_HTML_PY_IDENTIFIER")-1, SCE_HP_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_PHP_COMPLEX_VARIABLE", sizeof("SCINTILLA_HTML_PHP_COMPLEX_VARIABLE")-1, SCE_HPHP_COMPLEX_VARIABLE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_PY_ASP_START", sizeof("SCINTILLA_HTML_PY_ASP_START")-1, SCE_HPA_START);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_PY_ASP_DEFAULT", sizeof("SCINTILLA_HTML_PY_ASP_DEFAULT")-1, SCE_HPA_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_PY_ASP_COMMENTLINE", sizeof("SCINTILLA_HTML_PY_ASP_COMMENTLINE")-1, SCE_HPA_COMMENTLINE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_PY_ASP_NUMBER", sizeof("SCINTILLA_HTML_PY_ASP_NUMBER")-1, SCE_HPA_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_PY_ASP_STRING", sizeof("SCINTILLA_HTML_PY_ASP_STRING")-1, SCE_HPA_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_PY_ASP_CHARACTER", sizeof("SCINTILLA_HTML_PY_ASP_CHARACTER")-1, SCE_HPA_CHARACTER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_PY_ASP_WORD", sizeof("SCINTILLA_HTML_PY_ASP_WORD")-1, SCE_HPA_WORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_PY_ASP_TRIPLE", sizeof("SCINTILLA_HTML_PY_ASP_TRIPLE")-1, SCE_HPA_TRIPLE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_PY_ASP_TRIPLEDOUBLE", sizeof("SCINTILLA_HTML_PY_ASP_TRIPLEDOUBLE")-1, SCE_HPA_TRIPLEDOUBLE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_PY_ASP_CLASSNAME", sizeof("SCINTILLA_HTML_PY_ASP_CLASSNAME")-1, SCE_HPA_CLASSNAME);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_PY_ASP_DEFNAME", sizeof("SCINTILLA_HTML_PY_ASP_DEFNAME")-1, SCE_HPA_DEFNAME);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_PY_ASP_OPERATOR", sizeof("SCINTILLA_HTML_PY_ASP_OPERATOR")-1, SCE_HPA_OPERATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_PY_ASP_IDENTIFIER", sizeof("SCINTILLA_HTML_PY_ASP_IDENTIFIER")-1, SCE_HPA_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_PHP_DEFAULT", sizeof("SCINTILLA_HTML_PHP_DEFAULT")-1, SCE_HPHP_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_PHP_HSTRING", sizeof("SCINTILLA_HTML_PHP_HSTRING")-1, SCE_HPHP_HSTRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_PHP_SIMPLESTRING", sizeof("SCINTILLA_HTML_PHP_SIMPLESTRING")-1, SCE_HPHP_SIMPLESTRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_PHP_WORD", sizeof("SCINTILLA_HTML_PHP_WORD")-1, SCE_HPHP_WORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_PHP_NUMBER", sizeof("SCINTILLA_HTML_PHP_NUMBER")-1, SCE_HPHP_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_PHP_VARIABLE", sizeof("SCINTILLA_HTML_PHP_VARIABLE")-1, SCE_HPHP_VARIABLE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_PHP_COMMENT", sizeof("SCINTILLA_HTML_PHP_COMMENT")-1, SCE_HPHP_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_PHP_COMMENTLINE", sizeof("SCINTILLA_HTML_PHP_COMMENTLINE")-1, SCE_HPHP_COMMENTLINE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_PHP_HSTRING_VARIABLE", sizeof("SCINTILLA_HTML_PHP_HSTRING_VARIABLE")-1, SCE_HPHP_HSTRING_VARIABLE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HTML_PHP_OPERATOR", sizeof("SCINTILLA_HTML_PHP_OPERATOR")-1, SCE_HPHP_OPERATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PEARL_DEFAULT", sizeof("SCINTILLA_PEARL_DEFAULT")-1, SCE_PL_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PEARL_ERROR", sizeof("SCINTILLA_PEARL_ERROR")-1, SCE_PL_ERROR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PEARL_COMMENTLINE", sizeof("SCINTILLA_PEARL_COMMENTLINE")-1, SCE_PL_COMMENTLINE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PEARL_POD", sizeof("SCINTILLA_PEARL_POD")-1, SCE_PL_POD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PEARL_NUMBER", sizeof("SCINTILLA_PEARL_NUMBER")-1, SCE_PL_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PEARL_WORD", sizeof("SCINTILLA_PEARL_WORD")-1, SCE_PL_WORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PEARL_STRING", sizeof("SCINTILLA_PEARL_STRING")-1, SCE_PL_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PEARL_CHARACTER", sizeof("SCINTILLA_PEARL_CHARACTER")-1, SCE_PL_CHARACTER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PEARL_PUNCTUATION", sizeof("SCINTILLA_PEARL_PUNCTUATION")-1, SCE_PL_PUNCTUATION);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PEARL_PREPROCESSOR", sizeof("SCINTILLA_PEARL_PREPROCESSOR")-1, SCE_PL_PREPROCESSOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PEARL_OPERATOR", sizeof("SCINTILLA_PEARL_OPERATOR")-1, SCE_PL_OPERATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PEARL_IDENTIFIER", sizeof("SCINTILLA_PEARL_IDENTIFIER")-1, SCE_PL_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PEARL_SCALAR", sizeof("SCINTILLA_PEARL_SCALAR")-1, SCE_PL_SCALAR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PEARL_ARRAY", sizeof("SCINTILLA_PEARL_ARRAY")-1, SCE_PL_ARRAY);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PEARL_HASH", sizeof("SCINTILLA_PEARL_HASH")-1, SCE_PL_HASH);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PEARL_SYMBOLTABLE", sizeof("SCINTILLA_PEARL_SYMBOLTABLE")-1, SCE_PL_SYMBOLTABLE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PEARL_VARIABLE_INDEXER", sizeof("SCINTILLA_PEARL_VARIABLE_INDEXER")-1, SCE_PL_VARIABLE_INDEXER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PEARL_REGEX", sizeof("SCINTILLA_PEARL_REGEX")-1, SCE_PL_REGEX);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PEARL_REGSUBST", sizeof("SCINTILLA_PEARL_REGSUBST")-1, SCE_PL_REGSUBST);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PEARL_LONGQUOTE", sizeof("SCINTILLA_PEARL_LONGQUOTE")-1, SCE_PL_LONGQUOTE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PEARL_BACKTICKS", sizeof("SCINTILLA_PEARL_BACKTICKS")-1, SCE_PL_BACKTICKS);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PEARL_DATASECTION", sizeof("SCINTILLA_PEARL_DATASECTION")-1, SCE_PL_DATASECTION);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PEARL_HERE_DELIM", sizeof("SCINTILLA_PEARL_HERE_DELIM")-1, SCE_PL_HERE_DELIM);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PEARL_HERE_Q", sizeof("SCINTILLA_PEARL_HERE_Q")-1, SCE_PL_HERE_Q);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PEARL_HERE_QQ", sizeof("SCINTILLA_PEARL_HERE_QQ")-1, SCE_PL_HERE_QQ);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PEARL_HERE_QX", sizeof("SCINTILLA_PEARL_HERE_QX")-1, SCE_PL_HERE_QX);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PEARL_STRING_Q", sizeof("SCINTILLA_PEARL_STRING_Q")-1, SCE_PL_STRING_Q);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PEARL_STRING_QQ", sizeof("SCINTILLA_PEARL_STRING_QQ")-1, SCE_PL_STRING_QQ);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PEARL_STRING_QX", sizeof("SCINTILLA_PEARL_STRING_QX")-1, SCE_PL_STRING_QX);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PEARL_STRING_QR", sizeof("SCINTILLA_PEARL_STRING_QR")-1, SCE_PL_STRING_QR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PEARL_STRING_QW", sizeof("SCINTILLA_PEARL_STRING_QW")-1, SCE_PL_STRING_QW);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PEARL_POD_VERB", sizeof("SCINTILLA_PEARL_POD_VERB")-1, SCE_PL_POD_VERB);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_RUBY_DEFAULT", sizeof("SCINTILLA_RUBY_DEFAULT")-1, SCE_RB_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_RUBY_ERROR", sizeof("SCINTILLA_RUBY_ERROR")-1, SCE_RB_ERROR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_RUBY_COMMENTLINE", sizeof("SCINTILLA_RUBY_COMMENTLINE")-1, SCE_RB_COMMENTLINE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_RUBY_POD", sizeof("SCINTILLA_RUBY_POD")-1, SCE_RB_POD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_RUBY_NUMBER", sizeof("SCINTILLA_RUBY_NUMBER")-1, SCE_RB_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_RUBY_WORD", sizeof("SCINTILLA_RUBY_WORD")-1, SCE_RB_WORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_RUBY_STRING", sizeof("SCINTILLA_RUBY_STRING")-1, SCE_RB_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_RUBY_CHARACTER", sizeof("SCINTILLA_RUBY_CHARACTER")-1, SCE_RB_CHARACTER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_RUBY_CLASSNAME", sizeof("SCINTILLA_RUBY_CLASSNAME")-1, SCE_RB_CLASSNAME);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_RUBY_DEFNAME", sizeof("SCINTILLA_RUBY_DEFNAME")-1, SCE_RB_DEFNAME);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_RUBY_OPERATOR", sizeof("SCINTILLA_RUBY_OPERATOR")-1, SCE_RB_OPERATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_RUBY_IDENTIFIER", sizeof("SCINTILLA_RUBY_IDENTIFIER")-1, SCE_RB_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_RUBY_REGEX", sizeof("SCINTILLA_RUBY_REGEX")-1, SCE_RB_REGEX);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_RUBY_GLOBAL", sizeof("SCINTILLA_RUBY_GLOBAL")-1, SCE_RB_GLOBAL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_RUBY_SYMBOL", sizeof("SCINTILLA_RUBY_SYMBOL")-1, SCE_RB_SYMBOL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_RUBY_MODULE_NAME", sizeof("SCINTILLA_RUBY_MODULE_NAME")-1, SCE_RB_MODULE_NAME);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_RUBY_INSTANCE_VAR", sizeof("SCINTILLA_RUBY_INSTANCE_VAR")-1, SCE_RB_INSTANCE_VAR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_RUBY_CLASS_VAR", sizeof("SCINTILLA_RUBY_CLASS_VAR")-1, SCE_RB_CLASS_VAR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_RUBY_BACKTICKS", sizeof("SCINTILLA_RUBY_BACKTICKS")-1, SCE_RB_BACKTICKS);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_RUBY_DATASECTION", sizeof("SCINTILLA_RUBY_DATASECTION")-1, SCE_RB_DATASECTION);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_RUBY_HERE_DELIM", sizeof("SCINTILLA_RUBY_HERE_DELIM")-1, SCE_RB_HERE_DELIM);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_RUBY_HERE_Q", sizeof("SCINTILLA_RUBY_HERE_Q")-1, SCE_RB_HERE_Q);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_RUBY_HERE_QQ", sizeof("SCINTILLA_RUBY_HERE_QQ")-1, SCE_RB_HERE_QQ);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_RUBY_HERE_QX", sizeof("SCINTILLA_RUBY_HERE_QX")-1, SCE_RB_HERE_QX);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_RUBY_STRING_Q", sizeof("SCINTILLA_RUBY_STRING_Q")-1, SCE_RB_STRING_Q);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_RUBY_STRING_QQ", sizeof("SCINTILLA_RUBY_STRING_QQ")-1, SCE_RB_STRING_QQ);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_RUBY_STRING_QX", sizeof("SCINTILLA_RUBY_STRING_QX")-1, SCE_RB_STRING_QX);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_RUBY_STRING_QR", sizeof("SCINTILLA_RUBY_STRING_QR")-1, SCE_RB_STRING_QR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_RUBY_STRING_QW", sizeof("SCINTILLA_RUBY_STRING_QW")-1, SCE_RB_STRING_QW);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_RUBY_WORD_DEMOTED", sizeof("SCINTILLA_RUBY_WORD_DEMOTED")-1, SCE_RB_WORD_DEMOTED);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_RUBY_STDIN", sizeof("SCINTILLA_RUBY_STDIN")-1, SCE_RB_STDIN);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_RUBY_STDOUT", sizeof("SCINTILLA_RUBY_STDOUT")-1, SCE_RB_STDOUT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_RUBY_STDERR", sizeof("SCINTILLA_RUBY_STDERR")-1, SCE_RB_STDERR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_RUBY_UPPER_BOUND", sizeof("SCINTILLA_RUBY_UPPER_BOUND")-1, SCE_RB_UPPER_BOUND);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VB_DEFAULT", sizeof("SCINTILLA_VB_DEFAULT")-1, SCE_B_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VB_COMMENT", sizeof("SCINTILLA_VB_COMMENT")-1, SCE_B_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VB_NUMBER", sizeof("SCINTILLA_VB_NUMBER")-1, SCE_B_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VB_KEYWORD", sizeof("SCINTILLA_VB_KEYWORD")-1, SCE_B_KEYWORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VB_STRING", sizeof("SCINTILLA_VB_STRING")-1, SCE_B_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VB_PREPROCESSOR", sizeof("SCINTILLA_VB_PREPROCESSOR")-1, SCE_B_PREPROCESSOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VB_OPERATOR", sizeof("SCINTILLA_VB_OPERATOR")-1, SCE_B_OPERATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VB_IDENTIFIER", sizeof("SCINTILLA_VB_IDENTIFIER")-1, SCE_B_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VB_DATE", sizeof("SCINTILLA_VB_DATE")-1, SCE_B_DATE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VB_STRINGEOL", sizeof("SCINTILLA_VB_STRINGEOL")-1, SCE_B_STRINGEOL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VB_KEYWORD2", sizeof("SCINTILLA_VB_KEYWORD2")-1, SCE_B_KEYWORD2);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VB_KEYWORD3", sizeof("SCINTILLA_VB_KEYWORD3")-1, SCE_B_KEYWORD3);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VB_KEYWORD4", sizeof("SCINTILLA_VB_KEYWORD4")-1, SCE_B_KEYWORD4);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VB_CONSTANT", sizeof("SCINTILLA_VB_CONSTANT")-1, SCE_B_CONSTANT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VB_ASM", sizeof("SCINTILLA_VB_ASM")-1, SCE_B_ASM);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VB_LABEL", sizeof("SCINTILLA_VB_LABEL")-1, SCE_B_LABEL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VB_ERROR", sizeof("SCINTILLA_VB_ERROR")-1, SCE_B_ERROR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VB_HEXNUMBER", sizeof("SCINTILLA_VB_HEXNUMBER")-1, SCE_B_HEXNUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VB_BINNUMBER", sizeof("SCINTILLA_VB_BINNUMBER")-1, SCE_B_BINNUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PROPS_DEFAULT", sizeof("SCINTILLA_PROPS_DEFAULT")-1, SCE_PROPS_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PROPS_COMMENT", sizeof("SCINTILLA_PROPS_COMMENT")-1, SCE_PROPS_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PROPS_SECTION", sizeof("SCINTILLA_PROPS_SECTION")-1, SCE_PROPS_SECTION);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PROPS_ASSIGNMENT", sizeof("SCINTILLA_PROPS_ASSIGNMENT")-1, SCE_PROPS_ASSIGNMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PROPS_DEFVAL", sizeof("SCINTILLA_PROPS_DEFVAL")-1, SCE_PROPS_DEFVAL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PROPS_KEY", sizeof("SCINTILLA_PROPS_KEY")-1, SCE_PROPS_KEY);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LATEX_DEFAULT", sizeof("SCINTILLA_LATEX_DEFAULT")-1, SCE_L_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LATEX_COMMAND", sizeof("SCINTILLA_LATEX_COMMAND")-1, SCE_L_COMMAND);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LATEX_TAG", sizeof("SCINTILLA_LATEX_TAG")-1, SCE_L_TAG);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LATEX_MATH", sizeof("SCINTILLA_LATEX_MATH")-1, SCE_L_MATH);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LATEX_COMMENT", sizeof("SCINTILLA_LATEX_COMMENT")-1, SCE_L_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LUA_DEFAULT", sizeof("SCINTILLA_LUA_DEFAULT")-1, SCE_LUA_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LUA_COMMENT", sizeof("SCINTILLA_LUA_COMMENT")-1, SCE_LUA_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LUA_COMMENTLINE", sizeof("SCINTILLA_LUA_COMMENTLINE")-1, SCE_LUA_COMMENTLINE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LUA_COMMENTDOC", sizeof("SCINTILLA_LUA_COMMENTDOC")-1, SCE_LUA_COMMENTDOC);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LUA_NUMBER", sizeof("SCINTILLA_LUA_NUMBER")-1, SCE_LUA_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LUA_WORD", sizeof("SCINTILLA_LUA_WORD")-1, SCE_LUA_WORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LUA_STRING", sizeof("SCINTILLA_LUA_STRING")-1, SCE_LUA_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LUA_CHARACTER", sizeof("SCINTILLA_LUA_CHARACTER")-1, SCE_LUA_CHARACTER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LUA_LITERALSTRING", sizeof("SCINTILLA_LUA_LITERALSTRING")-1, SCE_LUA_LITERALSTRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LUA_PREPROCESSOR", sizeof("SCINTILLA_LUA_PREPROCESSOR")-1, SCE_LUA_PREPROCESSOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LUA_OPERATOR", sizeof("SCINTILLA_LUA_OPERATOR")-1, SCE_LUA_OPERATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LUA_IDENTIFIER", sizeof("SCINTILLA_LUA_IDENTIFIER")-1, SCE_LUA_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LUA_STRINGEOL", sizeof("SCINTILLA_LUA_STRINGEOL")-1, SCE_LUA_STRINGEOL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LUA_WORD2", sizeof("SCINTILLA_LUA_WORD2")-1, SCE_LUA_WORD2);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LUA_WORD3", sizeof("SCINTILLA_LUA_WORD3")-1, SCE_LUA_WORD3);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LUA_WORD4", sizeof("SCINTILLA_LUA_WORD4")-1, SCE_LUA_WORD4);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LUA_WORD5", sizeof("SCINTILLA_LUA_WORD5")-1, SCE_LUA_WORD5);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LUA_WORD6", sizeof("SCINTILLA_LUA_WORD6")-1, SCE_LUA_WORD6);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LUA_WORD7", sizeof("SCINTILLA_LUA_WORD7")-1, SCE_LUA_WORD7);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LUA_WORD8", sizeof("SCINTILLA_LUA_WORD8")-1, SCE_LUA_WORD8);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ERR_DEFAULT", sizeof("SCINTILLA_ERR_DEFAULT")-1, SCE_ERR_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ERR_PYTHON", sizeof("SCINTILLA_ERR_PYTHON")-1, SCE_ERR_PYTHON);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ERR_GCC", sizeof("SCINTILLA_ERR_GCC")-1, SCE_ERR_GCC);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ERR_MS", sizeof("SCINTILLA_ERR_MS")-1, SCE_ERR_MS);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ERR_CMD", sizeof("SCINTILLA_ERR_CMD")-1, SCE_ERR_CMD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ERR_BORLAND", sizeof("SCINTILLA_ERR_BORLAND")-1, SCE_ERR_BORLAND);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ERR_PERL", sizeof("SCINTILLA_ERR_PERL")-1, SCE_ERR_PERL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ERR_NET", sizeof("SCINTILLA_ERR_NET")-1, SCE_ERR_NET);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ERR_LUA", sizeof("SCINTILLA_ERR_LUA")-1, SCE_ERR_LUA);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ERR_CTAG", sizeof("SCINTILLA_ERR_CTAG")-1, SCE_ERR_CTAG);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ERR_DIFF_CHANGED", sizeof("SCINTILLA_ERR_DIFF_CHANGED")-1, SCE_ERR_DIFF_CHANGED);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ERR_DIFF_ADDITION", sizeof("SCINTILLA_ERR_DIFF_ADDITION")-1, SCE_ERR_DIFF_ADDITION);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ERR_DIFF_DELETION", sizeof("SCINTILLA_ERR_DIFF_DELETION")-1, SCE_ERR_DIFF_DELETION);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ERR_DIFF_MESSAGE", sizeof("SCINTILLA_ERR_DIFF_MESSAGE")-1, SCE_ERR_DIFF_MESSAGE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ERR_PHP", sizeof("SCINTILLA_ERR_PHP")-1, SCE_ERR_PHP);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ERR_ELF", sizeof("SCINTILLA_ERR_ELF")-1, SCE_ERR_ELF);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ERR_IFC", sizeof("SCINTILLA_ERR_IFC")-1, SCE_ERR_IFC);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ERR_IFORT", sizeof("SCINTILLA_ERR_IFORT")-1, SCE_ERR_IFORT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ERR_ABSF", sizeof("SCINTILLA_ERR_ABSF")-1, SCE_ERR_ABSF);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ERR_TIDY", sizeof("SCINTILLA_ERR_TIDY")-1, SCE_ERR_TIDY);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ERR_JAVA_STACK", sizeof("SCINTILLA_ERR_JAVA_STACK")-1, SCE_ERR_JAVA_STACK);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_BAT_DEFAULT", sizeof("SCINTILLA_BAT_DEFAULT")-1, SCE_BAT_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_BAT_COMMENT", sizeof("SCINTILLA_BAT_COMMENT")-1, SCE_BAT_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_BAT_WORD", sizeof("SCINTILLA_BAT_WORD")-1, SCE_BAT_WORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_BAT_LABEL", sizeof("SCINTILLA_BAT_LABEL")-1, SCE_BAT_LABEL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_BAT_HIDE", sizeof("SCINTILLA_BAT_HIDE")-1, SCE_BAT_HIDE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_BAT_COMMAND", sizeof("SCINTILLA_BAT_COMMAND")-1, SCE_BAT_COMMAND);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_BAT_IDENTIFIER", sizeof("SCINTILLA_BAT_IDENTIFIER")-1, SCE_BAT_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_BAT_OPERATOR", sizeof("SCINTILLA_BAT_OPERATOR")-1, SCE_BAT_OPERATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MAKE_DEFAULT", sizeof("SCINTILLA_MAKE_DEFAULT")-1, SCE_MAKE_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MAKE_COMMENT", sizeof("SCINTILLA_MAKE_COMMENT")-1, SCE_MAKE_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MAKE_PREPROCESSOR", sizeof("SCINTILLA_MAKE_PREPROCESSOR")-1, SCE_MAKE_PREPROCESSOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MAKE_IDENTIFIER", sizeof("SCINTILLA_MAKE_IDENTIFIER")-1, SCE_MAKE_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MAKE_OPERATOR", sizeof("SCINTILLA_MAKE_OPERATOR")-1, SCE_MAKE_OPERATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MAKE_TARGET", sizeof("SCINTILLA_MAKE_TARGET")-1, SCE_MAKE_TARGET);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MAKE_IDEOL", sizeof("SCINTILLA_MAKE_IDEOL")-1, SCE_MAKE_IDEOL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_DIFF_DEFAULT", sizeof("SCINTILLA_DIFF_DEFAULT")-1, SCE_DIFF_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_DIFF_COMMENT", sizeof("SCINTILLA_DIFF_COMMENT")-1, SCE_DIFF_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_DIFF_COMMAND", sizeof("SCINTILLA_DIFF_COMMAND")-1, SCE_DIFF_COMMAND);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_DIFF_HEADER", sizeof("SCINTILLA_DIFF_HEADER")-1, SCE_DIFF_HEADER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_DIFF_POSITION", sizeof("SCINTILLA_DIFF_POSITION")-1, SCE_DIFF_POSITION);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_DIFF_DELETED", sizeof("SCINTILLA_DIFF_DELETED")-1, SCE_DIFF_DELETED);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_DIFF_ADDED", sizeof("SCINTILLA_DIFF_ADDED")-1, SCE_DIFF_ADDED);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CONF_DEFAULT", sizeof("SCINTILLA_CONF_DEFAULT")-1, SCE_CONF_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CONF_COMMENT", sizeof("SCINTILLA_CONF_COMMENT")-1, SCE_CONF_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CONF_NUMBER", sizeof("SCINTILLA_CONF_NUMBER")-1, SCE_CONF_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CONF_IDENTIFIER", sizeof("SCINTILLA_CONF_IDENTIFIER")-1, SCE_CONF_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CONF_EXTENSION", sizeof("SCINTILLA_CONF_EXTENSION")-1, SCE_CONF_EXTENSION);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CONF_PARAMETER", sizeof("SCINTILLA_CONF_PARAMETER")-1, SCE_CONF_PARAMETER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CONF_STRING", sizeof("SCINTILLA_CONF_STRING")-1, SCE_CONF_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CONF_OPERATOR", sizeof("SCINTILLA_CONF_OPERATOR")-1, SCE_CONF_OPERATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CONF_IP", sizeof("SCINTILLA_CONF_IP")-1, SCE_CONF_IP);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CONF_DIRECTIVE", sizeof("SCINTILLA_CONF_DIRECTIVE")-1, SCE_CONF_DIRECTIVE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_AVE_DEFAULT", sizeof("SCINTILLA_AVE_DEFAULT")-1, SCE_AVE_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_AVE_COMMENT", sizeof("SCINTILLA_AVE_COMMENT")-1, SCE_AVE_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_AVE_NUMBER", sizeof("SCINTILLA_AVE_NUMBER")-1, SCE_AVE_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_AVE_WORD", sizeof("SCINTILLA_AVE_WORD")-1, SCE_AVE_WORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_AVE_STRING", sizeof("SCINTILLA_AVE_STRING")-1, SCE_AVE_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_AVE_ENUM", sizeof("SCINTILLA_AVE_ENUM")-1, SCE_AVE_ENUM);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_AVE_STRINGEOL", sizeof("SCINTILLA_AVE_STRINGEOL")-1, SCE_AVE_STRINGEOL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_AVE_IDENTIFIER", sizeof("SCINTILLA_AVE_IDENTIFIER")-1, SCE_AVE_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_AVE_OPERATOR", sizeof("SCINTILLA_AVE_OPERATOR")-1, SCE_AVE_OPERATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_AVE_WORD1", sizeof("SCINTILLA_AVE_WORD1")-1, SCE_AVE_WORD1);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_AVE_WORD2", sizeof("SCINTILLA_AVE_WORD2")-1, SCE_AVE_WORD2);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_AVE_WORD3", sizeof("SCINTILLA_AVE_WORD3")-1, SCE_AVE_WORD3);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_AVE_WORD4", sizeof("SCINTILLA_AVE_WORD4")-1, SCE_AVE_WORD4);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_AVE_WORD5", sizeof("SCINTILLA_AVE_WORD5")-1, SCE_AVE_WORD5);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_AVE_WORD6", sizeof("SCINTILLA_AVE_WORD6")-1, SCE_AVE_WORD6);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ADA_DEFAULT", sizeof("SCINTILLA_ADA_DEFAULT")-1, SCE_ADA_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ADA_WORD", sizeof("SCINTILLA_ADA_WORD")-1, SCE_ADA_WORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ADA_IDENTIFIER", sizeof("SCINTILLA_ADA_IDENTIFIER")-1, SCE_ADA_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ADA_NUMBER", sizeof("SCINTILLA_ADA_NUMBER")-1, SCE_ADA_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ADA_DELIMITER", sizeof("SCINTILLA_ADA_DELIMITER")-1, SCE_ADA_DELIMITER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ADA_CHARACTER", sizeof("SCINTILLA_ADA_CHARACTER")-1, SCE_ADA_CHARACTER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ADA_CHARACTEREOL", sizeof("SCINTILLA_ADA_CHARACTEREOL")-1, SCE_ADA_CHARACTEREOL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ADA_STRING", sizeof("SCINTILLA_ADA_STRING")-1, SCE_ADA_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ADA_STRINGEOL", sizeof("SCINTILLA_ADA_STRINGEOL")-1, SCE_ADA_STRINGEOL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ADA_LABEL", sizeof("SCINTILLA_ADA_LABEL")-1, SCE_ADA_LABEL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ADA_COMMENTLINE", sizeof("SCINTILLA_ADA_COMMENTLINE")-1, SCE_ADA_COMMENTLINE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ADA_ILLEGAL", sizeof("SCINTILLA_ADA_ILLEGAL")-1, SCE_ADA_ILLEGAL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_BAAN_DEFAULT", sizeof("SCINTILLA_BAAN_DEFAULT")-1, SCE_BAAN_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_BAAN_COMMENT", sizeof("SCINTILLA_BAAN_COMMENT")-1, SCE_BAAN_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_BAAN_COMMENTDOC", sizeof("SCINTILLA_BAAN_COMMENTDOC")-1, SCE_BAAN_COMMENTDOC);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_BAAN_NUMBER", sizeof("SCINTILLA_BAAN_NUMBER")-1, SCE_BAAN_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_BAAN_WORD", sizeof("SCINTILLA_BAAN_WORD")-1, SCE_BAAN_WORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_BAAN_STRING", sizeof("SCINTILLA_BAAN_STRING")-1, SCE_BAAN_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_BAAN_PREPROCESSOR", sizeof("SCINTILLA_BAAN_PREPROCESSOR")-1, SCE_BAAN_PREPROCESSOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_BAAN_OPERATOR", sizeof("SCINTILLA_BAAN_OPERATOR")-1, SCE_BAAN_OPERATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_BAAN_IDENTIFIER", sizeof("SCINTILLA_BAAN_IDENTIFIER")-1, SCE_BAAN_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_BAAN_STRINGEOL", sizeof("SCINTILLA_BAAN_STRINGEOL")-1, SCE_BAAN_STRINGEOL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_BAAN_WORD2", sizeof("SCINTILLA_BAAN_WORD2")-1, SCE_BAAN_WORD2);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LISP_DEFAULT", sizeof("SCINTILLA_LISP_DEFAULT")-1, SCE_LISP_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LISP_COMMENT", sizeof("SCINTILLA_LISP_COMMENT")-1, SCE_LISP_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LISP_NUMBER", sizeof("SCINTILLA_LISP_NUMBER")-1, SCE_LISP_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LISP_KEYWORD", sizeof("SCINTILLA_LISP_KEYWORD")-1, SCE_LISP_KEYWORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LISP_KEYWORD_KW", sizeof("SCINTILLA_LISP_KEYWORD_KW")-1, SCE_LISP_KEYWORD_KW);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LISP_SYMBOL", sizeof("SCINTILLA_LISP_SYMBOL")-1, SCE_LISP_SYMBOL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LISP_STRING", sizeof("SCINTILLA_LISP_STRING")-1, SCE_LISP_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LISP_STRINGEOL", sizeof("SCINTILLA_LISP_STRINGEOL")-1, SCE_LISP_STRINGEOL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LISP_IDENTIFIER", sizeof("SCINTILLA_LISP_IDENTIFIER")-1, SCE_LISP_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LISP_OPERATOR", sizeof("SCINTILLA_LISP_OPERATOR")-1, SCE_LISP_OPERATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LISP_SPECIAL", sizeof("SCINTILLA_LISP_SPECIAL")-1, SCE_LISP_SPECIAL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LISP_MULTI_COMMENT", sizeof("SCINTILLA_LISP_MULTI_COMMENT")-1, SCE_LISP_MULTI_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_EIFFEL_DEFAULT", sizeof("SCINTILLA_EIFFEL_DEFAULT")-1, SCE_EIFFEL_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_EIFFEL_COMMENTLINE", sizeof("SCINTILLA_EIFFEL_COMMENTLINE")-1, SCE_EIFFEL_COMMENTLINE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_EIFFEL_NUMBER", sizeof("SCINTILLA_EIFFEL_NUMBER")-1, SCE_EIFFEL_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_EIFFEL_WORD", sizeof("SCINTILLA_EIFFEL_WORD")-1, SCE_EIFFEL_WORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_EIFFEL_STRING", sizeof("SCINTILLA_EIFFEL_STRING")-1, SCE_EIFFEL_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_EIFFEL_CHARACTER", sizeof("SCINTILLA_EIFFEL_CHARACTER")-1, SCE_EIFFEL_CHARACTER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_EIFFEL_OPERATOR", sizeof("SCINTILLA_EIFFEL_OPERATOR")-1, SCE_EIFFEL_OPERATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_EIFFEL_IDENTIFIER", sizeof("SCINTILLA_EIFFEL_IDENTIFIER")-1, SCE_EIFFEL_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_EIFFEL_STRINGEOL", sizeof("SCINTILLA_EIFFEL_STRINGEOL")-1, SCE_EIFFEL_STRINGEOL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_NNCRONTAB_DEFAULT", sizeof("SCINTILLA_NNCRONTAB_DEFAULT")-1, SCE_NNCRONTAB_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_NNCRONTAB_COMMENT", sizeof("SCINTILLA_NNCRONTAB_COMMENT")-1, SCE_NNCRONTAB_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_NNCRONTAB_TASK", sizeof("SCINTILLA_NNCRONTAB_TASK")-1, SCE_NNCRONTAB_TASK);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_NNCRONTAB_SECTION", sizeof("SCINTILLA_NNCRONTAB_SECTION")-1, SCE_NNCRONTAB_SECTION);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_NNCRONTAB_KEYWORD", sizeof("SCINTILLA_NNCRONTAB_KEYWORD")-1, SCE_NNCRONTAB_KEYWORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_NNCRONTAB_MODIFIER", sizeof("SCINTILLA_NNCRONTAB_MODIFIER")-1, SCE_NNCRONTAB_MODIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_NNCRONTAB_ASTERISK", sizeof("SCINTILLA_NNCRONTAB_ASTERISK")-1, SCE_NNCRONTAB_ASTERISK);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_NNCRONTAB_NUMBER", sizeof("SCINTILLA_NNCRONTAB_NUMBER")-1, SCE_NNCRONTAB_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_NNCRONTAB_STRING", sizeof("SCINTILLA_NNCRONTAB_STRING")-1, SCE_NNCRONTAB_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_NNCRONTAB_ENVIRONMENT", sizeof("SCINTILLA_NNCRONTAB_ENVIRONMENT")-1, SCE_NNCRONTAB_ENVIRONMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_NNCRONTAB_IDENTIFIER", sizeof("SCINTILLA_NNCRONTAB_IDENTIFIER")-1, SCE_NNCRONTAB_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FORTH_DEFAULT", sizeof("SCINTILLA_FORTH_DEFAULT")-1, SCE_FORTH_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FORTH_COMMENT", sizeof("SCINTILLA_FORTH_COMMENT")-1, SCE_FORTH_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FORTH_COMMENT_ML", sizeof("SCINTILLA_FORTH_COMMENT_ML")-1, SCE_FORTH_COMMENT_ML);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FORTH_IDENTIFIER", sizeof("SCINTILLA_FORTH_IDENTIFIER")-1, SCE_FORTH_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FORTH_CONTROL", sizeof("SCINTILLA_FORTH_CONTROL")-1, SCE_FORTH_CONTROL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FORTH_KEYWORD", sizeof("SCINTILLA_FORTH_KEYWORD")-1, SCE_FORTH_KEYWORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FORTH_DEFWORD", sizeof("SCINTILLA_FORTH_DEFWORD")-1, SCE_FORTH_DEFWORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FORTH_PREWORD1", sizeof("SCINTILLA_FORTH_PREWORD1")-1, SCE_FORTH_PREWORD1);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FORTH_PREWORD2", sizeof("SCINTILLA_FORTH_PREWORD2")-1, SCE_FORTH_PREWORD2);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FORTH_NUMBER", sizeof("SCINTILLA_FORTH_NUMBER")-1, SCE_FORTH_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FORTH_STRING", sizeof("SCINTILLA_FORTH_STRING")-1, SCE_FORTH_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FORTH_LOCALE", sizeof("SCINTILLA_FORTH_LOCALE")-1, SCE_FORTH_LOCALE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MATLAB_DEFAULT", sizeof("SCINTILLA_MATLAB_DEFAULT")-1, SCE_MATLAB_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MATLAB_COMMENT", sizeof("SCINTILLA_MATLAB_COMMENT")-1, SCE_MATLAB_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MATLAB_COMMAND", sizeof("SCINTILLA_MATLAB_COMMAND")-1, SCE_MATLAB_COMMAND);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MATLAB_NUMBER", sizeof("SCINTILLA_MATLAB_NUMBER")-1, SCE_MATLAB_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MATLAB_KEYWORD", sizeof("SCINTILLA_MATLAB_KEYWORD")-1, SCE_MATLAB_KEYWORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MATLAB_STRING", sizeof("SCINTILLA_MATLAB_STRING")-1, SCE_MATLAB_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MATLAB_OPERATOR", sizeof("SCINTILLA_MATLAB_OPERATOR")-1, SCE_MATLAB_OPERATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MATLAB_IDENTIFIER", sizeof("SCINTILLA_MATLAB_IDENTIFIER")-1, SCE_MATLAB_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MATLAB_DOUBLEQUOTESTRING", sizeof("SCINTILLA_MATLAB_DOUBLEQUOTESTRING")-1, SCE_MATLAB_DOUBLEQUOTESTRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SCRIPTOL_DEFAULT", sizeof("SCINTILLA_SCRIPTOL_DEFAULT")-1, SCE_SCRIPTOL_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SCRIPTOL_WHITE", sizeof("SCINTILLA_SCRIPTOL_WHITE")-1, SCE_SCRIPTOL_WHITE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SCRIPTOL_COMMENTLINE", sizeof("SCINTILLA_SCRIPTOL_COMMENTLINE")-1, SCE_SCRIPTOL_COMMENTLINE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SCRIPTOL_PERSISTENT", sizeof("SCINTILLA_SCRIPTOL_PERSISTENT")-1, SCE_SCRIPTOL_PERSISTENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SCRIPTOL_CSTYLE", sizeof("SCINTILLA_SCRIPTOL_CSTYLE")-1, SCE_SCRIPTOL_CSTYLE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SCRIPTOL_COMMENTBLOCK", sizeof("SCINTILLA_SCRIPTOL_COMMENTBLOCK")-1, SCE_SCRIPTOL_COMMENTBLOCK);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SCRIPTOL_NUMBER", sizeof("SCINTILLA_SCRIPTOL_NUMBER")-1, SCE_SCRIPTOL_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SCRIPTOL_STRING", sizeof("SCINTILLA_SCRIPTOL_STRING")-1, SCE_SCRIPTOL_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SCRIPTOL_CHARACTER", sizeof("SCINTILLA_SCRIPTOL_CHARACTER")-1, SCE_SCRIPTOL_CHARACTER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SCRIPTOL_STRINGEOL", sizeof("SCINTILLA_SCRIPTOL_STRINGEOL")-1, SCE_SCRIPTOL_STRINGEOL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SCRIPTOL_KEYWORD", sizeof("SCINTILLA_SCRIPTOL_KEYWORD")-1, SCE_SCRIPTOL_KEYWORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SCRIPTOL_OPERATOR", sizeof("SCINTILLA_SCRIPTOL_OPERATOR")-1, SCE_SCRIPTOL_OPERATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SCRIPTOL_IDENTIFIER", sizeof("SCINTILLA_SCRIPTOL_IDENTIFIER")-1, SCE_SCRIPTOL_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SCRIPTOL_TRIPLE", sizeof("SCINTILLA_SCRIPTOL_TRIPLE")-1, SCE_SCRIPTOL_TRIPLE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SCRIPTOL_CLASSNAME", sizeof("SCINTILLA_SCRIPTOL_CLASSNAME")-1, SCE_SCRIPTOL_CLASSNAME);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SCRIPTOL_PREPROCESSOR", sizeof("SCINTILLA_SCRIPTOL_PREPROCESSOR")-1, SCE_SCRIPTOL_PREPROCESSOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ASM_DEFAULT", sizeof("SCINTILLA_ASM_DEFAULT")-1, SCE_ASM_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ASM_COMMENT", sizeof("SCINTILLA_ASM_COMMENT")-1, SCE_ASM_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ASM_NUMBER", sizeof("SCINTILLA_ASM_NUMBER")-1, SCE_ASM_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ASM_STRING", sizeof("SCINTILLA_ASM_STRING")-1, SCE_ASM_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ASM_OPERATOR", sizeof("SCINTILLA_ASM_OPERATOR")-1, SCE_ASM_OPERATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ASM_IDENTIFIER", sizeof("SCINTILLA_ASM_IDENTIFIER")-1, SCE_ASM_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ASM_CPUINSTRUCTION", sizeof("SCINTILLA_ASM_CPUINSTRUCTION")-1, SCE_ASM_CPUINSTRUCTION);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ASM_MATHINSTRUCTION", sizeof("SCINTILLA_ASM_MATHINSTRUCTION")-1, SCE_ASM_MATHINSTRUCTION);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ASM_REGISTER", sizeof("SCINTILLA_ASM_REGISTER")-1, SCE_ASM_REGISTER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ASM_DIRECTIVE", sizeof("SCINTILLA_ASM_DIRECTIVE")-1, SCE_ASM_DIRECTIVE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ASM_DIRECTIVEOPERAND", sizeof("SCINTILLA_ASM_DIRECTIVEOPERAND")-1, SCE_ASM_DIRECTIVEOPERAND);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ASM_COMMENTBLOCK", sizeof("SCINTILLA_ASM_COMMENTBLOCK")-1, SCE_ASM_COMMENTBLOCK);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ASM_CHARACTER", sizeof("SCINTILLA_ASM_CHARACTER")-1, SCE_ASM_CHARACTER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ASM_STRINGEOL", sizeof("SCINTILLA_ASM_STRINGEOL")-1, SCE_ASM_STRINGEOL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ASM_EXTINSTRUCTION", sizeof("SCINTILLA_ASM_EXTINSTRUCTION")-1, SCE_ASM_EXTINSTRUCTION);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FORTRAN_DEFAULT", sizeof("SCINTILLA_FORTRAN_DEFAULT")-1, SCE_F_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FORTRAN_COMMENT", sizeof("SCINTILLA_FORTRAN_COMMENT")-1, SCE_F_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FORTRAN_NUMBER", sizeof("SCINTILLA_FORTRAN_NUMBER")-1, SCE_F_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FORTRAN_STRING1", sizeof("SCINTILLA_FORTRAN_STRING1")-1, SCE_F_STRING1);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FORTRAN_STRING2", sizeof("SCINTILLA_FORTRAN_STRING2")-1, SCE_F_STRING2);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FORTRAN_STRINGEOL", sizeof("SCINTILLA_FORTRAN_STRINGEOL")-1, SCE_F_STRINGEOL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FORTRAN_OPERATOR", sizeof("SCINTILLA_FORTRAN_OPERATOR")-1, SCE_F_OPERATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FORTRAN_IDENTIFIER", sizeof("SCINTILLA_FORTRAN_IDENTIFIER")-1, SCE_F_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FORTRAN_WORD", sizeof("SCINTILLA_FORTRAN_WORD")-1, SCE_F_WORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FORTRAN_WORD2", sizeof("SCINTILLA_FORTRAN_WORD2")-1, SCE_F_WORD2);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FORTRAN_WORD3", sizeof("SCINTILLA_FORTRAN_WORD3")-1, SCE_F_WORD3);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FORTRAN_PREPROCESSOR", sizeof("SCINTILLA_FORTRAN_PREPROCESSOR")-1, SCE_F_PREPROCESSOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FORTRAN_OPERATOR2", sizeof("SCINTILLA_FORTRAN_OPERATOR2")-1, SCE_F_OPERATOR2);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FORTRAN_LABEL", sizeof("SCINTILLA_FORTRAN_LABEL")-1, SCE_F_LABEL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FORTRAN_CONTINUATION", sizeof("SCINTILLA_FORTRAN_CONTINUATION")-1, SCE_F_CONTINUATION);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CSS_DEFAULT", sizeof("SCINTILLA_CSS_DEFAULT")-1, SCE_CSS_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CSS_TAG", sizeof("SCINTILLA_CSS_TAG")-1, SCE_CSS_TAG);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CSS_CLASS", sizeof("SCINTILLA_CSS_CLASS")-1, SCE_CSS_CLASS);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CSS_PSEUDOCLASS", sizeof("SCINTILLA_CSS_PSEUDOCLASS")-1, SCE_CSS_PSEUDOCLASS);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CSS_UNKNOWN_PSEUDOCLASS", sizeof("SCINTILLA_CSS_UNKNOWN_PSEUDOCLASS")-1, SCE_CSS_UNKNOWN_PSEUDOCLASS);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CSS_OPERATOR", sizeof("SCINTILLA_CSS_OPERATOR")-1, SCE_CSS_OPERATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CSS_IDENTIFIER", sizeof("SCINTILLA_CSS_IDENTIFIER")-1, SCE_CSS_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CSS_UNKNOWN_IDENTIFIER", sizeof("SCINTILLA_CSS_UNKNOWN_IDENTIFIER")-1, SCE_CSS_UNKNOWN_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CSS_VALUE", sizeof("SCINTILLA_CSS_VALUE")-1, SCE_CSS_VALUE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CSS_COMMENT", sizeof("SCINTILLA_CSS_COMMENT")-1, SCE_CSS_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CSS_ID", sizeof("SCINTILLA_CSS_ID")-1, SCE_CSS_ID);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CSS_IMPORTANT", sizeof("SCINTILLA_CSS_IMPORTANT")-1, SCE_CSS_IMPORTANT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CSS_DIRECTIVE", sizeof("SCINTILLA_CSS_DIRECTIVE")-1, SCE_CSS_DIRECTIVE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CSS_DOUBLESTRING", sizeof("SCINTILLA_CSS_DOUBLESTRING")-1, SCE_CSS_DOUBLESTRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CSS_SINGLESTRING", sizeof("SCINTILLA_CSS_SINGLESTRING")-1, SCE_CSS_SINGLESTRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CSS_IDENTIFIER2", sizeof("SCINTILLA_CSS_IDENTIFIER2")-1, SCE_CSS_IDENTIFIER2);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CSS_ATTRIBUTE", sizeof("SCINTILLA_CSS_ATTRIBUTE")-1, SCE_CSS_ATTRIBUTE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_POV_DEFAULT", sizeof("SCINTILLA_POV_DEFAULT")-1, SCE_POV_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_POV_COMMENT", sizeof("SCINTILLA_POV_COMMENT")-1, SCE_POV_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_POV_COMMENTLINE", sizeof("SCINTILLA_POV_COMMENTLINE")-1, SCE_POV_COMMENTLINE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_POV_NUMBER", sizeof("SCINTILLA_POV_NUMBER")-1, SCE_POV_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_POV_OPERATOR", sizeof("SCINTILLA_POV_OPERATOR")-1, SCE_POV_OPERATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_POV_IDENTIFIER", sizeof("SCINTILLA_POV_IDENTIFIER")-1, SCE_POV_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_POV_STRING", sizeof("SCINTILLA_POV_STRING")-1, SCE_POV_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_POV_STRINGEOL", sizeof("SCINTILLA_POV_STRINGEOL")-1, SCE_POV_STRINGEOL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_POV_DIRECTIVE", sizeof("SCINTILLA_POV_DIRECTIVE")-1, SCE_POV_DIRECTIVE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_POV_BADDIRECTIVE", sizeof("SCINTILLA_POV_BADDIRECTIVE")-1, SCE_POV_BADDIRECTIVE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_POV_WORD2", sizeof("SCINTILLA_POV_WORD2")-1, SCE_POV_WORD2);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_POV_WORD3", sizeof("SCINTILLA_POV_WORD3")-1, SCE_POV_WORD3);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_POV_WORD4", sizeof("SCINTILLA_POV_WORD4")-1, SCE_POV_WORD4);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_POV_WORD5", sizeof("SCINTILLA_POV_WORD5")-1, SCE_POV_WORD5);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_POV_WORD6", sizeof("SCINTILLA_POV_WORD6")-1, SCE_POV_WORD6);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_POV_WORD7", sizeof("SCINTILLA_POV_WORD7")-1, SCE_POV_WORD7);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_POV_WORD8", sizeof("SCINTILLA_POV_WORD8")-1, SCE_POV_WORD8);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LOUT_DEFAULT", sizeof("SCINTILLA_LOUT_DEFAULT")-1, SCE_LOUT_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LOUT_COMMENT", sizeof("SCINTILLA_LOUT_COMMENT")-1, SCE_LOUT_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LOUT_NUMBER", sizeof("SCINTILLA_LOUT_NUMBER")-1, SCE_LOUT_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LOUT_WORD", sizeof("SCINTILLA_LOUT_WORD")-1, SCE_LOUT_WORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LOUT_WORD2", sizeof("SCINTILLA_LOUT_WORD2")-1, SCE_LOUT_WORD2);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LOUT_WORD3", sizeof("SCINTILLA_LOUT_WORD3")-1, SCE_LOUT_WORD3);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LOUT_WORD4", sizeof("SCINTILLA_LOUT_WORD4")-1, SCE_LOUT_WORD4);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LOUT_STRING", sizeof("SCINTILLA_LOUT_STRING")-1, SCE_LOUT_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LOUT_OPERATOR", sizeof("SCINTILLA_LOUT_OPERATOR")-1, SCE_LOUT_OPERATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LOUT_IDENTIFIER", sizeof("SCINTILLA_LOUT_IDENTIFIER")-1, SCE_LOUT_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LOUT_STRINGEOL", sizeof("SCINTILLA_LOUT_STRINGEOL")-1, SCE_LOUT_STRINGEOL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ESCRIPT_DEFAULT", sizeof("SCINTILLA_ESCRIPT_DEFAULT")-1, SCE_ESCRIPT_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ESCRIPT_COMMENT", sizeof("SCINTILLA_ESCRIPT_COMMENT")-1, SCE_ESCRIPT_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ESCRIPT_COMMENTLINE", sizeof("SCINTILLA_ESCRIPT_COMMENTLINE")-1, SCE_ESCRIPT_COMMENTLINE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ESCRIPT_COMMENTDOC", sizeof("SCINTILLA_ESCRIPT_COMMENTDOC")-1, SCE_ESCRIPT_COMMENTDOC);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ESCRIPT_NUMBER", sizeof("SCINTILLA_ESCRIPT_NUMBER")-1, SCE_ESCRIPT_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ESCRIPT_WORD", sizeof("SCINTILLA_ESCRIPT_WORD")-1, SCE_ESCRIPT_WORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ESCRIPT_STRING", sizeof("SCINTILLA_ESCRIPT_STRING")-1, SCE_ESCRIPT_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ESCRIPT_OPERATOR", sizeof("SCINTILLA_ESCRIPT_OPERATOR")-1, SCE_ESCRIPT_OPERATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ESCRIPT_IDENTIFIER", sizeof("SCINTILLA_ESCRIPT_IDENTIFIER")-1, SCE_ESCRIPT_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ESCRIPT_BRACE", sizeof("SCINTILLA_ESCRIPT_BRACE")-1, SCE_ESCRIPT_BRACE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ESCRIPT_WORD2", sizeof("SCINTILLA_ESCRIPT_WORD2")-1, SCE_ESCRIPT_WORD2);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ESCRIPT_WORD3", sizeof("SCINTILLA_ESCRIPT_WORD3")-1, SCE_ESCRIPT_WORD3);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PS_DEFAULT", sizeof("SCINTILLA_PS_DEFAULT")-1, SCE_PS_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PS_COMMENT", sizeof("SCINTILLA_PS_COMMENT")-1, SCE_PS_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PS_DSC_COMMENT", sizeof("SCINTILLA_PS_DSC_COMMENT")-1, SCE_PS_DSC_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PS_DSC_VALUE", sizeof("SCINTILLA_PS_DSC_VALUE")-1, SCE_PS_DSC_VALUE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PS_NUMBER", sizeof("SCINTILLA_PS_NUMBER")-1, SCE_PS_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PS_NAME", sizeof("SCINTILLA_PS_NAME")-1, SCE_PS_NAME);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PS_KEYWORD", sizeof("SCINTILLA_PS_KEYWORD")-1, SCE_PS_KEYWORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PS_LITERAL", sizeof("SCINTILLA_PS_LITERAL")-1, SCE_PS_LITERAL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PS_IMMEVAL", sizeof("SCINTILLA_PS_IMMEVAL")-1, SCE_PS_IMMEVAL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PS_PAREN_ARRAY", sizeof("SCINTILLA_PS_PAREN_ARRAY")-1, SCE_PS_PAREN_ARRAY);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PS_PAREN_DICT", sizeof("SCINTILLA_PS_PAREN_DICT")-1, SCE_PS_PAREN_DICT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PS_PAREN_PROC", sizeof("SCINTILLA_PS_PAREN_PROC")-1, SCE_PS_PAREN_PROC);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PS_TEXT", sizeof("SCINTILLA_PS_TEXT")-1, SCE_PS_TEXT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PS_HEXSTRING", sizeof("SCINTILLA_PS_HEXSTRING")-1, SCE_PS_HEXSTRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PS_BASE85STRING", sizeof("SCINTILLA_PS_BASE85STRING")-1, SCE_PS_BASE85STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_PS_BADSTRINGCHAR", sizeof("SCINTILLA_PS_BADSTRINGCHAR")-1, SCE_PS_BADSTRINGCHAR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_NSIS_DEFAULT", sizeof("SCINTILLA_NSIS_DEFAULT")-1, SCE_NSIS_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_NSIS_COMMENT", sizeof("SCINTILLA_NSIS_COMMENT")-1, SCE_NSIS_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_NSIS_STRINGDQ", sizeof("SCINTILLA_NSIS_STRINGDQ")-1, SCE_NSIS_STRINGDQ);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_NSIS_STRINGLQ", sizeof("SCINTILLA_NSIS_STRINGLQ")-1, SCE_NSIS_STRINGLQ);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_NSIS_STRINGRQ", sizeof("SCINTILLA_NSIS_STRINGRQ")-1, SCE_NSIS_STRINGRQ);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_NSIS_FUNCTION", sizeof("SCINTILLA_NSIS_FUNCTION")-1, SCE_NSIS_FUNCTION);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_NSIS_VARIABLE", sizeof("SCINTILLA_NSIS_VARIABLE")-1, SCE_NSIS_VARIABLE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_NSIS_LABEL", sizeof("SCINTILLA_NSIS_LABEL")-1, SCE_NSIS_LABEL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_NSIS_USERDEFINED", sizeof("SCINTILLA_NSIS_USERDEFINED")-1, SCE_NSIS_USERDEFINED);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_NSIS_SECTIONDEF", sizeof("SCINTILLA_NSIS_SECTIONDEF")-1, SCE_NSIS_SECTIONDEF);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_NSIS_SUBSECTIONDEF", sizeof("SCINTILLA_NSIS_SUBSECTIONDEF")-1, SCE_NSIS_SUBSECTIONDEF);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_NSIS_IFDEFINEDEF", sizeof("SCINTILLA_NSIS_IFDEFINEDEF")-1, SCE_NSIS_IFDEFINEDEF);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_NSIS_MACRODEF", sizeof("SCINTILLA_NSIS_MACRODEF")-1, SCE_NSIS_MACRODEF);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_NSIS_STRINGVAR", sizeof("SCINTILLA_NSIS_STRINGVAR")-1, SCE_NSIS_STRINGVAR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_NSIS_NUMBER", sizeof("SCINTILLA_NSIS_NUMBER")-1, SCE_NSIS_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_NSIS_SECTIONGROUP", sizeof("SCINTILLA_NSIS_SECTIONGROUP")-1, SCE_NSIS_SECTIONGROUP);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_NSIS_PAGEEX", sizeof("SCINTILLA_NSIS_PAGEEX")-1, SCE_NSIS_PAGEEX);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_NSIS_FUNCTIONDEF", sizeof("SCINTILLA_NSIS_FUNCTIONDEF")-1, SCE_NSIS_FUNCTIONDEF);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_NSIS_COMMENTBOX", sizeof("SCINTILLA_NSIS_COMMENTBOX")-1, SCE_NSIS_COMMENTBOX);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MMIXAL_LEADWS", sizeof("SCINTILLA_MMIXAL_LEADWS")-1, SCE_MMIXAL_LEADWS);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MMIXAL_COMMENT", sizeof("SCINTILLA_MMIXAL_COMMENT")-1, SCE_MMIXAL_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MMIXAL_LABEL", sizeof("SCINTILLA_MMIXAL_LABEL")-1, SCE_MMIXAL_LABEL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MMIXAL_OPCODE", sizeof("SCINTILLA_MMIXAL_OPCODE")-1, SCE_MMIXAL_OPCODE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MMIXAL_OPCODE_PRE", sizeof("SCINTILLA_MMIXAL_OPCODE_PRE")-1, SCE_MMIXAL_OPCODE_PRE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MMIXAL_OPCODE_VALID", sizeof("SCINTILLA_MMIXAL_OPCODE_VALID")-1, SCE_MMIXAL_OPCODE_VALID);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MMIXAL_OPCODE_UNKNOWN", sizeof("SCINTILLA_MMIXAL_OPCODE_UNKNOWN")-1, SCE_MMIXAL_OPCODE_UNKNOWN);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MMIXAL_OPCODE_POST", sizeof("SCINTILLA_MMIXAL_OPCODE_POST")-1, SCE_MMIXAL_OPCODE_POST);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MMIXAL_OPERANDS", sizeof("SCINTILLA_MMIXAL_OPERANDS")-1, SCE_MMIXAL_OPERANDS);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MMIXAL_NUMBER", sizeof("SCINTILLA_MMIXAL_NUMBER")-1, SCE_MMIXAL_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MMIXAL_REF", sizeof("SCINTILLA_MMIXAL_REF")-1, SCE_MMIXAL_REF);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MMIXAL_CHAR", sizeof("SCINTILLA_MMIXAL_CHAR")-1, SCE_MMIXAL_CHAR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MMIXAL_STRING", sizeof("SCINTILLA_MMIXAL_STRING")-1, SCE_MMIXAL_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MMIXAL_REGISTER", sizeof("SCINTILLA_MMIXAL_REGISTER")-1, SCE_MMIXAL_REGISTER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MMIXAL_HEX", sizeof("SCINTILLA_MMIXAL_HEX")-1, SCE_MMIXAL_HEX);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MMIXAL_OPERATOR", sizeof("SCINTILLA_MMIXAL_OPERATOR")-1, SCE_MMIXAL_OPERATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MMIXAL_SYMBOL", sizeof("SCINTILLA_MMIXAL_SYMBOL")-1, SCE_MMIXAL_SYMBOL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MMIXAL_INCLUDE", sizeof("SCINTILLA_MMIXAL_INCLUDE")-1, SCE_MMIXAL_INCLUDE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CLW_DEFAULT", sizeof("SCINTILLA_CLW_DEFAULT")-1, SCE_CLW_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CLW_LABEL", sizeof("SCINTILLA_CLW_LABEL")-1, SCE_CLW_LABEL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CLW_COMMENT", sizeof("SCINTILLA_CLW_COMMENT")-1, SCE_CLW_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CLW_STRING", sizeof("SCINTILLA_CLW_STRING")-1, SCE_CLW_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CLW_USER_IDENTIFIER", sizeof("SCINTILLA_CLW_USER_IDENTIFIER")-1, SCE_CLW_USER_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CLW_INTEGER_CONSTANT", sizeof("SCINTILLA_CLW_INTEGER_CONSTANT")-1, SCE_CLW_INTEGER_CONSTANT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CLW_REAL_CONSTANT", sizeof("SCINTILLA_CLW_REAL_CONSTANT")-1, SCE_CLW_REAL_CONSTANT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CLW_PICTURE_STRING", sizeof("SCINTILLA_CLW_PICTURE_STRING")-1, SCE_CLW_PICTURE_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CLW_KEYWORD", sizeof("SCINTILLA_CLW_KEYWORD")-1, SCE_CLW_KEYWORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CLW_COMPILER_DIRECTIVE", sizeof("SCINTILLA_CLW_COMPILER_DIRECTIVE")-1, SCE_CLW_COMPILER_DIRECTIVE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CLW_RUNTIME_EXPRESSIONS", sizeof("SCINTILLA_CLW_RUNTIME_EXPRESSIONS")-1, SCE_CLW_RUNTIME_EXPRESSIONS);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CLW_BUILTIN_PROCEDURES_FUNCTION", sizeof("SCINTILLA_CLW_BUILTIN_PROCEDURES_FUNCTION")-1, SCE_CLW_BUILTIN_PROCEDURES_FUNCTION);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CLW_STRUCTURE_DATA_TYPE", sizeof("SCINTILLA_CLW_STRUCTURE_DATA_TYPE")-1, SCE_CLW_STRUCTURE_DATA_TYPE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CLW_ATTRIBUTE", sizeof("SCINTILLA_CLW_ATTRIBUTE")-1, SCE_CLW_ATTRIBUTE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CLW_STANDARD_EQUATE", sizeof("SCINTILLA_CLW_STANDARD_EQUATE")-1, SCE_CLW_STANDARD_EQUATE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CLW_ERROR", sizeof("SCINTILLA_CLW_ERROR")-1, SCE_CLW_ERROR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CLW_DEPRECATED", sizeof("SCINTILLA_CLW_DEPRECATED")-1, SCE_CLW_DEPRECATED);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LOT_DEFAULT", sizeof("SCINTILLA_LOT_DEFAULT")-1, SCE_LOT_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LOT_HEADER", sizeof("SCINTILLA_LOT_HEADER")-1, SCE_LOT_HEADER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LOT_BREAK", sizeof("SCINTILLA_LOT_BREAK")-1, SCE_LOT_BREAK);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LOT_SET", sizeof("SCINTILLA_LOT_SET")-1, SCE_LOT_SET);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LOT_PASS", sizeof("SCINTILLA_LOT_PASS")-1, SCE_LOT_PASS);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LOT_FAIL", sizeof("SCINTILLA_LOT_FAIL")-1, SCE_LOT_FAIL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_LOT_ABORT", sizeof("SCINTILLA_LOT_ABORT")-1, SCE_LOT_ABORT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_YAML_DEFAULT", sizeof("SCINTILLA_YAML_DEFAULT")-1, SCE_YAML_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_YAML_COMMENT", sizeof("SCINTILLA_YAML_COMMENT")-1, SCE_YAML_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_YAML_IDENTIFIER", sizeof("SCINTILLA_YAML_IDENTIFIER")-1, SCE_YAML_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_YAML_KEYWORD", sizeof("SCINTILLA_YAML_KEYWORD")-1, SCE_YAML_KEYWORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_YAML_NUMBER", sizeof("SCINTILLA_YAML_NUMBER")-1, SCE_YAML_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_YAML_REFERENCE", sizeof("SCINTILLA_YAML_REFERENCE")-1, SCE_YAML_REFERENCE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_YAML_DOCUMENT", sizeof("SCINTILLA_YAML_DOCUMENT")-1, SCE_YAML_DOCUMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_YAML_TEXT", sizeof("SCINTILLA_YAML_TEXT")-1, SCE_YAML_TEXT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_YAML_ERROR", sizeof("SCINTILLA_YAML_ERROR")-1, SCE_YAML_ERROR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_TEX_DEFAULT", sizeof("SCINTILLA_TEX_DEFAULT")-1, SCE_TEX_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_TEX_SPECIAL", sizeof("SCINTILLA_TEX_SPECIAL")-1, SCE_TEX_SPECIAL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_TEX_GROUP", sizeof("SCINTILLA_TEX_GROUP")-1, SCE_TEX_GROUP);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_TEX_SYMBOL", sizeof("SCINTILLA_TEX_SYMBOL")-1, SCE_TEX_SYMBOL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_TEX_COMMAND", sizeof("SCINTILLA_TEX_COMMAND")-1, SCE_TEX_COMMAND);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_TEX_TEXT", sizeof("SCINTILLA_TEX_TEXT")-1, SCE_TEX_TEXT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_METAPOST_DEFAULT", sizeof("SCINTILLA_METAPOST_DEFAULT")-1, SCE_METAPOST_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_METAPOST_SPECIAL", sizeof("SCINTILLA_METAPOST_SPECIAL")-1, SCE_METAPOST_SPECIAL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_METAPOST_GROUP", sizeof("SCINTILLA_METAPOST_GROUP")-1, SCE_METAPOST_GROUP);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_METAPOST_SYMBOL", sizeof("SCINTILLA_METAPOST_SYMBOL")-1, SCE_METAPOST_SYMBOL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_METAPOST_COMMAND", sizeof("SCINTILLA_METAPOST_COMMAND")-1, SCE_METAPOST_COMMAND);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_METAPOST_TEXT", sizeof("SCINTILLA_METAPOST_TEXT")-1, SCE_METAPOST_TEXT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_METAPOST_EXTRA", sizeof("SCINTILLA_METAPOST_EXTRA")-1, SCE_METAPOST_EXTRA);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ERLANG_DEFAULT", sizeof("SCINTILLA_ERLANG_DEFAULT")-1, SCE_ERLANG_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ERLANG_COMMENT", sizeof("SCINTILLA_ERLANG_COMMENT")-1, SCE_ERLANG_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ERLANG_VARIABLE", sizeof("SCINTILLA_ERLANG_VARIABLE")-1, SCE_ERLANG_VARIABLE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ERLANG_NUMBER", sizeof("SCINTILLA_ERLANG_NUMBER")-1, SCE_ERLANG_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ERLANG_KEYWORD", sizeof("SCINTILLA_ERLANG_KEYWORD")-1, SCE_ERLANG_KEYWORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ERLANG_STRING", sizeof("SCINTILLA_ERLANG_STRING")-1, SCE_ERLANG_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ERLANG_OPERATOR", sizeof("SCINTILLA_ERLANG_OPERATOR")-1, SCE_ERLANG_OPERATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ERLANG_ATOM", sizeof("SCINTILLA_ERLANG_ATOM")-1, SCE_ERLANG_ATOM);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ERLANG_FUNCTION_NAME", sizeof("SCINTILLA_ERLANG_FUNCTION_NAME")-1, SCE_ERLANG_FUNCTION_NAME);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ERLANG_CHARACTER", sizeof("SCINTILLA_ERLANG_CHARACTER")-1, SCE_ERLANG_CHARACTER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ERLANG_MACRO", sizeof("SCINTILLA_ERLANG_MACRO")-1, SCE_ERLANG_MACRO);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ERLANG_RECORD", sizeof("SCINTILLA_ERLANG_RECORD")-1, SCE_ERLANG_RECORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ERLANG_SEPARATOR", sizeof("SCINTILLA_ERLANG_SEPARATOR")-1, SCE_ERLANG_SEPARATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ERLANG_NODE_NAME", sizeof("SCINTILLA_ERLANG_NODE_NAME")-1, SCE_ERLANG_NODE_NAME);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ERLANG_UNKNOWN", sizeof("SCINTILLA_ERLANG_UNKNOWN")-1, SCE_ERLANG_UNKNOWN);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MSSQL_DEFAULT", sizeof("SCINTILLA_MSSQL_DEFAULT")-1, SCE_MSSQL_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MSSQL_COMMENT", sizeof("SCINTILLA_MSSQL_COMMENT")-1, SCE_MSSQL_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MSSQL_LINE_COMMENT", sizeof("SCINTILLA_MSSQL_LINE_COMMENT")-1, SCE_MSSQL_LINE_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MSSQL_NUMBER", sizeof("SCINTILLA_MSSQL_NUMBER")-1, SCE_MSSQL_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MSSQL_STRING", sizeof("SCINTILLA_MSSQL_STRING")-1, SCE_MSSQL_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MSSQL_OPERATOR", sizeof("SCINTILLA_MSSQL_OPERATOR")-1, SCE_MSSQL_OPERATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MSSQL_IDENTIFIER", sizeof("SCINTILLA_MSSQL_IDENTIFIER")-1, SCE_MSSQL_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MSSQL_VARIABLE", sizeof("SCINTILLA_MSSQL_VARIABLE")-1, SCE_MSSQL_VARIABLE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MSSQL_COLUMN_NAME", sizeof("SCINTILLA_MSSQL_COLUMN_NAME")-1, SCE_MSSQL_COLUMN_NAME);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MSSQL_STATEMENT", sizeof("SCINTILLA_MSSQL_STATEMENT")-1, SCE_MSSQL_STATEMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MSSQL_DATATYPE", sizeof("SCINTILLA_MSSQL_DATATYPE")-1, SCE_MSSQL_DATATYPE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MSSQL_SYSTABLE", sizeof("SCINTILLA_MSSQL_SYSTABLE")-1, SCE_MSSQL_SYSTABLE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MSSQL_GLOBAL_VARIABLE", sizeof("SCINTILLA_MSSQL_GLOBAL_VARIABLE")-1, SCE_MSSQL_GLOBAL_VARIABLE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MSSQL_FUNCTION", sizeof("SCINTILLA_MSSQL_FUNCTION")-1, SCE_MSSQL_FUNCTION);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MSSQL_STORED_PROCEDURE", sizeof("SCINTILLA_MSSQL_STORED_PROCEDURE")-1, SCE_MSSQL_STORED_PROCEDURE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MSSQL_DEFAULT_PREF_DATATYPE", sizeof("SCINTILLA_MSSQL_DEFAULT_PREF_DATATYPE")-1, SCE_MSSQL_DEFAULT_PREF_DATATYPE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_MSSQL_COLUMN_NAME_2", sizeof("SCINTILLA_MSSQL_COLUMN_NAME_2")-1, SCE_MSSQL_COLUMN_NAME_2);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VERILOG_DEFAULT", sizeof("SCINTILLA_VERILOG_DEFAULT")-1, SCE_V_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VERILOG_COMMENT", sizeof("SCINTILLA_VERILOG_COMMENT")-1, SCE_V_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VERILOG_COMMENTLINE", sizeof("SCINTILLA_VERILOG_COMMENTLINE")-1, SCE_V_COMMENTLINE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VERILOG_COMMENTLINEBANG", sizeof("SCINTILLA_VERILOG_COMMENTLINEBANG")-1, SCE_V_COMMENTLINEBANG);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VERILOG_NUMBER", sizeof("SCINTILLA_VERILOG_NUMBER")-1, SCE_V_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VERILOG_WORD", sizeof("SCINTILLA_VERILOG_WORD")-1, SCE_V_WORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VERILOG_STRING", sizeof("SCINTILLA_VERILOG_STRING")-1, SCE_V_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VERILOG_WORD2", sizeof("SCINTILLA_VERILOG_WORD2")-1, SCE_V_WORD2);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VERILOG_WORD3", sizeof("SCINTILLA_VERILOG_WORD3")-1, SCE_V_WORD3);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VERILOG_PREPROCESSOR", sizeof("SCINTILLA_VERILOG_PREPROCESSOR")-1, SCE_V_PREPROCESSOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VERILOG_OPERATOR", sizeof("SCINTILLA_VERILOG_OPERATOR")-1, SCE_V_OPERATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VERILOG_IDENTIFIER", sizeof("SCINTILLA_VERILOG_IDENTIFIER")-1, SCE_V_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VERILOG_STRINGEOL", sizeof("SCINTILLA_VERILOG_STRINGEOL")-1, SCE_V_STRINGEOL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VERILOG_USER", sizeof("SCINTILLA_VERILOG_USER")-1, SCE_V_USER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_KIX_DEFAULT", sizeof("SCINTILLA_KIX_DEFAULT")-1, SCE_KIX_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_KIX_COMMENT", sizeof("SCINTILLA_KIX_COMMENT")-1, SCE_KIX_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_KIX_STRING1", sizeof("SCINTILLA_KIX_STRING1")-1, SCE_KIX_STRING1);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_KIX_STRING2", sizeof("SCINTILLA_KIX_STRING2")-1, SCE_KIX_STRING2);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_KIX_NUMBER", sizeof("SCINTILLA_KIX_NUMBER")-1, SCE_KIX_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_KIX_VAR", sizeof("SCINTILLA_KIX_VAR")-1, SCE_KIX_VAR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_KIX_MACRO", sizeof("SCINTILLA_KIX_MACRO")-1, SCE_KIX_MACRO);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_KIX_KEYWORD", sizeof("SCINTILLA_KIX_KEYWORD")-1, SCE_KIX_KEYWORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_KIX_FUNCTIONS", sizeof("SCINTILLA_KIX_FUNCTIONS")-1, SCE_KIX_FUNCTIONS);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_KIX_OPERATOR", sizeof("SCINTILLA_KIX_OPERATOR")-1, SCE_KIX_OPERATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_KIX_IDENTIFIER", sizeof("SCINTILLA_KIX_IDENTIFIER")-1, SCE_KIX_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_GC_DEFAULT", sizeof("SCINTILLA_GC_DEFAULT")-1, SCE_GC_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_GC_COMMENTLINE", sizeof("SCINTILLA_GC_COMMENTLINE")-1, SCE_GC_COMMENTLINE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_GC_COMMENTBLOCK", sizeof("SCINTILLA_GC_COMMENTBLOCK")-1, SCE_GC_COMMENTBLOCK);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_GC_GLOBAL", sizeof("SCINTILLA_GC_GLOBAL")-1, SCE_GC_GLOBAL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_GC_EVENT", sizeof("SCINTILLA_GC_EVENT")-1, SCE_GC_EVENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_GC_ATTRIBUTE", sizeof("SCINTILLA_GC_ATTRIBUTE")-1, SCE_GC_ATTRIBUTE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_GC_CONTROL", sizeof("SCINTILLA_GC_CONTROL")-1, SCE_GC_CONTROL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_GC_COMMAND", sizeof("SCINTILLA_GC_COMMAND")-1, SCE_GC_COMMAND);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_GC_STRING", sizeof("SCINTILLA_GC_STRING")-1, SCE_GC_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_GC_OPERATOR", sizeof("SCINTILLA_GC_OPERATOR")-1, SCE_GC_OPERATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SN_DEFAULT", sizeof("SCINTILLA_SN_DEFAULT")-1, SCE_SN_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SN_CODE", sizeof("SCINTILLA_SN_CODE")-1, SCE_SN_CODE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SN_COMMENTLINE", sizeof("SCINTILLA_SN_COMMENTLINE")-1, SCE_SN_COMMENTLINE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SN_COMMENTLINEBANG", sizeof("SCINTILLA_SN_COMMENTLINEBANG")-1, SCE_SN_COMMENTLINEBANG);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SN_NUMBER", sizeof("SCINTILLA_SN_NUMBER")-1, SCE_SN_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SN_WORD", sizeof("SCINTILLA_SN_WORD")-1, SCE_SN_WORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SN_STRING", sizeof("SCINTILLA_SN_STRING")-1, SCE_SN_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SN_WORD2", sizeof("SCINTILLA_SN_WORD2")-1, SCE_SN_WORD2);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SN_WORD3", sizeof("SCINTILLA_SN_WORD3")-1, SCE_SN_WORD3);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SN_PREPROCESSOR", sizeof("SCINTILLA_SN_PREPROCESSOR")-1, SCE_SN_PREPROCESSOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SN_OPERATOR", sizeof("SCINTILLA_SN_OPERATOR")-1, SCE_SN_OPERATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SN_IDENTIFIER", sizeof("SCINTILLA_SN_IDENTIFIER")-1, SCE_SN_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SN_STRINGEOL", sizeof("SCINTILLA_SN_STRINGEOL")-1, SCE_SN_STRINGEOL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SN_REGEXTAG", sizeof("SCINTILLA_SN_REGEXTAG")-1, SCE_SN_REGEXTAG);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SN_SIGNAL", sizeof("SCINTILLA_SN_SIGNAL")-1, SCE_SN_SIGNAL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SN_USER", sizeof("SCINTILLA_SN_USER")-1, SCE_SN_USER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_AU3_DEFAULT", sizeof("SCINTILLA_AU3_DEFAULT")-1, SCE_AU3_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_AU3_COMMENT", sizeof("SCINTILLA_AU3_COMMENT")-1, SCE_AU3_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_AU3_COMMENTBLOCK", sizeof("SCINTILLA_AU3_COMMENTBLOCK")-1, SCE_AU3_COMMENTBLOCK);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_AU3_NUMBER", sizeof("SCINTILLA_AU3_NUMBER")-1, SCE_AU3_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_AU3_FUNCTION", sizeof("SCINTILLA_AU3_FUNCTION")-1, SCE_AU3_FUNCTION);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_AU3_KEYWORD", sizeof("SCINTILLA_AU3_KEYWORD")-1, SCE_AU3_KEYWORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_AU3_MACRO", sizeof("SCINTILLA_AU3_MACRO")-1, SCE_AU3_MACRO);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_AU3_STRING", sizeof("SCINTILLA_AU3_STRING")-1, SCE_AU3_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_AU3_OPERATOR", sizeof("SCINTILLA_AU3_OPERATOR")-1, SCE_AU3_OPERATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_AU3_VARIABLE", sizeof("SCINTILLA_AU3_VARIABLE")-1, SCE_AU3_VARIABLE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_AU3_SENT", sizeof("SCINTILLA_AU3_SENT")-1, SCE_AU3_SENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_AU3_PREPROCESSOR", sizeof("SCINTILLA_AU3_PREPROCESSOR")-1, SCE_AU3_PREPROCESSOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_AU3_SPECIAL", sizeof("SCINTILLA_AU3_SPECIAL")-1, SCE_AU3_SPECIAL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_AU3_EXPAND", sizeof("SCINTILLA_AU3_EXPAND")-1, SCE_AU3_EXPAND);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_AU3_COMOBJ", sizeof("SCINTILLA_AU3_COMOBJ")-1, SCE_AU3_COMOBJ);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_AU3_UDF", sizeof("SCINTILLA_AU3_UDF")-1, SCE_AU3_UDF);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_APDL_DEFAULT", sizeof("SCINTILLA_APDL_DEFAULT")-1, SCE_APDL_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_APDL_COMMENT", sizeof("SCINTILLA_APDL_COMMENT")-1, SCE_APDL_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_APDL_COMMENTBLOCK", sizeof("SCINTILLA_APDL_COMMENTBLOCK")-1, SCE_APDL_COMMENTBLOCK);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_APDL_NUMBER", sizeof("SCINTILLA_APDL_NUMBER")-1, SCE_APDL_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_APDL_STRING", sizeof("SCINTILLA_APDL_STRING")-1, SCE_APDL_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_APDL_OPERATOR", sizeof("SCINTILLA_APDL_OPERATOR")-1, SCE_APDL_OPERATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_APDL_WORD", sizeof("SCINTILLA_APDL_WORD")-1, SCE_APDL_WORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_APDL_PROCESSOR", sizeof("SCINTILLA_APDL_PROCESSOR")-1, SCE_APDL_PROCESSOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_APDL_COMMAND", sizeof("SCINTILLA_APDL_COMMAND")-1, SCE_APDL_COMMAND);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_APDL_SLASHCOMMAND", sizeof("SCINTILLA_APDL_SLASHCOMMAND")-1, SCE_APDL_SLASHCOMMAND);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_APDL_STARCOMMAND", sizeof("SCINTILLA_APDL_STARCOMMAND")-1, SCE_APDL_STARCOMMAND);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_APDL_ARGUMENT", sizeof("SCINTILLA_APDL_ARGUMENT")-1, SCE_APDL_ARGUMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_APDL_FUNCTION", sizeof("SCINTILLA_APDL_FUNCTION")-1, SCE_APDL_FUNCTION);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SH_DEFAULT", sizeof("SCINTILLA_SH_DEFAULT")-1, SCE_SH_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SH_ERROR", sizeof("SCINTILLA_SH_ERROR")-1, SCE_SH_ERROR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SH_COMMENTLINE", sizeof("SCINTILLA_SH_COMMENTLINE")-1, SCE_SH_COMMENTLINE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SH_NUMBER", sizeof("SCINTILLA_SH_NUMBER")-1, SCE_SH_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SH_WORD", sizeof("SCINTILLA_SH_WORD")-1, SCE_SH_WORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SH_STRING", sizeof("SCINTILLA_SH_STRING")-1, SCE_SH_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SH_CHARACTER", sizeof("SCINTILLA_SH_CHARACTER")-1, SCE_SH_CHARACTER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SH_OPERATOR", sizeof("SCINTILLA_SH_OPERATOR")-1, SCE_SH_OPERATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SH_IDENTIFIER", sizeof("SCINTILLA_SH_IDENTIFIER")-1, SCE_SH_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SH_SCALAR", sizeof("SCINTILLA_SH_SCALAR")-1, SCE_SH_SCALAR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SH_PARAM", sizeof("SCINTILLA_SH_PARAM")-1, SCE_SH_PARAM);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SH_BACKTICKS", sizeof("SCINTILLA_SH_BACKTICKS")-1, SCE_SH_BACKTICKS);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SH_HERE_DELIM", sizeof("SCINTILLA_SH_HERE_DELIM")-1, SCE_SH_HERE_DELIM);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SH_HERE_Q", sizeof("SCINTILLA_SH_HERE_Q")-1, SCE_SH_HERE_Q);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ASN1_DEFAULT", sizeof("SCINTILLA_ASN1_DEFAULT")-1, SCE_ASN1_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ASN1_COMMENT", sizeof("SCINTILLA_ASN1_COMMENT")-1, SCE_ASN1_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ASN1_IDENTIFIER", sizeof("SCINTILLA_ASN1_IDENTIFIER")-1, SCE_ASN1_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ASN1_STRING", sizeof("SCINTILLA_ASN1_STRING")-1, SCE_ASN1_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ASN1_OID", sizeof("SCINTILLA_ASN1_OID")-1, SCE_ASN1_OID);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ASN1_SCALAR", sizeof("SCINTILLA_ASN1_SCALAR")-1, SCE_ASN1_SCALAR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ASN1_KEYWORD", sizeof("SCINTILLA_ASN1_KEYWORD")-1, SCE_ASN1_KEYWORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ASN1_ATTRIBUTE", sizeof("SCINTILLA_ASN1_ATTRIBUTE")-1, SCE_ASN1_ATTRIBUTE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ASN1_DESCRIPTOR", sizeof("SCINTILLA_ASN1_DESCRIPTOR")-1, SCE_ASN1_DESCRIPTOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ASN1_TYPE", sizeof("SCINTILLA_ASN1_TYPE")-1, SCE_ASN1_TYPE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ASN1_OPERATOR", sizeof("SCINTILLA_ASN1_OPERATOR")-1, SCE_ASN1_OPERATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VHDL_DEFAULT", sizeof("SCINTILLA_VHDL_DEFAULT")-1, SCE_VHDL_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VHDL_COMMENT", sizeof("SCINTILLA_VHDL_COMMENT")-1, SCE_VHDL_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VHDL_COMMENTLINEBANG", sizeof("SCINTILLA_VHDL_COMMENTLINEBANG")-1, SCE_VHDL_COMMENTLINEBANG);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VHDL_NUMBER", sizeof("SCINTILLA_VHDL_NUMBER")-1, SCE_VHDL_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VHDL_STRING", sizeof("SCINTILLA_VHDL_STRING")-1, SCE_VHDL_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VHDL_OPERATOR", sizeof("SCINTILLA_VHDL_OPERATOR")-1, SCE_VHDL_OPERATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VHDL_IDENTIFIER", sizeof("SCINTILLA_VHDL_IDENTIFIER")-1, SCE_VHDL_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VHDL_STRINGEOL", sizeof("SCINTILLA_VHDL_STRINGEOL")-1, SCE_VHDL_STRINGEOL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VHDL_KEYWORD", sizeof("SCINTILLA_VHDL_KEYWORD")-1, SCE_VHDL_KEYWORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VHDL_STDOPERATOR", sizeof("SCINTILLA_VHDL_STDOPERATOR")-1, SCE_VHDL_STDOPERATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VHDL_ATTRIBUTE", sizeof("SCINTILLA_VHDL_ATTRIBUTE")-1, SCE_VHDL_ATTRIBUTE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VHDL_STDFUNCTION", sizeof("SCINTILLA_VHDL_STDFUNCTION")-1, SCE_VHDL_STDFUNCTION);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VHDL_STDPACKAGE", sizeof("SCINTILLA_VHDL_STDPACKAGE")-1, SCE_VHDL_STDPACKAGE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VHDL_STDTYPE", sizeof("SCINTILLA_VHDL_STDTYPE")-1, SCE_VHDL_STDTYPE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_VHDL_USERWORD", sizeof("SCINTILLA_VHDL_USERWORD")-1, SCE_VHDL_USERWORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CAML_DEFAULT", sizeof("SCINTILLA_CAML_DEFAULT")-1, SCE_CAML_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CAML_IDENTIFIER", sizeof("SCINTILLA_CAML_IDENTIFIER")-1, SCE_CAML_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CAML_TAGNAME", sizeof("SCINTILLA_CAML_TAGNAME")-1, SCE_CAML_TAGNAME);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CAML_KEYWORD", sizeof("SCINTILLA_CAML_KEYWORD")-1, SCE_CAML_KEYWORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CAML_KEYWORD2", sizeof("SCINTILLA_CAML_KEYWORD2")-1, SCE_CAML_KEYWORD2);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CAML_KEYWORD3", sizeof("SCINTILLA_CAML_KEYWORD3")-1, SCE_CAML_KEYWORD3);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CAML_LINENUM", sizeof("SCINTILLA_CAML_LINENUM")-1, SCE_CAML_LINENUM);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CAML_OPERATOR", sizeof("SCINTILLA_CAML_OPERATOR")-1, SCE_CAML_OPERATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CAML_NUMBER", sizeof("SCINTILLA_CAML_NUMBER")-1, SCE_CAML_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CAML_CHAR", sizeof("SCINTILLA_CAML_CHAR")-1, SCE_CAML_CHAR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CAML_STRING", sizeof("SCINTILLA_CAML_STRING")-1, SCE_CAML_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CAML_COMMENT", sizeof("SCINTILLA_CAML_COMMENT")-1, SCE_CAML_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CAML_COMMENT1", sizeof("SCINTILLA_CAML_COMMENT1")-1, SCE_CAML_COMMENT1);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CAML_COMMENT2", sizeof("SCINTILLA_CAML_COMMENT2")-1, SCE_CAML_COMMENT2);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CAML_COMMENT3", sizeof("SCINTILLA_CAML_COMMENT3")-1, SCE_CAML_COMMENT3);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HA_DEFAULT", sizeof("SCINTILLA_HA_DEFAULT")-1, SCE_HA_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HA_IDENTIFIER", sizeof("SCINTILLA_HA_IDENTIFIER")-1, SCE_HA_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HA_KEYWORD", sizeof("SCINTILLA_HA_KEYWORD")-1, SCE_HA_KEYWORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HA_NUMBER", sizeof("SCINTILLA_HA_NUMBER")-1, SCE_HA_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HA_STRING", sizeof("SCINTILLA_HA_STRING")-1, SCE_HA_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HA_CHARACTER", sizeof("SCINTILLA_HA_CHARACTER")-1, SCE_HA_CHARACTER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HA_CLASS", sizeof("SCINTILLA_HA_CLASS")-1, SCE_HA_CLASS);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HA_MODULE", sizeof("SCINTILLA_HA_MODULE")-1, SCE_HA_MODULE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HA_CAPITAL", sizeof("SCINTILLA_HA_CAPITAL")-1, SCE_HA_CAPITAL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HA_DATA", sizeof("SCINTILLA_HA_DATA")-1, SCE_HA_DATA);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HA_IMPORT", sizeof("SCINTILLA_HA_IMPORT")-1, SCE_HA_IMPORT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HA_OPERATOR", sizeof("SCINTILLA_HA_OPERATOR")-1, SCE_HA_OPERATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HA_INSTANCE", sizeof("SCINTILLA_HA_INSTANCE")-1, SCE_HA_INSTANCE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HA_COMMENTLINE", sizeof("SCINTILLA_HA_COMMENTLINE")-1, SCE_HA_COMMENTLINE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HA_COMMENTBLOCK", sizeof("SCINTILLA_HA_COMMENTBLOCK")-1, SCE_HA_COMMENTBLOCK);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HA_COMMENTBLOCK2", sizeof("SCINTILLA_HA_COMMENTBLOCK2")-1, SCE_HA_COMMENTBLOCK2);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_HA_COMMENTBLOCK3", sizeof("SCINTILLA_HA_COMMENTBLOCK3")-1, SCE_HA_COMMENTBLOCK3);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_T3_DEFAULT", sizeof("SCINTILLA_T3_DEFAULT")-1, SCE_T3_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_T3_X_DEFAULT", sizeof("SCINTILLA_T3_X_DEFAULT")-1, SCE_T3_X_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_T3_PREPROCESSOR", sizeof("SCINTILLA_T3_PREPROCESSOR")-1, SCE_T3_PREPROCESSOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_T3_BLOCK_COMMENT", sizeof("SCINTILLA_T3_BLOCK_COMMENT")-1, SCE_T3_BLOCK_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_T3_LINE_COMMENT", sizeof("SCINTILLA_T3_LINE_COMMENT")-1, SCE_T3_LINE_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_T3_OPERATOR", sizeof("SCINTILLA_T3_OPERATOR")-1, SCE_T3_OPERATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_T3_KEYWORD", sizeof("SCINTILLA_T3_KEYWORD")-1, SCE_T3_KEYWORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_T3_NUMBER", sizeof("SCINTILLA_T3_NUMBER")-1, SCE_T3_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_T3_IDENTIFIER", sizeof("SCINTILLA_T3_IDENTIFIER")-1, SCE_T3_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_T3_S_STRING", sizeof("SCINTILLA_T3_S_STRING")-1, SCE_T3_S_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_T3_D_STRING", sizeof("SCINTILLA_T3_D_STRING")-1, SCE_T3_D_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_T3_X_STRING", sizeof("SCINTILLA_T3_X_STRING")-1, SCE_T3_X_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_T3_LIB_DIRECTIVE", sizeof("SCINTILLA_T3_LIB_DIRECTIVE")-1, SCE_T3_LIB_DIRECTIVE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_T3_MSG_PARAM", sizeof("SCINTILLA_T3_MSG_PARAM")-1, SCE_T3_MSG_PARAM);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_T3_HTML_TAG", sizeof("SCINTILLA_T3_HTML_TAG")-1, SCE_T3_HTML_TAG);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_T3_HTML_DEFAULT", sizeof("SCINTILLA_T3_HTML_DEFAULT")-1, SCE_T3_HTML_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_T3_HTML_STRING", sizeof("SCINTILLA_T3_HTML_STRING")-1, SCE_T3_HTML_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_T3_USER1", sizeof("SCINTILLA_T3_USER1")-1, SCE_T3_USER1);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_T3_USER2", sizeof("SCINTILLA_T3_USER2")-1, SCE_T3_USER2);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_T3_USER3", sizeof("SCINTILLA_T3_USER3")-1, SCE_T3_USER3);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_T3_BRACE", sizeof("SCINTILLA_T3_BRACE")-1, SCE_T3_BRACE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_REBOL_DEFAULT", sizeof("SCINTILLA_REBOL_DEFAULT")-1, SCE_REBOL_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_REBOL_COMMENTLINE", sizeof("SCINTILLA_REBOL_COMMENTLINE")-1, SCE_REBOL_COMMENTLINE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_REBOL_COMMENTBLOCK", sizeof("SCINTILLA_REBOL_COMMENTBLOCK")-1, SCE_REBOL_COMMENTBLOCK);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_REBOL_PREFACE", sizeof("SCINTILLA_REBOL_PREFACE")-1, SCE_REBOL_PREFACE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_REBOL_OPERATOR", sizeof("SCINTILLA_REBOL_OPERATOR")-1, SCE_REBOL_OPERATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_REBOL_CHARACTER", sizeof("SCINTILLA_REBOL_CHARACTER")-1, SCE_REBOL_CHARACTER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_REBOL_QUOTEDSTRING", sizeof("SCINTILLA_REBOL_QUOTEDSTRING")-1, SCE_REBOL_QUOTEDSTRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_REBOL_BRACEDSTRING", sizeof("SCINTILLA_REBOL_BRACEDSTRING")-1, SCE_REBOL_BRACEDSTRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_REBOL_NUMBER", sizeof("SCINTILLA_REBOL_NUMBER")-1, SCE_REBOL_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_REBOL_PAIR", sizeof("SCINTILLA_REBOL_PAIR")-1, SCE_REBOL_PAIR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_REBOL_TUPLE", sizeof("SCINTILLA_REBOL_TUPLE")-1, SCE_REBOL_TUPLE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_REBOL_BINARY", sizeof("SCINTILLA_REBOL_BINARY")-1, SCE_REBOL_BINARY);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_REBOL_MONEY", sizeof("SCINTILLA_REBOL_MONEY")-1, SCE_REBOL_MONEY);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_REBOL_ISSUE", sizeof("SCINTILLA_REBOL_ISSUE")-1, SCE_REBOL_ISSUE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_REBOL_TAG", sizeof("SCINTILLA_REBOL_TAG")-1, SCE_REBOL_TAG);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_REBOL_FILE", sizeof("SCINTILLA_REBOL_FILE")-1, SCE_REBOL_FILE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_REBOL_EMAIL", sizeof("SCINTILLA_REBOL_EMAIL")-1, SCE_REBOL_EMAIL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_REBOL_URL", sizeof("SCINTILLA_REBOL_URL")-1, SCE_REBOL_URL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_REBOL_DATE", sizeof("SCINTILLA_REBOL_DATE")-1, SCE_REBOL_DATE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_REBOL_TIME", sizeof("SCINTILLA_REBOL_TIME")-1, SCE_REBOL_TIME);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_REBOL_IDENTIFIER", sizeof("SCINTILLA_REBOL_IDENTIFIER")-1, SCE_REBOL_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_REBOL_WORD", sizeof("SCINTILLA_REBOL_WORD")-1, SCE_REBOL_WORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_REBOL_WORD2", sizeof("SCINTILLA_REBOL_WORD2")-1, SCE_REBOL_WORD2);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_REBOL_WORD3", sizeof("SCINTILLA_REBOL_WORD3")-1, SCE_REBOL_WORD3);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_REBOL_WORD4", sizeof("SCINTILLA_REBOL_WORD4")-1, SCE_REBOL_WORD4);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_REBOL_WORD5", sizeof("SCINTILLA_REBOL_WORD5")-1, SCE_REBOL_WORD5);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_REBOL_WORD6", sizeof("SCINTILLA_REBOL_WORD6")-1, SCE_REBOL_WORD6);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_REBOL_WORD7", sizeof("SCINTILLA_REBOL_WORD7")-1, SCE_REBOL_WORD7);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_REBOL_WORD8", sizeof("SCINTILLA_REBOL_WORD8")-1, SCE_REBOL_WORD8);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SQL_DEFAULT", sizeof("SCINTILLA_SQL_DEFAULT")-1, SCE_SQL_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SQL_COMMENT", sizeof("SCINTILLA_SQL_COMMENT")-1, SCE_SQL_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SQL_COMMENTLINE", sizeof("SCINTILLA_SQL_COMMENTLINE")-1, SCE_SQL_COMMENTLINE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SQL_COMMENTDOC", sizeof("SCINTILLA_SQL_COMMENTDOC")-1, SCE_SQL_COMMENTDOC);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SQL_NUMBER", sizeof("SCINTILLA_SQL_NUMBER")-1, SCE_SQL_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SQL_WORD", sizeof("SCINTILLA_SQL_WORD")-1, SCE_SQL_WORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SQL_STRING", sizeof("SCINTILLA_SQL_STRING")-1, SCE_SQL_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SQL_CHARACTER", sizeof("SCINTILLA_SQL_CHARACTER")-1, SCE_SQL_CHARACTER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SQL_SQLPLUS", sizeof("SCINTILLA_SQL_SQLPLUS")-1, SCE_SQL_SQLPLUS);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SQL_SQLPLUS_PROMPT", sizeof("SCINTILLA_SQL_SQLPLUS_PROMPT")-1, SCE_SQL_SQLPLUS_PROMPT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SQL_OPERATOR", sizeof("SCINTILLA_SQL_OPERATOR")-1, SCE_SQL_OPERATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SQL_IDENTIFIER", sizeof("SCINTILLA_SQL_IDENTIFIER")-1, SCE_SQL_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SQL_SQLPLUS_COMMENT", sizeof("SCINTILLA_SQL_SQLPLUS_COMMENT")-1, SCE_SQL_SQLPLUS_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SQL_COMMENTLINEDOC", sizeof("SCINTILLA_SQL_COMMENTLINEDOC")-1, SCE_SQL_COMMENTLINEDOC);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SQL_WORD2", sizeof("SCINTILLA_SQL_WORD2")-1, SCE_SQL_WORD2);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SQL_COMMENTDOCKEYWORD", sizeof("SCINTILLA_SQL_COMMENTDOCKEYWORD")-1, SCE_SQL_COMMENTDOCKEYWORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SQL_COMMENTDOCKEYWORDERROR", sizeof("SCINTILLA_SQL_COMMENTDOCKEYWORDERROR")-1, SCE_SQL_COMMENTDOCKEYWORDERROR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SQL_USER1", sizeof("SCINTILLA_SQL_USER1")-1, SCE_SQL_USER1);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SQL_USER2", sizeof("SCINTILLA_SQL_USER2")-1, SCE_SQL_USER2);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SQL_USER3", sizeof("SCINTILLA_SQL_USER3")-1, SCE_SQL_USER3);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SQL_USER4", sizeof("SCINTILLA_SQL_USER4")-1, SCE_SQL_USER4);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SQL_QUOTEDIDENTIFIER", sizeof("SCINTILLA_SQL_QUOTEDIDENTIFIER")-1, SCE_SQL_QUOTEDIDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ST_DEFAULT", sizeof("SCINTILLA_ST_DEFAULT")-1, SCE_ST_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ST_STRING", sizeof("SCINTILLA_ST_STRING")-1, SCE_ST_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ST_NUMBER", sizeof("SCINTILLA_ST_NUMBER")-1, SCE_ST_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ST_COMMENT", sizeof("SCINTILLA_ST_COMMENT")-1, SCE_ST_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ST_SYMBOL", sizeof("SCINTILLA_ST_SYMBOL")-1, SCE_ST_SYMBOL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ST_BINARY", sizeof("SCINTILLA_ST_BINARY")-1, SCE_ST_BINARY);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ST_BOOL", sizeof("SCINTILLA_ST_BOOL")-1, SCE_ST_BOOL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ST_SELF", sizeof("SCINTILLA_ST_SELF")-1, SCE_ST_SELF);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ST_SUPER", sizeof("SCINTILLA_ST_SUPER")-1, SCE_ST_SUPER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ST_NIL", sizeof("SCINTILLA_ST_NIL")-1, SCE_ST_NIL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ST_GLOBAL", sizeof("SCINTILLA_ST_GLOBAL")-1, SCE_ST_GLOBAL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ST_RETURN", sizeof("SCINTILLA_ST_RETURN")-1, SCE_ST_RETURN);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ST_SPECIAL", sizeof("SCINTILLA_ST_SPECIAL")-1, SCE_ST_SPECIAL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ST_KWSEND", sizeof("SCINTILLA_ST_KWSEND")-1, SCE_ST_KWSEND);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ST_ASSIGN", sizeof("SCINTILLA_ST_ASSIGN")-1, SCE_ST_ASSIGN);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ST_CHARACTER", sizeof("SCINTILLA_ST_CHARACTER")-1, SCE_ST_CHARACTER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_ST_SPEC_SEL", sizeof("SCINTILLA_ST_SPEC_SEL")-1, SCE_ST_SPEC_SEL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FS_DEFAULT", sizeof("SCINTILLA_FS_DEFAULT")-1, SCE_FS_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FS_COMMENT", sizeof("SCINTILLA_FS_COMMENT")-1, SCE_FS_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FS_COMMENTLINE", sizeof("SCINTILLA_FS_COMMENTLINE")-1, SCE_FS_COMMENTLINE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FS_COMMENTDOC", sizeof("SCINTILLA_FS_COMMENTDOC")-1, SCE_FS_COMMENTDOC);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FS_COMMENTLINEDOC", sizeof("SCINTILLA_FS_COMMENTLINEDOC")-1, SCE_FS_COMMENTLINEDOC);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FS_COMMENTDOCKEYWORD", sizeof("SCINTILLA_FS_COMMENTDOCKEYWORD")-1, SCE_FS_COMMENTDOCKEYWORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FS_COMMENTDOCKEYWORDERROR", sizeof("SCINTILLA_FS_COMMENTDOCKEYWORDERROR")-1, SCE_FS_COMMENTDOCKEYWORDERROR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FS_KEYWORD", sizeof("SCINTILLA_FS_KEYWORD")-1, SCE_FS_KEYWORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FS_KEYWORD2", sizeof("SCINTILLA_FS_KEYWORD2")-1, SCE_FS_KEYWORD2);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FS_KEYWORD3", sizeof("SCINTILLA_FS_KEYWORD3")-1, SCE_FS_KEYWORD3);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FS_KEYWORD4", sizeof("SCINTILLA_FS_KEYWORD4")-1, SCE_FS_KEYWORD4);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FS_NUMBER", sizeof("SCINTILLA_FS_NUMBER")-1, SCE_FS_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FS_STRING", sizeof("SCINTILLA_FS_STRING")-1, SCE_FS_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FS_PREPROCESSOR", sizeof("SCINTILLA_FS_PREPROCESSOR")-1, SCE_FS_PREPROCESSOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FS_OPERATOR", sizeof("SCINTILLA_FS_OPERATOR")-1, SCE_FS_OPERATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FS_IDENTIFIER", sizeof("SCINTILLA_FS_IDENTIFIER")-1, SCE_FS_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FS_DATE", sizeof("SCINTILLA_FS_DATE")-1, SCE_FS_DATE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FS_STRINGEOL", sizeof("SCINTILLA_FS_STRINGEOL")-1, SCE_FS_STRINGEOL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FS_CONSTANT", sizeof("SCINTILLA_FS_CONSTANT")-1, SCE_FS_CONSTANT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FS_ASM", sizeof("SCINTILLA_FS_ASM")-1, SCE_FS_ASM);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FS_LABEL", sizeof("SCINTILLA_FS_LABEL")-1, SCE_FS_LABEL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FS_ERROR", sizeof("SCINTILLA_FS_ERROR")-1, SCE_FS_ERROR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FS_HEXNUMBER", sizeof("SCINTILLA_FS_HEXNUMBER")-1, SCE_FS_HEXNUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_FS_BINNUMBER", sizeof("SCINTILLA_FS_BINNUMBER")-1, SCE_FS_BINNUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CSOUND_DEFAULT", sizeof("SCINTILLA_CSOUND_DEFAULT")-1, SCE_CSOUND_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CSOUND_COMMENT", sizeof("SCINTILLA_CSOUND_COMMENT")-1, SCE_CSOUND_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CSOUND_NUMBER", sizeof("SCINTILLA_CSOUND_NUMBER")-1, SCE_CSOUND_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CSOUND_OPERATOR", sizeof("SCINTILLA_CSOUND_OPERATOR")-1, SCE_CSOUND_OPERATOR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CSOUND_INSTR", sizeof("SCINTILLA_CSOUND_INSTR")-1, SCE_CSOUND_INSTR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CSOUND_IDENTIFIER", sizeof("SCINTILLA_CSOUND_IDENTIFIER")-1, SCE_CSOUND_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CSOUND_OPCODE", sizeof("SCINTILLA_CSOUND_OPCODE")-1, SCE_CSOUND_OPCODE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CSOUND_HEADERSTMT", sizeof("SCINTILLA_CSOUND_HEADERSTMT")-1, SCE_CSOUND_HEADERSTMT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CSOUND_USERKEYWORD", sizeof("SCINTILLA_CSOUND_USERKEYWORD")-1, SCE_CSOUND_USERKEYWORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CSOUND_COMMENTBLOCK", sizeof("SCINTILLA_CSOUND_COMMENTBLOCK")-1, SCE_CSOUND_COMMENTBLOCK);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CSOUND_PARAM", sizeof("SCINTILLA_CSOUND_PARAM")-1, SCE_CSOUND_PARAM);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CSOUND_ARATE_VAR", sizeof("SCINTILLA_CSOUND_ARATE_VAR")-1, SCE_CSOUND_ARATE_VAR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CSOUND_KRATE_VAR", sizeof("SCINTILLA_CSOUND_KRATE_VAR")-1, SCE_CSOUND_KRATE_VAR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CSOUND_IRATE_VAR", sizeof("SCINTILLA_CSOUND_IRATE_VAR")-1, SCE_CSOUND_IRATE_VAR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CSOUND_GLOBAL_VAR", sizeof("SCINTILLA_CSOUND_GLOBAL_VAR")-1, SCE_CSOUND_GLOBAL_VAR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CSOUND_STRINGEOL", sizeof("SCINTILLA_CSOUND_STRINGEOL")-1, SCE_CSOUND_STRINGEOL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_INNO_DEFAULT", sizeof("SCINTILLA_INNO_DEFAULT")-1, SCE_INNO_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_INNO_COMMENT", sizeof("SCINTILLA_INNO_COMMENT")-1, SCE_INNO_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_INNO_KEYWORD", sizeof("SCINTILLA_INNO_KEYWORD")-1, SCE_INNO_KEYWORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_INNO_PARAMETER", sizeof("SCINTILLA_INNO_PARAMETER")-1, SCE_INNO_PARAMETER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_INNO_SECTION", sizeof("SCINTILLA_INNO_SECTION")-1, SCE_INNO_SECTION);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_INNO_PREPROC", sizeof("SCINTILLA_INNO_PREPROC")-1, SCE_INNO_PREPROC);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_INNO_PREPROC_INLINE", sizeof("SCINTILLA_INNO_PREPROC_INLINE")-1, SCE_INNO_PREPROC_INLINE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_INNO_COMMENT_PASCAL", sizeof("SCINTILLA_INNO_COMMENT_PASCAL")-1, SCE_INNO_COMMENT_PASCAL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_INNO_KEYWORD_PASCAL", sizeof("SCINTILLA_INNO_KEYWORD_PASCAL")-1, SCE_INNO_KEYWORD_PASCAL);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_INNO_KEYWORD_USER", sizeof("SCINTILLA_INNO_KEYWORD_USER")-1, SCE_INNO_KEYWORD_USER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_INNO_STRING_DOUBLE", sizeof("SCINTILLA_INNO_STRING_DOUBLE")-1, SCE_INNO_STRING_DOUBLE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_INNO_STRING_SINGLE", sizeof("SCINTILLA_INNO_STRING_SINGLE")-1, SCE_INNO_STRING_SINGLE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_INNO_IDENTIFIER", sizeof("SCINTILLA_INNO_IDENTIFIER")-1, SCE_INNO_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_OPAL_SPACE", sizeof("SCINTILLA_OPAL_SPACE")-1, SCE_OPAL_SPACE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_OPAL_COMMENT_BLOCK", sizeof("SCINTILLA_OPAL_COMMENT_BLOCK")-1, SCE_OPAL_COMMENT_BLOCK);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_OPAL_COMMENT_LINE", sizeof("SCINTILLA_OPAL_COMMENT_LINE")-1, SCE_OPAL_COMMENT_LINE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_OPAL_INTEGER", sizeof("SCINTILLA_OPAL_INTEGER")-1, SCE_OPAL_INTEGER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_OPAL_KEYWORD", sizeof("SCINTILLA_OPAL_KEYWORD")-1, SCE_OPAL_KEYWORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_OPAL_SORT", sizeof("SCINTILLA_OPAL_SORT")-1, SCE_OPAL_SORT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_OPAL_STRING", sizeof("SCINTILLA_OPAL_STRING")-1, SCE_OPAL_STRING);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_OPAL_PAR", sizeof("SCINTILLA_OPAL_PAR")-1, SCE_OPAL_PAR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_OPAL_BOOL_CONST", sizeof("SCINTILLA_OPAL_BOOL_CONST")-1, SCE_OPAL_BOOL_CONST);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_OPAL_DEFAULT", sizeof("SCINTILLA_OPAL_DEFAULT")-1, SCE_OPAL_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SPICE_DEFAULT", sizeof("SCINTILLA_SPICE_DEFAULT")-1, SCE_SPICE_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SPICE_IDENTIFIER", sizeof("SCINTILLA_SPICE_IDENTIFIER")-1, SCE_SPICE_IDENTIFIER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SPICE_KEYWORD", sizeof("SCINTILLA_SPICE_KEYWORD")-1, SCE_SPICE_KEYWORD);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SPICE_KEYWORD2", sizeof("SCINTILLA_SPICE_KEYWORD2")-1, SCE_SPICE_KEYWORD2);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SPICE_KEYWORD3", sizeof("SCINTILLA_SPICE_KEYWORD3")-1, SCE_SPICE_KEYWORD3);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SPICE_NUMBER", sizeof("SCINTILLA_SPICE_NUMBER")-1, SCE_SPICE_NUMBER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SPICE_DELIMITER", sizeof("SCINTILLA_SPICE_DELIMITER")-1, SCE_SPICE_DELIMITER);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SPICE_VALUE", sizeof("SCINTILLA_SPICE_VALUE")-1, SCE_SPICE_VALUE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_SPICE_COMMENTLINE", sizeof("SCINTILLA_SPICE_COMMENTLINE")-1, SCE_SPICE_COMMENTLINE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CMAKE_DEFAULT", sizeof("SCINTILLA_CMAKE_DEFAULT")-1, SCE_CMAKE_DEFAULT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CMAKE_COMMENT", sizeof("SCINTILLA_CMAKE_COMMENT")-1, SCE_CMAKE_COMMENT);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CMAKE_STRINGDQ", sizeof("SCINTILLA_CMAKE_STRINGDQ")-1, SCE_CMAKE_STRINGDQ);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CMAKE_STRINGLQ", sizeof("SCINTILLA_CMAKE_STRINGLQ")-1, SCE_CMAKE_STRINGLQ);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CMAKE_STRINGRQ", sizeof("SCINTILLA_CMAKE_STRINGRQ")-1, SCE_CMAKE_STRINGRQ);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CMAKE_COMMANDS", sizeof("SCINTILLA_CMAKE_COMMANDS")-1, SCE_CMAKE_COMMANDS);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CMAKE_PARAMETERS", sizeof("SCINTILLA_CMAKE_PARAMETERS")-1, SCE_CMAKE_PARAMETERS);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CMAKE_VARIABLE", sizeof("SCINTILLA_CMAKE_VARIABLE")-1, SCE_CMAKE_VARIABLE);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CMAKE_USERDEFINED", sizeof("SCINTILLA_CMAKE_USERDEFINED")-1, SCE_CMAKE_USERDEFINED);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CMAKE_WHILEDEF", sizeof("SCINTILLA_CMAKE_WHILEDEF")-1, SCE_CMAKE_WHILEDEF);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CMAKE_FOREACHDEF", sizeof("SCINTILLA_CMAKE_FOREACHDEF")-1, SCE_CMAKE_FOREACHDEF);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CMAKE_IFDEFINEDEF", sizeof("SCINTILLA_CMAKE_IFDEFINEDEF")-1, SCE_CMAKE_IFDEFINEDEF);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CMAKE_MACRODEF", sizeof("SCINTILLA_CMAKE_MACRODEF")-1, SCE_CMAKE_MACRODEF);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CMAKE_STRINGVAR", sizeof("SCINTILLA_CMAKE_STRINGVAR")-1, SCE_CMAKE_STRINGVAR);
	phpg_register_int_constant(scintilla_ce, "SCINTILLA_CMAKE_NUMBER", sizeof("SCINTILLA_CMAKE_NUMBER")-1, SCE_CMAKE_NUMBER);
};

#endif /* HAVE_SCINTILLA */
