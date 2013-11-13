#include "sawlog.h"
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <time.h>
#include <pthread.h>

typedef enum {
	cl_none,
	cl_black,
	cl_darkgrey,
	cl_darkred,
	cl_red,
	cl_darkgreen,
	cl_green,
	cl_darkyellow,
	cl_yellow,
	cl_darkblue,
	cl_blue,
	cl_darkmagenta,
	cl_magenta,
	cl_darkcyan,
	cl_cyan,
	cl_grey,
	cl_white
} color_t;

typedef struct log_entry_t {
	char *msg;
	int line;
	int level;
	const char *func;
	const char *file;
	struct timeval tv;
	pthread_t thread;
	struct log_entry_t *next;
} log_entry;

// The message queue
log_entry *log_qfirst = NULL;
log_entry *log_qlast = NULL;
static int log_thread_quit = 0;

// condition variable to signal there is content in the queue
pthread_cond_t queue_data_present = PTHREAD_COND_INITIALIZER;
pthread_cond_t wait_log_thread = PTHREAD_COND_INITIALIZER;

// Background thread
pthread_t log_thread;

// Default config
static int log_level = LOG_DEBUG;
static FILE **log_out = NULL;
static int do_color = 0;

// Configure date/time output
static color_t cl_datetime = cl_none;
static color_t cl_datetime_br = cl_darkgrey;
static char date_time_s = '[';
static char date_time_e = ']';
static char *format_date = "%04d-%02d-%02d";
static char *format_time = "%02d:%02d:%02d+%04ld";


// Configure thread-id output
static color_t cl_thread_id_br = cl_darkgrey;
static char thread_id_s = '[';
static char thread_id_e = ']';

// Configure line end output
static color_t cl_lineend = cl_none;
static color_t cl_lineend_br = cl_darkgrey;
static char line_end_s = '[';
static char line_end_e = ']';

// Mutex for threading - lock the queue array
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutexattr_t mutex_atr;

// Function definitions
static void init_log();
static void setColor(color_t color);
static void log_time(struct timeval *tv);
static void set_lvlcolor(int lvl);
static void log_type(int lvl);
static void log_fileline(const char* func, const char *file, int line);
static void log_lineend();

static log_entry *log_entry_create(int lvl, const char *func, const char *file, int line);
static void log_entry_push(log_entry *entry);
static log_entry *log_entry_pop();
static void log_entry_destroy(log_entry *entry);

static void log_wait_thread();
static void log_thread_process();
static void log_unlock();
static int log_lock();


static inline void logthread_ready()
{
	pthread_mutex_t tmpm = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_lock(&tmpm);
	pthread_cond_wait(&wait_log_thread, &tmpm);
	//printf("LOGGER: Logger thread ready\n"); fflush(stdout);
	pthread_mutex_unlock(&tmpm);
}

static inline void init_log()
{
	static int init = 0;
	if (init) return;
	init = 1;
	
	// Threading support
	pthread_mutexattr_init(&mutex_atr);
	//pthread_mutexattr_settype(&mutex_atr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutexattr_settype(&mutex_atr, PTHREAD_MUTEX_RECURSIVE_NP);
	pthread_mutex_init(&mutex, &mutex_atr);
	log_lock();

	// Must be called before localtime_r
	tzset();

	log_setoutput(NULL);
#ifdef LOG_THREADED
	if (pthread_create(&log_thread, NULL, (void *(*)(void *))log_thread_process, NULL)) {
		fprintf(stderr, "LOGGER: ERROR CREATING BACKGROUND THREAD!\n");
		exit(1);
	} else {
		atexit(log_wait_thread);
		logthread_ready();
	}
#endif
	log_unlock();
}


