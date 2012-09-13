/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensCtrlCfgWidget.hpp: Controller configuration widget.                 *
 *                                                                         *
 * Copyright (c) 2011 by David Korth.                                      *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#include "GensCtrlCfgWidget.hpp"

// Qt includes.
#include <QtGui/QLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QSpacerItem>
#include <QtGui/QPushButton>
#include <QtGui/QHBoxLayout>

// GensKeySequenceWidget.
// TODO: Add property for "single key" and add gamepad support.
#include "GensKeySequenceWidget.hpp"
#include "GensCtrlKeyWidget.hpp"

// Controller configuration.
#include "Config/CtrlConfig.hpp"

namespace GensQt4
{

class GensCtrlCfgWidgetPrivate
{
	public:
		GensCtrlCfgWidgetPrivate(GensCtrlCfgWidget *q);
		~GensCtrlCfgWidgetPrivate();
		void init(void);

		inline LibGens::IoManager::IoType_t ioType(void);
		void setIoType(LibGens::IoManager::IoType_t newIoType);

		QString buttonName_l(LibGens::IoManager::ButtonName_t buttonName);

		void clearAllButtons(void);
	
	private:
		GensCtrlCfgWidget *const q;
		Q_DISABLE_COPY(GensCtrlCfgWidgetPrivate)

		LibGens::IoManager::IoType_t m_ioType;

		QGridLayout *m_layout;
		QLabel *m_lblButtonName[LibGens::IoManager::BTNI_MAX];
		QLabel *m_lblKeyDisplay[LibGens::IoManager::BTNI_MAX];
		GensCtrlKeyWidget *m_btnCfg[LibGens::IoManager::BTNI_MAX];
		QSpacerItem *m_vspcCfg;

