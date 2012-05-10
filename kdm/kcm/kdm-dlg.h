/*
    Copyright (C) 1997 Thomas Tanghus (tanghus@earthling.net)

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/


#ifndef __KDMDLG_H__
#define __KDMDLG_H__

#include <QWidget>

class KLineEdit;
class Positioner;
class QLabel;
class QPushButton;
class QRadioButton;

class KDMDialogWidget : public QWidget {
    Q_OBJECT

  public:
    KDMDialogWidget(QWidget *parent);

    void load();
    void save();
    void defaults();
    QString quickHelp() const;

    bool eventFilter(QObject *, QEvent *);

  Q_SIGNALS:
    void changed();

  protected:
    void iconLoaderDragEnterEvent(QDragEnterEvent *event);
    void iconLoaderDropEvent(QDropEvent *event);
    bool setLogo(const QString &logo);

  private Q_SLOTS:
    void slotAreaRadioClicked(int id);
    void slotLogoButtonClicked();

  private:
    enum { KdmNone, KdmClock, KdmLogo };
    QLabel       *logoLabel;
    QPushButton  *logobutton;
    KLineEdit    *greetstr_lined;
    QString      logopath;
    QRadioButton *noneRadio;
    QRadioButton *clockRadio;
    QRadioButton *logoRadio;
    Positioner   *positioner;

};

#endif
