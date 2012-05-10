/*
 * This file is part of the KDE project, module kcmdisplay.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 *
 * You can Freely distribute this program under the GNU General Public
 * License. See the file "COPYING" for the exact licensing terms.
 */

#ifndef __BACKGROUND__
#define __BACKGROUND__

#include <QWidget>
#include <ksharedconfig.h>

class BGDialog;
class QCheckBox;
class QLabel;

class KBackground : public QWidget {
    Q_OBJECT

  public:
    KBackground(KSharedConfigPtr config, QWidget *parent = 0);
    ~KBackground();

    void load();
    void save();
    void defaults();

  Q_SIGNALS:
    void changed();

  private Q_SLOTS:
    void slotEnableChanged();

  private:
    void init();
    void apply();

    QCheckBox *m_pCBEnable;
    QLabel *m_pMLabel;
    KSharedConfigPtr m_simpleConf;
    BGDialog *m_background;
};

#endif // __Bgnd_h_Included__
