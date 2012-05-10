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

#ifndef __KDMGEN_H__
#define __KDMGEN_H__

#include <QWidget>

class KBackedComboBox;
class KFontRequester;
class KLanguageButton;
class QCheckBox;

class KDMGeneralWidget : public QWidget {
    Q_OBJECT

  public:
    KDMGeneralWidget(QWidget *parent = 0);

    void load();
    void save();
    void defaults();

    void loadColorSchemes(KBackedComboBox *combo);
    void loadGuiStyles(KBackedComboBox *combo);

  Q_SIGNALS:
    void changed();
    void useThemeChanged(bool);

  protected Q_SLOTS:
    void slotUseThemeChanged();

  private:
    void set_def();

    QCheckBox *useThemeCheck;
    KBackedComboBox *guicombo;
    KBackedComboBox *colcombo;
    KLanguageButton *langcombo;
    QCheckBox *aacb;
    KFontRequester *greetingFontChooser;
    KFontRequester *failFontChooser;
    KFontRequester *stdFontChooser;
};

#endif
