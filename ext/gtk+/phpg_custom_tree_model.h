/*
 * PHP-GTK - The PHP language bindings for GTK+
 *
 * Copyright (C) 2001-2006 Andrei Zmievski <andrei@php.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
 
/* $Id$: */

#include <gtk/gtktreemodel.h>

#define PHPG_TYPE_CUSTOM_TREE_MODEL            (pygtk_custom_tree_model_get_type())
#define PHPG_CUSTOM_TREE_MODEL(object)         (G_TYPE_CHECK_INSTANCE_CAST((object), PHPG_TYPE_CUSTOM_TREE_MODEL, PhpGtkCustomTreeModel))
#define PHPG_CUSTOM_TREE_MODEL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), PHPG_TYPE_CUSTOM_TREE_MODEL, PhpGtkCustomTreeModelClass))
#define PHPG_IS_CUSTOM_TREE_MODEL(object)      (G_TYPE_CHECK_INSTANCE_TYPE((object), PHPG_TYPE_CUSTOM_TREE_MODEL))
#define PHPG_IS_CUSTOM_TREE_MODEL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), PHPG_TYPE_CUSTOM_TREE_MODEL))
#define PHPG_CUSTOM_TREE_MODEL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), PHPG_TYPE_CUSTOM_TREE_MODEL, PhpGtkCustomTreeModelClass))

typedef struct _PhpGtkCustomTreeModel PhpGtkCustomTreeModel;
typedef struct _PhpGtkCustomTreeModelClass PhpGtkCustomTreeModelClass;

struct _PhpGtkCustomTreeModel {
    GObject parent_instance;

    gint stamp;
};

struct _PhpGtkCustomTreeModelClass {
    GObjectClass parent_class;
};

GType                   phpg_custom_tree_model_get_type (void);
PhpGtkCustomTreeModel * phpg_custom_tree_model_new      (void);
void                    phpg_custom_tree_model_invalidate_iters(PhpGtkCustomTreeModel *);
gboolean                phpg_custom_tree_model_iter_is_valid(PhpGtkCustomTreeModel *, GtkTreeIter *);

