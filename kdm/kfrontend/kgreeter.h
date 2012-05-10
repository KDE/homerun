/*

Greeter widget for kdm

Copyright (C) 1997, 1998 Steffen Hansen <hansen@kde.org>
Copyright (C) 2000-2004 Oswald Buddenhagen <ossi@kde.org>


This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/


#ifndef KGREETER_H
#define KGREETER_H

#include "kgverify.h"
#include "kgdialog.h"

class UserListView;
class KdmClock;
class KdmItem;

class KConfigGroup;
class QListWidgetItem;
class QActionGroup;

struct SessType {
    QString name, type;
    QAction *action;
    bool hid;
    int prio;

    SessType() {}
    SessType(const QString &n, const QString &t, bool h, int p) :
        name(n), type(t), action(0), hid(h), prio(p) {}
    bool operator<(const SessType &st) const {
        return hid != st.hid ? hid < st.hid :
               prio != st.prio ? prio < st.prio :
               name < st.name;
    }
};

class KGreeter : public KGDialog, public KGVerifyHandler {
    Q_OBJECT
    typedef KGDialog inherited;

  public:
    KGreeter(bool themed = false);
    ~KGreeter();

  public Q_SLOTS:
    void accept();
    void reject();
    void slotUserClicked(QListWidgetItem *);
    void slotSessionSelected();
    void slotUserEntered();

  protected:
    void insertUser(const QImage &, const QString &, struct passwd *);
    void insertUsers();
    void putSession(const QString &, const QString &, bool, const char *);
    void insertSessions();
    virtual void pluginSetup();
    void setPrevWM(QAction *);

    QString curUser, dName;
    KConfigGroup *stsGroup;
    UserListView *userView;
    QStringList *userList;
    QMenu *sessMenu;
    QActionGroup *sessGroup;
    QVector<SessType> sessionTypes;
    int nNormals, nSpecials;
    QAction *curPrev;
    bool prevValid;
    bool needLoad;

    static int curPlugin;
    static PluginList pluginList;

  private Q_SLOTS:
    void slotLoadPrevWM();

  public: // from KGVerifyHandler
    virtual void verifyPluginChanged(int id);
    virtual void verifyClear();
    virtual void verifyOk();
    virtual void verifyFailed();
    virtual void verifyRetry();
    virtual void verifySetUser(const QString &user);
};

class KStdGreeter : public KGreeter {
    Q_OBJECT
    typedef KGreeter inherited;

  public:
    KStdGreeter();

  protected:
    virtual void pluginSetup();

  private:
    KdmClock *clock;
    QLabel *pixLabel;
    QPushButton *goButton;

  public: // from KGVerifyHandler
    virtual void verifyFailed();
    virtual void verifyRetry();
};

class KThemedGreeter : public KGreeter {
    Q_OBJECT
    typedef KGreeter inherited;

  public:
    KThemedGreeter(KdmThemer *_themer);
    ~KThemedGreeter();

  public Q_SLOTS:
    void slotThemeActivated(const QString &id);
    void slotSessMenu();
    void slotActionMenu();
    void slotDebugToggled();

  protected:
    virtual void updateStatus(bool fail, bool caps, int timedleft);
    virtual void pluginSetup();
    virtual void keyPressEvent(QKeyEvent *);
    virtual bool event(QEvent *e);

  private:
//    KdmClock *clock;
    KdmThemer *themer;
    KdmItem *caps_warning, *xauth_warning, *pam_error, *timed_label,
            *userlist_node, *userlist_rect,
            *session_button, *system_button;

//  public: // from KGVerifyHandler
//    virtual void verifyFailed();
//    virtual void verifyRetry();
};

#endif /* KGREETER_H */
