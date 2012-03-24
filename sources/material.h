/*
    mixan
    Analysis of granular material mixes and emulsions.

    File: material.h

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

#ifndef MATERIAL_H
#define MATERIAL_H

#include <QString>
#include <QImage>
#include <QVector>

class Material {

public:

    Material();
    virtual ~Material();

    void analyze(const QString &, const ptrdiff_t &);
    void clear();
    bool isEmpty() const;

    QString imageFileName()            const { return fileName;    }
    QImage originalImage()             const { return origImage;   }
    size_t thresholdColor()            const { return threshColor; }
    QVector<double> histogramValues()  const { return histogram;   }
    QVector<double> polynomValues()    const { return polyVal;     }
    QVector<ptrdiff_t> polynomLimits() const { return polylimits;  }

private:

    QString fileName;
    QImage origImage;

    QVector<double> histogram;
    size_t threshColor;

    ptrdiff_t polynomPower;
    QVector<double> polyVal;
    QVector<ptrdiff_t> polylimits;

    void defHistogram();
    void defThreshColor();
    void corrPolyVals();

};

#endif // MATERIAL_H
