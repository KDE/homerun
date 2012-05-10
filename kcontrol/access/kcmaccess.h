/**
 * kcmaccess.h
 *
 * Copyright (c) 2000 Matthias H�zer-Klpfel <hoelzer@kde.org>
 *
 */

#ifndef __kcmaccess_h__
#define __kcmaccess_h__


#define KDE3_SUPPORT
#include <kcmodule.h>
#undef KDE3_SUPPORT
#include <knuminput.h>


class QCheckBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class KColorButton;
class QSlider;

class ExtendedIntNumInput : public KIntNumInput
{
	Q_OBJECT

	public:
    /**
	  * Constructs an input control for integer values
	  * with base 10 and initial value 0.
	  */
		ExtendedIntNumInput(QWidget *parent=0);

    /**
	  * Destructor
	  */
		virtual ~ExtendedIntNumInput();

    /**
	  * @param min  minimum value
	  * @param max  maximum value
	  * @param step step size for the QSlider
	  * @param slider whether the slider is created or not
	  */
		void setRange(int min, int max, int step=1, bool slider=true);

	private Q_SLOTS:
		void slotSpinValueChanged(int);
		void slotSliderValueChanged(int);

	private:
		int min, max;
		int sliderMax;
};

class KAccessConfig : public KCModule
{
  Q_OBJECT

public:

  KAccessConfig( QWidget *parent, const QVariantList& );
  virtual ~KAccessConfig();

  void load();
  void save();
  void defaults();

protected Q_SLOTS:

  void configChanged();
  void checkAccess();
  void invertClicked();
  void flashClicked();
  void selectSound();
  void changeFlashScreenColor();
  void configureKNotify();

private:

  QCheckBox *systemBell, *customBell, *visibleBell;
  QRadioButton *invertScreen, *flashScreen;
  QLabel    *soundLabel, *colorLabel;
  QLineEdit *soundEdit;
  QPushButton *soundButton;
  KColorButton *colorButton;
  ExtendedIntNumInput *durationSlider;

  QCheckBox *stickyKeys, *stickyKeysLock, *stickyKeysAutoOff;
  QCheckBox *stickyKeysBeep, *toggleKeysBeep, *kNotifyModifiers;
  QPushButton *kNotifyModifiersButton;

  QCheckBox *slowKeys, *bounceKeys;
  ExtendedIntNumInput *slowKeysDelay, *bounceKeysDelay;
  QCheckBox *slowKeysPressBeep, *slowKeysAcceptBeep;
  QCheckBox *slowKeysRejectBeep, *bounceKeysRejectBeep;

  QCheckBox *gestures, *gestureConfirmation;
  QCheckBox *timeout;
  KIntNumInput *timeoutDelay;
  QCheckBox *accessxBeep, *kNotifyAccessX;
  QPushButton *kNotifyAccessXButton;
};


#endif
