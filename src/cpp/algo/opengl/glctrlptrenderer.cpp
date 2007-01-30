/* -*-c++-*-
 *  ----------------------------------------------------------------------------
 *
 *       AMAPmod: Exploring and Modeling Plant Architecture
 *
 *       Copyright 1995-2000 UMR Cirad/Inra Modelisation des Plantes
 *
 *       File author(s): C. Nouguier & F. Boudon (frederic.boudon@cirad.fr) nouguier
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




#include "actn_glctrlptrenderer.h"
#include "actn_discretizer.h"

#include "appe_material.h"
#include "geom_axisrotated.h"
#include "geom_beziercurve.h"
#include "geom_bezierpatch.h"
#include "geom_elevationgrid.h"
#include "geom_eulerrotated.h"
#include "geom_extrudedhull.h"
#include "geom_extrusion.h"
#include "geom_oriented.h"
#include "geom_polyline.h"
#include "geom_nurbscurve.h"
#include "geom_nurbspatch.h"
#include "geom_revolution.h"
#include "geom_scaled.h"
#include "geom_sphere.h"
#include "geom_tapered.h"
#include "geom_translated.h"
#include "scne_shape.h"

#include "geom_pointarray.h"
#include "geom_pointmatrix.h"

GEOM_USING_NAMESPACE
TOOLS_USING_NAMESPACE

/* ----------------------------------------------------------------------- */


#define GEOM_GLCTRLPOINTRENDERER_DRAW_LINE(Point4A) { \
    glBegin(GL_LINE_STRIP); \
    for (Point4Array::const_iterator _i = Point4A->getBegin(); \
         _i != Point4A->getEnd(); \
         _i++) \
      glGeomVertex(_i->x(), \
                   _i->y(), \
                   _i->z()); \
    glEnd(); \
  };

#define GEOM_GLCTRLPOINTRENDERER_DRAW_LINE3(Point3A) { \
    glBegin(GL_LINE_STRIP); \
    for (Point3Array::const_iterator _i = Point3A->getBegin(); \
         _i != Point3A->getEnd(); \
         _i++) \
      glGeomVertex(_i->x(), \
                   _i->y(), \
                   _i->z()); \
    glEnd(); \
  };

#define GEOM_GLCTRLPOINTRENDERER_DRAW_LINE2D(Point3A) { \
    glBegin(GL_LINE_STRIP); \
    for (Point3Array::const_iterator _i = Point3A->getBegin(); \
         _i != Point3A->getEnd(); \
         _i++) \
      glGeomVertex(_i->x(), \
                   _i->y(), \
                   0); \
    glEnd(); \
  };

#define GEOM_GLCTRLPOINTRENDERER_DRAW_HORLINE2D(Point3A) { \
    glBegin(GL_LINE_STRIP); \
    for (Point3Array::const_iterator _i = Point3A->getBegin(); \
         _i != Point3A->getEnd(); \
         _i++) \
      glGeomVertex(_i->x(), \
                   _i->y(), \
                   0); \
    glGeomVertex(Point3A->getBegin()->x(),Point3A->getBegin()->y(),0); \
    glEnd(); \
  };

#define GEOM_GLCTRLPOINTRENDERER_DRAW_VERTLINE2D(Point3A) { \
    glBegin(GL_LINE_STRIP); \
    for (Point3Array::const_iterator _i = Point3A->getBegin(); \
         _i != Point3A->getEnd(); \
         _i++) \
      glGeomVertex(_i->x(), \
                   0, \
                   _i->y()); \
    glGeomVertex(Point3A->getBegin()->x(),0,Point3A->getBegin()->y()); \
    glEnd(); \
  };

#define GEOM_GLCTRLPOINTRENDERER_DRAW_LINES(Point4Matrix) { \
    for (uint32_t _usize = 0 ; _usize < Point4Matrix->getRowsNb() ; _usize++){ \
      glBegin(GL_LINE_STRIP); \
      for (uint32_t _vsize = 0; \
         _vsize <  Point4Matrix->getColsNb() ; _vsize++){ \
         Vector4 mypoint=Point4Matrix->getAt(_usize,_vsize); \
         glGeomVertex(mypoint.x(), \
                      mypoint.y(), \
                      mypoint.z()); \
         } \
      glEnd(); \
    }; \
    for (uint32_t _vsize = 0 ; _vsize < Point4Matrix->getColsNb(); _vsize++){ \
      glBegin(GL_LINE_STRIP); \
      for (uint32_t _usize = 0; \
         _usize <  Point4Matrix->getRowsNb() ; _usize++){ \
         Vector4 mypoint=Point4Matrix->getAt(_usize,_vsize); \
         glGeomVertex(mypoint.x(), \
                      mypoint.y(), \
                      mypoint.z()); \
         } \
      glEnd(); \
    }; \
  };



AppearancePtr GLCtrlPointRenderer::DEFAULT_APPEARANCE(new Material(Color3::RED));

/* ----------------------------------------------------------------------- */


GLCtrlPointRenderer::GLCtrlPointRenderer( Discretizer& discretizer ) :
  GLRenderer(discretizer) {
}

bool GLCtrlPointRenderer::beginProcess(){
  DEFAULT_APPEARANCE->apply(*this);
  return GLRenderer::beginProcess();
}

