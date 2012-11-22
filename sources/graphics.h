/*
    mixan
    Analysis of granular material mixes and emulsions.

    File: graphics.h

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

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <QVector>
#include <QSharedPointer>
#include <QImage>
#include <QString>

#include "material.h"
#include "granules.h"
#include "settings.h"

struct HistXSetup {

    double minval;
    double maxval;
    double step;
};

void createGraphics(QVector<QImage> &,
                    const QSharedPointer<Material> &,
                    const QSharedPointer<Material> &,
                    const QSharedPointer<Settings> &,
                    const QString &);

void createHistograms(QVector<QImage> &,
                      const QVector< QSharedPointer<Granules> > &,
                      const QSharedPointer<Settings> &,
                      const QString &,
                      double,
                      double,
                      double,
                      double);

#endif // GRAPHICS_H
