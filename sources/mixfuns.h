/*
    mixan
    Analysis of granular material mixes and emulsions.

    File: mixfuns.h

    Copyright (C) 2011-2015 Artem Petrov <pa2311@gmail.com>

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

#ifndef MIXFUNS_H
#define MIXFUNS_H

#include <QVector>

#include "material.h"
#include "granules.h"
#include "settings.h"

#include <memory>

double Vc(const QVector<double> &,
          const double);

size_t defThreshColor(const std::shared_ptr<const Material> &,
                      const std::shared_ptr<const Material> &,
                      const std::shared_ptr<const Settings> &);

void defRemainders(const QVector< std::shared_ptr<Granules> > &,
                   QVector<double> &,
                   QVector<double> &,
                   QVector<double> &);

#endif // MIXFUNS_H
