/*
    mixan
    Analyze of granular material mix.

    File: probe.h

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

#ifndef PROBE_H
#define PROBE_H

#include <QString>
#include <QImage>

class Probe {

public:

    explicit Probe(QString);
    virtual ~Probe();

    bool analyze();
    QImage *originalImage();
    QImage *blackwhiteImage();
    size_t thresholdColor() const;
    double concentration() const;

private:

    QImage origImage;
    QImage bwImage;
    size_t *histogram;
    size_t threshColor;
    double conc;

    bool colorToBW();
    bool defThreshColor();
    bool defConc();

};

#endif // PROBE_H
