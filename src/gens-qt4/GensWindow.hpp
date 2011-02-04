/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensWindow.hpp: Gens Window.                                            *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2010 by David Korth.                                 *
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

#ifndef __GENS_QT4_GENSWINDOW_HPP__
#define __GENS_QT4_GENSWINDOW_HPP__

// Qt4 includes.
#include <QtGui/QMainWindow>
#include <QtGui/QVBoxLayout>
#include <QtGui/QCloseEvent>
#include <QtGui/QImage>

#include "VBackend/VBackend.hpp"
#include "GensMenuBar.hpp"
#include "EmuManager.hpp"
#include "Input/EventKeys.hpp"

// LibGens includes.
#include "libgens/lg_osd.h"

namespace GensQt4
{

class GensWindow : public QMainWindow
{
	Q_OBJECT
	
	public:
		GensWindow();
		~GensWindow();
		
		// LibGens OSD handler.
		void osd(OsdType osd_type, int param);
	
	protected:
		void setupUi(void);
		void retranslateUi(void);
		
		void closeEvent(QCloseEvent *event);
		
		// Widgets.
		VBackend *m_vBackend;	// GensQGLWidget.
		GensMenuBar *m_menubar;	// Gens menu bar.
		
		QWidget *centralwidget;
		QVBoxLayout *layout;
		
		// QMainWindow virtual functions.
		void showEvent(QShowEvent *event);
		
		// GensWindow functions.
		void gensResize(void);	// Resize the window.
		
		int m_scale;		// Temporary scaling variable.
		bool m_hasInitResize;	// Has the initial resize occurred?
		
		// Emulation Manager.
		EmuManager m_emuManager;
		
		// Event Keys.
		EventKeys *m_evKeys;
		
		// Set the Gens window title.
		void setGensTitle(void);
	
	protected slots:
		// Menu item selection.
		void menuTriggered(int id);
		
		/**
		 * updateFps(): Update the FPS counter.
		 */
		void updateFps(double fps)
		{
			m_vBackend->pushFps(fps);
		}
		
		/**
		 * stateChanged(): Emulation state changed.
		 * - Update the video backend "running" state.
		 * - Update the Gens title.
		 */
		void stateChanged(void);
		
		/**
		 * updateVideo(): Update video.
		 */
		void updateVideo(void)
		{
			m_vBackend->setVbDirty();
			m_vBackend->vbUpdate();
		}
		
		/**
		 * osdPrintMsg(): Print a message on the OSD.
		 * @param duration Duration for the message to appear, in milliseconds.
		 * @param msg Message to print.
		 */
		void osdPrintMsg(int duration, const QString& msg)
		{
			m_vBackend->osd_printf(duration, "%s", msg.toUtf8().constData());
		}
		
		/**
		 * osdShowPreview(): Show a preview image on the OSD.
		 * @param duration Duration for the preview image to appaer, in milliseconds.
		 * @param img Image to show.
		 */
		void osdShowPreview(int duration, const QImage& img)
		{
			m_vBackend->osd_show_preview(duration, img);
		}
		
		/**
		 * toggleFastBlur(): Toggle the Fast Blur effect.
		 */
		void toggleFastBlur(void);
};

}

#endif /* __GENS_QT4_GENSWINDOW_HPP__ */
