/*
 * Copyright (c) 2007 Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
 * Copyright (c) 2007, 2008 Harry Bock <hbock@providence.edu>
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef __RANDROUTPUT_H__
#define __RANDROUTPUT_H__

#include <QObject>
#include <QString>
#include <QRect>

#include "randr.h"
#include "randrmode.h"

class QAction;
class KConfig;
	
/** Class representing an RROutput identifier. This class is used
 * to control a particular output's configuration (i.e., the mode or
 * refresh rate of a DVI-I port, or the resolution of a VGA port). */
class RandROutput : public QObject
{
	Q_OBJECT

public:
	RandROutput(RandRScreen *parent, RROutput id);
	~RandROutput();
	
	/** Returns the internal RANDR identifier for a particular output. */
	RROutput id() const;
	
	/** Return the screen that this output belongs to. */
	RandRScreen *screen() const;
	
	void loadSettings(bool notify = false);
	
	/** Handle an event from RANDR signifying a change in this output's
	 * configuration. */
	void handleEvent(XRROutputChangeNotifyEvent *event);
	void handlePropertyEvent(XRROutputPropertyNotifyEvent *event);

	/** The name of this output, as returned by the X device driver.
	 * Examples may be VGA, TMDS, DVI-I_2/digital, etc. Note:
	 * this is usually NOT the name returned in the EDID of your
	 * display. */
	QString name() const;

	/** Return the icon name according to the device type. */
	QString icon() const;

	/** List possible CRT controllers for this output. */
	CrtcList possibleCrtcs() const;
	
	/** Returns the current CRTC for this output. */
	RandRCrtc *crtc() const;

	void disconnectFromCrtc();

	/** Returns a list of all RRModes supported by this output. */
	ModeList modes() const;

	/** Returns the current mode for this output. */
	RandRMode mode() const;

	/** Returns the preferred mode for this output,
	 * or an invalid mode if no preferred mode is known. */
	RandRMode preferredMode() const;
	
	/** The list of supported sizes */
	SizeList sizes() const;
	QRect rect() const;

	/** The list of refresh rates for the given size.
	 * If no size is specified, it will use the current size */
	RateList refreshRates(const QSize &s = QSize()) const;

	/** The current refresh rate. */
	float refreshRate() const;

	/** Return all possible rotations for all CRTCs this output can be connected
	 * to. */
	int rotations() const;

	/** Returns the curent rotation of the CRTC this output is currently 
	 * connected to */
	int rotation() const;

	/** Determines whether this output is connected to a display device.
	 * It is not necessarily active. */
	bool isConnected() const;
	
	/** Determines whether this output is currently driving a display
	 * device. */
	bool isActive() const;

	bool applyProposed(int changes = 0xffffff, bool confirm = false);
	void proposeOriginal();

	// proposal functions
	void proposeRefreshRate(float rate);
	void proposeRect(const QRect &r);
	void proposeRotation(int rotation);

	void load(KConfig &config);
	void save(KConfig &config);
	QStringList startupCommands() const;

public slots:
	void slotChangeSize(QAction *action);
	void slotChangeRotation(QAction *action);
	void slotChangeRefreshRate(QAction *action);
	void slotDisable();
	void slotEnable();
	void slotSetAsPrimary(bool primary);

private slots:
	void slotCrtcChanged(RRCrtc c, int changes);

signals:
	/** This signal is emitted when any relevant change
	 * occurs in an output (mode, CRTC, resolution,
	 * connection, etc.) */
	void outputChanged(RROutput o, int changes);

protected:
	/** Query Xrandr for information about this output, and set
	 * up this instance accordingly. */
	void queryOutputInfo(void);
	
	/** Find the first CRTC that is not controlling any
	 * display devices. */
	RandRCrtc *findEmptyCrtc(void);
	bool tryCrtc(RandRCrtc *crtc, int changes);

	/** Set the current CRT controller for this output.
	 * The CRTC should never be set directly; it should be added through 
	 * this function to properly manage signals related to this output. */
	bool setCrtc(RandRCrtc *crtc, bool applyNow = true);
	
private:	
	RROutput m_id;
	XRROutputInfo* m_info;
	QString m_name;
	QString m_alias;

	CrtcList m_possibleCrtcs;

	RandRScreen *m_screen;
	RandRCrtc *m_crtc;
	
	//proposed stuff (mostly to read from the configuration)
	QRect m_proposedRect;
	int   m_proposedRotation;
	float m_proposedRate;

	QRect m_originalRect;
	int   m_originalRotation;
	float m_originalRate;

	ModeList m_modes;
	RandRMode m_preferredMode;

	int m_rotations;
	bool m_connected;
};
#endif
// vim:noet:sts=8:sw=8:
