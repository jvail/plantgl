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
#include <plantgl/algo/codec/serializer.h>

PGL_USING_NAMESPACE

struct SerializedScene {
  bool status = false;
  boost::python::object data = boost::python::object();
  boost::python::list offsets = boost::python::list();
};

SerializedScene serialize_scene(Scene *scene, bool single_mesh=false)
{

  SerializedScene serialized;
  serialized.status = false;
  Serializer serializer;
  serializer.setSingleMesh(single_mesh);

  if (scene->apply(serializer)) {
    size_t size = serializer.size();
    char *data = static_cast<char*>(malloc(size * sizeof(char)));
    memcpy(data, serializer.data(), size * sizeof(char));
    boost::python::object memoryView(boost::python::handle<>(PyMemoryView_FromMemory(data, size, PyBUF_READ)));
    serialized.data = memoryView;
    boost::python::list list;
    std::vector<size_t> offsets = serializer.offsets();
    std::vector<size_t>::iterator it;
    for (it = offsets.begin(); it != offsets.end(); ++it) {
      list.append(*it);
    }
    serialized.offsets = list;
    serialized.status = true;
  }

  return serialized;
}

BOOST_PYTHON_FUNCTION_OVERLOADS(serialize_scene_overloads, serialize_scene, 1, 2)

void export_Jupyter()
{
  boost::python::def("serialize_scene", &serialize_scene, serialize_scene_overloads(
    (boost::python::arg("scene"),
     boost::python::arg("single_mesh")=true)));

  boost::python::class_< SerializedScene > ("SerializedScene", boost::python::no_init )
    .add_property("status", &SerializedScene::status)
    .add_property("data", &SerializedScene::data)
    .add_property("offsets", &SerializedScene::offsets);
}
