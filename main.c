/*
  +----------------------------------------------------------------------+
  | Copyright (c) 2009-2014, Johannes Schlüter <johannes@schlueters.de>  |
  | All rights reserved.                                                 |
  +----------------------------------------------------------------------+
  | Redistribution and use in source and binary forms, with or without   |
  | modification, are permitted provided that the conditions which are   |
  | bundled with this package in the file LICENSE.                       |
  | This product includes PHP software, freely available from            |
  |<http://www.php.net/software/>                                        |
  +----------------------------------------------------------------------+
*/

#include <stdio.h>
#ifdef PHP_WIN32

#else
#include <unistd.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "pconnect.h"
#include "main/php_version.h"
#include "main/php_getopt.h"
#include "main/php_streams.h"
#include "pconnect-sapi.h"
#include "pconnect-phptparser.h"

#define MAX_THREADS 255

typedef struct {
	int iterations;
	char *phpt_skipif_script_data;
	char *filename;
	char *mmapped;
	size_t mmapped_len;
	struct phpt phpt;
	char *startup_script;
	char *shutdown_script;
} req_data;

const opt_struct OPTIONS[] = {
	{'v', 0, "version"},
	{'h', 0, "help"},
	{'i', 0, "info"},
	{'c', 1, "php-ini"},
#if ZTS
	{'t', 1, "threads"},
#endif
	{'n', 1, "iterations"},
	{'p', 0, "progress"},
	{'a', 1, "init-script"},
	{'z', 1, "shutdown-script"},
	{10,  0, "phpt"},
	{'-', 0, NULL}
};

int pconn_report_progress = 0;

static void run_php(req_data *data TSRMLS_DC)
{
	unsigned char *user_data = NULL;
	size_t user_data_len = 0;
	int i = data->iterations;

	if (data->startup_script) {
		pconn_do_request_f(data->startup_script, &user_data, &user_data_len TSRMLS_CC);
	}

	while (i--) {
		int retval;

		if (data->phpt.file.begin) {
			retval = pconn_do_request_d(data->filename, data->phpt.file.begin, data->phpt.file.end - data->phpt.file.begin, &user_data, &user_data_len TSRMLS_CC);
		} else {
			retval = pconn_do_request_d(data->filename, data->mmapped, data->mmapped_len, &user_data, &user_data_len TSRMLS_CC);
		}

		if (retval == FAILURE) {
#ifdef FIX_ME
			/* On Windows I always reach this code path, dunno why */
			break;
#endif
		}
		if (pconn_report_progress) {
			printf("(%d/%d done)\r", data->iterations - i, data->iterations);
		}
	}
	if (pconn_report_progress) {
		printf("\n");
	}
	if (data->shutdown_script) {
		pconn_do_request_f(data->shutdown_script, &user_data, &user_data_len TSRMLS_CC);
	}
	if (user_data) {
	    free(user_data);
	}
}

#ifdef ZTS
#ifdef PHP_WIN32
DWORD WINAPI php_thread(LPVOID arg)
#else
static void *php_thread(void *arg)
#endif
{
	req_data *data = (req_data *)arg;
	TSRMLS_FETCH();
	run_php(data TSRMLS_CC);
#ifdef PHP_WIN32
	return 0;
#else
	return NULL;
#endif
}

#ifdef PHP_WIN32
static void run_threads(req_data *data, int concurrency)
{
	int i;
	HANDLE threads[MAX_THREADS];

	for (i=0; i < concurrency && i < MAX_THREADS; i++) {
		threads[i] = CreateThread(NULL, 0, php_thread, data, 0, NULL);
	}
	
	WaitForMultipleObjects(concurrency, threads, TRUE, INFINITE);

	for (i=0; i < concurrency && i < MAX_THREADS; i++) {
		CloseHandle(threads[i]);
	}
}
#else /* PHP_WIN32 */
static void run_threads(req_data *data, int concurrency)
{
	int i;
	pthread_t threads[MAX_THREADS];

	for (i=0; i < concurrency && i < MAX_THREADS; i++) {
		pthread_create(&threads[i], NULL, php_thread, data);
	}
	
	for (i=0; i < concurrency && i < MAX_THREADS; i++) {
		pthread_join(threads[i], NULL);
	}
}
#endif /* PHP_WIN32 */
#endif /* ZTS */

