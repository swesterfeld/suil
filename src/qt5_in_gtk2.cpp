/*
  Copyright 2011-2017 David Robillard <http://drobilla.net>

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THIS SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include <gtk/gtk.h>

#include <QApplication>
#include <QVBoxLayout>
#include <QWidget>
#include <QWindow>

#include "./suil_internal.h"

extern "C" {

#define SUIL_TYPE_QT5_WRAPPER (suil_qt5_wrapper_get_type())
#define SUIL_QT5_WRAPPER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SUIL_TYPE_QT5_WRAPPER, SuilQt5Wrapper))

typedef struct _SuilQt5Wrapper      SuilQt5Wrapper;
typedef struct _SuilQt5WrapperClass SuilQt5WrapperClass;

struct _SuilQt5Wrapper
{
	GtkSocket     socket;
	QApplication* app;
	QWidget*      qembed;
	SuilWrapper*  wrapper;
	SuilInstance* instance;
};

struct _SuilQt5WrapperClass
{
	GtkSocketClass parent_class;
};

GType suil_qt5_wrapper_get_type(void); // Accessor for SUIL_TYPE_QT5_WRAPPER

G_DEFINE_TYPE(SuilQt5Wrapper, suil_qt5_wrapper, GTK_TYPE_SOCKET)

static void
suil_qt5_wrapper_finalize(GObject* gobject)
{
	SuilQt5Wrapper* const self = SUIL_QT5_WRAPPER(gobject);

	if (self->instance->handle) {
		self->instance->descriptor->cleanup(self->instance->handle);
		self->instance->handle = NULL;
	}

	delete self->qembed;

	self->qembed        = NULL;
	self->app           = NULL;
	self->wrapper->impl = NULL;

	G_OBJECT_CLASS(suil_qt5_wrapper_parent_class)->finalize(gobject);
}

static void
suil_qt5_wrapper_class_init(SuilQt5WrapperClass* klass)
{
	GObjectClass* const gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->finalize = suil_qt5_wrapper_finalize;
}

static void
suil_qt5_wrapper_init(SuilQt5Wrapper* self)
{
	self->app      = NULL;
	self->qembed   = NULL;
	self->instance = NULL;
}

static void
suil_qt5_wrapper_realize(GtkWidget* w, gpointer data)
{
	SuilQt5Wrapper* const wrap = SUIL_QT5_WRAPPER(w);
	GtkSocket* const     s    = GTK_SOCKET(w);
	const WId            id   = gtk_socket_get_id(s);

	wrap->qembed->winId();
	wrap->qembed->windowHandle()->setParent(QWindow::fromWinId(id));
	wrap->qembed->show();
}

static int
wrapper_wrap(SuilWrapper* wrapper, SuilInstance* instance)
{
	SuilQt5Wrapper* const wrap = SUIL_QT5_WRAPPER(wrapper->impl);

	wrap->qembed   = new QWidget();
	wrap->wrapper  = wrapper;
	wrap->instance = instance;

	QWidget*     qwidget = (QWidget*)instance->ui_widget;
	QVBoxLayout* layout  = new QVBoxLayout(wrap->qembed);
	layout->addWidget(qwidget);

	qwidget->setParent(wrap->qembed);

	g_signal_connect_after(G_OBJECT(wrap), "realize",
	                       G_CALLBACK(suil_qt5_wrapper_realize), NULL);

	instance->host_widget = GTK_WIDGET(wrap);

	return 0;
}

static void
wrapper_free(SuilWrapper* wrapper)
{
	if (wrapper->impl) {
		SuilQt5Wrapper* const wrap = SUIL_QT5_WRAPPER(wrapper->impl);
		gtk_object_destroy(GTK_OBJECT(wrap));
	}
}

SUIL_LIB_EXPORT
SuilWrapper*
suil_wrapper_new(SuilHost*      host,
                 const char*    host_type_uri,
                 const char*    ui_type_uri,
                 LV2_Feature*** features,
                 unsigned       n_features)
{
	SuilWrapper* wrapper = (SuilWrapper*)calloc(1, sizeof(SuilWrapper));
	wrapper->wrap = wrapper_wrap;
	wrapper->free = wrapper_free;

	SuilQt5Wrapper* const wrap = SUIL_QT5_WRAPPER(
		g_object_new(SUIL_TYPE_QT5_WRAPPER, NULL));

	if (qApp) {
		wrap->app = qApp;
	} else {
		wrap->app = new QApplication(host->argc, host->argv, true);
	}

	wrap->wrapper = NULL;
	wrapper->impl = wrap;

	return wrapper;
}

} // extern "C"
