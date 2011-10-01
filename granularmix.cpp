/*
    mixan
    Analyze of granular material mix.

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

GranularMix::GranularMix() {

    threshColor = 0;
    lightThreshColor = 0;
    darkThreshColor = 0;
    conc = 0;

    for ( ptrdiff_t i=0; i<256; i++ ) { histogram[i] = 0; }
}

GranularMix::~GranularMix() {
}

bool GranularMix::analyze(QString imgFileName,
                          size_t lcol, size_t dcol) {

    if ( !origImage.load(imgFileName) ) { return false; }
    bwImage = origImage;

    lightThreshColor = lcol;
    darkThreshColor = dcol;

    //

    if ( !colorToBW()      ) { return false; }
    if ( !defThreshColor() ) { return false; }
    if ( !defConc()        ) { return false; }

    for ( ptrdiff_t i=0; i<256; i++ ) { histogram[i] = 0; }

    return true;
}

double GranularMix::concentration() const {

    return conc;
}

bool GranularMix::defThreshColor() {

    threshColor = (lightThreshColor + darkThreshColor) / 2;

    return true;
}

bool GranularMix::defConc() {

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
