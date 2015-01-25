/*
  +----------------------------------------------------------------------+
  | Copyright (c) 2009-2015, Johannes Schlüter <johannes@schlueters.de>  |
  | All rights reserved.                                                 |
  +----------------------------------------------------------------------+
  | Redistribution and use in source and binary forms, with or without   |
  | modification, are permitted provided that the conditions which are   |
  | bundled with this package in the file LICENSE.                       |
  | This product includes PHP software, freely available from            |
  | <http://www.php.net/software/>                                       |
  +----------------------------------------------------------------------+
*/

#ifndef SUCCESS
#define SUCCESS 0
#define FAILURE -1 
#endif

#include "Zend/zend.h"
#include <TSRM.h>

int pconn_init_php();
int pconn_shutdown_php();
void pconn_set_ini_file(const char *file);
int pconn_phpinfo();
int pconn_do_request(zend_file_handle *file_handle, char *filename, unsigned char **user_data, size_t *user_data_len TSRMLS_DC);
int pconn_do_request_f(char *filename, unsigned char **user_data, size_t *user_data_len TSRMLS_DC);
int pconn_do_request_d(char *filename, char *data, size_t data_len, unsigned char **user_data, size_t *user_data_len TSRMLS_DC);

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