static void setColor(color_t color)
{
	const char *cs = "";
	if (!do_color) return;

	switch(color) {
	case cl_none       : cs = "0"; break;
	case cl_black      : cs = "30"; break;
	case cl_darkgrey   : cs = "30;1"; break;
	case cl_darkred    : cs = "31"; break;
	case cl_red        : cs = "31;1"; break;
	case cl_darkgreen  : cs = "32"; break;
	case cl_green      : cs = "32;1"; break;
	case cl_darkyellow : cs = "33"; break;
	case cl_yellow     : cs = "33;1"; break;
	case cl_darkblue   : cs = "34"; break;
	case cl_blue       : cs = "34;1"; break;
	case cl_darkmagenta: cs = "35"; break;
	case cl_magenta    : cs = "35;1"; break;
	case cl_darkcyan   : cs = "36"; break;
	case cl_cyan       : cs = "36;1"; break;
	case cl_grey       : cs = "37"; break;
	case cl_white      : cs = "37;1"; break;
	}
	if (color != cl_none) {
		fwrite("\x1b[0m",4,1,*log_out);
	}
	fputc('\x1b', *log_out);
	fputc('[', *log_out);
	fwrite(cs, strlen(cs), 1, *log_out);
	fputc('m', *log_out);
}


static void log_time(struct timeval *tv)
{
	//struct timeval tv;
	struct tm tms;
	time_t t;

	//gettimeofday(&tv, NULL);
	t = (time_t)tv->tv_sec;
	localtime_r(&t, &tms);

	setColor(cl_datetime_br);
	putc(date_time_s, *log_out);
	setColor(cl_datetime);
	fprintf(*log_out, format_date, 1900+tms.tm_year, tms.tm_mon + 1, tms.tm_mday);
	putc(' ', *log_out);
	fprintf(*log_out, format_time, tms.tm_hour, tms.tm_min, tms.tm_sec, (tv->tv_usec / 100) );
	setColor(cl_datetime_br);
	putc(date_time_e, *log_out);
}

static void set_lvlcolor(int lvl)
{
	switch(lvl) {
		case LOG_ERROR:
			setColor(cl_darkred);
			break;
		case LOG_WARNING:
			setColor(cl_red);
			break;
		case LOG_NOTICE:
			setColor(cl_green);
			break;
		case LOG_INFO:
			setColor(cl_none);
			break;
		case LOG_DEBUG:
			setColor(cl_darkgrey);
			break;
		default:
			setColor(cl_none);
	}

}


// Set psuedo-random, always the same color based on a number (used with thread-id)
void setNumColor(unsigned long col)
{
	switch ((int)(col % 13UL)) { // use a prime number :)
	case 0:
		setColor(cl_darkgrey);
		break;
	case 1:
		setColor(cl_darkred);
		break;
	case 2:
		setColor(cl_red);
		break;
	case 3:
		setColor(cl_darkgreen);
		break;
	case 4:
		setColor(cl_green);
		break;
	case 5:
		setColor(cl_darkyellow);
		break;
	case 6:
		setColor(cl_yellow);
		break;
	case 7:
		setColor(cl_darkblue);
		break;
	case 8:
		setColor(cl_blue);
		break;
	case 9:
		setColor(cl_darkmagenta);
		break;
	case 10:
		setColor(cl_magenta);
		break;
	case 11:
		setColor(cl_darkcyan);
		break;
	case 12:
		setColor(cl_cyan);
		break;
	}
}


void log_threadid(pthread_t threadid)
{
	setColor(cl_thread_id_br);
	fputc(thread_id_s, *log_out);
	
	// Set random color
	setNumColor(threadid);
	// FIXME: Not sure if this is ok what I do here, thread id's always seem to be in the form of 7fxxxxxxxx00
	//        probably memory manager related so what I do here is probably wrong :p
	fprintf(*log_out, "%07lx", 0xFFFFFFF & (threadid >> 12) );
	//fprintf(*log_out, "%07lx", threadid );

	setColor(cl_thread_id_br);
	fputc(thread_id_e, *log_out);
	setColor(cl_none);
}


static void log_type(int lvl)
{
	set_lvlcolor(lvl);
	switch(lvl) {
		case LOG_ERROR:
			fwrite("ERR ", 4, 1, *log_out);
			break;
		case LOG_WARNING:
			fwrite("WAR ", 4, 1, *log_out);
			break;
		case LOG_NOTICE:
			fwrite("NOT ", 4, 1, *log_out);
			break;
		case LOG_INFO:
			fwrite("INF ", 4, 1, *log_out);
			break;
		case LOG_DEBUG:
			fwrite("DBG ", 4, 1, *log_out);
			break;
	}
	setColor(cl_none);
}


