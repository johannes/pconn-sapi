/*
pconnect test for PHP

This product includes PHP software, freely available from
<http://www.php.net/software/>

Author: Johannes Schlüter
*/

#include <main/php.h>
#include <main/php_main.h>
#include <main/SAPI.h>
#include <ext/standard/info.h>
#include <ext/standard/php_var.h>
#include <main/php_variables.h>

#define PCONN_VAR "_PCONN"
#define PCONN_SIZE sizeof(PCONN_VAR)

static int startup(sapi_module_struct *sapi_module)
{
	if (php_module_startup(sapi_module, NULL, 0)==FAILURE) {
		return FAILURE;
	}
	return SUCCESS;
}

static int ub_write(const char *str, uint str_length TSRMLS_DC)
{
	/* This is not unbuffered ;-) */
	printf("%s", str);
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

static void log_message(char *message)
{
	fprintf (stderr, "%s\n", message);
}


sapi_module_struct pconn_module = {
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

	if (pconn_module.startup(&pconn_module)==FAILURE) {
		return FAILURE;
	}
	zend_register_auto_global(PCONN_VAR, PCONN_SIZE-1, NULL TSRMLS_CC);
	return SUCCESS;
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

#define SMART_STR_USE_REALLOC 1
int pconn_do_request(char *filename, unsigned char **user_data, size_t *user_data_len TSRMLS_DC)
{
	zval *z_user_data_p;
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
			free(*user_data);
		} else  {
			array_init(z_user_data_p);
		}
		ZEND_SET_GLOBAL_VAR_WITH_LENGTH(PCONN_VAR, PCONN_SIZE, z_user_data_p, 2, 0);
	}

	zend_first_try {
		php_execute_script(&file_handle TSRMLS_CC);
	} zend_end_try();

	if (user_data) {
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
			efree(buf.c);

			zval_dtor(z_user_data_p);
		}
	}

	php_request_shutdown((void *) 0);

	return SUCCESS;
}
#undef SMART_STR_USE_REALLOC

