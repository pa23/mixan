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

void createGraphics(QVector<QImage> &graphics,
                    const QSharedPointer<Material> &material1,
                    const QSharedPointer<Material> &material2,
                    const QSharedPointer<Settings> &settings,
                    const QString &path) {

    graphics.clear();

    //

    QVector<double> x(256);
    for ( ptrdiff_t i=0; i<x.size(); i++ ) { x[i] = i; }

    //

    QwtText xAxisTitle(QObject::tr("gray color"));
    xAxisTitle.setFont(QFont("DejaVu Sans", 12));

    QwtText yAxisTitle("n_i / N");
    yAxisTitle.setFont(QFont("DejaVu Sans", 12));

    //

    QSharedPointer<QwtPlot> plot1(new QwtPlot());
    plot1->setPalette(QPalette(QColor(Qt::white)));
    plot1->setFrameShape(QFrame::NoFrame);
    plot1->setFrameShadow(QFrame::Plain);
    plot1->setCanvasLineWidth(0);
    plot1->setAxisAutoScale(QwtPlot::xBottom, true);
    plot1->setAxisTitle(QwtPlot::xBottom, xAxisTitle);
    plot1->setAxisAutoScale(QwtPlot::yLeft, true);
    plot1->setAxisTitle(QwtPlot::yLeft, yAxisTitle);

    QSharedPointer<QwtPlotCurve> curve11(new QwtPlotCurve());
    curve11->setRenderHint(QwtPlotItem::RenderAntialiased);
    curve11->setStyle(QwtPlotCurve::NoCurve);
    curve11->setSymbol( new QwtSymbol(QwtSymbol::Ellipse, Qt::NoBrush,
                                      QPen(Qt::black), QSize(1, 1)) );

    QVector<double> y11 = material1->histogramValues();

    curve11->setRawSamples(x.data(), y11.data(), x.size());
    curve11->attach(plot1.data());

    QSharedPointer<QwtPlotCurve> curve12(new QwtPlotCurve());
    curve12->setRenderHint(QwtPlotItem::RenderAntialiased);

    QVector<double> y12 = material1->polynomValues();

    curve12->setRawSamples(x.data(), y12.data(), x.size());
    curve12->attach(plot1.data());

    plot1->resize(600, 400);
    plot1->replot();

    QPixmap pixmap1(plot1->size());
    plot1->render(&pixmap1);

    graphics.push_back(pixmap1.toImage());

    //

    QSharedPointer<QwtPlot> plot2(new QwtPlot());
    plot2->setPalette(QPalette(QColor(Qt::white)));
    plot2->setFrameShape(QFrame::NoFrame);
    plot2->setFrameShadow(QFrame::Plain);
    plot2->setCanvasLineWidth(0);
    plot2->setAxisAutoScale(QwtPlot::xBottom, true);
    plot2->setAxisTitle(QwtPlot::xBottom, xAxisTitle);
    plot2->setAxisAutoScale(QwtPlot::yLeft, true);
    plot2->setAxisTitle(QwtPlot::yLeft, yAxisTitle);

    QSharedPointer<QwtPlotCurve> curve21(new QwtPlotCurve());
    curve21->setRenderHint(QwtPlotItem::RenderAntialiased);
    curve21->setStyle(QwtPlotCurve::NoCurve);
    curve21->setSymbol( new QwtSymbol(QwtSymbol::Ellipse, Qt::NoBrush,
                                      QPen(Qt::black), QSize(1, 1)) );

    QVector<double> y21 = material2->histogramValues();

    curve21->setRawSamples(x.data(), y21.data(), x.size());
    curve21->attach(plot2.data());

    QSharedPointer<QwtPlotCurve> curve22(new QwtPlotCurve());
    curve22->setRenderHint(QwtPlotItem::RenderAntialiased);

    QVector<double> y22 = material2->polynomValues();

    curve22->setRawSamples(x.data(), y22.data(), x.size());
    curve22->attach(plot2.data());

    plot2->resize(600, 400);
    plot2->replot();

    QPixmap pixmap2(plot2->size());
    plot2->render(&pixmap2);

    graphics.push_back(pixmap2.toImage());

    //

    QSharedPointer<QwtPlot> plot3(new QwtPlot());
    plot3->setPalette(QPalette(QColor(Qt::white)));
    plot3->setFrameShape(QFrame::NoFrame);
    plot3->setFrameShadow(QFrame::Plain);
    plot3->setCanvasLineWidth(0);
    plot3->setAxisAutoScale(QwtPlot::xBottom, true);
    plot3->setAxisTitle(QwtPlot::xBottom, xAxisTitle);
    plot3->setAxisAutoScale(QwtPlot::yLeft, true);
    plot3->setAxisTitle(QwtPlot::yLeft, yAxisTitle);

    QSharedPointer<QwtPlotCurve> curve31(new QwtPlotCurve());
    curve31->setRenderHint(QwtPlotItem::RenderAntialiased);
    curve31->setRawSamples(x.data(), y12.data(), x.size());
    curve31->attach(plot3.data());

    QSharedPointer<QwtPlotCurve> curve31_s(new QwtPlotCurve());
    curve31_s->setRenderHint(QwtPlotItem::RenderAntialiased);
    curve31_s->setStyle(QwtPlotCurve::NoCurve);
    curve31_s->setSymbol( new QwtSymbol(QwtSymbol::Ellipse, Qt::NoBrush,
                                        QPen(Qt::black), QSize(7, 7)) );

    QVector<double> x_s;
    QVector<double> y12_s;

    for ( ptrdiff_t i=0; i<x.size(); i+=10 ) {

        x_s.push_back(x[i]);
        y12_s.push_back(y12[i]);
    }

    curve31_s->setRawSamples(x_s.data(), y12_s.data(), x_s.size());
    curve31_s->attach(plot3.data());

    QSharedPointer<QwtPlotCurve> curve32(new QwtPlotCurve());
    curve32->setRenderHint(QwtPlotItem::RenderAntialiased);
    curve32->setRawSamples(x.data(), y22.data(), x.size());
    curve32->attach(plot3.data());

    QSharedPointer<QwtPlotCurve> curve32_s(new QwtPlotCurve());
    curve32_s->setRenderHint(QwtPlotItem::RenderAntialiased);
    curve32_s->setStyle(QwtPlotCurve::NoCurve);
    curve32_s->setSymbol( new QwtSymbol(QwtSymbol::Triangle, Qt::NoBrush,
                                        QPen(Qt::black), QSize(7, 7)) );

    QVector<double> y22_s;

    for ( ptrdiff_t i=0; i<x.size(); i+=10 ) {

        y22_s.push_back(y22[i]);
    }

    curve32_s->setRawSamples(x_s.data(), y22_s.data(), x_s.size());
    curve32_s->attach(plot3.data());

    QSharedPointer<QwtPlotCurve> curve33(new QwtPlotCurve());
    curve33->setRenderHint(QwtPlotItem::RenderAntialiased);

    const double tcolm = defThreshColor(material1.data(),
                                        material2.data(),
                                        settings->val_thrAccur());

    const double max1 = y12[material1->thresholdColor()];
    const double max2 = y22[material2->thresholdColor()];

    QVector<double> x33(2, tcolm);
    QVector<double> y33(2);

    if ( max1 > max2 ) { y33[1] = max1; }
    else               { y33[1] = max2; }

    curve33->setRawSamples(x33.data(), y33.data(), x33.size());
    curve33->attach(plot3.data());

    plot3->resize(600, 400);
    plot3->replot();

    QPixmap pixmap3(plot3->size());
    plot3->render(&pixmap3);

    graphics.push_back(pixmap3.toImage());

    //

    if ( settings->val_createTmpImg() ) {

        saveGraphics(pixmap1, pixmap2, pixmap3, path);
    }
}

