//-----------------------------------------------------------------------------
//
// kblankscrn - Basic screen saver for KDE
//
// Copyright (c)  Martin R. Jones 1996
//

#ifndef BLANKSCRN_H
#define BLANKSCRN_H

#include <QColor>
#include <kdialog.h>
#include "kscreensaver.h"



class KBlankSaver : public KScreenSaver
{
	Q_OBJECT
public:
	KBlankSaver( WId drawable );
	virtual ~KBlankSaver();

	void setColor( const QColor &col );

private:
	void readSettings();
	void blank();

private:
	QColor color;
};

class KBlankSetup : public KDialog
{
	Q_OBJECT
public:
	KBlankSetup( QWidget *parent = NULL, const char *name = NULL );

protected:
	void readSettings();

private Q_SLOTS:
	void slotColor( const QColor & );
	void slotOk();

private:
	QWidget *preview;
	KBlankSaver *saver;

	QColor color;
};

#endif

