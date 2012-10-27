/*
    mixan
    Analysis of granular material mixes and emulsions.

    File: mixfuns.cpp

    Copyright (C) 2011-2012 Artem Petrov <pa2311@gmail.com>

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
#include "material.h"

#include <cmath>

double Vc(const QVector<double> &concentrations,
          const double &idealconc) {

    double summ_concdiff = 0;
    size_t probesnum = concentrations.size();

    for ( size_t i=0; i<probesnum; i++ ) {

        summ_concdiff += pow( (concentrations[i] - idealconc), 2 );
    }

    return 100.0 / idealconc *
            pow( 1.0 / ( probesnum - 1.0 ) * summ_concdiff, 0.5 );
}

size_t defThreshColor(const Material *m1,
                      const Material *m2,
                      const double &intersectaccur) {

    const Material *mat1;
    const Material *mat2;

    if ( m1->thresholdColor() < m2->thresholdColor() ) {

        mat1 = m1;
        mat2 = m2;
    }
    else {

        mat1 = m2;
        mat2 = m1;
    }

    //

    QVector<ptrdiff_t> lims1 = mat1->polynomLimits();
    QVector<ptrdiff_t> lims2 = mat2->polynomLimits();

    if ( lims1[1] < lims2[0] ) {

        return ( mat1->thresholdColor() + mat2->thresholdColor() ) / 2;
    }

    //

    size_t tcol = 0;

    QVector<double> poly1 = mat1->polynomValues();
    QVector<double> poly2 = mat2->polynomValues();

    for ( size_t i=mat1->thresholdColor(); i<256; i++ ) {

        if ( fabs(poly1[i]-poly2[i]) < intersectaccur ) {

            tcol = i;
            break;
        }
    }

    if ( tcol >= mat1->thresholdColor() &&
         tcol <= mat2->thresholdColor() ) {

        return tcol;
    }
    else {

        return ( mat1->thresholdColor() + mat2->thresholdColor() ) / 2;
    }

    //

    return 0;
}

void defRemainders(const QVector< QSharedPointer<Granules> > &granules,
                   QVector<double> &sieveCells,
                   QVector<double> &partRemainders,
                   QVector<double> &totalRemainders) {

    QVector<double> areas = granules[0]->areaValues();
    QVector<double> minosizes = granules[0]->minOverallSizes();

    for ( ptrdiff_t n=1; n<granules.size(); n++ ) {

        areas += granules[n]->areaValues();
        minosizes += granules[n]->minOverallSizes();
    }

    double totalArea = 0;
    for ( ptrdiff_t i=0; i<areas.size(); i++ ) { totalArea += areas[i]; }

    //

    qSort(sieveCells);

    partRemainders.clear();
    partRemainders.resize(sieveCells.size());

    totalRemainders.clear();
    totalRemainders.resize(sieveCells.size());

    //

    for ( ptrdiff_t n=0; n<minosizes.size(); n++ ) {

        for ( ptrdiff_t m=(sieveCells.size()-1); m>=0; m-- ) {

            if ( minosizes[n] > sieveCells[m] ) {

                partRemainders[m] += areas[n];
                break;
            }
        }
    }

    for ( ptrdiff_t i=0; i<partRemainders.size(); i++ ) {

        partRemainders[i] /= totalArea;
    }

    //

    totalRemainders[partRemainders.size()-1] =
            partRemainders[partRemainders.size()-1];

    for ( ptrdiff_t i=(partRemainders.size()-2); i>=0; i-- ) {

        totalRemainders[i] = totalRemainders[i+1] + partRemainders[i];
    }
}
