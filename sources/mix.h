/*
    mixan
    Analysis of granular material mixes and emulsions.

    File: mix.h

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

#ifndef MIX_H
#define MIX_H

#include <QString>
#include <QImage>

#include "material.h"
#include "settings.h"

class Mix {

public:

    Mix(const QString &, const size_t, const Settings *);
    ~Mix();

    void analyze();

    QString imageFileName() const { return fileName;    }
    QImage originalImage()  const { return origImage;   }

    size_t thresholdColor() const { return threshColor; }
    double concentration()  const { return conc;        }

private:

    Mix(const Mix &);
    Mix & operator=(const Mix &);

    QString fileName;
    size_t threshColor;
    const Settings *settings;

    QImage origImage;
    double conc;

    void defConc();

};

#endif // MIX_H
