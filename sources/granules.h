/*
    mixan
    Analysis of granular material mixes and emulsions.

    File: granules.h

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

#ifndef GRANULES_H
#define GRANULES_H

#include <QString>
#include <QVector>
#include <QImage>

#include <opencv/cv.h>

struct HistXSetup {

    double minval;
    double maxval;
    double step;
};

class Granules {

public:

    Granules(const QString &, const size_t &, const size_t &);
    virtual ~Granules();

    void analyze();

    HistXSetup      histXSetup()    const { return histXSet;    }
    QVector<double> histValues()    const { return histVls;     }
    QString         imageFileName() const { return imgFileName; }
    QImage          resImage()      const { return img;         }

private:

    QString imgFileName;
    size_t limCol1;
    size_t limCol2;

    QImage img;

    QVector<double> areas;
    HistXSetup histXSet;
    QVector<double> histVls;

    void findAreas();
    void IplImage2QImage(const IplImage *);
    void defHistData();

};

#endif // GRANULES_H
