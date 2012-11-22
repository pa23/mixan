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
#include <QObject>

Mix::Mix(const QString &imgFileName,
         size_t threshCol,
         const Settings *set) :
    fileName(imgFileName),
    threshColor(threshCol),
    settings(set),
    conc(0) {
}

Mix::~Mix() {
}

void Mix::analyze() {

    if ( !origImage.load(fileName) ) {

        throw MixanError(
                    QObject::tr("Can not load image")
                    + " "
                    + fileName
                    + "!\n"
                    );
    }

    try {

        defConc();
    }
    catch(const MixanError &mixerr) {

        throw;
    }

    if ( !settings->val_showImgInReport() && !settings->val_createTmpImg() ) {

        origImage = QImage();
    }
}

void Mix::defConc() {

    if ( origImage.isNull() ) {

        throw MixanError(QObject::tr("No image!"));
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
