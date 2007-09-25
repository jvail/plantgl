/* -*-c++-*-
 *  ----------------------------------------------------------------------------
 *
 *       PlantGL: Plant Graphic Library
 *
 *       Copyright 1995-2007 UMR Cirad/Inria/Inra Dap - Virtual Plant Team
 *
 *       File author(s): F. Boudon
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
#include <plantgl/scenegraph/scene/shape.h>
#include <plantgl/scenegraph/geometry/geometry.h>
#include <plantgl/scenegraph/appearance/appearance.h>
#include <string>

#include "../util/export_refcountptr.h"
#include "../util/export_property.h"

PGL_USING_NAMESPACE
TOOLS_USING_NAMESPACE
using namespace boost::python;
using namespace std;

DEF_POINTEE(Shape)
DEF_POINTEE(Shape3D)

void export_Shape3D()
{
  class_< Shape3D, Shape3DPtr, bases< SceneObject >, boost::noncopyable >("Shape3D", no_init);
}


struct sh_pickle_suite : boost::python::pickle_suite
{
	static tuple getinitargs(Shape const& sh)
	{
		return make_tuple(sh.getGeometry(),sh.getAppearance(),sh.getId());
	}
};

void export_Shape()
{
  class_< Shape,ShapePtr, bases< Shape3D > , boost::noncopyable >("Shape", init<>())
    .def( init< const RefCountPtr<Geometry> &, 
	          optional< const RefCountPtr<Appearance> &,
	                     long unsigned int > >("Shape( geometry, appearance, id )") )
    .DEC_PTR_PROPERTY(appearance, Shape,Appearance, AppearancePtr)
    .DEC_PTR_PROPERTY(geometry, Shape, Geometry,GeometryPtr)
    .def_readwrite("id", &Shape::id)
    .def("setComputedName", &Shape::setComputedName)
	.def_pickle(sh_pickle_suite());
    ;

  implicitly_convertible<ShapePtr, Shape3DPtr >();
  
}
