/*
  +----------------------------------------------------------------------+
  | Copyright (c) 2009, Johannes Schlüter <johannes@schlueters.de>       |
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
#include <unistd.h>
#include <stdlib.h>

#include "pconnect.h"
#include "main/php_version.h"
#include "pconnect-sapi.h"

#define MAX_THREADS 255

typedef struct {
	int iterations;
	char *main_script;
	char *startup_script;
	char *shutdown_script;
} req_data;

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
static void *php_thread(void *arg)
{
	req_data *data = (req_data *)arg;
	TSRMLS_FETCH();
	run_php(data TSRMLS_CC);
	return NULL;
}

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
			"Copyright (c) 2009 Johannes Schlueter\n"
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
	req_data data = { 2, NULL, NULL, NULL };
 
	while ((opt = getopt(argc, argv, "vhit:n:c:a:z:")) != -1) {
		switch (opt) {
		case 'v':
			pconn_version();
			return 0;
		case 'i':
			pconn_phpinfo();
			return 0;
		case 'c':
			pconn_set_ini_file(optarg);
			break;
		case 'a':
			data.startup_script = optarg;
			break;
		case 'z':
			data.shutdown_script = optarg;
			break;
		case 't':
#ifdef ZTS
			threads = atoi(optarg);
			if (threads < 1 ||  threads > MAX_THREADS)
#endif
			{
				usage(argv[0], 1); /*terminates */
			}
			break;
		case 'n':
			data.iterations = atoi(optarg);
			if (data.iterations == 0) {
				usage(argv[0], 1); /*terminates */
			}
			break;
		default:
			usage(argv[0], 1); /* terminates */
		case 'h':		
			usage(argv[0], 0); /* terminates */
		}
	}

	if (optind >= argc) {
		usage(argv[0], 1); /* terminates */
	}

	data.main_script = argv[optind];

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