static void usage(const char *name, const int status)
{
	fprintf(stderr, "Usage: %s [-p] [-n iterations]"
#ifdef ZTS
		                 " [-t <threads>]"
#endif
		                 " [-a <startup>] [-z <shutdown>] [--phpt] <script>\n"
	                "       %s -v\n"
	                "       %s -i\n\n"
					"  -v               Print version information\n"
	                "  -i               Print phpinfo();\n"
	                "  -c <file>        Look for php.ini file in this directory\n"
	                "  -n <iterations>  Set number of iterations (default=2)\n"
#ifdef ZTS
	                "  -t <threads>     Set the number of concurrent threads (default=1, max=%i)\n"
#endif
					"  -p               Report progress (hides regular script output"
#ifdef ZTS
					"\n                   limited use with multiple threads"
#endif
					")\n"
	                "  -a <startup>     Startup script, executed once on start\n"
	                "  -z <shutdown>    Shutdown script, executed once on end\n"
	                "  --phpt           Treat <script> as phpt\n"
	                "  <script>         Main script to be executed multiple times\n\n"
			, name
			, name
			, name
#ifdef ZTS
			, MAX_THREADS
#endif
			);

	exit(status);
}

static void pconn_version()
{
	printf("pconn test %s for PHP %s (built: %s %s) %s %s\n"
			"Copyright (c) 2009-2014, Johannes Schlueter\n"
			"This product includes PHP software, freely available from <http://www.php.net/software/>.\n",
			PCONN_VERSION, PHP_VERSION, __DATE__, __TIME__,
#if ZEND_DEBUG && defined(HAVE_GCOV)
			"(DEBUG GCOV)",
#elif ZEND_DEBUG
			"(DEBUG)",
#elif defined(HAVE_GCOV)
			"(GCOV)",
#else
			"",
#endif
#if ZTS
			"(TSRM)"
#else
			""
#endif
			);
}

static int process_phpt(req_data *data)
{
	if (data->mmapped_len < 100) {
		printf("Too small, can't be a test!\n");
		return 1;
	}
	if (memcmp("--TEST--", data->mmapped, sizeof("--TEST--")-1)) {
		printf("Not a phpt test file\n");
		return 1;
	}

	parse_phpt(&data->phpt, data->mmapped, data->mmapped + data->mmapped_len);

	return 0;
}

static int init_script_data(req_data *data, int is_phpt)
{
	int fd;
	struct stat ssb;

	if (!(fd = open(data->filename, O_RDONLY))) {
		printf("Failed opening file\n");
		return 1;
	}

	if (fstat(fd, &ssb)) {
		close(fd);
		printf("Failed to stat the opened file");
		return 1;
	}

	data->mmapped_len = ssb.st_size;
	data->mmapped = mmap(NULL, data->mmapped_len, PROT_READ, MAP_PRIVATE, fd, 0);
	close(fd); 

	if (!data->mmapped) {
		printf("Error while mapping file content");
		return 1;
	}

	if (is_phpt) {
		if (process_phpt(data)) {
			return 1;
		}

		printf("Running test: ");
		fflush(stdout);
		write(STDOUT_FILENO, data->phpt.test.begin, data->phpt.test.end - data->phpt.test.begin);
	}

	return 0;
}

int main(int argc, char *argv[])
{
#ifdef ZTS
	int threads = 1;
#endif
	int opt;
	char *php_optarg = NULL;
	int php_optind = 1;
	req_data data = { 2, NULL, NULL, NULL, 0, {}, NULL, NULL };
	int is_phpt = 0;

	while ((opt = php_getopt(argc, argv, OPTIONS, &php_optarg, &php_optind, 0, 2)) != -1) {
		switch (opt) {
		case 'v':
			pconn_version();
			return 0;
		case 'i':
			pconn_phpinfo();
			return 0;
		case 'c':
			pconn_set_ini_file(php_optarg);
			break;
		case 'a':
			data.startup_script = php_optarg;
			break;
		case 'z':
			data.shutdown_script = php_optarg;
			break;
#ifdef ZTS
		case 't':
			threads = atoi(php_optarg);
			if (threads < 1 ||  threads > MAX_THREADS) {
				usage(argv[0], 1); /*terminates */
			}
			break;
#endif
		case 'n':
			data.iterations = atoi(php_optarg);
			if (data.iterations <= 0) {
				usage(argv[0], 1); /*terminates */
			}
			break;
		case 'p':
			pconn_report_progress = 1;
			break;
		case 10:
			is_phpt = 1;
			break;
		default:
			usage(argv[0], 1); /* terminates */
		case 'h':		
			usage(argv[0], 0); /* terminates */
		}
	}

	if (php_optind >= argc) {
		usage(argv[0], 1); /* terminates */
	}

	pconn_init_php();

	data.filename = argv[php_optind];
	if (init_script_data(&data, is_phpt)) {
		return 1;
	}

#ifdef ZTS
	run_threads(&data, threads);
#else
	run_php(&data);
#endif

	pconn_shutdown_php();
	munmap(data.mmapped, data.mmapped_len);
	return 0;
}


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
