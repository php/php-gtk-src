#ifndef GTKSCINTILLA_H
#define GTKSCINTILLA_H

#if HAVE_SCINTILLA

#define PLAT_GTK 2

#ifdef WIN32
#include "scintilla/Scintilla.h"
#include "scintilla/ScintillaWidget.h"
#include "scintilla/SciLexer.h"
#else
#include "libscintilla/include/Scintilla.h"
#include "libscintilla/include/ScintillaWidget.h"
#include "libscintilla/include/SciLexer.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif
#define GTK_TYPE_SCINTILLA            (gtk_scintilla_get_type ())
#define GTK_SCINTILLA(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_SCINTILLA, GtkScintilla))
#define GTK_SCINTILLA_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_SCINTILLA, GtkScintillaClass))
#define GTK_IS_SCINTILLA(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_SCINTILLA))
#define GTK_IS_SCINTILLA_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_SCINTILLA))
#define GTK_SCINTILLA_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), TYPE_GTK_SCINTILLA, ScintillaClass))

#define SCINTILLA_MESSAGE(message,var1,var2)	(GTK_SCINTILLA((PHPG_GOBJECT(this_ptr))))->pSciMsg(GTK_SCINTILLA((PHPG_GOBJECT(this_ptr)))->pSciWndData, message, var1, var2)
typedef struct _GtkScintilla      GtkScintilla;
typedef struct _GtkScintillaClass GtkScintillaClass;

struct _GtkScintilla {
    GtkFrame parent;
    
    GtkWidget *scintilla;
    
		SciFnDirect pSciMsg;
		sptr_t pSciWndData;
		
    GtkAccelGroup *accel_group;
};

struct _GtkScintillaClass {
    GtkFrameClass parent_class;

    void (* style_needed)        (GtkWidget *w, gint position);
    void (* char_added)          (GtkWidget *w, gint ch);
    void (* save_point_reached)  (GtkWidget *w);
    void (* save_point_left)     (GtkWidget *w);
    void (* modify_attempt_ro)   (GtkWidget *w);
    void (* key)                 (GtkWidget *w, gint ch, gint modifiers);
    void (* double_click)        (GtkWidget *w);
    void (* update_ui)           (GtkWidget *w);
    void (* modified)            (GtkWidget *w);
    void (* macro_record)        (GtkWidget *w, gint message, gint wparam, gint lparam);
    void (* margin_click)        (GtkWidget *w, gint modifiers, gint position, gint margin);
    void (* need_shown)          (GtkWidget *w, gint position, gint lenght);
    void (* painted)             (GtkWidget *w);
    void (* user_list_selection) (GtkWidget *w, gint type, gchar *text);
    void (* uri_dropped)         (GtkWidget *w, gchar *text);
    void (* dwell_start)         (GtkWidget *w, gint position);
    void (* dwell_end)           (GtkWidget *w, gint position);
    void (* zoom)                (GtkWidget *w);
};

GType      gtk_scintilla_get_type       (void) G_GNUC_CONST;
GtkWidget *gtk_scintilla_new            (void);

#ifdef __cplusplus
}
#endif

#endif	/* HAVE_SCINTILLA */

#endif	/* GTKSCINTILLA_H */
