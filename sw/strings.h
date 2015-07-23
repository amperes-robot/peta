#pragma once
#include <avr/pgmspace.h>
#include "WString.h"

#define TO_FSTR(x) ((__FlashStringHelper*) (x))
typedef __FlashStringHelper* FSTR;

#define DECL_FSTR(x) extern const PROGMEM char x[]
#define DEFN_FSTR(x) const PROGMEM char x[] = { #x }

namespace strings
{
	DECL_FSTR(course);
	DECL_FSTR(select);
	DECL_FSTR(opt);
	DECL_FSTR(opt_restore);
	DECL_FSTR(follow);
	DECL_FSTR(dbg);

	DECL_FSTR(flw_p);
	DECL_FSTR(flw_i);
	DECL_FSTR(flw_d);
	DECL_FSTR(flw_vel);
	DECL_FSTR(flw_thresh_left);
	DECL_FSTR(flw_thresh_side);
	DECL_FSTR(flw_thresh_right);
	DECL_FSTR(flw_recover);
	DECL_FSTR(flw_drecover);

	DECL_FSTR(home_p);
	DECL_FSTR(home_i);
	DECL_FSTR(home_d);
	DECL_FSTR(home_thresh);
	DECL_FSTR(home_vel);

	DECL_FSTR(beacon_thresh);
	DECL_FSTR(beacon_theta);

	DECL_FSTR(home);
	DECL_FSTR(retrieval);
	DECL_FSTR(view);
	DECL_FSTR(ppark);

	DECL_FSTR(rev_dbegin);
	DECL_FSTR(rev_dend);
	DECL_FSTR(rev_enable);
}
