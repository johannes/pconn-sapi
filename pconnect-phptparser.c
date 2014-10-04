/*
  +----------------------------------------------------------------------+
  | Copyright (c) 2009-2014, Johannes Schlüter <johannes@schlueters.de>  |
  | All rights reserved.                                                 |
  +----------------------------------------------------------------------+
  | Redistribution and use in source and binary forms, with or without   |
  | modification, are permitted provided that the conditions which are   |
  | bundled with this package in the file LICENSE.                       |
  | This product includes PHP software, freely available from            |
  |<http://www.php.net/software/>                                        |
  +----------------------------------------------------------------------+
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "pconnect-phptparser.h"

static void switch_section(struct phpt *phpt, phpt_section **section, char *begin, char *end)
{
	size_t len = end - begin + 1;

	if (*section) {
		(*section)->end = begin - 2; /* -2 for "--" */
	}

#define COMPARE_SECTION(name) len == sizeof(name)-1 && !memcmp(name, begin, len)

	if (COMPARE_SECTION("TEST")) {
		*section = &phpt->test;
	} else if (COMPARE_SECTION("INI")) {
		*section = &phpt->ini;
	} else if (COMPARE_SECTION("FILE")) {
		*section = &phpt->file;
	} else if (COMPARE_SECTION("XFAIL")) {
		*section = &phpt->xfail;
	} else if (COMPARE_SECTION("EXPECT")) {
		*section = &phpt->expect;
		phpt->type = EXPECT;
	} else if (COMPARE_SECTION("EXPECTF")) {
		*section = &phpt->expect;
		phpt->type = EXPECTF;
	} else if (COMPARE_SECTION("EXPECTREGEX")) {
		*section = &phpt->expect;
		phpt->type = EXPECTREGEX;
	} else {
		/* Error */
		*section = NULL;
		printf("Invalid section '%12s' (%li)\n", begin, len);
		return;
	}

	(*section)->begin = begin + len + 3;
}

void parse_phpt(struct phpt *phpt, char *buffer, char *end)
{
	int state = 0; /* 0 = beginning of line, 1 = in section title, 2 = some text */
	char *section_name = NULL;
	phpt_section *current_section = NULL;
	memset(phpt, 0, sizeof(struct phpt));

	for (; buffer < end; ++buffer) {
		switch (state) {
			case 0:
				if (*buffer == '-' && *(buffer+1) == '-') {
					buffer += 2;
					state = 1;
					section_name = buffer;
				} else if (*buffer == '\r' || *buffer == '\n') {
					/* ignored - we'Re still at the beginning of a line */
				} else {
					state = 2;
				}
				break;
			case 1:
				if (*buffer == '-' && *(buffer+1) == '-') {
					switch_section(phpt, &current_section, section_name, buffer-1);
					buffer += 3; /* 3 for "--\n", if the test uses \r\n test script will start with \n which is ok */
					state = 0;
				}
				break;
			case 2:
				if (*buffer == '\r' || *buffer == '\n') {
					state = 0;
				}
		}
	}
	if (current_section) {
		current_section->end = end;
	}
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: ft=c noet sw=4 ts=4 fdm=marker
 * vim<600: ft=c noet sw=4 ts=4
 */
