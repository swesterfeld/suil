/*
  Copyright 2007-2012 David Robillard <http://drobilla.net>

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

#ifndef SUIL_INTERNAL_H
#define SUIL_INTERNAL_H

#include <assert.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#define dlopen(path, flags) LoadLibrary(path)
#define dlclose(lib) FreeLibrary(lib)
#define dlsym GetProcAddress
static inline char* dlerror(void) { return "Unknown error"; }
#else
#include <dlfcn.h>
#endif

#include "lv2/lv2plug.in/ns/extensions/ui/ui.h"
#ifdef HAVE_LV2_UI_RESIZE
#include "lv2/lv2plug.in/ns/ext/ui-resize/ui-resize.h"
#endif

#include "suil/suil.h"

#define NS_UI "http://lv2plug.in/ns/extensions/ui#"

#define SUIL_ERRORF(fmt, ...) fprintf(stderr, "error: %s: " fmt, \
                                      __func__, __VA_ARGS__)

struct SuilHostImpl {
	SuilPortWriteFunc       write_func;
	SuilPortIndexFunc       index_func;
	SuilPortSubscribeFunc   subscribe_func;
	SuilPortUnsubscribeFunc unsubscribe_func;
	void*                   gtk_lib;
};

struct _SuilWrapper;

typedef void (*SuilWrapperFreeFunc)(struct _SuilWrapper*);

typedef int (*SuilWrapperWrapFunc)(struct _SuilWrapper* wrapper,
                                   SuilInstance*        instance);

typedef struct _SuilWrapper {
	SuilWrapperWrapFunc wrap;
	SuilWrapperFreeFunc free;
	void*               lib;
	LV2_Feature**       features;
	void*               impl;
#ifdef HAVE_LV2_UI_RESIZE
	LV2_UI_Resize_Feature resize;
#endif
} SuilWrapper;

struct SuilInstanceImpl {
	void*                   lib_handle;
	const LV2UI_Descriptor* descriptor;
	LV2UI_Handle            handle;
	SuilWrapper*            wrapper;
	SuilWidget              ui_widget;
	SuilWidget              host_widget;
};

/**
   The type of the suil_wrapper_new entry point in a wrapper module.

   This constructs a SuilWrapper which contains everything necessary
   to wrap a widget, including a possibly extended features array to
   be used for instantiating the UI.
*/
typedef SuilWrapper* (*SuilWrapperNewFunc)(
	SuilHost*                 host,
	const char*               host_type_uri,
	const char*               ui_type_uri,
	const LV2_Feature* const* features);

typedef void (*SuilVoidFunc)();

/** dlsym wrapper to return a function pointer (without annoying warning) */
static inline SuilVoidFunc
suil_dlfunc(void* handle, const char* symbol)
{
	typedef SuilVoidFunc (*VoidFuncGetter)(void*, const char*);
	VoidFuncGetter dlfunc = (VoidFuncGetter)dlsym;
	return dlfunc(handle, symbol);
}

#endif  // SUIL_INTERNAL_H
