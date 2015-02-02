#ifdef SGEN_DEFINE_OBJECT_VTABLE

#include <pthread.h>
#include <setjmp.h>

typedef struct {
	mword descriptor;
} GCVTable;

typedef struct _GCObject GCObject;
struct _GCObject {
	GCVTable *vtable;
};

typedef struct {
	GCVTable *vtable;
	size_t size;
} GCArray;

typedef struct {
	jmp_buf registers;
	void *stack_bottom;
	void *stack_top;
} SgenClientThreadInfo;

#define SGEN_LOAD_VTABLE_UNCHECKED(obj)	((void*)(((GCObject*)(obj))->vtable))

static inline mword
sgen_vtable_get_descriptor (GCVTable *vtable)
{
	return vtable->descriptor;
}

static mword
sgen_client_slow_object_get_size (GCVTable *vtable, GCObject* o)
{
	int desc_type = vtable->descriptor & DESC_TYPE_MASK;
	if (desc_type == DESC_TYPE_VECTOR)
		return ((GCArray*)o)->size;
	else if (desc_type == DESC_TYPE_BITMAP)
	  return vtable->descriptor & ~DESC_TYPE_MASK;
	return sizeof (GCObject);
}

static MONO_NEVER_INLINE mword
sgen_client_par_object_get_size (GCVTable *vtable, GCObject* o)
{
	return sgen_client_slow_object_get_size (vtable, o);
}

typedef pthread_t MonoNativeThreadId;

#else

extern pthread_key_t thread_info_key;
#define TLAB_ACCESS_INIT	SgenThreadInfo *__thread_info__ = mono_thread_info_current ()

typedef void* mono_native_thread_return_t;

#define mono_native_thread_id_get	pthread_self

static inline void
mono_native_thread_create (MonoNativeThreadId *thread, mono_native_thread_return_t (*func) (void*), void *user_data)
{
	int result = pthread_create (thread, NULL, func, user_data);
	g_assert (!result);
}

#define ENTER_CRITICAL_REGION
#define EXIT_CRITICAL_REGION

extern SgenThreadInfo main_thread_info;

#define FOREACH_THREAD(thread)	thread = &main_thread_info;
#define END_FOREACH_THREAD

typedef struct
{
	int value;
        pthread_mutex_t access;
	pthread_cond_t sig;
} SgenSemaphore;

static inline void
SGEN_SEMAPHORE_INIT (SgenSemaphore *sem, int initial)
{
	sem->value = initial;
	pthread_mutex_init(&sem->access, NULL);
	pthread_cond_init(&sem->sig, NULL);
}

static inline void
SGEN_SEMAPHORE_POST (SgenSemaphore* sem)
{
	g_assert (sem);

	pthread_mutex_lock (&sem->access);
	++sem->value;
	pthread_cond_signal (&sem->sig);
	pthread_mutex_unlock (&sem->access);
}

static inline void
SGEN_SEMAPHORE_WAIT (SgenSemaphore* sem)
{
	g_assert (sem);

	pthread_mutex_lock (&sem->access);
	while (sem->value < 1)
		pthread_cond_wait (&sem->sig, &sem->access);
	--sem->value;
	pthread_mutex_unlock (&sem->access);
}

#include <sys/time.h>

#define SGEN_TV_DECLARE(tv)	struct timeval tv
#define SGEN_TV_GETTIME(tv)	gettimeofday (&(tv), NULL)

static inline gint64
SGEN_TV_ELAPSED (struct timeval a, struct timeval b)
{
	gint64 elapsed = (b.tv_sec - a.tv_sec) * (gint64)1000000;
	elapsed += b.tv_usec;
	elapsed -= a.tv_usec;
	return elapsed * 10;
}

enum {
	INTERNAL_MEM_MAX = INTERNAL_MEM_FIRST_CLIENT
};

#define SGEN_CLIENT_OBJECT_HEADER_SIZE		(sizeof (GCObject))
#define SGEN_CLIENT_MINIMUM_OBJECT_SIZE		SGEN_CLIENT_OBJECT_HEADER_SIZE

static inline size_t
sgen_client_array_element_size (GCVTable *gc_vtable)
{
	g_assert_not_reached ();
}

static inline char*
sgen_client_array_data_start (GCObject *obj)
{
	g_assert_not_reached ();
}

static inline size_t
sgen_client_array_length (GCObject *obj)
{
	g_assert_not_reached ();
}

static inline void
sgen_client_pre_copy_checks (char *destination, GCVTable *gc_vtable, void *obj, mword objsize)
{
}

static inline void
sgen_client_update_copied_object (char *destination, GCVTable *gc_vtable, void *obj, mword objsize)
{
}

#define sgen_client_wbarrier_generic_nostore_check(ptr)

#endif
