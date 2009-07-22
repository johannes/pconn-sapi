/*
pconnect test for PHP

This product includes PHP software, freely available from
<http://www.php.net/software/>

Author: Johannes Schlüter
*/

#ifndef SUCCESS
#define SUCCESS 0
#define FAILURE -1 
#endif

#include <TSRM.h>

int pconn_init_php();
int pconn_shutdown_php();
int pconn_phpinfo();
int pconn_do_request(char *filename, unsigned char **user_data, size_t *user_data_len TSRMLS_DC);
