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

#include <QString>

class Settings {

public:

    Settings();

    void setPolyPwr(const ptrdiff_t);
    void setThrAccur(const double);
    void setIdealConc(const double);
    void setsizeinmm(const bool);
    void setpxpermm2(const double);
    void setSievesCellDiameter(const QString &);
    void setSievesCellDimension(const QString &);
    void setShowImgInReport(const bool);
    void setCreateTmpImg(const bool);
    void setImgWidth(const ptrdiff_t);

    ptrdiff_t val_polyPwr()             const { return polyPwr;             }
    double    val_thrAccur()            const { return thrAccur;            }
    double    val_idealConc()           const { return idealConc;           }
    bool      val_sizeinmm()            const { return sizeinmm;            }
    double    val_pxpermm2()            const { return pxpermm2;            }
    QString   val_sievesCellDiameter()  const { return sievesCellDiameter;  }
    QString   val_sievesCellDimension() const { return sievesCellDimension; }
    bool      val_showImgInReport()     const { return showImgInReport;     }
    bool      val_createTmpImg()        const { return createTmpImg;        }
    ptrdiff_t val_imgWidth()            const { return imgWidth;            }

private:

    ptrdiff_t polyPwr;
    double    thrAccur;
    double    idealConc;
    bool      sizeinmm;
    double    pxpermm2;
    QString   sievesCellDiameter;
    QString   sievesCellDimension;
    bool      showImgInReport;
    bool      createTmpImg;
    ptrdiff_t imgWidth;

};

#endif // SETTINGS_H
