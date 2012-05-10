/*

Shell for kdm conversation plugins

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


#ifndef KGVERIFY_H
#define KGVERIFY_H

#include "kgreeterplugin.h"
#include "kfdialog.h"

#include <QGridLayout>
#include <QSet>
#include <QTimer>

#include <sys/time.h>
#include <time.h>

class KdmThemer;

class KLibrary;
class KPushButton;
class QMenu;
class QSocketNotifier;

// helper class, nuke when qt supports suspend()/resume()
class QXTimer : public QObject {
    Q_OBJECT
    typedef QObject inherited;

  public:
    QXTimer();
    void start(int msec);
    void stop();
    void suspend();
    void resume();

  Q_SIGNALS:
    void timeout();

  private Q_SLOTS:
    void slotTimeout();

  private:
    QTimer timer;
    struct timeval stv;
    long left;
};

class KGVerifyHandler {
  public:
    virtual ~KGVerifyHandler() {}
    virtual void verifyPluginChanged(int id) = 0;
    virtual void verifyClear();
    virtual void verifyOk() = 0;
    virtual void verifyFailed() = 0;
    virtual void verifyRetry() = 0;
    virtual void verifySetUser(const QString &user) = 0;
    virtual void updateStatus(bool fail, bool caps, int left); // for themed only
};

struct GreeterPluginHandle {
    KLibrary *library;
    KGreeterPluginInfo *info;
    QAction *action;
};

typedef QVector<int> PluginList;

class KGVerify : public QObject, public KGreeterPluginHandler {
    Q_OBJECT
    typedef QObject inherited;

  public:
    KGVerify(KGVerifyHandler *handler,
             QWidget *parent, QWidget *predecessor,
             const QString &fixedEntity, const PluginList &pluginList,
             KGreeterPlugin::Function func, KGreeterPlugin::Context ctx);
    virtual ~KGVerify();
    QMenu *getPlugMenu();
    void loadUsers(const QStringList &users);
    void presetEntity(const QString &entity, int field);
    QString getEntity() const;
    void setUser(const QString &user);
    virtual void selectPlugin(int id);
    bool entitiesLocal() const;
    bool entitiesFielded() const;
    bool entityPresettable() const;
    bool isClassic() const;
    QString pluginName() const;
    void setEnabled(bool on);
    void abort();
    void suspend();
    void resume();
    void accept();
    void reject();

    enum { CoreIdle, CorePrompting, CoreBusy } coreState;

    static bool handleFailVerify(QWidget *parent, bool showUser);
    static PluginList init(const QStringList &plugins);
    static void done();

  public Q_SLOTS:
    void start();

  protected:
    bool eventFilter(QObject *, QEvent *);
    void msgBox(QMessageBox::Icon typ, const QString &msg);
    void setTimer();
    void updateLockStatus();
    virtual void updateStatus() = 0;

    QXTimer timer;
    QString fixedEntity, presEnt, curUser, pamUser;
    PluginList pluginList;
    KGVerifyHandler *handler;
    QSocketNotifier *sockNot;
    QWidget *parent, *predecessor;
    KGreeterPlugin *greet;
    QMenu *plugMenu;
    int curPlugin, presFld, timedLeft, deadTicks;
    QByteArray pName;
    KGreeterPlugin::Function func;
    KGreeterPlugin::Context ctx;
    bool capsLocked;
    bool enabled, running, suspended, failed, delayed;
    bool authTok, isClear, timeable;

    static void vrfMsgBox(QWidget *parent, const QString &user, QMessageBox::Icon type, const QString &mesg);
    static void vrfErrBox(QWidget *parent, const QString &user, const char *msg);
    static void vrfInfoBox(QWidget *parent, const QString &user, const char *msg);

    static QVector<GreeterPluginHandle> greetPlugins;

  private:
    bool applyPreset();
    void performAutoLogin();
    bool scheduleAutoLogin(bool initial);
    void doReject(bool initial);
    void talkerEdits();

  private Q_SLOTS:
    void slotPluginSelected(QAction *);
    void slotTimeout();
    void slotActivity();
    void handleVerify();

  public: // from KGreetPluginHandler
    virtual void gplugReturnText(const char *text, int tag);
    virtual void gplugReturnBinary(const char *data);
    virtual void gplugSetUser(const QString &user);
    virtual void gplugStart();
    virtual void gplugChanged();
    virtual void gplugActivity();
    virtual void gplugMsgBox(QMessageBox::Icon type, const QString &text);

    static QVariant getConf(void *ctx, const char *key, const QVariant &dflt);
};

class KGStdVerify : public KGVerify {
    Q_OBJECT
    typedef KGVerify inherited;

  public:
    KGStdVerify(KGVerifyHandler *handler, QWidget *parent,
                QWidget *predecessor, const QString &fixedEntity,
                const PluginList &pluginList,
                KGreeterPlugin::Function func, KGreeterPlugin::Context ctx);
    virtual ~KGStdVerify();
    QLayout *getLayout() const { return grid; }
    void selectPlugin(int id);

  protected:
    void updateStatus();

  private:
    QGridLayout *grid;
    QLabel *failedLabel;
    int failedLabelState;

  public: // from KGreetPluginHandler
    virtual bool gplugHasNode(const QString &id);
};

class KGThemedVerify : public KGVerify {
    Q_OBJECT
    typedef KGVerify inherited;

  public:
    KGThemedVerify(KGVerifyHandler *handler, KdmThemer *themer,
                   QWidget *parent, QWidget *predecessor,
                   const QString &fixedEntity,
                   const PluginList &pluginList,
                   KGreeterPlugin::Function func,
                   KGreeterPlugin::Context ctx);
    virtual ~KGThemedVerify();
    void selectPlugin(int id);

  protected:
    void updateStatus();

  private:
    KdmThemer *themer;
    QSet<QString> showTypes;

  public: // from KGreetPluginHandler
    virtual bool gplugHasNode(const QString &id);
};

class KGChTok : public FDialog, public KGVerifyHandler {
    Q_OBJECT
    typedef FDialog inherited;

  public:
    KGChTok(QWidget *parent, const QString &user,
            const PluginList &pluginList, int curPlugin,
            KGreeterPlugin::Function func, KGreeterPlugin::Context ctx);
    ~KGChTok();

  public Q_SLOTS:
    void accept();

  private:
    KPushButton *okButton, *cancelButton;
    KGStdVerify *verify;

  public: // from KGVerifyHandler
    virtual void verifyPluginChanged(int id);
    virtual void verifyOk();
    virtual void verifyFailed();
    virtual void verifyRetry();
    virtual void verifySetUser(const QString &user);
};

#endif /* KGVERIFY_H */
