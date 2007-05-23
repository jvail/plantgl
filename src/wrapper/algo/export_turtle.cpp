/* ---------------------------------------------------------------------------
 #
 #       File author(s): F. Boudon (frederic.boudon@loria.fr)
 #
 # ---------------------------------------------------------------------------
 #
 #                      GNU General Public Licence
 #
 #       This program is free software; you can redistribute it and/or
 #       modify it under the terms of the GNU General Public License as
 #       published by the Free Software Foundation; either version 2 of
 #       the License, or (at your option) any later version.
 #
 #       This program is distributed in the hope that it will be useful,
 #       but WITHOUT ANY WARRANTY; without even the implied warranty of
 #       MERCHANTABILITY or FITNESS For A PARTICULAR PURPOSE. See the
 #       GNU General Public License for more details.
 #
 #       You should have received a copy of the GNU General Public
 #       License along with this program; see the file COPYING. If not,
 #       write to the Free Software Foundation, Inc., 59
 #       Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 #
 # ---------------------------------------------------------------------------
 */

#include <algo/modelling/turtle.h>

#include <boost/python.hpp>
using namespace boost::python;
PGL_USING_NAMESPACE
TOOLS_USING_NAMESPACE

void py_error_handler(const std::string& msg){
    PyErr_SetString(PyExc_ValueError, msg.c_str() );
	throw_error_already_set();
}

