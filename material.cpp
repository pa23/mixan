/*
    mixan
    Analysis of granular material mixes and emulsions.

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

#include "material.h"
#include "numcompfuns.h"
#include "constants.h"

#include <vector>
#include <cmath>

#include <QVector>
#include <QImage>

using std::vector;
using std::size_t;
using std::ptrdiff_t;

Material::Material() {

    for ( ptrdiff_t i=0; i<256; i++ ) { histogram[i] = 0; }

    threshColor = 0;
}

Material::~Material() {
}

bool Material::isEmpty() const {

    if ( origImage.isNull() ) { return true;  }
    else                      { return false; }
}

bool Material::analyze(QString imgFileName, ptrdiff_t polyPwr) {

    fileName = "";
    for ( ptrdiff_t i=0; i<256; i++ ) { histogram[i] = 0; }

    polynomPower = polyPwr;

    //

    if ( !origImage.load(imgFileName) ) { return false; }

    if ( !defHistogram()   ) { return false; }
    if ( !defThreshColor() ) { return false; }

    fileName = imgFileName;

    //

    return true;
}

QString Material::imageFileName() const {

    return fileName;
}

QImage Material::originalImage() const {

    return origImage;
}

size_t Material::thresholdColor() const {

    return threshColor;
}

vector<double> Material::histogramValues() const {

    vector<double> hv(256, 0);

    for ( ptrdiff_t i=0; i<256; i++ ) { hv[i] = histogram[i]; }

    return hv;
}

vector<double> Material::polynomCoefficients() const {

    return polyCoeff;
}

vector<double> Material::polynomValues() const {

    return polyVal;
}

bool Material::defHistogram() {

    if ( origImage.isNull() ) { return false; }

    //

    double N = origImage.width() * origImage.height();

    for ( ptrdiff_t i=0; i<origImage.width(); i++ ) {

        for ( ptrdiff_t j=0; j<origImage.height(); j++ ) {

            histogram[(size_t)qGray(origImage.pixel(i, j))]++;
        }
    }

    for ( ptrdiff_t i=0; i<256; i++ ) {

        histogram[i] /= N;
    }

    return true;
}

bool Material::defThreshColor() {

    vector<double> x(256, 0);
    vector<double> y(256, 0);

    polyCoeff.clear();
    polyCoeff.resize(polynomPower+1, 0);

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

        for ( ptrdiff_t n=0; n<(polynomPower+1); n++ ) {

            p += polyCoeff[n] * pow( x[i], n );
        }

        polyVal[i] = p;
        p = 0;
    }

    //

    double maxpoly = 0;

    for ( ptrdiff_t i=0; i<256; i++ ) {

        if ( polyVal[i] > maxpoly ) {

            maxpoly = polyVal[i];
            threshColor = i;
        }
    }

    return true;
}