static void log_fileline(const char* func, const char *file, int line)
{
	// Output file/line number
	fputc(' ', *log_out);
	setColor(cl_lineend_br);
	fputc(line_end_s, *log_out);

	setColor(cl_lineend);
	fprintf(*log_out, "%s:%s+%d", func, file, line);

	setColor(cl_lineend_br);
	fputc(line_end_e, *log_out);
}


static void log_lineend()
{
	setColor(cl_none);
	putc('\n', *log_out);
}


static void log_unlock()
{
	pthread_mutex_unlock(&mutex);
}


static int log_lock()
{
	return pthread_mutex_lock(&mutex);
}


static log_entry *log_entry_create(int lvl, const char *func, const char *file, int line)
{
	log_entry *ret = malloc(sizeof(log_entry));

	gettimeofday(&ret->tv, NULL);
	ret->level = lvl;
	ret->func = func;
	ret->file = file;
	ret->line = line;
	ret->thread = pthread_self();
	ret->msg = NULL;
	return ret;
}

static void log_entry_push(log_entry *entry)
{
	log_lock();
	// Add to queue
	entry->next = NULL;

	if (log_qlast != NULL) {
		log_qlast->next = entry;
	}
	log_qlast = entry;

	if (log_qfirst == NULL) {
		log_qfirst = entry;
	}
	log_unlock();
	pthread_cond_signal(&queue_data_present);
}

static log_entry *log_entry_pop()
{
	log_entry *ret = NULL;
	log_lock();

	ret = log_qfirst;
	if (ret != NULL) {
		log_qfirst = ret->next;
		ret->next = NULL;
	} else {
		ret = NULL;
	}
	if (log_qlast == ret) {
		log_qlast = NULL;
	}
	
	log_unlock();
	return ret;
}

static void log_entry_destroy(log_entry *entry)
{
	if (entry) {
		if (entry->msg) {
			free(entry->msg);
			entry->msg = NULL;
		}
		memset(entry, 0, sizeof(log_entry));
		free(entry);
	}
}

static void log_wait_thread()
{
	//printf("Killing threads...\n");
	log_lock();
	log_thread_quit = 1;
	log_unlock();
	pthread_cond_signal(&queue_data_present);

	pthread_join(log_thread, NULL);
	//printf("Threads finished\n");
}

// Background thread for logging
static void log_thread_process(void *arg)
{
	int rv;
	log_entry *entry;
	(void)(arg);
	
	// Signal the init function that the logger thread is ready
	pthread_cond_signal(&wait_log_thread);

	//printf("-- BACKGROUND THREAD STARTED\n");
	for (;;) {
		if (log_lock()) {
			break;
		}
		if (log_qfirst != NULL) {
			rv = 0;
		} else {
			rv = pthread_cond_wait(&queue_data_present, &mutex);
		}
		if (!rv) {
			// Get the next item
			//printf("-- GET ITEM\n");
			entry =  log_entry_pop();
			log_unlock();
			while (entry != NULL) {
				//printf("-- PRINT ITEM\n");
				// Log the type of the thing to log
				log_type(entry->level);
				// Log the timestamp
				log_time(&(entry->tv));
				// Log thread ID:
				log_threadid(entry->thread);
				// Output formatted string
				putc(' ', *log_out);
				set_lvlcolor(entry->level);
				fputs(entry->msg, *log_out);
				// Log function, file and line number
				log_fileline(entry->func, entry->file, entry->line);
				// Log EOL
				log_lineend();
				// Destroy the current instance
				log_entry_destroy(entry);
				
				// Get the next item (if present)
				entry = log_entry_pop();
			}
			if (log_thread_quit) {
				//printf("-- EXIT REQUESTED\n");
				break;
			}
		} else {
			//printf("-- SIGNAL FAILED\n");
			log_unlock();
		}
	}
	//printf("-- BACKGROUND THREAD EXIT\n");
	pthread_exit(NULL);
}


