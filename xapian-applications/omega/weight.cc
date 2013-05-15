/** @file weight.cc
 * @brief Set the weighting scheme for Omega
 */
/* Copyright (C) 2009 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include <weight.h>

#include "stringutils.h"

#include <cstdlib>
#include "safeerrno.h"

using namespace std;

static bool
double_param(const char ** p, double * ptr_val)
{
    char *end;
    errno = 0;
    double v = strtod(*p, &end);
    if (*p == end || errno) return false;
    *p = end;
    *ptr_val = v;
    return true;
}

void
set_weighting_scheme(Xapian::Enquire & enq, const map<string, string> & opt,
		     bool force_boolean)
{
    if (!force_boolean) {
	map<string, string>::const_iterator i = opt.find("weighting");
	if (i == opt.end()) return;

	const string & scheme = i->second;
	if (scheme.empty()) return;

	if (startswith(scheme, "bm25")) {
	    const char *p = scheme.c_str() + 4;
	    if (*p == '\0') {
	        enq.set_weighting_scheme(Xapian::BM25Weight());
	        return;
	    }
	    if (C_isspace((unsigned char)*p)) {
		    double k1 = 1;
		    double k2 = 0;
		    double k3 = 1;
		    double b = 0.5;
		    double min_normlen = 0.5;
		    if (!double_param(&p, &k1)) throw "Parameter k1 is invalid";
		    if (!double_param(&p, &k2)) throw "Parameter k2 is invalid";
		    if (!double_param(&p, &k3)) throw "Parameter k3 is invalid";
		    if (!double_param(&p, &b)) throw "Parameter b is invalid";
		    if (!double_param(&p, &min_normlen)) throw "Parameter min_normlen is invalid";
		    Xapian::BM25Weight wt(k1, k2, k3, b, min_normlen);
		    enq.set_weighting_scheme(wt);
		    return;
	    }
	}

	if (startswith(scheme, "trad")) {
	    const char *p = scheme.c_str() + 4;
	    if (*p == '\0' || C_isspace((unsigned char)*p)) {
		double k = 1;
		double_param(&p, &k);
		enq.set_weighting_scheme(Xapian::TradWeight(k));
		return;
	    }
	}

	if (startswith(scheme, "tfidf")) {
	    const char *p = scheme.c_str() + 5;
	    if (*p == '\0') {
		enq.set_weighting_scheme(Xapian::TfIdfWeight("NTN"));
		return;
	    }
	    else if (C_isspace((unsigned char)*p)) {
		p++; //Move one ahead to the parameter
		string word = p;
		enq.set_weighting_scheme(Xapian::TfIdfWeight(p));
		return;
	    }
	}

	if (scheme != "bool") {
	    throw "Unknown $opt{weighting} setting: " + scheme;
	}
    }

    enq.set_weighting_scheme(Xapian::BoolWeight());
}
