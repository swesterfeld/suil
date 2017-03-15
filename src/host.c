/*
  Copyright 2011-2015 David Robillard <http://drobilla.net>

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

#include "./suil_internal.h"
#include <stdio.h>

SUIL_API
SuilHost*
suil_host_new(SuilPortWriteFunc       write_func,
              SuilPortIndexFunc       index_func,
              SuilPortSubscribeFunc   subscribe_func,
              SuilPortUnsubscribeFunc unsubscribe_func)
{
	SuilHost* host = (SuilHost*)calloc(1, sizeof(struct SuilHostImpl));
	host->write_func       = write_func;
	host->index_func       = index_func;
	host->subscribe_func   = subscribe_func;
	host->unsubscribe_func = unsubscribe_func;
	return host;
}

SUIL_API
void
suil_host_set_touch_func(SuilHost*     host,
                         SuilTouchFunc touch_func)
{
	host->touch_func = touch_func;
}

SUIL_API
void
suil_host_free(SuilHost* host)
{
	if (host) {
		if (host->gtk_lib) {
			dlclose(host->gtk_lib);
		}
		free(host);
	}
}

static void
suil_init_module (const char *module_name, const char *func_name)
{
	void *lib;

	SuilVoidFunc suil_init_func = (SuilVoidFunc)suil_module_open (module_name, func_name, &lib);
	if (suil_init_func) {
		(*suil_init_func)();
	}

}

SUIL_API
void
suil_init()
{
#if SUIL_WITH_X11
	suil_init_module ("suil_x11", "suil_init_x11");
#endif

}
