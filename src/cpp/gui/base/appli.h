/* -*-c++-*-
 *  ----------------------------------------------------------------------------
 *
 *       AMAPmod: Exploring and Modeling Plant Architecture
 *
 *       Copyright 1995-2000 UMR Cirad/Inra Modelisation des Plantes
 *                           UMR PIAF INRA-UBP Clermont-Ferrand
 *
 *       File author(s): F. Boudon
 *
 *       $Source$
 *       $Id$
 *
 *       Forum for AMAPmod developers    : amldevlp@cirad.fr
 *
 *  ----------------------------------------------------------------------------
 *
 *                      GNU General Public Licence
 *
 *       This program is free software; you can redistribute it and/or
 *       modify it under the terms of the GNU General Public License as
 *       published by the Free Software Foundation; either version 2 of
 *       the License, or (at your option) any later version.
 *
 *       This program is distributed in the hope that it will be useful,
 *       but WITHOUT ANY WARRANTY; without even the implied warranty of
 *       MERCHANTABILITY or FITNESS For A PARTICULAR PURPOSE. See the
 *       GNU General Public License for more details.
 *
 *       You should have received a copy of the GNU General Public
 *       License along with this program; see the file COPYING. If not,
 *       write to the Free Software Foundation, Inc., 59
 *       Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  ----------------------------------------------------------------------------
 */

#ifndef __view_appli_h__
#define __view_appli_h__

#include "Tools/config.h"
#include "Tools/util_vector.h"

#include <string>
#include <vector>
#include "scne_scene.h"

class ViewRayBuffer;
class ViewZBuffer;
class QCustomEvent;

class ViewerAppli {
	public:
	ViewerAppli();
	virtual ~ViewerAppli();

	virtual void startSession()= 0;
	virtual bool stopSession() = 0;
	virtual bool exit() = 0;
	virtual void sendAnEvent(QCustomEvent *e) = 0;
	virtual void postAnEvent(QCustomEvent *e) = 0;

    virtual bool isRunning() = 0;
    virtual bool Wait ( unsigned long time = ULONG_MAX ) = 0;

	virtual void display(const GEOM(ScenePtr)& s);
	virtual void add(const GEOM(ScenePtr)& s);
	virtual const std::vector<uint32_t> getSelection();

	virtual int question(const std::string& caption,
						 const std::string& text,
						 const std::string& but0txt = "",
						 const std::string& but1txt = "",
						 const std::string& but2txt = "");


	virtual std::string itemSelection(const std::string& caption,
									  const std::string& text,
									  const std::vector<std::string> & values,
									  bool& ok,
									  bool editable);
	virtual std::string getFile(const std::string& caption,
					            const std::string& startPath,
				                const std::string& filter,
					            bool existing,
								bool dir);

	virtual ViewRayBuffer * castRays(const TOOLS(Vector3)& pos, 
									  const TOOLS(Vector3)& dir,
									  const TOOLS(Vector3)& dx, 
									  const TOOLS(Vector3)& dy,
									  int sx, int sy);
	virtual ViewZBuffer * grabZBuffer();

};

#endif
