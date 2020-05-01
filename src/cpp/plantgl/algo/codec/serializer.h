/* -*-c++-*-
 *  ----------------------------------------------------------------------------
 *
 *       PlantGL: The Plant Graphic Library
 *
 *       Copyright 1995-2007 UMR CIRAD/INRIA/INRA DAP
 *
 *       File author(s): F. Boudon, J. Vaillant et al.
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


/*! \file serializer.h
    \brief Serialize GEOMs
*/



#ifndef __actn_serializer_h__
#define __actn_serializer_h__

#include <draco/mesh/triangle_soup_mesh_builder.h>
#include <draco/compression/expert_encode.h>

#include <plantgl/scenegraph/core/action.h>
#include <plantgl/tool/rcobject.h>
#include <plantgl/tool/util_cache.h>
#include <plantgl/scenegraph/appearance/appearance.h>
#include <plantgl/scenegraph/geometry/explicitmodel.h>
#include <plantgl/algo/base/matrixcomputer.h>
#include <plantgl/scenegraph/appearance/color.h>
#include <plantgl/algo/base/discretizer.h>
#include <plantgl/algo/base/tesselator.h>

PGL_BEGIN_NAMESPACE

// TODO: add texture, material (color) per instance?
struct TriangleSoup
{
    uint_t id;
    TriangleSet *triangles;
    std::vector<Matrix4> instances;
    int red;
    int green;
    int blue;
    TriangleSoup(uint_t id, TriangleSet *triangles, Matrix4 &matrix)
        : id(id), triangles(triangles), instances({matrix}) {}
};

/**
   \class Serializer
   \brief An action which serializes GEOMs.
*/

class ALGO_API Serializer : public Action
{

public:

  /// Constructor
  Serializer();

//   enum eGeomType {};

  /// Destructor
  virtual ~Serializer( );

  /// Begining of the Action.
  virtual bool beginProcess();

  /// End of the Action.
  virtual bool endProcess();

  virtual bool process(Inline * geomInline);

  virtual bool process( Material * material );

  virtual bool process( MonoSpectral * monoSpectral );

  virtual bool process( MultiSpectral * multiSpectral );

  virtual bool process( Texture2D * texture );

  virtual bool process( ImageTexture * texture );

  virtual bool process( Texture2DTransformation * texturetransformation );

  virtual bool process(Shape *  Shape );

  virtual bool processAppereance(Shape *  Shape );

  virtual bool processGeometry(Shape *  Shape );

  virtual bool process( AmapSymbol * amapSymbol );

  virtual bool process( AsymmetricHull * asymmetricHull );

  virtual bool process( AxisRotated * axisRotated );

  virtual bool process( BezierCurve * bezierCurve );

  virtual bool process( BezierPatch * bezierPatch );

  virtual bool process( Box * box );

  virtual bool process( Cone * cone );

  virtual bool process( Cylinder * cylinder );

  virtual bool process( ElevationGrid * elevationGrid );

  virtual bool process( EulerRotated * eulerRotated );

  virtual bool process( ExtrudedHull * extrudedHull );

  virtual bool process( FaceSet * faceSet );

  virtual bool process( Frustum * frustum );

  virtual bool process( Extrusion * extrusion );

  virtual bool process( Group * group );

  virtual bool process( IFS * ifs );

  virtual bool process( NurbsCurve * nurbsCurve );

  virtual bool process( NurbsPatch * nurbsPatch );

  virtual bool process( Oriented * oriented );

  virtual bool process( Paraboloid * paraboloid );

  virtual bool process( PointSet * pointSet );

  virtual bool process( Polyline * polyline );

  virtual bool process( QuadSet * quadSet );

  virtual bool process( Revolution * revolution );

  virtual bool process( Swung * swung );

  virtual bool process( Scaled * scaled );

  virtual bool process( ScreenProjected * scp );

  virtual bool process( Sphere * sphere );

  virtual bool process( Tapered * tapered );

  virtual bool process( Translated * translated );

  virtual bool process( TriangleSet * triangleSet );

  virtual bool process( BezierCurve2D * bezierCurve );

  virtual bool process( Disc * disc );

  virtual bool process( NurbsCurve2D * nurbsCurve );

  virtual bool process( PointSet2D * pointSet );

  virtual bool process( Polyline2D * polyline );

  virtual bool process( Text * text );

  virtual bool process( Font * font );

  void setSpeed(int speed) { __speed = std::max(0, std::min(10, speed)); }
  char* data() { return __data.data(); }
  size_t size() { return __data.size(); }
  std::vector<size_t> offsets() { return __offsets; }

protected:

  typedef Cache<ExplicitModelPtr> DiscretizerCache;
  DiscretizerCache __discretizer_cache;
  typedef Cache<TriangleSetPtr> TesselatorCache;
  TesselatorCache __tesselator_cache;
  typedef Cache<TriangleSoup> TriangleSoupCache;
  TriangleSoupCache __triangle_soup_cache;

  /// The discretizer used to store the discretize parametric while
  Discretizer __discretizer;
  Tesselator __tesselator;

  /// The current material
  AppearancePtr __appearance;

  MatrixStack __modelmatrix;
  MatrixStack __texturematrix;

  int __red = 255;
  int __green = 255;
  int __blue = 255;

  int __speed = 3;


private:
  bool addMesh(size_t id, const TriangleSoup & soup);
  bool addInstances(size_t id, const TriangleSoup & soup);
  std::vector<char> __data;
  std::vector<size_t> __offsets;
  template<class T>
  bool discretize(T * geom);
  template<class T>
  bool tesselate(T * geom);
};

/* ----------------------------------------------------------------------- */

PGL_END_NAMESPACE

/* ----------------------------------------------------------------------- */

// __actn_serializer_h__
#endif
