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

#ifndef PCONNECT_PHPTPARSER_H
#define PCONNECT_PHPTPARSER_H

typedef struct {
	char *begin;
	char *end;
} phpt_section;

enum expect_type {
	EXPECT,
	EXPECTF,
	EXPECTREGEX
};

struct phpt {
	phpt_section test;
	phpt_section ini;
	phpt_section file;
	phpt_section xfail;
	enum expect_type type;
	phpt_section expect;
};

void parse_phpt(struct phpt *phpt, char *buffer, char *end);

#endif
/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: ft=c noet sw=4 ts=4 fdm=marker
 * vim<600: ft=c noet sw=4 ts=4
 */
