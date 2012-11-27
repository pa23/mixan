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
#include <memory>

#include <QVector>
#include <QImage>
#include <QObject>

using std::shared_ptr;

Material::Material() :
    fileName(),
    origImage(),
    histogramAbs(),
    histogramRel(),
    threshColor(0),
    polynomPower(0),
    polyVal(),
    polylimits() {

    histogramAbs.clear();
    histogramAbs.resize(256);

    histogramRel.clear();
    histogramRel.resize(256);
}

Material::~Material() {
}

void Material::analyze(const QString &imgFileName,
                       const shared_ptr<const Settings> &settings) {

    fileName.clear();

    histogramAbs.clear();
    histogramAbs.resize(256);

    histogramRel.clear();
    histogramRel.resize(256);

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

    defHistogram();

    if ( settings->val_thrColDefMethod() == THRCOLDEFMETHOD_POLYAPPROX ) {

        defThreshColorPA();
    }
    else {

        defThreshColorGC();
    }

    //

    fileName = imgFileName;

    if ( !settings->val_showImgInReport() && !settings->val_createTmpImg() ) {

        origImage = QImage();
    }
}

void Material::clear() {

    fileName.clear();
    origImage = QImage();
    histogramAbs.clear();
    histogramRel.clear();
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

            histogramAbs[(size_t)qGray(origImage.pixel(i, j))]++;
        }
    }

    for ( ptrdiff_t i=0; i<256; i++ ) {

        histogramRel[i] = histogramAbs[i] / N;
    }
}

void Material::defThreshColorPA() {

    polyVal.clear();
    polyVal.resize(256);

    //

    QVector<double> x(256);
    for ( ptrdiff_t i=0; i<256; i++ ) { x[i] = i; }

    //

    QVector<double> polyCoeff;
    polyCoeff = polyapprox(x, histogramRel, polynomPower);

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

void Material::defThreshColorGC() {

    double My = 0;
    const double S = origImage.width() * origImage.height();

    for ( ptrdiff_t i=0; i<256; i++ ) {

        My += i * histogramAbs[i];
    }

    threshColor = My / S;
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
