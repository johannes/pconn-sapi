/*
  +----------------------------------------------------------------------+
  | Copyright (c) 2009-2012, Johannes Schlüter <johannes@schlueters.de>  |
  | All rights reserved.                                                 |
  +----------------------------------------------------------------------+
  | Redistribution and use in source and binary forms, with or without   |
  | modification, are permitted provided that the conditions which are   |
  | bundled with this package in the file LICENSE.                       |
  | This product includes PHP software, freely available from            |
  |<http://www.php.net/software/>                                        |
  +----------------------------------------------------------------------+
*/

#include <main/php.h>
#include <main/php_main.h>
#include <main/SAPI.h>
#include <ext/standard/info.h>
#include <ext/standard/php_var.h>
#include <main/php_variables.h>
#ifdef JO0
#include <ext/standard/php_smart_str.h>
#endif

#include "pconnect.h"
#include "pconnect-module.h"

#define PCONN_VAR "_PCONN"
#define PCONN_SIZE sizeof(PCONN_VAR)

const char HARDCODED_INI[] =
	"html_errors=0\n"
	"implicit_flush=1\n"
	"output_buffering=0\n";

static int startup(sapi_module_struct *sapi_module)
{
	if (php_module_startup(sapi_module, &pconnect_module_entry, 1)==FAILURE) {
		return FAILURE;
	}
	return SUCCESS;
}

#if PHP_VERSION_ID >= 70000
static size_t ub_write(const char *str, size_t str_length TSRMLS_DC)
#else
static int ub_write(const char *str, uint str_length TSRMLS_DC)
#endif
{
	/* This is not unbuffered ;-) */
	if (!pconn_report_progress) {
		printf("%s", str);
	}
	return str_length;
}

static void pconn_flush(void *server_context)
{
	fflush(stdout);
}

static void send_header(sapi_header_struct *sapi_header, void *server_context TSRMLS_DC)
{
}

static char* read_cookies(TSRMLS_D)
{
	return NULL;
}

static void register_variables(zval *track_vars_array TSRMLS_DC)
{
	php_import_environment_variables(track_vars_array TSRMLS_CC);
}

#if PHP_VERSION_ID >= 50400
static void log_message(char *message TSRMLS_DC)
#else
static void log_message(char *message)
#endif
{
	fprintf (stderr, "%s\n", message);
}


static sapi_module_struct pconn_module = {
	"pconnect",               /* name */
	"persistent connection test", /* pretty name */

	startup,             /* startup */
	php_module_shutdown_wrapper,   /* shutdown */

	NULL,                          /* activate */
	NULL,                          /* deactivate */

	ub_write,            /* unbuffered write */
	pconn_flush,               /* flush */
	NULL,                          /* get uid */
	NULL,                          /* getenv */

	php_error,                     /* error handler */

	NULL,                          /* header handler */
	NULL,                          /* send headers handler */
	send_header,         /* send header handler */

	NULL,                          /* read POST data */
	read_cookies,        /* read Cookies */

	register_variables,  /* register server variables */
	log_message,         /* Log message */
	NULL,                          /* Get request time */
	NULL,                          /* Child terminate */

	STANDARD_SAPI_MODULE_PROPERTIES
};

int pconn_init_php()
{
#ifdef ZTS
	void ***tsrm_ls;
	tsrm_startup(1, 1, 0, NULL);
	tsrm_ls = (void ***) ts_resource_ex(0, NULL);
#endif

	sapi_startup(&pconn_module);
	pconn_module.phpinfo_as_text = 1;

	pconn_module.ini_entries = malloc(sizeof(HARDCODED_INI));
	memcpy(pconn_module.ini_entries, HARDCODED_INI, sizeof(HARDCODED_INI));

	if (pconn_module.startup(&pconn_module)==FAILURE) {
		return FAILURE;
	}
#if PHP_VERSION_ID >= 70000
	{
		zend_string *pconn_var_name = zend_string_init(PCONN_VAR, PCONN_SIZE-1, 0);
		zend_register_auto_global(pconn_var_name, 0, NULL TSRMLS_CC);
		zend_string_release(pconn_var_name);
	}
#elif PHP_VERSION_ID >= 50400
	zend_register_auto_global(PCONN_VAR, PCONN_SIZE-1, 0, NULL TSRMLS_CC);
#else
	zend_register_auto_global(PCONN_VAR, PCONN_SIZE-1, NULL TSRMLS_CC);
#endif
	return SUCCESS;
}

