/*
    mixan
    Analysis of granular material mixes and emulsions.

    File: mix.cpp

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

#include "mix.h"
#include "mixanerror.h"

#include <cmath>

#include <QVector>
#include <QImage>

Mix::Mix(const QString &imgFileName, const size_t &threshCol) :
    threshColor(0),
    conc(0) {

    fileName = imgFileName;
    threshColor = threshCol;
}

Mix::~Mix() {
}

bool Mix::isEmpty() const {

    if ( origImage.isNull() ) { return true;  }
    else                      { return false; }
}

void Mix::analyze() {

    if ( !origImage.load(fileName) ) {

        throw MixanError("Can not load image " + fileName + "!");
    }

    try {

        defConc();
    }
    catch(MixanError &mixerr) {

        throw;
    }
}

void Mix::defConc() {

    if ( origImage.isNull() ) {

        throw MixanError("No image!");
    }

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
}