void _logout_threaded(int lvl, const char *file, int line, const char *func, const char *format, ...)
{
	va_list ap;
	int ret, size = 120;
	log_entry *log_line;

	if (lvl > log_level) return;
	//printf("THREADED LOG\n");

	// Get the time as soon as possible
	log_line = log_entry_create(lvl, func, file, line);

	// Init the log if needed
	init_log();

	// Format output string. First try with a default sized buffer to avoid double work
	log_line->msg = malloc(size);
	for (;;) {
		va_start(ap, format);
		ret = vsnprintf(log_line->msg, size, format, ap);
		va_end(ap);
		if ((ret > -1) && ( ret < size)) {
			break;
		}
		if (ret > -1) {
			size = ret + 1; // glibc 2.1: we get exactly the size required
		} else {
			size *= 2; // old glibc: double the buffer.
		}
		log_line->msg = realloc(log_line->msg, size);
	}

	log_entry_push(log_line);
}

void _logout(int lvl, const char *file, int line, const char *func, const char *format, ...)
{
	va_list ap, ap_t;
	char *buf = NULL;
	int ret;
	struct timeval tv;

	//printf("UNTHREADED LOG\n");
	if (lvl > log_level) return;

	// Get the time as soon as possible
	gettimeofday(&tv, NULL);

	// Init the log if needed
	init_log();

	// Format output string
	va_start(ap, format);
	va_copy(ap_t, ap);
	ret = vsnprintf(buf, 0, format, ap_t);
	buf = malloc(ret+1);
	ret = vsnprintf(buf, ret+1, format, ap);
	va_end(ap);

	log_lock();
	// TODO: Queue to background thread
	
	// Log the type of the thing to log
	log_type(lvl);
	// Log the timestamp
	log_time(&tv);
	// Output formatted string
	putc(' ', *log_out);
	set_lvlcolor(lvl);
	fputs(buf, *log_out);
	// Log function, file and line number
	log_fileline(func, file, line);
	// Log EOL
	log_lineend();
	// Unlock the thread
	log_unlock();
	
	// Free the buffer
	if (buf) free(buf);
}


/////////////////////////////////////////////////////////////////////////////
// Public functions
/////////////////////////////////////////////////////////////////////////////

void log_setlevel(int lvl)
{
	log_level = lvl;
}


void log_setoutput(FILE *out)
{
	log_lock();
	if (out == NULL) {
		log_out = NULL;
	} else {
		log_out = &out;
	}
	if (log_out == NULL) {
		log_out = &stdout;
	}
	do_color = isatty(fileno(*log_out));
	log_unlock();
}


/////////////////////////////////////////////////////////////////////////////
// Stress test functions
/////////////////////////////////////////////////////////////////////////////

#ifdef DEBUG_SAWLOG_C

//#define SPAM_THREADS 200
#define SPAM_THREADS LOG_LVL_MAX
//#define SPAM_THREADS 1
#define SPAM_LINES 100

void random_sleep()
{
	struct timespec rqtp;
	rqtp.tv_sec = 0;
	rqtp.tv_nsec = (rand() % 100) * 1000L;

	nanosleep(&rqtp, NULL);
}

void spam_log(void *arg)
{
	int i;
	int tid = (*(int*)arg);
	int lvl = ((*(int*)arg) % LOG_LVL_MAX) + 1;
	for (i = 0; i < 10; i++) {
		random_sleep();
	}
	for (i = 0; i < SPAM_LINES; i++) {
		if (tid % 2) random_sleep();
		LOG(lvl, "[%d/%d] SPAM LOG: %d", lvl, tid, i);
	}
	pthread_exit(NULL);
}


int main()
{
#ifdef SPAM_THREADS
	int thread_lvl[SPAM_THREADS];
	pthread_t spammers[SPAM_THREADS];
	int i;
	srand(time(NULL));
	printf("## START THREADS\n");
	for (i = 0; i < SPAM_THREADS; i++) {
		thread_lvl[i] = i + 1;
		pthread_create(&spammers[i], NULL, (void *(*)(void *))spam_log, &thread_lvl[i]);
	}
	printf("## START THREADS DONE\n");
#endif

	NOTICE("ERROR TEST");
	ERR("ERROR TEST: %d", 123);
	WARN("WARNING TEST: %d", 123);
	NOTICE("NOTICE TEST: %d", 123);
	INFO("INFO TEST: %d", 123);
	DBG("DEBUG TEST: %d", 123);

#ifdef SPAM_THREADS
	printf("## JOIN THREADS\n");
	for (i = 0; i < SPAM_THREADS; i++) {
		pthread_join(spammers[i], NULL);
	}
	printf("## JOIN THREADS DONE\n");
#endif
	return 0;
}
#endif
