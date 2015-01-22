#include "mono/metadata/sgen-gc.h"
#include "mono/metadata/gc-internal-agnostic.h"

static GCVTable cons_vtable;

typedef struct _ConsObject ConsObject;

struct _ConsObject {
  GCObject gcobj;
  ConsObject* car;
  ConsObject* cdr;
};

#define LIST_LENGTH	100000000

static void
check_list (ConsObject *list, int length)
{
	ConsObject *iter = list;
	for (int i = 0; i < length - 1; ++i) {
		g_assert (!iter->car);
		g_assert (iter->cdr);
		iter = iter->cdr;
	}
	g_assert (!iter->cdr);
}

static void
run (void)
{
	ConsObject *cons = NULL;

	for (int i = 0; i < LIST_LENGTH; ++i) {
		GCObject *new_gcobj = sgen_alloc_obj (&cons_vtable, sizeof (ConsObject));
		g_assert (new_gcobj->vtable == &cons_vtable);
		ConsObject *new_cons = (ConsObject*)new_gcobj;
		g_assert (!new_cons->car && !new_cons->cdr);
		mono_gc_wbarrier_generic_store (&new_cons->cdr, new_gcobj);
		cons = new_cons;
		//sgen_gc_collect (GENERATION_NURSERY);
		//check_list (cons, i + 1);
	}
	check_list (cons, LIST_LENGTH);
}

int
main (void)
{
	void *dummy;
	gsize cons_bitmap = 6;

	sgen_gc_init ();
	sgen_thread_register (&main_thread_info, &dummy);

	cons_vtable.descriptor = (mword)mono_gc_make_descr_for_object (&cons_bitmap, 2, sizeof (ConsObject));

	run ();

	return 0;
}
