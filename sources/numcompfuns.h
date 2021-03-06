/*
    mixan
    Analysis of granular material mixes and emulsions.

    File: numcompfuns.h

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

#ifndef NUMCOMPFUNS_H
#define NUMCOMPFUNS_H

#include <QVector>

QVector<double> polyapprox(const QVector<double> &,  // x
                           const QVector<double> &,  // y
                           const ptrdiff_t);         // polyPower

// size of x must be equal of size y

#endif // NUMCOMPFUNS_H
