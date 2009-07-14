/*
pconnect test for PHP

This product includes PHP software, freely available from
<http://www.php.net/software/>

Author: Johannes Schlüter
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "pconnect-sapi.h"

char *filename;
char *startup_script = NULL;
char *shutdown_script = NULL;
int iterations;

void run_php(TSRMLS_D)
{
	int i = iterations;
	if (startup_script)
		pconn_do_request(startup_script TSRMLS_CC);
	while (i--) {
		pconn_do_request(filename TSRMLS_CC);
	}
	if (shutdown_script)
		pconn_do_request(shutdown_script TSRMLS_CC);
}

#ifdef ZTS
void *php_thread(void *arg)
{
	TSRMLS_FETCH();
	run_php(TSRMLS_C);
	return NULL;
}

void run_threads(int concurrency)
{
	int i;
	/* TODO: LIMIT!!!! */
	pthread_t threads[100];

	
	for (i=0; i < concurrency && i < 100; i++) {
		pthread_create(&threads[i], NULL, php_thread, NULL);
	}
	
	for (i=0; i < concurrency && i < 100; i++) {
		pthread_join(threads[i], NULL);
	}
}
#endif

void usage(char *name)
{
	fprintf(stderr, "Usage: %s [-c <iterations>] [-a <startup>] [-z <shutdown>] <script>\n", name);
	fprintf(stderr, "       %s -i\n\n", name);
	fprintf(stderr, "  -i               Print phpinfo();\n");
	fprintf(stderr, "  -n <iterations>  Set number of iterations (default=2)\n");
#ifdef ZTS
	fprintf(stderr, "  -c <concurrency> Set the number of concurrent threads\n");
#endif
	fprintf(stderr, "  -a <startup>     Startup script, run once on start\n");
	fprintf(stderr, "  -z <shutdown>    Shutdown script, executed one on end\n");
	fprintf(stderr, "  <script>         Main script to be executed multiple times\n\n");
}

int main(int argc, char *argv[])
{
#ifdef ZTS
	int concurrency = 1;
#endif
	int opt;
 
	while ((opt = getopt(argc, argv, "hin:c:a:z:")) != -1) {
		switch (opt) {
		case 'i':
			pconn_phpinfo();
			return 0;
		case 'a':
			startup_script = optarg;
			break;
		case 'z':
			shutdown_script = optarg;
			break;
#ifdef ZTS
		case 'c':
			concurrency = atoi(optarg);
			break;
#endif
		case 'n':
			iterations = atoi(optarg);
			if (iterations > 0) {
				break;
			}
			/* fall through */
		default:
			usage(argv[0]);
			return 1;
		case 'h':		
			usage(argv[0]);
			return 0;
		}
	}

	if (optind >= argc) {
		usage(argv[0]);
		return 1;
	}

	filename = argv[optind];

	pconn_init_php();
#ifdef ZTS
	run_threads(concurrency);
#else
	run_php();
#endif
	pconn_shutdown_php();
	return 0;
}

