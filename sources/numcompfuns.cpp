/*
    mixan
    Analysis of granular material mixes and emulsions.

    File: numcompfuns.cpp

    Copyright (C) 2011-2015 Artem Petrov <pa2311@gmail.com>

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
#include "mixanerror.h"

#include <cmath>

#include <QVector>
#include <QObject>

QVector<double> polyapprox(const QVector<double> &x,
                           const QVector<double> &y,
                           const ptrdiff_t K) {             // polynom power

    const ptrdiff_t N = x.size();

    if ( N != y.size() ) {
        throw MixanError(QObject::tr("When calculating the coefficients of the "
                                     "approximating polynomial arrays x and y "
                                     "must be the same size!"));
    }

    if ( K >= N ) {
        throw MixanError(QObject::tr("The degree of approximating polynomial "
                                     "must be less than the size of arrays "
                                     "with data!"));
    }

    QVector<double> coeff(K+1);

    //

    QVector<double> b(K+1);

    QVector< QVector<double> > sums;
    QVector<double> sum(K+1);
    for ( ptrdiff_t i=0; i<(K+1); i++ ) { sums.push_back(sum); }

    //

    for ( ptrdiff_t i=0; i<(K+1); i++ ) {

        for ( ptrdiff_t j=0; j<(K+1); j++ ) {

            sums[i][j] = 0;

            for ( ptrdiff_t k=0; k<N; k++ ) {
                sums[i][j] += pow(x[k], i+j);
            }
        }
    }

    //

    for ( ptrdiff_t i=0; i<(K+1); i++ ) {

        for ( ptrdiff_t k=0; k<N; k++ ) {
            b[i] += pow(x[k], i) * y[k];
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

            if ( sums[k][k] == 0 ) {
                throw MixanError(QObject::tr("Polynomial approximation: "
                                             "solution does not exist!"));
            }

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
            s += sums[i][j] * coeff[j];
        }

        coeff[i] = (b[i] - s) / sums[i][i];
    }

    return coeff;
}
