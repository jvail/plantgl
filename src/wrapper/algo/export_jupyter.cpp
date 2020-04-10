/* -*-c++-*-
 *  ----------------------------------------------------------------------------
 *
 *       PlantGL: The Plant Graphic Library
 *
 *       Copyright 1995-2007 UMR CIRAD/INRIA/INRA DAP
 *
 *       File author(s): J. Vaillant, F. Boudon (frederic.boudon@cirad.fr) et al.
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


#include <boost/python.hpp>
#include <plantgl/scenegraph/scene/scene.h>
#include <plantgl/algo/codec/drcprinter.h>

PGL_USING_NAMESPACE

boost::python::object  scene_to_drc(Scene *scene, int speed = 0)
{
  size_t size = 0;
  char* data = DrcPrinter::print(scene, &size, speed);
  boost::python::object  memoryView(boost::python::handle<>(PyMemoryView_FromMemory(data, size, PyBUF_READ)));
  return memoryView;
}

void export_Jupyter()
{

  boost::python::def("scene_to_drc", &scene_to_drc);

}
