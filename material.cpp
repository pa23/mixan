/*
    mixan
    Analysis of granular material mixes and emulsions.

    File: material.cpp

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
#include "mixanerror.h"

#include <cmath>
#include <numeric>

#include <QVector>
#include <QImage>

Material::Material() :
    threshColor(0),
    polynomPower(0) {

    histogram.clear();
    histogram.resize(256);
}

Material::~Material() {
}

bool Material::isEmpty() const {

    if ( origImage.isNull() ) { return true;  }
    else                      { return false; }
}

void Material::analyze(const QString &imgFileName, const ptrdiff_t &polyPwr) {

    fileName = "";

    histogram.clear();
    histogram.resize(256);

    polynomPower = polyPwr;

    //

    if ( !origImage.load(imgFileName) ) {

        throw MixanError("Errors during loading image file " +
                         imgFileName + "!");
    }

    try {

        defHistogram();
        defThreshColor();
    }
    catch(MixanError &mixerr) {

        throw;
    }

    fileName = imgFileName;
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

QVector<double> Material::histogramValues() const {

    return histogram;
}

QVector<double> Material::polynomValues() const {

    return polyVal;
}

QVector<ptrdiff_t> Material::polynomLimits() const {

    return polylimits;
}

void Material::defHistogram() {

    if ( origImage.isNull() ) {

        throw MixanError("No image!");
    }

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
}

void Material::defThreshColor() {

    polyVal.clear();
    polyVal.resize(256);

    //

    QVector<double> x(256);
    QVector<double> y(256);

    for ( ptrdiff_t i=0; i<256; i++ ) {

        x[i] = i;
        y[i] = histogram[i];
    }

    //

    QVector<double> polyCoeff;

    try {

        polyCoeff = polyapprox(x, y, polynomPower);
    }
    catch(MixanError &mixerr) {

        throw;
    }

    //

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

    //

    corrPolyVals();
}

void Material::corrPolyVals() {

    polylimits.clear();
    polylimits.resize(2);

    for ( ptrdiff_t i=threshColor; i>=0; i-- ) {

        if ( polyVal[i] < 0 ) {

            polylimits[0] = i;
            break;
        }
    }

    for ( ptrdiff_t i=threshColor; i<=255; i++ ) {

        if ( polyVal[i] < 0 ) {

            polylimits[1] = i;
            break;
        }
    }

    //

    for ( ptrdiff_t i=0; i<=polylimits[0]; i++   ) { polyVal[i] = 0; }
    for ( ptrdiff_t i=polylimits[1]; i<=255; i++ ) { polyVal[i] = 0; }
}