		// "Change All", "Clear All".
		QPushButton *btnChangeAll;
		QPushButton *btnClearAll;
		QHBoxLayout *hboxOptions;
};


/***************************************
 * GensCtrlCfgWidgetPrivate functions. *
 ***************************************/

GensCtrlCfgWidgetPrivate::GensCtrlCfgWidgetPrivate(GensCtrlCfgWidget *q)
	: q(q)
	, m_ioType(LibGens::IoManager::IOT_NONE)
	, m_layout(new QGridLayout(q))
{
	// Eliminate margins.
	m_layout->setContentsMargins(0, 0, 0, 0);
	
	// Reduce vertical spacing in the grid layout.
	m_layout->setVerticalSpacing(0);
}


GensCtrlCfgWidgetPrivate::~GensCtrlCfgWidgetPrivate()
{
	// Delete all the labels and buttons.
	// TODO: Is this necessary?
	for (int i = 0; i < LibGens::IoManager::BTNI_MAX; i++) {
		delete m_lblButtonName[i];
		delete m_lblKeyDisplay[i];
		delete m_btnCfg[i];
	}
}


/**
 * Initialize the grid layout.
 */
void GensCtrlCfgWidgetPrivate::init(void)
{
	// Monospaced font.
	QFont fntMonospace(QLatin1String("Monospace"));
	fntMonospace.setStyleHint(QFont::TypeWriter);

	// Add CtrlConfig::MAX_BTNS items to the grid layout.
	for (int i = 0; i < LibGens::IoManager::BTNI_MAX; i++) {
		m_lblButtonName[i] = new QLabel();
		m_lblButtonName[i]->setVisible(false);
		m_lblKeyDisplay[i] = new QLabel();
		m_lblKeyDisplay[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
		m_lblKeyDisplay[i]->setVisible(false);
		m_lblKeyDisplay[i]->setFont(fntMonospace);
		m_btnCfg[i] = new GensCtrlKeyWidget(NULL, m_lblKeyDisplay[i]);
		m_btnCfg[i]->setVisible(false);

		m_layout->addWidget(m_lblButtonName[i], i, 0, Qt::AlignLeft);
		m_layout->addWidget(m_lblKeyDisplay[i], i, 1, Qt::AlignLeft);
		m_layout->addWidget(m_btnCfg[i], i, 2, Qt::AlignRight);
	}

	// Add a vertical spacer at the bottom of the layout.
	m_vspcCfg = new QSpacerItem(128, 128, QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_layout->addItem(m_vspcCfg, LibGens::IoManager::BTNI_MAX, 1, 1, 1, Qt::AlignCenter);

	// Create the HBox.
	hboxOptions = new QHBoxLayout();
	hboxOptions->setContentsMargins(0, 8, 0, 0); // TODO: Use style default for Top margin.
	m_layout->addLayout(hboxOptions, LibGens::IoManager::BTNI_MAX+1, 0, 1, 3, Qt::AlignCenter);

	// Add the "Change All" and "Clear All" buttons.
	// TODO: Icons.
	btnChangeAll = new QPushButton(q->tr("&Change All Buttons"), q);
	hboxOptions->addWidget(btnChangeAll);

	btnClearAll = new QPushButton(q->tr("C&lear All Buttons"), q);
	QObject::connect(btnClearAll, SIGNAL(clicked(bool)),
			 q, SLOT(clearAllButtons()));
	hboxOptions->addWidget(btnClearAll);
}


/**
 * Get the current I/O type.
 * @return Current I/O type.
 */
inline LibGens::IoManager::IoType_t GensCtrlCfgWidgetPrivate::ioType(void)
	{ return m_ioType; }

/**
 * Set the I/O type.
 * @param newIoType New I/O type.
 */
void GensCtrlCfgWidgetPrivate::setIoType(LibGens::IoManager::IoType_t newIoType)
{
	if (m_ioType == newIoType)
		return;

	// Save the new I/O type.
	m_ioType = newIoType;

	// Update the grid layout based on the specified controller type.
	int numButtons = LibGens::IoManager::NumDevButtons(newIoType);
	if (numButtons > LibGens::IoManager::BTNI_MAX)
		numButtons = LibGens::IoManager::BTNI_MAX;

	// Show the buttons, in logical button order.
	QString sBtnLabel;
	for (int i = 0, button = 0;
	     i < numButtons && button >= 0; i++) {
		LibGens::IoManager::ButtonName_t buttonName =
					LibGens::IoManager::ButtonName(newIoType, button);
		sBtnLabel = buttonName_l(buttonName) + QChar(L':');

		m_lblButtonName[i]->setText(sBtnLabel);
		m_lblButtonName[i]->setVisible(true);
		m_lblKeyDisplay[i]->setVisible(true);
		m_btnCfg[i]->setVisible(true);

		// Get the next logical button. (TODO: Update for IoManager.)
		button = LibGens::IoManager::NextLogicalButton(newIoType, button);
	}

	// Hide other buttons.
	for (int i = numButtons; i < LibGens::IoManager::BTNI_MAX; i++) {
		m_lblButtonName[i]->setVisible(false);
		m_lblKeyDisplay[i]->setVisible(false);
		m_btnCfg[i]->setVisible(false);
	}
}


/**
 * Get a localized LibGens button name.
 * @param buttonName LibGens button name.
 * @return Localized button name, or empty string on error.
 */
QString GensCtrlCfgWidgetPrivate::buttonName_l(LibGens::IoManager::ButtonName_t buttonName)
{
	switch (buttonName) {
		// Standard controller buttons.
		case LibGens::IoManager::BTNNAME_UP:
			//: Standard controller: D-Pad UP.
			return q->tr("Up", "controller-standard");

		case LibGens::IoManager::BTNNAME_DOWN:
			//: Standard controller: D-Pad DOWN.
			return q->tr("Down", "controller-standard");

		case LibGens::IoManager::BTNNAME_LEFT:
			//: Standard controller: D-Pad LEFT.
			return q->tr("Left", "controller-standard");

		case LibGens::IoManager::BTNNAME_RIGHT:
			//: Standard controller: D-Pad RIGHT.
			return q->tr("Right", "controller-standard");

		case LibGens::IoManager::BTNNAME_B:
			//: Standard controller: B button.
			return q->tr("B", "controller-standard");

		case LibGens::IoManager::BTNNAME_C:
			//: Standard controller: C button.
			return q->tr("C", "controller-standard");

		case LibGens::IoManager::BTNNAME_A:
			//: Standard controller: A button.
			return q->tr("A", "controller-standard");

		case LibGens::IoManager::BTNNAME_START:
			//: Standard controller: START button.
			return q->tr("Start", "controller-standard");

		case LibGens::IoManager::BTNNAME_Z:
			//: Standard controller: Z button.
			return q->tr("Z", "controller-standard");

		case LibGens::IoManager::BTNNAME_Y:
			//: Standard controller: Y button.
			return q->tr("Y", "controller-standard");

		case LibGens::IoManager::BTNNAME_X:
			//: Standard controller: X button.
			return q->tr("X", "controller-standard");

		case LibGens::IoManager::BTNNAME_MODE:
			//: Standard controller: MODE button.
			return q->tr("Mode", "controller-standard");

		/** SMS/GG buttons. **/

		case LibGens::IoManager::BTNNAME_1:
			//: SMS/Game Gear: 1 button.
			return q->tr("1", "controller-sms-gg");

		case LibGens::IoManager::BTNNAME_2:
			//: SMS/Game Gear: 2 button.
			return q->tr("2", "controller-sms-gg");

		/** Sega Mega Mouse buttons. **/

		case LibGens::IoManager::BTNNAME_MOUSE_LEFT:
			//: Sega Mega Mouse: LEFT mouse button.
			return q->tr("Left", "controller-mouse");

		case LibGens::IoManager::BTNNAME_MOUSE_RIGHT:
			//: Sega Mega Mouse: RIGHT mouse button.
			return q->tr("Right", "controller-mouse");

		case LibGens::IoManager::BTNNAME_MOUSE_MIDDLE:
			//: Sega Mega Mouse: MIDDLE mouse button.
			return q->tr("Middle", "controller-mouse");

		case LibGens::IoManager::BTNNAME_MOUSE_START:
			//: Sega Mega Mouse: START button.
			return q->tr("Start", "controller-mouse");

		default:
			return QString();
	}
	
	// Should not get here...
	return QString();
}


/**
 * Clear all mapped buttons.
 * WRAPPER SLOT for GensCtrlCfgWidetPrivate.
 */
void GensCtrlCfgWidgetPrivate::clearAllButtons(void)
{
	for (int i = 0; i < LibGens::IoManager::BTNI_MAX; i++)
		m_btnCfg[i]->clearKey();
}


/********************************
 * GensCtrlCfgWidget functions. *
 ********************************/

GensCtrlCfgWidget::GensCtrlCfgWidget(QWidget* parent)
	: QWidget(parent)
	, d(new GensCtrlCfgWidgetPrivate(this))
{
	// Initialize the private members.
	d->init();
}

GensCtrlCfgWidget::~GensCtrlCfgWidget()
{
	delete d;
}


/**
 * GensCtrlCfgWidget::ioType(): Get the current I/O type.
 * @return Current I/O type.
 */
LibGens::IoManager::IoType_t GensCtrlCfgWidget::ioType(void)
	{ return d->ioType(); }

/**
 * GensCtrlCfgWidget::setIoType(): Set the current I/O type.
 * @param newIoType New I/O type.
 */
void GensCtrlCfgWidget::setIoType(LibGens::IoManager::IoType_t newIoType)
	{ d->setIoType(newIoType); }


/**
 * GensCtrlCfgWidget::clearAllButtons(): Clear all mapped buttons.
 * WRAPPER SLOT for GensCtrlCfgWidetPrivate.
 */
void GensCtrlCfgWidget::clearAllButtons(void)
{
	d->clearAllButtons();
}

}
