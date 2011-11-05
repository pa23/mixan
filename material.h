/*
    mixan
    Analysis of granular material mixes and emulsions.

    File: granularmaterial.h

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

#ifndef MATERIAL_H
#define MATERIAL_H

#include <QString>
#include <QImage>

#include <vector>

class Material {

public:

    explicit Material();
    virtual ~Material();

    bool isEmpty() const;

    bool analyze(QString, ptrdiff_t);

    QString imageFileName() const;
    QImage originalImage() const;

    size_t thresholdColor() const; // gray color

    std::vector<double> histogramValues() const;
    std::vector<double> polynomCoefficients() const;
    std::vector<double> polynomValues() const;
    std::vector<ptrdiff_t> polynomLimits() const;

private:

    QString fileName;
    QImage origImage;

    double histogram[256];
    size_t threshColor;

    ptrdiff_t polynomPower;

    std::vector<double> polyCoeff;
    std::vector<double> polyVal;

    std::vector<ptrdiff_t> polylimits;

    bool defHistogram();
    bool defThreshColor();
    bool corrPolyVals();

};

#endif // MATERIAL_H
