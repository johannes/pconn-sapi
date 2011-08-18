/*
  +----------------------------------------------------------------------+
  | Copyright (c) 2011, Johannes Schlüter <johannes@schlueters.de>       |
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

#include "pconnect.h"
#include "main/php_version.h"
#include "main/php_getopt.h"
#include "pconnect-sapi.h"

#define MAX_THREADS 255

typedef struct {
	int iterations;
	char *main_script;
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
	{'a', 1, "init-script"},
	{'z', 1, "shutdown-script"},
	{'-', 0, NULL}
};


static void run_php(req_data *data TSRMLS_DC)
{
	unsigned char *user_data = NULL;
	size_t user_data_len = 0;
	int i = data->iterations;

	if (data->startup_script) {
		pconn_do_request(data->startup_script, &user_data, &user_data_len TSRMLS_CC);
	}
	while (i--) {
		int retval;
		retval = pconn_do_request(data->main_script, &user_data, &user_data_len TSRMLS_CC);
		if (retval) {
			break;
		}
	}
	if (data->shutdown_script) {
		pconn_do_request(data->shutdown_script, &user_data, &user_data_len TSRMLS_CC);
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

static void run_threads(req_data *data, int concurrency)
{
	int i;
	HANDLE threads[MAX_THREADS];

	
	for (i=0; i < concurrency && i < MAX_THREADS; i++) {
#ifdef PHP_WIN32
		threads[i] = CreateThread(NULL, 0, php_thread, data, 0, NULL);
#else
		pthread_create(&threads[i], NULL, php_thread, data);
#endif
	}
	
#ifdef PHP_WIN32
	WaitForMultipleObjects(concurrency, threads, TRUE, INFINITE);
#else
	for (i=0; i < concurrency && i < MAX_THREADS; i++) {
		pthread_join(threads[i], NULL);
	}
#endif
}
#endif

static void usage(const char *name, const int status)
{
	fprintf(stderr, "Usage: %s [-n iterations]"
#ifdef ZTS
		                 " [-t <threads>]"
#endif
		                 " [-a <startup>] [-z <shutdown>] <script>\n"
	                "       %s -v\n"
	                "       %s -i\n\n"
					"  -v               Print version information\n"
	                "  -i               Print phpinfo();\n"
	                "  -c <file>        Look for php.ini file in this directory\n"
	                "  -n <iterations>  Set number of iterations (default=2)\n"
#ifdef ZTS
	                "  -t <threads>     Set the number of concurrent threads (default=1, max=%i)\n"
#endif
	                "  -a <startup>     Startup script, run once on start\n"
	                "  -z <shutdown>    Shutdown script, executed one on end\n"
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
			"Copyright (c) 2011 Johannes Schlueter\n"
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

int main(int argc, char *argv[])
{
#ifdef ZTS
	int threads = 1;
#endif
	int opt;
	char *php_optarg = NULL;
	int php_optind = 1;
	req_data data = { 2, NULL, NULL, NULL };

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
		default:
			usage(argv[0], 1); /* terminates */
		case 'h':		
			usage(argv[0], 0); /* terminates */
		}
	}

	if (php_optind >= argc) {
		usage(argv[0], 1); /* terminates */
	}

	data.main_script = argv[php_optind];
	
	pconn_init_php();
#ifdef ZTS
	run_threads(&data, threads);
#else
	run_php(&data);
#endif
	pconn_shutdown_php();
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
