/*
    KSysGuard, the KDE System Guard

    Copyright (c) 1999 - 2001 Chris Schlaeger <cs@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License version 2 or at your option version 3 as published by
    the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#ifndef KSG_DANCINGBARS_H
#define KSG_DANCINGBARS_H

#include <SensorDisplay.h>

#include <QBitArray>
#include <QVector>



class BarGraph;

class DancingBars : public KSGRD::SensorDisplay
{
  Q_OBJECT

  public:
    DancingBars( QWidget *parent, const QString &title, SharedSettings *workSheetSettings );
    virtual ~DancingBars();

    void configureSettings();

    bool addSensor( const QString &hostName, const QString &name,
                    const QString &type, const QString &title );
    bool removeSensor( uint pos );

    void updateSamples( const QVector<double> &samples );

    virtual void answerReceived( int id, const QList<QByteArray> &answerlist );

    bool restoreSettings( QDomElement& );
    bool saveSettings( QDomDocument&, QDomElement& );

    virtual bool hasSettingsDialog() const;

  public Q_SLOTS:
    virtual void applyStyle();

  private:
    uint mBars;

    BarGraph* mPlotter;

    /**
      The sample buffer and the flags are needed to store the incoming
      samples for each beam until all samples of the period have been
      received. The flags variable is used to ensure that all samples have
      been received.
     */
    QVector<double> mSampleBuffer;
    QBitArray mFlags;
};

#endif

