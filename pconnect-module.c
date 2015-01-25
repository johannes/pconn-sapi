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

#include <php.h>
#include "pconnect.h"

/*
TODO: (or rather "IDEAS")
 - Add functions to terminate the current/all thread(s)
 - Maybe some support for simple messaging
 - Export iteration counter or at least max number of iterations
*/

PHP_MINIT_FUNCTION(pconnect)
{
	REGISTER_LONG_CONSTANT("PCONN_SUCCESS",	0, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PCONN_FAIL", -1, CONST_CS | CONST_PERSISTENT);

	return SUCCESS;
}

zend_module_entry pconnect_module_entry = {
	STANDARD_MODULE_HEADER,
	"pconnect",
	NULL,
	PHP_MINIT(pconnect),
	NULL,
	NULL,
	NULL,
	NULL,
	PCONN_VERSION,
	STANDARD_MODULE_PROPERTIES
};


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
