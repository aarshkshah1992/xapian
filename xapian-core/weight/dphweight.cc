/** @file dphweight.cc
 * @brief Xapian::DPHWeight class - The DPH weighting scheme of the DFR framework.
 */
/* Copyright (C) 2013 Aarsh Shah
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <config.h>

#include "xapian/weight.h"
#include <cmath>

using namespace std;

namespace Xapian {

DPHWeight *
DPHWeight::clone() const
{
    return new DPHWeight();
}

void
DPHWeight::init(double)
{
     // None Required
}

string
DPHWeight::name() const
{
    return "Xapian::DPHWeight";
}

string
DPHWeight::serialise() const
{
    return string();
}

DPHWeight *
DPHWeight::unserialise(const string & ) const
{
    return new DPHWeight();
}

double
DPHWeight::get_sumpart(Xapian::termcount wdf, Xapian::termcount len) const
{
    if (wdf == 0) return 0.0;

    double base_change(log(2));
    double wdf_to_len = double(wdf) / len;

    double normalization = (pow((1 - wdf_to_len), 2) / (wdf + 1));

    double weight = normalization * (wdf * (log((wdf * get_average_length() / len) *
    (get_collection_size() / get_collection_freq())) / base_change) +
    (0.5 * (log(2 * 3.14 * wdf * (1 - wdf_to_len)) / base_change)));

    return (get_wqf() * weight);
}

double
DPHWeight::get_maxpart() const
{
    if (get_wdf_upper_bound() == 0) return 0.0;

    double wdf_lower(1.0);
    double base_change(log(2));
    double wdf_upper(get_wdf_upper_bound());

    double min_wdf_to_len = (wdf_lower / get_doclength_upper_bound());
    double max_normalization = (pow((1 - min_wdf_to_len), 2) / (wdf_lower + 1));

    double max_weight = max_normalization * (wdf_upper * (log((wdf_upper *
    get_average_length() / get_doclength_lower_bound()) * (get_collection_size() /
    get_collection_freq())) / base_change) + (0.5 * (log(2 * 3.14 * wdf_upper *
    (1 - min_wdf_to_len)) / base_change)));

    return (get_wqf() * max_weight);
}

double
DPHWeight::get_sumextra(Xapian::termcount) const
{
    return 0;
}

double
DPHWeight::get_maxextra() const
{
    return 0;
}

}
