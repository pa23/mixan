/*
    mixan
    Analysis of granular material mixes and emulsions.

    File: settings.h

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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <iostream>

using std::ptrdiff_t;

class Settings {

public:

    Settings();

    void setPolyPwr(ptrdiff_t);
    void setThrAccur(double);
    void setIdealConc(double);
    void setImgWidth(ptrdiff_t);
    void setCreateTmpImg(bool);

    ptrdiff_t val_polyPwr()      const { return polyPwr;      }
    double    val_thrAccur()     const { return thrAccur;     }
    double    val_idealConc()    const { return idealConc;    }
    ptrdiff_t val_imgWidth()     const { return imgWidth;     }
    bool      val_createTmpImg() const { return createTmpImg; }

private:

    ptrdiff_t polyPwr;
    double    thrAccur;
    double    idealConc;
    ptrdiff_t imgWidth;
    bool      createTmpImg;

};

#endif // SETTINGS_H