void export_Turtle()
{
    Turtle::register_error_handler(&py_error_handler);


  class_< Turtle >("Turtle", init< optional<TurtleParam * > >("Turtle([TurtleParam]) -> Create Turtle"))

    .def("f", (void (Turtle::*) ())         &Turtle::f )
    .def("f", (void (Turtle::*) (real_t))     &Turtle::f )
    .def("F", (void (Turtle::*) ())         &Turtle::F )
    .def("F", (void (Turtle::*) (real_t))     &Turtle::F )
    .def("F", (void (Turtle::*) (real_t,real_t))&Turtle::F )

	.def("setColor", &Turtle::setColor )
    .def("incColor", &Turtle::incColor )
    .def("decColor", &Turtle::decColor )
    .def("setWidth", &Turtle::setWidth )
    .def("incWidth", &Turtle::incWidth )
    .def("decWidth", &Turtle::decWidth )
    .def("setTexture", &Turtle::setTexture )
    .def("incTexture", &Turtle::incTexture )
    .def("decTexture", &Turtle::decTexture )

    .def("isValid", &Turtle::isValid)
    .def("__str__", &Turtle::str)
    .def("getParameters", 
		 (const TurtleParam& (Turtle::*) () const) &Turtle::getParameters,
		 return_value_policy<copy_const_reference>())
    .def("start", &Turtle::start)
    .def("stop", &Turtle::stop)
    .def("emptyStack", &Turtle::emptyStack)
    .def("getPosition", &Turtle::getPosition, return_value_policy<copy_const_reference>())
    .def("getHeading",  &Turtle::getHeading,  return_value_policy<copy_const_reference>())
    .def("getUp",       &Turtle::getUp,       return_value_policy<copy_const_reference>())
    .def("getLeft",     &Turtle::getLeft,     return_value_policy<copy_const_reference>())
    .def("getScale",    &Turtle::getScale,    return_value_policy<copy_const_reference>())
    .def("getWidth",    &Turtle::getWidth,    return_value_policy<return_by_value>())
    .def("getColor",    &Turtle::getColor,    return_value_policy<return_by_value>())

    .def("getColorListSize",  &Turtle::getColorListSize )
    .def("getTextureListSize",&Turtle::getTextureListSize )
    
    .def("reset", &Turtle::reset)
    .def("resetValues", &Turtle::resetValues)
    .def("dump", &Turtle::dump)

    .def("push", &Turtle::push)
    .def("pop", &Turtle::pop)


    .def("left",   (void (Turtle::*) ())    &Turtle::left )
    .def("left",   (void (Turtle::*) (real_t))&Turtle::left )
    .def("right",  (void (Turtle::*) ())    &Turtle::right )
    .def("right",  (void (Turtle::*) (real_t))&Turtle::right )
    .def("up",     (void (Turtle::*) ())    &Turtle::up )
    .def("up",     (void (Turtle::*) (real_t))&Turtle::up )
    .def("down",   (void (Turtle::*) ())    &Turtle::down )
    .def("down",   (void (Turtle::*) (real_t))&Turtle::down )
    .def("rollL",  (void (Turtle::*) ())    &Turtle::rollL )
    .def("rollL",  (void (Turtle::*) (real_t))&Turtle::rollL )
    .def("rollR",  (void (Turtle::*) ())    &Turtle::rollR )
    .def("rollR",  (void (Turtle::*) (real_t))&Turtle::rollR )
    .def("rollToVert",  &Turtle::rollToVert )
    .def("setHead",(void (Turtle::*) (const Vector3&,const Vector3&)) &Turtle::setHead )
    .def("setHead",(void (Turtle::*) (real_t,real_t,real_t,real_t,real_t,real_t))&Turtle::setHead )
    .def("move",   (void (Turtle::*) (const Vector3&))&Turtle::move )
    .def("move",   (void (Turtle::*) (real_t,real_t,real_t))&Turtle::move )
    .def("decal",  (void (Turtle::*) (const Vector3&))&Turtle::decal )
    .def("decal",  (void (Turtle::*) (real_t,real_t,real_t))&Turtle::decal )
    .def("traceTo",(void (Turtle::*) (const Vector3&))&Turtle::traceTo )
    .def("traceTo",(void (Turtle::*) (real_t,real_t,real_t))&Turtle::traceTo )
    .def("scale",  (void (Turtle::*) (const Vector3&))&Turtle::scale )
    .def("scale",  (void (Turtle::*) (real_t,real_t,real_t))&Turtle::scale )
    .def("scale",  (void (Turtle::*) (real_t))          &Turtle::scale )
    .def("multScale",(void (Turtle::*) (const Vector3&))&Turtle::multScale )
    .def("multScale",(void (Turtle::*) (real_t,real_t,real_t))&Turtle::multScale )
    .def("multScale",(void (Turtle::*) (real_t))          &Turtle::multScale )
    .def("divScale", (void (Turtle::*) (const Vector3&))&Turtle::divScale )
    .def("divScale", (void (Turtle::*) (real_t,real_t,real_t))&Turtle::divScale )
    .def("divScale", (void (Turtle::*) (real_t))          &Turtle::divScale )

    .def("startPolygon", &Turtle::startPolygon )
    .def("stopPolygon",  &Turtle::stopPolygon )
    .def("startGC",      &Turtle::startGC )
    .def("stopGC",       &Turtle::stopGC )

    .def("sphere", (void (Turtle::*) ())       &Turtle::sphere )
    .def("sphere", (void (Turtle::*) (real_t))   &Turtle::sphere )
    .def("circle", (void (Turtle::*) ())       &Turtle::circle )
    .def("circle", (void (Turtle::*) (real_t))   &Turtle::circle )
    .def("surface", &Turtle::surface )
    .def("label", &Turtle::label )

    .def("setDefaultStep",    &Turtle::setDefaultStep )
    .def("setAngleIncrement", &Turtle::setAngleIncrement )
    .def("setWidthIncrement", &Turtle::setWidthIncrement )
    .def("setColorIncrement", &Turtle::setColorIncrement )
    .def("setScaleMultiplier",&Turtle::setScaleMultiplier )

/*    .def("_frustum",&Turtle::_frustum )
    .def("_cylinder",&Turtle::_cylinder )
//    .def("_polygon",&Turtle::_polygon )
//    .def("_generalizedCylinder",&Turtle::_generalizedCylinder )
    .def("_sphere",&Turtle::_sphere )
    .def("_circle",&Turtle::_circle )
    .def("_surface",&Turtle::_surface )
	.def("_label",&Turtle::_label )*/
    ;
}