/* ----------------------------------------------------------------------- */
bool GLCtrlPointRenderer::processAppereance(GeomShape * geomShape){
    GEOM_ASSERT(geomShape);
    return true;
}


bool GLCtrlPointRenderer::process( BezierCurve * bezierCurve ) {
  GEOM_ASSERT(bezierCurve);

  GEOM_GLCTRLPOINTRENDERER_DRAW_LINE(bezierCurve->getCtrlPointList());

  return true;
}

/* ----------------------------------------------------------------------- */


bool GLCtrlPointRenderer::process( BezierPatch * bezierPatch ) {
  GEOM_ASSERT(bezierPatch);

  GEOM_GLCTRLPOINTRENDERER_DRAW_LINES(bezierPatch->getCtrlPointMatrix());

  return true;
}


/* ----------------------------------------------------------------------- */


bool GLCtrlPointRenderer::process( ExtrudedHull * extrudedHull ) {
  GEOM_ASSERT(extrudedHull);

  if(extrudedHull->getHorizontal()->apply(__discretizer)){
    GeomPolylinePtr p;
    p.cast(__discretizer.getDiscretization());
    if(p){
      GEOM_GLCTRLPOINTRENDERER_DRAW_HORLINE2D(p->getPointList());
    }
  }

  if(extrudedHull->getVertical()->apply(__discretizer)){
    GeomPolylinePtr p;
    p.cast(__discretizer.getDiscretization());
    if(p){
      GEOM_GLCTRLPOINTRENDERER_DRAW_VERTLINE2D(p->getPointList());
    }
  }


  return true;
}



/* ----------------------------------------------------------------------- */


bool GLCtrlPointRenderer::process( Extrusion * extrusion ) {
  GEOM_ASSERT(extrusion);

  extrusion->getAxis()->apply(*this);

  return true;
}



/* ----------------------------------------------------------------------- */


bool GLCtrlPointRenderer::process( NurbsCurve * nurbsCurve ) {
  GEOM_ASSERT(nurbsCurve);

  GEOM_GLCTRLPOINTRENDERER_DRAW_LINE(nurbsCurve->getCtrlPointList());

  return true;
}

/* ----------------------------------------------------------------------- */


bool GLCtrlPointRenderer::process( NurbsPatch * nurbsPatch ) {
  GEOM_ASSERT(nurbsPatch);

  GEOM_GLCTRLPOINTRENDERER_DRAW_LINES(nurbsPatch->getCtrlPointMatrix());

  return true;
}


/* ----------------------------------------------------------------------- */


bool GLCtrlPointRenderer::process( Tapered * tapered ) {
  GEOM_ASSERT(tapered);
  BezierCurvePtr _curve;
  if(_curve.cast(tapered->getPrimitive())){
      Transformation3DPtr _taper=tapered->getTransformation();
      Point4ArrayPtr mypoints =  _taper->transform(_curve->getCtrlPoints());
      GEOM_GLCTRLPOINTRENDERER_DRAW_LINE(mypoints);
  }
  else{
      BezierPatchPtr _patch;
      if(_patch.cast(tapered->getPrimitive())){
          Transformation3DPtr _taper=tapered->getTransformation();
          Point4MatrixPtr mypoints =  _taper->transform(_patch->getCtrlPoints());
          GEOM_GLCTRLPOINTRENDERER_DRAW_LINES(mypoints);
      }
      else {
        ExtrudedHullPtr extrudedHull;
        if(extrudedHull.cast(tapered->getPrimitive())){
          if(extrudedHull->getHorizontal()->apply(__discretizer)){
	    GeomPolylinePtr p;
            if(p.cast(__discretizer.getDiscretization())){
              GEOM_GLCTRLPOINTRENDERER_DRAW_HORLINE2D(p->getPointList());
            }
          }

          if(extrudedHull->getVertical()->apply(__discretizer)){
            GeomPolylinePtr p;
	    p.cast(__discretizer.getDiscretization());
            Point3ArrayPtr points(new Point3Array(*(p->getPointList())));
            points->pushBack(points->getAt(0));
            for(Point3Array::iterator _it = points->getBegin();
                _it != points->getEnd();
                _it++){ _it->z() = _it->y();_it->y() = 0;}
            Transformation3DPtr _taper=tapered->getTransformation();
            _taper->transform(points);
            if(p){
              GEOM_GLCTRLPOINTRENDERER_DRAW_LINE3(points);
            }
          }
        }
        else return false;
      }
  }

  return true;
}


/* ----------------------------------------------------------------------- */


bool GLCtrlPointRenderer::process( BezierCurve2D * bezierCurve ) {
  GEOM_ASSERT(bezierCurve);

  GEOM_GLCTRLPOINTRENDERER_DRAW_LINE2D(bezierCurve->getCtrlPointList());

  return true;
}

/* ----------------------------------------------------------------------- */


bool GLCtrlPointRenderer::process( NurbsCurve2D * nurbsCurve ) {
  GEOM_ASSERT(nurbsCurve);

  GEOM_GLCTRLPOINTRENDERER_DRAW_LINE2D(nurbsCurve->getCtrlPointList());

  return true;
}

/* ----------------------------------------------------------------------- */