void pconn_set_ini_file(const char *file)
{
	if (pconn_module.php_ini_path_override) {
		free(pconn_module.php_ini_path_override);
	}
	pconn_module.php_ini_path_override = strdup(file);
}


int pconn_shutdown_php()
{
	TSRMLS_FETCH();

	php_module_shutdown(TSRMLS_C);
	sapi_shutdown();
#ifdef ZTS
	tsrm_shutdown();
#endif
	return SUCCESS;
}

int pconn_phpinfo()
{
#ifdef ZTS
	void ***tsrm_ls = NULL;
#endif

	pconn_init_php();

#ifdef ZTS
	tsrm_ls = ts_resource(0);
#endif

	if (php_request_startup(TSRMLS_C)==FAILURE) {
		php_module_shutdown(TSRMLS_C);
		return FAILURE;
	}
	php_print_info(0xFFFFFFFF TSRMLS_CC);
	php_request_shutdown((void *) 0);
	pconn_shutdown_php();
	return SUCCESS;
}

int pconn_do_request(char *filename, unsigned char **user_data, size_t *user_data_len TSRMLS_DC)
{
	int retval = FAILURE; /* failure by default */
	zval *z_user_data_p;
	unsigned char *user_data_p = *user_data;
	zend_file_handle file_handle;

	SG(options) |= SAPI_OPTION_NO_CHDIR;
	SG(request_info).argc=0;
	SG(request_info).argv=NULL;

	if (php_request_startup(TSRMLS_C)==FAILURE) {
		php_module_shutdown(TSRMLS_C);
		return FAILURE;
	}

	SG(headers_sent) = 1;
	SG(request_info).no_headers = 1;
	php_register_variable("PHP_SELF", filename, NULL TSRMLS_CC);
	
	file_handle.type = ZEND_HANDLE_FILENAME;
	file_handle.filename = filename;
	file_handle.handle.fp = NULL;
	file_handle.opened_path = NULL;
	file_handle.free_filename = 0;


	if (user_data) {
#ifdef JO0
		zval z_user_data;
		z_user_data_p = &z_user_data;

		if (*user_data) {
			php_unserialize_data_t var_hash;

			INIT_ZVAL(z_user_data);
			PHP_VAR_UNSERIALIZE_INIT(var_hash);
			if (!php_var_unserialize(&z_user_data_p, (const unsigned char **)user_data, (const unsigned char *)*user_data_len, &var_hash TSRMLS_CC)) {
				PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
				php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Error unserializing user data");
			}
			PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
			free(user_data_p);
		} else  {
			array_init(z_user_data_p);
		}
		ZEND_SET_GLOBAL_VAR_WITH_LENGTH(PCONN_VAR, PCONN_SIZE, z_user_data_p, 2, 0);
#endif
	}

	zend_first_try {
		retval = php_execute_script(&file_handle TSRMLS_CC);
	} zend_end_try();

	if (user_data) {
#ifdef JO0
		zval **z_user_data_pp;
		if (zend_hash_find(&EG(symbol_table), PCONN_VAR, PCONN_SIZE, (void **) &z_user_data_pp) == SUCCESS) {
			
			php_serialize_data_t var_hash;
			smart_str buf = {0};

			PHP_VAR_SERIALIZE_INIT(var_hash);
			php_var_serialize(&buf, z_user_data_pp, &var_hash TSRMLS_CC);
			PHP_VAR_SERIALIZE_DESTROY(var_hash);

			*user_data = malloc(buf.len);
			strncpy(*user_data, buf.c, buf.len);
			*user_data_len = buf.len;
			smart_str_free(&buf);

			zval_dtor(z_user_data_p);
		}
#endif
	}

	php_request_shutdown((void *) 0);

	return (retval == SUCCESS) ? SUCCESS : FAILURE;
}


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
