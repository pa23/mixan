/*
    mixan
    Analysis of granular material mixes and emulsions.

    File: material.cpp

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

#include "material.h"
#include "numcompfuns.h"
#include "constants.h"
#include "mixanerror.h"

#include <cmath>
#include <numeric>

#include <QVector>
#include <QImage>
#include <QObject>

Material::Material() :
    threshColor(0),
    polynomPower(0) {

    histogram.clear();
    histogram.resize(256);
}

Material::~Material() {
}

void Material::analyze(const QString &imgFileName, const Settings *settings) {

    fileName.clear();

    histogram.clear();
    histogram.resize(256);

    polynomPower = settings->val_polyPwr();

    //

    if ( !origImage.load(imgFileName) ) {

        throw MixanError(
                    QObject::tr("Can not load image")
                    + " "
                    + imgFileName
                    + "!\n"
                    );
    }

    try {

        defHistogram();
        defThreshColor();
    }
    catch(const MixanError &mixerr) {

        throw;
    }

    fileName = imgFileName;

    if ( !settings->val_showImgInReport() && !settings->val_createTmpImg() ) {

        origImage = QImage();
    }
}

void Material::clear() {

    fileName.clear();
    origImage = QImage();
    histogram.clear();
    threshColor = 0;
    polynomPower = 0;
    polyVal.clear();
    polylimits.clear();
}

void Material::defHistogram() {

    if ( origImage.isNull() ) {

        throw MixanError(QObject::tr("No image!"));
    }

    //

    const double N = origImage.width() * origImage.height();

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
    for ( ptrdiff_t i=0; i<256; i++ ) { x[i] = i; }

    //

    QVector<double> polyCoeff;

    try {

        polyCoeff = polyapprox(x, histogram, polynomPower);
    }
    catch(const MixanError &mixerr) {

        throw;
    }

    //

    double p = 0;

    for ( ptrdiff_t i=0; i<256; i++ ) {

        for ( ptrdiff_t n=0; n<(polynomPower+1); n++ ) {

            p += polyCoeff[n] * pow(x[i], n);
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

    if ( polylimits[0] != 0 ) {

        for ( ptrdiff_t i=0; i<=polylimits[0]; i++ ) { polyVal[i] = 0; }
    }

    if ( polylimits[1] != 255 ) {

        for ( ptrdiff_t i=polylimits[1]; i<=255; i++ ) { polyVal[i] = 0; }
    }
}
