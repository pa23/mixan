/*
    mixan
    Analysis of granular material mix.

    File: granularmaterial.cpp

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

#include "granularmaterial.h"
#include "numcompfuns.h"
#include "constants.h"

#include <vector>
#include <cmath>

#include <QVector>
#include <QImage>

using std::vector;
using std::size_t;
using std::ptrdiff_t;

GranularMaterial::GranularMaterial() {

    for ( ptrdiff_t i=0; i<256; i++ ) { histogram[i] = 0; }

    threshColor = 0;
}

GranularMaterial::~GranularMaterial() {
}

bool GranularMaterial::isEmpty() const {

    if ( origImage.isNull() ) { return true;  }
    else                      { return false; }
}

bool GranularMaterial::analyze(QString imgFileName) {

    fileName = "";
    for ( ptrdiff_t i=0; i<256; i++ ) { histogram[i] = 0; }

    //

    if ( !origImage.load(imgFileName) ) { return false; }

    if ( !defHistogram()   ) { return false; }
    if ( !defThreshColor() ) { return false; }

    fileName = imgFileName;

    //

    return true;
}

QString GranularMaterial::imageFileName() const {

    return fileName;
}

QImage GranularMaterial::originalImage() const {

    return origImage;
}

size_t GranularMaterial::thresholdColor() const {

    return threshColor;
}

vector<size_t> GranularMaterial::histogramValues() const {

    vector<size_t> hv(256, 0);

    for ( ptrdiff_t i=0; i<256; i++ ) { hv[i] = histogram[i]; }

    return hv;
}

vector<double> GranularMaterial::polynomCoefficients() const {

    return polyCoeff;
}

vector<double> GranularMaterial::polynomValues() const {

    return polyVal;
}

bool GranularMaterial::defHistogram() {

    if ( origImage.isNull() ) { return false; }

    for ( ptrdiff_t i=0; i<origImage.width(); i++ ) {

        for ( ptrdiff_t j=0; j<origImage.height(); j++ ) {

            histogram[(size_t)qGray(origImage.pixel(i, j))]++;
        }
    }

    return true;
}

bool GranularMaterial::defThreshColor() {

    vector<double> x(256, 0);
    vector<double> y(256, 0);

    polyCoeff.clear();
    polyCoeff.resize(POLYPOWER+1, 0);

    for ( ptrdiff_t i=0; i<256; i++ ) {

        x[i] = i;
        y[i] = histogram[i];
    }

    //

    if ( !polyapprox(&x, &y, &polyCoeff) ) { return false; }

    //

    polyVal.clear();
    polyVal.resize(256, 0);

    double p = 0;

    for ( ptrdiff_t i=0; i<256; i++ ) {

        for ( ptrdiff_t n=0; n<(POLYPOWER+1); n++ ) {

            p += polyCoeff[n] * pow( x[i], n );
        }

        polyVal[i] = p;
        p = 0;
    }

    //

    ptrdiff_t maxpoly = 0;

    for ( ptrdiff_t i=0; i<256; i++ ) {

        if ( polyVal[i] > maxpoly ) {

            maxpoly = polyVal[i];
            threshColor = i;
        }
    }

    return true;
}
