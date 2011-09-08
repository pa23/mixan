/*
    mixan
    Analyze of granular material mix.

    File: probe.cpp

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

#include "probe.h"
#include "numcompfuns.h"
#include "constants.h"

#include <vector>
#include <cmath>

#include <QVector>
#include <QImage>

#include <QDebug>

using std::vector;

Probe::Probe(QString imgFileName) :
    threshColor(0),
    conc(0) {

    origImage.load(imgFileName);
    bwImage = origImage;

    histogram = new size_t[256];
    for ( ptrdiff_t i=0; i<256; i++ ) { histogram[i] = 0; }
}

Probe::~Probe() {

    delete histogram;
}

bool Probe::analyze() {

    if ( !colorToBW()      ) { return false; }
    if ( !defThreshColor() ) { return false; }
    if ( !defConc()        ) { return false; }

    for ( ptrdiff_t i=0; i<256; i++ ) { histogram[i] = 0; }

    return true;
}

QImage *Probe::originalImage() {

    return &origImage;
}

QImage *Probe::blackwhiteImage() {

    return &bwImage;
}

size_t Probe::thresholdColor() const {

    return threshColor;
}

double Probe::concentration() const {

    return conc;
}

bool Probe::colorToBW() {

    if ( origImage.isNull() ) { return false; }

    size_t gray = 0;

    for ( ptrdiff_t i=0; i<origImage.width(); i++ ) {

        for ( ptrdiff_t j=0; j<origImage.height(); j++ ) {

            gray = qGray(origImage.pixel(i, j));
            bwImage.setPixel(i, j, qRgb(gray, gray, gray));

            histogram[gray]++;
        }
    }

    return true;
}

bool Probe::defThreshColor() {

    vector<double> x(256, 0);
    vector<double> y(256, 0);
    vector<double> coeff(POLYPOWER+1, 0);

    for ( ptrdiff_t i=0; i<256; i++ ) {

        x[i] = i;
        y[i] = histogram[i];
    }

    //

    if ( !polyapprox(&x, &y, &coeff) ) { return false; }

    //

    QVector<double> poly(256, 0);
    double p = 0;

    for ( ptrdiff_t i=0; i<256; i++ ) {

        for ( ptrdiff_t n=0; n<(POLYPOWER+1); n++ ) {

            p += coeff[n] * pow( x[i], n );
        }

        poly[i] = p;
        p = 0;
    }

    //

    ptrdiff_t iextr = 0;

    for ( ptrdiff_t i=1; i<256; i++ ) {

        if ( poly[i] > poly[i-1] ) { iextr = i; }
    }

    threshColor = (size_t)qRgb(iextr, iextr, iextr);

    return true;
}

bool Probe::defConc() {

    if ( bwImage.isNull() ) { return false; }

    size_t curColor = 0;
    size_t part1 = 0; // light
    size_t part2 = 0; // dark

    for (ptrdiff_t i=0; i<bwImage.width(); i++) {

        for (ptrdiff_t j=0; j<bwImage.height(); j++) {

            curColor = (size_t)bwImage.pixel(i, j);

            if ( curColor < threshColor ) { part2++; }
            else                          { part1++; }
        }
    }

    conc = (double)part1 / ( (double)part1 + (double)part2 );

    return true;
}
