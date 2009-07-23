/*
pconnect test for PHP

This product includes PHP software, freely available from
<http://www.php.net/software/>

Author: Johannes Schlüter
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

