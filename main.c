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

typedef struct {
	int iterations;
	char *main_script;
	char *startup_script;
	char *shutdown_script;
} req_data;

void run_php(req_data *data TSRMLS_DC)
{
	unsigned char *user_data = NULL;
	size_t user_data_len = 0;
	int i = data->iterations;

	if (data->startup_script) {
		pconn_do_request(data->startup_script, &user_data, &user_data_len TSRMLS_CC);
	}
	while (i--) {
		pconn_do_request(data->main_script, &user_data, &user_data_len TSRMLS_CC);
		usleep(1);
	}
	if (data->shutdown_script) {
		pconn_do_request(data->shutdown_script, &user_data, &user_data_len TSRMLS_CC);
	}
}

#ifdef ZTS
void *php_thread(void *arg)
{
	req_data *data = (req_data *)arg;
	TSRMLS_FETCH();
	run_php(data TSRMLS_CC);
	return NULL;
}

void run_threads(req_data *data, int concurrency)
{
	int i;
	/* TODO: LIMIT!!!! */
	pthread_t threads[100];

	
	for (i=0; i < concurrency && i < 100; i++) {
		pthread_create(&threads[i], NULL, php_thread, data);
	}
	
	for (i=0; i < concurrency && i < 100; i++) {
		pthread_join(threads[i], NULL);
	}
}
#endif

void usage(char *name, int status)
{
	fprintf(stderr, "Usage: %s [-n iterations] [-c <concurrency>] [-a <startup>] [-z <shutdown>] <script>\n", name);
	fprintf(stderr, "       %s -i\n\n", name);
	fprintf(stderr, "  -i               Print phpinfo();\n");
	fprintf(stderr, "  -n <iterations>  Set number of iterations (default=2)\n");
#ifdef ZTS
	fprintf(stderr, "  -c <concurrency> Set the number of concurrent threads (def=1)\n");
#endif
	fprintf(stderr, "  -a <startup>     Startup script, run once on start\n");
	fprintf(stderr, "  -z <shutdown>    Shutdown script, executed one on end\n");
	fprintf(stderr, "  <script>         Main script to be executed multiple times\n\n");

	exit(status);
}

int main(int argc, char *argv[])
{
#ifdef ZTS
	int concurrency = 1;
#endif
	int opt;
	req_data data = { 2, NULL, NULL, NULL };
 
	while ((opt = getopt(argc, argv, "hin:c:a:z:")) != -1) {
		switch (opt) {
		case 'i':
			pconn_phpinfo();
			return 0;
		case 'a':
			data.startup_script = optarg;
			break;
		case 'z':
			data.shutdown_script = optarg;
			break;
#ifdef ZTS
		case 'c':
			concurrency = atoi(optarg);
			break;
#endif
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
		usage(argv[0], 1); /* termina */
	}

	data.main_script = argv[optind];

	pconn_init_php();
#ifdef ZTS
	run_threads(&data, concurrency);
#else
	run_php(&data);
#endif
	pconn_shutdown_php();
	return 0;
}

