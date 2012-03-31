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
    imgWidth (0),
    createTmpImg(false) {
}

void Settings::setPolyPwr(ptrdiff_t pp)  { polyPwr      = pp;  }
void Settings::setThrAccur(double ta)    { thrAccur     = ta;  }
void Settings::setIdealConc(double ic)   { idealConc    = ic;  }
void Settings::setImgWidth(ptrdiff_t iw) { imgWidth     = iw;  }
void Settings::setCreateTmpImg(bool cti) { createTmpImg = cti; }
