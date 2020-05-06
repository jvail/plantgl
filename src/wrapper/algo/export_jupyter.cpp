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
#include <plantgl/algo/codec/serializer.h>

#include <fstream>

PGL_USING_NAMESPACE

boost::python::object scene_to_drc(Scene *scene, int speed=0)
{
  size_t size = 0;
  char* data = DrcPrinter::print(scene, &size, speed);
  boost::python::object memoryView(boost::python::handle<>(PyMemoryView_FromMemory(data, size, PyBUF_READ)));
  return memoryView;
}

boost::python::dict serialize(Scene *scene, bool single_mesh=true, int speed=0)
{

  boost::python::dict dict;
  dict["status"] = false;
  Serializer serializer;
  serializer.setSpeed(speed);
  serializer.setSingleMesh(single_mesh);

  if (scene->apply(serializer)) {
    size_t size = serializer.size();
    char *data = static_cast<char*>(malloc(size * sizeof(char)));
    memcpy(data, serializer.data(), size * sizeof(char));
    boost::python::object memoryView(boost::python::handle<>(PyMemoryView_FromMemory(data, size, PyBUF_READ)));
    dict["data"] = memoryView;
    boost::python::list list;
    std::vector<size_t> offsets = serializer.offsets();
    std::vector<size_t>::iterator it;
    for (it = offsets.begin(); it != offsets.end(); ++it) {
      list.append(*it);
    }
    dict["offsets"] = list;
    dict["status"] = true;
  }

  return dict;
}

BOOST_PYTHON_FUNCTION_OVERLOADS(serialize_overloads, serialize, 1, 3)

void export_Jupyter()
{
  boost::python::def("scene_to_drc", &scene_to_drc);
  boost::python::def("serialize", &serialize, serialize_overloads(
    (boost::python::arg("scene"),
     boost::python::arg("single_mesh")=true,
     boost::python::arg("speed")=0)));
}
