/*
    mixan
    Analysis of granular material mixes and emulsions.

    File: tmpfiles.h

    Copyright (C) 2012-2015 Artem Petrov <pa2311@gmail.com>

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

#ifndef TMPFILES_H
#define TMPFILES_H

#include <QPixmap>
#include <QImage>
#include <QVector>

#include "granules.h"

#include <memory>

void saveGraphics(const QPixmap &,
                  const QString &);

void saveHistograms(const QVector<QImage> &,
                    const QString &);

void saveImages(const QVector< std::shared_ptr<Granules> > &,
                const QString &);

#endif // TMPFILES_H
