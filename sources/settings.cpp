/*
    mixan
    Analysis of granular material mixes and emulsions.

    File: settings.cpp

    Copyright (C) 2012 Artem Petrov <pa2311@gmail.com>

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

#include "settings.h"

Settings::Settings() :
    polyPwr  (0),
    thrAccur (0),
    idealConc(0),
    sizeinmm (true),
    pxpermm2 (0),
    showImgInReport(true),
    createTmpImg(false),
    imgWidth (0) {
}

void Settings::setPolyPwr(const ptrdiff_t pp) {

    polyPwr = pp;
}

void Settings::setThrAccur(const double ta) {

    thrAccur = ta;
}

void Settings::setIdealConc(const double ic) {

    idealConc = ic;
}

void Settings::setsizeinmm(const bool sizemm) {

    sizeinmm = sizemm;
}

void Settings::setpxpermm2(const double pxmm) {

    pxpermm2 = pxmm;
}

void Settings::setSievesCellDiameter(const QString &d) {

    sievesCellDiameter = d;
}

void Settings::setSievesCellDimension(const QString &d) {

    sievesCellDimension = d;
}

void Settings::setShowImgInReport(const bool sir) {

    showImgInReport = sir;
}

void Settings::setCreateTmpImg(const bool cti) {

    createTmpImg = cti;
}

void Settings::setImgWidth(const ptrdiff_t iw) {

    imgWidth = iw;
}
