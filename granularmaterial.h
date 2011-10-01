/*
    mixan
    Analyze of granular material mix.

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

#ifndef GRANULARMATERIAL_H
#define GRANULARMATERIAL_H

#include <QString>
#include <QImage>

#include <vector>

class GranularMaterial {

public:

    explicit GranularMaterial();
    virtual ~GranularMaterial();

    bool isEmpty() const;
    bool analyze(QString);
    QImage originalImage() const;
    QImage blackwhiteImage() const;
    size_t thresholdColor() const;
    std::vector<double> polynomCoefficients() const;

protected:

    QImage origImage;
    QImage bwImage;
    size_t histogram[256];
    size_t threshColor;
    std::vector<double> polyCoeff;

    bool colorToBW();
    bool defThreshColor();

};

#endif // GRANULARMATERIAL_H
