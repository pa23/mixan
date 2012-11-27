/*
    mixan
    Analysis of granular material mixes and emulsions.

    File: graphics.cpp

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

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_symbol.h>
#include <qwt_series_data.h>
#include <qwt_plot_histogram.h>

#include <QPen>
#include <QPalette>
#include <QObject>

#include "graphics.h"
#include "material.h"
#include "granules.h"
#include "settings.h"
#include "mixfuns.h"
#include "constants.h"
#include "tmpfiles.h"

#include <memory>

using std::shared_ptr;

void createGraphics(QVector<QImage> &graphics,
                    const shared_ptr<const Material> &material1,
                    const shared_ptr<const Material> &material2,
                    const shared_ptr<const Settings> &settings,
                    const QString &path) {

    graphics.clear();

    //

    QwtText xAxisTitle(QObject::tr("gray color"));
    xAxisTitle.setFont(QFont("DejaVu Sans", 10));

    QwtText yAxisTitle("n_i / N");
    yAxisTitle.setFont(QFont("DejaVu Sans", 10));

    //

    shared_ptr<QwtPlot> plot3(new QwtPlot());
    plot3->setPalette(QPalette(QColor(Qt::white)));
    plot3->setFrameShape(QFrame::NoFrame);
    plot3->setFrameShadow(QFrame::Plain);
    plot3->setCanvasLineWidth(0);
    plot3->setAxisAutoScale(QwtPlot::xBottom, true);
    plot3->setAxisTitle(QwtPlot::xBottom, xAxisTitle);
    plot3->setAxisAutoScale(QwtPlot::yLeft, true);
    plot3->setAxisTitle(QwtPlot::yLeft, yAxisTitle);

    //

    QVector<double> x(256);
    for ( ptrdiff_t i=0; i<x.size(); i++ ) { x[i] = i; }

    QVector<double> y1 = material1->histogramRelValues(); // histogram  1
    QVector<double> y2 = material2->histogramRelValues(); // histogram  2
    QVector<double> y3 = material1->polynomValues();      // polynomial 1
    QVector<double> y4 = material2->polynomValues();      // polynomial 2

    //

    shared_ptr<QwtPlotCurve> curve1(new QwtPlotCurve()); // histogram 1
    curve1->setRenderHint(QwtPlotItem::RenderAntialiased);
    curve1->setStyle(QwtPlotCurve::NoCurve);
    curve1->setSymbol( new QwtSymbol(QwtSymbol::Ellipse, Qt::NoBrush,
                                     QPen(Qt::black), QSize(1, 1)) );

    curve1->setRawSamples(x.data(), y1.data(), x.size());
    curve1->attach(plot3.get());

    //

    shared_ptr<QwtPlotCurve> curve2(new QwtPlotCurve()); // histogram 2
    curve2->setRenderHint(QwtPlotItem::RenderAntialiased);
    curve2->setStyle(QwtPlotCurve::NoCurve);
    curve2->setSymbol( new QwtSymbol(QwtSymbol::Ellipse, Qt::NoBrush,
                                     QPen(Qt::black), QSize(1, 1)) );

    curve2->setRawSamples(x.data(), y2.data(), x.size());
    curve2->attach(plot3.get());

    //

    shared_ptr<QwtPlotCurve> curve3(new QwtPlotCurve()); // polynomial 1
    curve3->setRenderHint(QwtPlotItem::RenderAntialiased);

    curve3->setRawSamples(x.data(), y3.data(), x.size());

    //

    shared_ptr<QwtPlotCurve> curve4(new QwtPlotCurve()); // polynomial 2
    curve4->setRenderHint(QwtPlotItem::RenderAntialiased);

    curve4->setRawSamples(x.data(), y4.data(), x.size());

    //

    if ( settings->val_thrColDefMethod() == THRCOLDEFMETHOD_POLYAPPROX ) {

        curve3->attach(plot3.get());
        curve4->attach(plot3.get());
    }

    //

    shared_ptr<QwtPlotCurve> curve5(new QwtPlotCurve()); // threshold color
    curve5->setRenderHint(QwtPlotItem::RenderAntialiased);

    const double tcolm = defThreshColor(material1,
                                        material2,
                                        settings);

    QVector<double> x5(2, tcolm);
    QVector<double> y5(2);

    plot3->replot();

    y5[1] = plot3->axisInterval(QwtPlot::yLeft).maxValue();

    curve5->setRawSamples(x5.data(), y5.data(), x5.size());
    curve5->attach(plot3.get());

    //

    plot3->resize(600, 400);
    QPixmap pixmap1(plot3->size());
    plot3->render(&pixmap1);

    graphics.push_back(pixmap1.toImage());

    //

    if ( settings->val_createTmpImg() ) {

        saveGraphics(pixmap1, path);
    }
}

void createHistograms(QVector<QImage> &histograms,
                      const QVector< shared_ptr<Granules> > &granules,
                      const shared_ptr<const Settings> &settings,
                      const QString &path,
                      double &minArea,
                      double &maxArea,
                      double &meanArea,
                      double &meanCompact) {

    histograms.clear();

    //

    if ( granules.size() == 0 ) { return; }

    //

    QVector<double> areas = granules[0]->areaValues();
    QVector<double> compacts = granules[0]->compactValues();

    for ( ptrdiff_t n=1; n<granules.size(); n++ ) {

        areas += granules[n]->areaValues();
        compacts += granules[n]->compactValues();
    }

    if ( areas.isEmpty()    ||
         compacts.isEmpty() ||
         (areas.size() != compacts.size()) ) { return; }

    //

    HistXSetup histAreasXSet;
    histAreasXSet.minval = areas[0];
    histAreasXSet.maxval = areas[0];

    HistXSetup histCompactsXSet;
    histCompactsXSet.minval = compacts[0];
    histCompactsXSet.maxval = compacts[0];

    minArea = areas[0];
    maxArea = areas[0];
    meanArea = areas[0];
    meanCompact = compacts[0];

    for ( ptrdiff_t i=1; i<areas.size(); i++ ) {

        if ( areas[i] < minArea ) { minArea = areas[i]; }
        if ( areas[i] > maxArea ) { maxArea = areas[i]; }

        meanArea += areas[i];
        meanCompact += compacts[i];

        //

        if ( areas[i] < histAreasXSet.minval ) {

            histAreasXSet.minval = areas[i];
        }

        if ( areas[i] > histAreasXSet.maxval ) {

            histAreasXSet.maxval = areas[i];
        }

        if ( compacts[i] < histCompactsXSet.minval ) {

            histCompactsXSet.minval = compacts[i];
        }

        if ( compacts[i] > histCompactsXSet.maxval ) {

            histCompactsXSet.maxval = compacts[i];
        }
    }

    meanArea /= areas.size();
    meanCompact /= compacts.size();

    histAreasXSet.step =
            (histAreasXSet.maxval - histAreasXSet.minval) / HISTDIMENSION;

    histCompactsXSet.step =
            (histCompactsXSet.maxval - histCompactsXSet.minval) / HISTDIMENSION;

    //

    QVector<double> histAreasVls;
    histAreasVls.resize(HISTDIMENSION);

    QVector<double> histCompactsVls;
    histCompactsVls.resize(HISTDIMENSION);

    double tmpmin1 = histAreasXSet.minval;
    double tmpmax1 = tmpmin1 + histAreasXSet.step;

    for ( ptrdiff_t n=0; n<areas.size(); n++ ) {

        for ( ptrdiff_t i=0; i<HISTDIMENSION; i++ ) {

            if ( i == (HISTDIMENSION-1) ) {

                if ( areas[n]>=tmpmin1 &&
                     areas[n]<(tmpmax1+histAreasXSet.step) ) {

                    histAreasVls[i]++;
                    break;
                }
            }
            else {

                if ( areas[n]>=tmpmin1 && areas[n]<tmpmax1 ) {

                    histAreasVls[i]++;
                    break;
                }
            }

            tmpmin1 += histAreasXSet.step;
            tmpmax1 += histAreasXSet.step;
        }

        tmpmin1 = histAreasXSet.minval;
        tmpmax1 = tmpmin1 + histAreasXSet.step;
    }

    for ( ptrdiff_t i=0; i<histAreasVls.size(); i++ ) {

        histAreasVls[i] /= areas.size();
    }

    //

    double tmpmin2 = histCompactsXSet.minval;
    double tmpmax2 = tmpmin2 + histCompactsXSet.step;

    for ( ptrdiff_t n=0; n<compacts.size(); n++ ) {

        for ( ptrdiff_t i=0; i<HISTDIMENSION; i++ ) {

            if ( i == (HISTDIMENSION-1) ) {

                if ( compacts[n]>=tmpmin2 &&
                     compacts[n]<(tmpmax2+histCompactsXSet.step) ) {

                    histCompactsVls[i]++;
                    break;
                }
            }
            else {

                if ( compacts[n]>=tmpmin2 && compacts[n]<tmpmax2 ) {

                    histCompactsVls[i]++;
                    break;
                }
            }

            tmpmin2 += histCompactsXSet.step;
            tmpmax2 += histCompactsXSet.step;
        }

        tmpmin2 = histCompactsXSet.minval;
        tmpmax2 = tmpmin2 + histCompactsXSet.step;
    }

    for ( ptrdiff_t i=0; i<histCompactsVls.size(); i++ ) {

        histCompactsVls[i] /= compacts.size();
    }

    //

    QVector<QwtIntervalSample> hist1data;

    double tmpmin1g = histAreasXSet.minval;
    double tmpmax1g = tmpmin1g + histAreasXSet.step;

    for ( ptrdiff_t i=0; i<HISTDIMENSION; i++ ) {

        hist1data.push_back(QwtIntervalSample(histAreasVls[i],
                                              tmpmin1g,
                                              tmpmax1g));

        tmpmin1g += histAreasXSet.step;
        tmpmax1g += histAreasXSet.step;
    }

    //

    QwtText xAxisTitle1(QObject::tr("Granule area"));
    xAxisTitle1.setFont(QFont("Liberation Sans", 12));

    QwtText yAxisTitle1("n_i / N");
    yAxisTitle1.setFont(QFont("Liberation Sans", 12));

    shared_ptr<QwtPlot> histogram1(new QwtPlot());
    histogram1->setPalette(QPalette(QColor(Qt::white)));
    histogram1->setFrameShape(QFrame::NoFrame);
    histogram1->setFrameShadow(QFrame::Plain);
    histogram1->setCanvasLineWidth(0);
    histogram1->setAxisTitle(QwtPlot::xBottom, xAxisTitle1);
    histogram1->setAxisTitle(QwtPlot::yLeft, yAxisTitle1);

    shared_ptr<QwtPlotHistogram> hist1(new QwtPlotHistogram());
    hist1->setStyle(QwtPlotHistogram::Columns);
    hist1->setRenderHint(QwtPlotItem::RenderAntialiased);

    hist1->setSamples(hist1data);
    hist1->attach(histogram1.get());

    histogram1->resize(600, 400);
    histogram1->replot();

    QPixmap pixmap1(histogram1->size());
    histogram1->render(&pixmap1);

    histograms.push_back(pixmap1.toImage());

    //

    QVector<QwtIntervalSample> hist2data;

    double tmpmin2g = histCompactsXSet.minval;
    double tmpmax2g = tmpmin2g + histCompactsXSet.step;

    for ( ptrdiff_t i=0; i<HISTDIMENSION; i++ ) {

        hist2data.push_back(QwtIntervalSample(histCompactsVls[i],
                                              tmpmin2g,
                                              tmpmax2g));

        tmpmin2g += histCompactsXSet.step;
        tmpmax2g += histCompactsXSet.step;
    }

    //

    QwtText xAxisTitle2(QObject::tr("Granule circularity"));
    xAxisTitle2.setFont(QFont("DejaVu Sans", 12));

    QwtText yAxisTitle2("n_i / N");
    yAxisTitle2.setFont(QFont("DejaVu Sans", 12));

    shared_ptr<QwtPlot> histogram2(new QwtPlot());
    histogram2->setPalette(QPalette(QColor(Qt::white)));
    histogram2->setFrameShape(QFrame::NoFrame);
    histogram2->setFrameShadow(QFrame::Plain);
    histogram2->setCanvasLineWidth(0);
    histogram2->setAxisTitle(QwtPlot::xBottom, xAxisTitle2);
    histogram2->setAxisTitle(QwtPlot::yLeft, yAxisTitle2);

    shared_ptr<QwtPlotHistogram> hist2(new QwtPlotHistogram());
    hist2->setStyle(QwtPlotHistogram::Columns);
    hist2->setRenderHint(QwtPlotItem::RenderAntialiased);

    hist2->setSamples(hist2data);
    hist2->attach(histogram2.get());

    histogram2->resize(600, 400);
    histogram2->replot();

    QPixmap pixmap2(histogram2->size());
    histogram2->render(&pixmap2);

    histograms.push_back(pixmap2.toImage());

    //

    if ( settings->val_createTmpImg() ) {

        saveHistograms(histograms, path);
    }
}
