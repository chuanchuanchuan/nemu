#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint WP;

WP* wp_new();
void wp_free(WP *wp);
void wp_set_expr(WP *wp, const char *exp);
bool wp_eval(WP *wp);
uint32_t wp_get_result(WP *wp);
int wp_get_no(WP *wp);

#endif