void createHistograms(QVector<QImage> &histograms,
                      const QVector< QSharedPointer<Granules> > &granules,
                      const QSharedPointer<Settings> &settings,
                      const QString &path,
                      double minArea,
                      double maxArea,
                      double meanArea,
                      double meanCompact) {

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

    QSharedPointer<QwtPlot> histogram1(new QwtPlot());
    histogram1->setPalette(QPalette(QColor(Qt::white)));
    histogram1->setFrameShape(QFrame::NoFrame);
    histogram1->setFrameShadow(QFrame::Plain);
    histogram1->setCanvasLineWidth(0);
    histogram1->setAxisTitle(QwtPlot::xBottom, xAxisTitle1);
    histogram1->setAxisTitle(QwtPlot::yLeft, yAxisTitle1);

    QSharedPointer<QwtPlotHistogram> hist1(new QwtPlotHistogram());
    hist1->setStyle(QwtPlotHistogram::Columns);
    hist1->setRenderHint(QwtPlotItem::RenderAntialiased);

    hist1->setSamples(hist1data);
    hist1->attach(histogram1.data());

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

    QSharedPointer<QwtPlot> histogram2(new QwtPlot());
    histogram2->setPalette(QPalette(QColor(Qt::white)));
    histogram2->setFrameShape(QFrame::NoFrame);
    histogram2->setFrameShadow(QFrame::Plain);
    histogram2->setCanvasLineWidth(0);
    histogram2->setAxisTitle(QwtPlot::xBottom, xAxisTitle2);
    histogram2->setAxisTitle(QwtPlot::yLeft, yAxisTitle2);

    QSharedPointer<QwtPlotHistogram> hist2(new QwtPlotHistogram());
    hist2->setStyle(QwtPlotHistogram::Columns);
    hist2->setRenderHint(QwtPlotItem::RenderAntialiased);

    hist2->setSamples(hist2data);
    hist2->attach(histogram2.data());

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
