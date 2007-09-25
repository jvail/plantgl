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

#include <plantgl/tool/util_array.h>
#include <plantgl/tool/util_array2.h>

#include <plantgl/scenegraph/geometry/parametricmodel.h>
#include <plantgl/scenegraph/container/pointmatrix.h>
#include <plantgl/scenegraph/geometry/patch.h>
#include <plantgl/scenegraph/geometry/bezierpatch.h>
#include <plantgl/scenegraph/geometry/nurbspatch.h>

#include "../util/export_refcountptr.h"
#include "../util/export_property.h"

PGL_USING_NAMESPACE
TOOLS_USING_NAMESPACE
using namespace boost::python;
using namespace std;

DEF_POINTEE(Patch)
DEF_POINTEE(BezierPatch)
DEF_POINTEE(NurbsPatch)

void export_Patch()
{
  class_< Patch, PatchPtr, bases< ParametricModel >,boost::noncopyable >
    ("Patch",no_init)
    .DEC_BT_NR_PROPERTY_WD(ccw,Patch,CCW,bool)
    ;

  implicitly_convertible< PatchPtr,ParametricModelPtr >();
}

void export_BezierPatch()
{
  class_< BezierPatch, BezierPatchPtr, bases< Patch >,boost::noncopyable >
    ("BezierPatch",init<Point4MatrixPtr, optional<uint32_t,uint32_t,bool> >
     ("BezierPatch(Point4Matrix ctrlPoints [,ustride,vstride,ccw])"))
    .DEC_BT_PROPERTY_WD(ustride,BezierPatch,UStride,uint32_t)
    .DEC_BT_PROPERTY_WD(vstride,BezierPatch,VStride,uint32_t)
    .DEC_PTR_PROPERTY(ctrlPointMatrix,BezierPatch,CtrlPointMatrix,Point4MatrixPtr)
    ;

  implicitly_convertible< BezierPatchPtr,PatchPtr >();
}

void export_NurbsPatch()
{
  class_< NurbsPatch, NurbsPatchPtr, bases< BezierPatch >,boost::noncopyable >
    ("NurbsPatch",init<Point4MatrixPtr, optional< RealArrayPtr, RealArrayPtr, 
     uint32_t,uint32_t,uint32_t,uint32_t,bool> >
     ("NurbsPatch(Point4Matrix ctrlPoints, RealArray uKnotList,RealArray vKnotList [,uDeg, vDeg,ustride,vstride,ccw])"))
    .DEC_BT_NR_PROPERTY_WD(udegree,NurbsPatch,UDegree,uint32_t)
    .DEC_BT_NR_PROPERTY_WD(vdegree,NurbsPatch,VDegree,uint32_t)
    .DEC_PTR_PROPERTY_WD(uknotList,NurbsPatch,UKnotList,RealArrayPtr)
    .DEC_PTR_PROPERTY_WD(vknotList,NurbsPatch,VKnotList,RealArrayPtr)
    .def("setVKnotListToDefault",&NurbsPatch::setVKnotListToDefault)
    .def("setUKnotListToDefault",&NurbsPatch::setUKnotListToDefault)
    ;

  implicitly_convertible< NurbsPatchPtr,BezierPatchPtr >();

}


