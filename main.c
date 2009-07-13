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

void run_php(int iterations, char *filename)
{
	pconn_init_php();
	while (iterations--) {
		pconn_do_request(filename);
	}
	pconn_shutdown_php();
}

void usage(char *name)
{
	fprintf(stderr, "Usage: %s [-i iterations] script\n", name);
}

int main(int argc, char *argv[])
{
	int iterations = 2;
	int opt;
 
	while ((opt = getopt(argc, argv, "hi:")) != -1) {
		switch (opt) {
		case 'i':
			iterations = atoi(optarg);
			if (iterations > 1) {
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

	run_php(iterations, argv[optind]);
	return 0;
}

