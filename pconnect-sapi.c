/*
pconnect test for PHP

This product includes PHP software, freely available from
<http://www.php.net/software/>

Author: Johannes Schlüter
*/

#include <php_embed.h>
#include <ext/standard/info.h>

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
	zend_llist global_vars;

#ifdef ZTS
	tsrm_startup(1, 1, 0, NULL);
#endif

	sapi_startup(&pconn_module);
	pconn_module.phpinfo_as_text = 1;
	if (pconn_module.startup(&pconn_module)==FAILURE) {
		return FAILURE;
	}

	zend_llist_init(&global_vars, sizeof(char *), NULL, 0);
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

int pconn_do_request(char *filename TSRMLS_DC)
{
	zend_file_handle file_handle;

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

/*
	if (php_fopen_primary_script(&file_handle TSRMLS_CC) == FAILURE) {
		fprintf(stderr, "Failed opening %s.\n", filename);
		exit(1);
	}
*/

	zend_first_try {
		php_execute_script(&file_handle TSRMLS_CC);
	} zend_end_try();

	php_request_shutdown((void *) 0);

	return SUCCESS;
}

