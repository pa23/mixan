/*
    mixan
    Analysis of granular material mix and emulsions.

    File: mixfuns.cpp

    Copyright (C) 2011 Artem Petrov <pa2311@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "mixfuns.h"
#include "constants.h"

#include <vector>
#include <cmath>

using std::size_t;
using std::vector;

double Vc(vector<double> *concentrations) {

    double summ_concdiff = 0;
    size_t probesnum = concentrations->size();

    for ( size_t i=0; i<probesnum; i++ ) {

        summ_concdiff += pow( (concentrations->at(i) - IDEALCONC), 2 );
    }

    return 100.0 / IDEALCONC *
            pow( 1.0 / ( probesnum - 1.0 ) * summ_concdiff, 0.5 );
}
