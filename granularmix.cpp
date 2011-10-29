/*
    mixan
    Analysis of granular material mix and emulsions.

    File: granularmix.cpp

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

#include "granularmix.h"
#include "numcompfuns.h"
#include "constants.h"

#include <vector>
#include <cmath>

#include <QVector>
#include <QImage>

using std::vector;

GranularMix::GranularMix() :
    threshColor(0),
    conc(0) {
}

GranularMix::~GranularMix() {
}

size_t GranularMix::defThreshColor(size_t lcol, size_t dcol) {

    return (lcol + dcol) / 2;
}

bool GranularMix::isEmpty() const {

    if ( origImage.isNull() ) { return true;  }
    else                      { return false; }
}

bool GranularMix::analyze(QString imgFileName, size_t threshCol) {

    fileName = "";

    //

    threshColor = threshCol;

    if ( !origImage.load(imgFileName) ) { return false; }
    if ( !defConc()                   ) { return false; }

    fileName = imgFileName;

    //

    return true;
}

QString GranularMix::imageFileName() const {

    return fileName;
}

QImage GranularMix::originalImage() const {

    return origImage;
}

size_t GranularMix::thresholdColor() const {

    return threshColor;
}

double GranularMix::concentration() const {

    return conc;
}

bool GranularMix::defConc() {

    if ( origImage.isNull() ) { return false; }

    size_t part1 = 0; // light
    size_t part2 = 0; // dark

    for (ptrdiff_t i=0; i<origImage.width(); i++) {

        for (ptrdiff_t j=0; j<origImage.height(); j++) {

            if ( (size_t)qGray(origImage.pixel(i, j)) < threshColor ) {

                part2++;
            }
            else {

                part1++;
            }
        }
    }

    conc = (double)part1 / ( (double)part1 + (double)part2 );

    return true;
}
