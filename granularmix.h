/*
    mixan
    Analysis of granular material mix and emulsions.

    File: granularmix.h

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

#ifndef GRANULARMIX_H
#define GRANULARMIX_H

#include <QString>
#include <QImage>

class GranularMix {

public:

    explicit GranularMix();
    virtual ~GranularMix();

    static size_t defThreshColor(size_t, size_t);

    bool isEmpty() const;

    bool analyze(QString, size_t);

    QString imageFileName() const;
    QImage originalImage() const;

    size_t thresholdColor() const;
    double concentration() const;

private:

    QString fileName;
    QImage origImage;

    size_t threshColor;
    double conc;

    bool defConc();

};

#endif // GRANULARMIX_H
