/*
    mixan
    Analysis of granular material mix.

    File: numcompfuns.cpp

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

#include "numcompfuns.h"

#include <vector>
#include <cmath>

using std::vector;
using std::ptrdiff_t;
using std::size_t;

bool polyapprox(vector<double> *x, vector<double> *y, vector<double> *coeff) {

    // it was rewritten. original algorithm realization:
    // http://alexeypetrov.narod.ru/C/sqr_less_about.html

    ptrdiff_t N = x->size();
    ptrdiff_t K = coeff->size() - 1;

    if ( (size_t)N != y->size() ) { return false; }

    //

    vector<double> b(K+1, 0);

    vector< vector<double> > sums;
    vector<double> sum(K+1, 0);
    for ( ptrdiff_t i=0; i<(K+1); i++ ) { sums.push_back(sum); }

    //

    for ( ptrdiff_t i=0; i<(K+1); i++ ) {

        for ( ptrdiff_t j=0; j<(K+1); j++ ) {

            sums[i][j] = 0;

            for ( ptrdiff_t k=0; k<N; k++ ) {

                sums[i][j] += pow( x->at(k), i+j );
            }
        }
    }

    //

    for ( ptrdiff_t i=0; i<(K+1); i++ ) {

        for ( ptrdiff_t k=0; k<N; k++ ) {

            b[i] += pow( x->at(k), i ) * y->at(k);
        }
    }

    //

    double temp = 0;

    for ( ptrdiff_t i=0; i<(K+1); i++ ) {

        if ( sums[i][i] == 0 ) {

            for ( ptrdiff_t j=0; j<(K+1); j++ ) {

                if ( j == i ) { continue; }

                if ( (sums[j][i] != 0) && (sums[i][j] != 0) ) {

                    for ( ptrdiff_t k=0; k<(K+1); k++ ) {

                        temp = sums[j][k];
                        sums[j][k] = sums[i][k];
                        sums[i][k] = temp;
                    }

                    temp = b[j];
                    b[j] = b[i];
                    b[i] = temp;

                    break;
                }
            }
        }
    }

    //

    double M = 0;

    for ( ptrdiff_t k=0; k<(K+1); k++ ) {

        for ( ptrdiff_t i=(k+1); i<(K+1); i++ ) {

            if ( sums[k][k] == 0 ) { return false; } // solution is not exist

            M = sums[i][k] / sums[k][k];

            for ( ptrdiff_t j=k; j<(K+1); j++ ) {

                sums[i][j] -= M * sums[k][j];
            }

            b[i] -= M * b[k];
        }
    }

    //

    double s = 0;

    for ( ptrdiff_t i=((K+1)-1); i>=0; i-- ) {

        s = 0;

        for ( ptrdiff_t j=i; j<(K+1); j++ ) {

            s += sums[i][j] * coeff->at(j);
        }

        coeff->at(i) = (b[i] - s) / sums[i][i];
    }

    return true;
}
