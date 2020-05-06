/* -*-c++-*-
 *  ----------------------------------------------------------------------------
 *
 *       PlantGL: The Plant Graphic Library
 *
 *       Copyright CIRAD/INRIA/INRA
 *
 *       File author(s): F. Boudon (frederic.boudon@cirad.fr) et al.
 *
 *  ----------------------------------------------------------------------------
 *
 *   This software is governed by the CeCILL-C license under French law and
 *   abiding by the rules of distribution of free software.  You can  use,
 *   modify and/ or redistribute the software under the terms of the CeCILL-C
 *   license as circulated by CEA, CNRS and INRIA at the following URL
 *   "http://www.cecill.info".
 *
 *   As a counterpart to the access to the source code and  rights to copy,
 *   modify and redistribute granted by the license, users are provided only
 *   with a limited warranty  and the software's author,  the holder of the
 *   economic rights,  and the successive licensors  have only  limited
 *   liability.
 *
 *   In this respect, the user's attention is drawn to the risks associated
 *   with loading,  using,  modifying and/or developing or reproducing the
 *   software by the user in light of its specific status of free software,
 *   that may mean  that it is complicated to manipulate,  and  that  also
 *   therefore means  that it is reserved for developers  and  experienced
 *   professionals having in-depth computer knowledge. Users are therefore
 *   encouraged to load and test the software's suitability as regards their
 *   requirements in conditions enabling the security of their systems and/or
 *   data to be ensured and,  more generally, to use and operate it in the
 *   same conditions as regards security.
 *
 *   The fact that you are presently reading this means that you have had
 *   knowledge of the CeCILL-C license and that you accept its terms.
 *
 *  ----------------------------------------------------------------------------
 */

#include <plantgl/tool/bfstream.h>
#include <plantgl/tool/util_string.h>

#include "drcprinter.h"
#include <plantgl/algo/base/tesselator.h>

#include <plantgl/pgl_scene.h>
#include <plantgl/pgl_appearance.h>
#include <plantgl/pgl_geometry.h>
#include <plantgl/pgl_transformation.h>

#include <plantgl/scenegraph/container/pointarray.h>
#include <plantgl/scenegraph/container/indexarray.h>

PGL_USING_NAMESPACE

using namespace std;

/* ----------------------------------------------------------------------- */

#define GEOM_DRC_MESH(obj, gindex, len) \
  if (__pass == 1)  \
  { \
    __face += obj->getIndexList()->size(); \
  } \
  else if (__pass == 2) \
  { \
    auto points = obj->getPointList(); \
    \
    for (gindex##Array::const_iterator _it = obj->getIndexList()->begin();  \
         _it != obj->getIndexList()->end(); _it++)  \
    { \
      \
      \
        gindex::const_iterator _it2 = _it->begin(); \
        Vector3 p1 = points->getAt(*(_it2)); \
        Vector3 p2 = points->getAt(*(++_it2)); \
        Vector3 p3 = points->getAt(*(++_it2)); \
      \
      __mesh_builder.SetAttributeValuesForFace( \
          pos_att_id, draco::FaceIndex(__index),  \
          draco::Vector3f(p1.x(), p1.y(), p1.z()).data(), \
          draco::Vector3f(p2.x(), p2.y(), p2.z()).data(), \
          draco::Vector3f(p3.x(), p3.y(), p3.z()).data()  \
      \
      ); \
      \
      uchar_t colors[3] = { (uchar_t)__red, (uchar_t)__green, (uchar_t)__blue }; \
      \
      __mesh_builder.SetPerFaceAttributeValueForFace( \
          col_att_id, draco::FaceIndex(__index), &colors  \
        );  \
      \
      __index++;  \
    } \
  } \
  return true; \


/* ----------------------------------------------------------------------- */

DrcPrinter::DrcPrinter(ostream &stream, Discretizer &discretizer, draco::TriangleSoupMeshBuilder &mesh_builder, draco::EncoderBuffer &buffer, int speed)
 : Printer(stream, stream, stream),
  __discretizer(discretizer),
  __mesh_builder(mesh_builder),
  __buffer(buffer),
  __speed(speed),
  __pass(0),
  __vertex(0),
  __face(0),
  __red(160),
  __green(160),
  __blue(160),
  __index(0)
{
}

DrcPrinter::DrcPrinter(Discretizer &discretizer, draco::TriangleSoupMeshBuilder &mesh_builder, draco::EncoderBuffer &buffer, int speed)
  : Printer(),
  __discretizer(discretizer),
  __mesh_builder(mesh_builder),
  __buffer(buffer),
  __speed(speed),
  __pass(0),
  __vertex(0),
  __face(0),
  __red(160),
  __green(160),
  __blue(160),
  __index(0)
{

}

DrcPrinter::~DrcPrinter()
{
}

#define stream __geomStream
/* ----------------------------------------------------------------------- */

bool
DrcPrinter::beginProcess()
{
  return true;
}

bool
DrcPrinter::endProcess()
{
  return true;
}


bool
DrcPrinter::process( Inline * geominline )
{
  GEOM_ASSERT( geominline );
  return geominline->getScene()->apply(*this);
}


/* ----------------------------------------------------------------------- */


bool
DrcPrinter::process( Shape * Shape )
{
  GEOM_ASSERT( Shape );
  if( __pass == 2 ){
    Shape->getAppearance()->apply(*this);
    // if (Shape->appearance) {
    //   if (Shape->appearance->isTexture()) {
    //     // std::cout << "isTexture" << Shape->getName();
    //     //__discretizer.computeTexCoord(true);

    //   }
    // }
    if(Shape->getGeometry()->apply(__discretizer))
      return __discretizer.getDiscretization()->apply(*this);
    else return false;
  }
  else if( __pass == 1){
    if(Shape->getGeometry()->apply(__discretizer))
      return __discretizer.getDiscretization()->apply(*this);
    else return false;
  }
  return false;
}


/* ----------------------------------------------------------------------- */


bool
DrcPrinter::process( Material * material )
{
  GEOM_ASSERT( material );

  __red = int(material->getAmbient().getRed());
  __green = int(material->getAmbient().getGreen());
  __blue = int(material->getAmbient().getBlue());
  return true;
}


/* ----------------------------------------------------------------------- */

bool
DrcPrinter::process( Texture2D * texture )
{
  GEOM_ASSERT( texture );
  // std::cout << "Texture2D" << texture->getName() << texture->getBaseColor().getBlue() << std::endl;
  // GLfloat _rgba[4];

  const Color4 &_color = texture->getBaseColor();
  __red = _color.getRed();
  __green = _color.getGreen();
  __blue = _color.getBlue();
  return true;
}


/* ----------------------------------------------------------------------- */

bool
DrcPrinter::process( ImageTexture * texture )
{
  GEOM_ASSERT( texture );
  return true;
}


/* ----------------------------------------------------------------------- */

bool
DrcPrinter::process( Texture2DTransformation * texture )
{
  GEOM_ASSERT( texture );
  return true;
}


/* ----------------------------------------------------------------------- */


bool
DrcPrinter::process( MonoSpectral * monoSpectral )
{
  GEOM_ASSERT( monoSpectral );
  __red = 160;
  __green = 160;
  __blue = 160;
  return true;
}


/* ----------------------------------------------------------------------- */


bool
DrcPrinter::process( MultiSpectral * multiSpectral )
{
  GEOM_ASSERT( multiSpectral );
  __red = 160;
  __green = 160;
  __blue = 160;
  return true;
}


/* ----------------------------------------------------------------------- */


bool
DrcPrinter::process( AmapSymbol * amapSymbol )
{
  GEOM_ASSERT( amapSymbol );
  GEOM_DRC_MESH( amapSymbol ,Index,_it->size() );

}


/* ----------------------------------------------------------------------- */


bool
DrcPrinter::process( AsymmetricHull * asymmetricHull )
{
  GEOM_ASSERT( asymmetricHull );
  return false;
}


/* ----------------------------------------------------------------------- */


bool
DrcPrinter::process( AxisRotated * axisRotated )
{
  GEOM_ASSERT( axisRotated );
  return false;
}


/* ----------------------------------------------------------------------- */


bool
DrcPrinter::process( BezierCurve * bezierCurve )
{
  GEOM_ASSERT( bezierCurve );
  return false;
}


/* ----------------------------------------------------------------------- */


bool
DrcPrinter::process( BezierPatch * bezierPatch )
{
  GEOM_ASSERT( bezierPatch );
  return false;
}


/* ----------------------------------------------------------------------- */


bool
DrcPrinter::process( Box * box )
{
  GEOM_ASSERT( box );
  return false;
}


/* ----------------------------------------------------------------------- */


bool
DrcPrinter::process( Cone * cone )
{
  GEOM_ASSERT( cone );
  return false;
}


/* ----------------------------------------------------------------------- */


bool
DrcPrinter::process( Cylinder * cylinder )
{
  GEOM_ASSERT( cylinder );
  return false;
}


/* ----------------------------------------------------------------------- */


bool
DrcPrinter::process( ElevationGrid * elevationGrid )
{
  GEOM_ASSERT( elevationGrid );
  return false;
}


/* ----------------------------------------------------------------------- */


bool
DrcPrinter::process( EulerRotated * eulerRotated )
{
  GEOM_ASSERT( eulerRotated );
  return false;
}


/* ----------------------------------------------------------------------- */


bool
DrcPrinter::process( ExtrudedHull * extrudedHull )
{
  GEOM_ASSERT( extrudedHull );
  return false;
}


/* ----------------------------------------------------------------------- */


bool
DrcPrinter::process( Extrusion * extrusion )
{
  GEOM_ASSERT( extrusion );
  return false;
}


/* ----------------------------------------------------------------------- */


bool
DrcPrinter::process( FaceSet * faceSet )
{
  GEOM_ASSERT( faceSet );
  GEOM_DRC_MESH( faceSet ,Index,_it->size() );
}


/* ----------------------------------------------------------------------- */


bool
DrcPrinter::process( Frustum * frustum )
{
  GEOM_ASSERT( frustum );
  return false;
}


/* ----------------------------------------------------------------------- */


bool
DrcPrinter::process( Group * group )
{
  GEOM_ASSERT( group );
  return false;
}


/* ----------------------------------------------------------------------- */


bool
DrcPrinter::process( NurbsCurve * nurbsCurve )
{
  GEOM_ASSERT( nurbsCurve );
  return false;
}


/* ----------------------------------------------------------------------- */


bool
DrcPrinter::process( NurbsPatch * nurbsPatch )
{
  GEOM_ASSERT( nurbsPatch );
  return false;
}


/* ----------------------------------------------------------------------- */


bool
DrcPrinter::process( Oriented * oriented )
{
  GEOM_ASSERT( oriented );
  return false;
}


/* ----------------------------------------------------------------------- */


bool
DrcPrinter::process( Paraboloid * paraboloid )
{
  GEOM_ASSERT( paraboloid );
  return false;
}


/* ----------------------------------------------------------------------- */


bool
DrcPrinter::process( PointSet * pointSet )
{
  GEOM_ASSERT( pointSet );
  return false;
}


/* ----------------------------------------------------------------------- */


bool
DrcPrinter::process( Polyline * polyline )
{
  GEOM_ASSERT( polyline );
  return false;
}


/* ----------------------------------------------------------------------- */


bool
DrcPrinter::process( Revolution * revolution )
{
  GEOM_ASSERT( revolution );
  return false;
}


/* ----------------------------------------------------------------------- */


bool
DrcPrinter::process( QuadSet * quadSet )
{
  GEOM_ASSERT( quadSet );
  GEOM_DRC_MESH( quadSet , Index4 , 4 );
}


/* ----------------------------------------------------------------------- */


bool
DrcPrinter::process( Scaled * scaled )
{
  GEOM_ASSERT( scaled );
  return false;
}


/* ----------------------------------------------------------------------- */


bool
DrcPrinter::process( ScreenProjected * scp )
{
  GEOM_ASSERT( scp );
  return false;
}

/* ----------------------------------------------------------------------- */


bool
DrcPrinter::process( Sphere * sphere )
{
  GEOM_ASSERT( sphere );
  return false;
}


/* ----------------------------------------------------------------------- */


bool
DrcPrinter::process( Tapered * tapered )
{
  GEOM_ASSERT( tapered );
  return false;
}


/* ----------------------------------------------------------------------- */


bool
DrcPrinter::process( Translated * translated )
{
  GEOM_ASSERT( translated );
  return false;
}


/* ----------------------------------------------------------------------- */


bool
DrcPrinter::process( TriangleSet * triangleSet )
{
  GEOM_ASSERT( triangleSet );
  GEOM_DRC_MESH( triangleSet ,Index3 , 3 );


}


/* ----------------------------------------------------------------------- */


bool
DrcPrinter::process( BezierCurve2D * bezierCurve )
{
  GEOM_ASSERT( bezierCurve );
  return false;
}


/* ----------------------------------------------------------------------- */


bool
DrcPrinter::process( Disc * disc )
{
  GEOM_ASSERT( disc );
  return false;
}


/* ----------------------------------------------------------------------- */


bool
DrcPrinter::process( NurbsCurve2D * nurbsCurve )
{
  GEOM_ASSERT( nurbsCurve );
  return false;
}


/* ----------------------------------------------------------------------- */


bool
DrcPrinter::process( PointSet2D * pointSet )
{
  GEOM_ASSERT( pointSet );
  return false;
}


/* ----------------------------------------------------------------------- */


bool
DrcPrinter::process( Polyline2D * polyline )
{
  GEOM_ASSERT( polyline );
  return false;
}


/* ----------------------------------------------------------------------- */

bool
DrcPrinter::process( Text * text )
{
  GEOM_ASSERT( text );
  return true;
}

/* ----------------------------------------------------------------------- */
bool
DrcPrinter::process( Font * font )
{
  GEOM_ASSERT( font );
  return true;
}

/* ----------------------------------------------------------------------- */


bool
DrcPrinter::process(ScenePtr scene)
{
  GEOM_ASSERT(scene);
  __pass = 1;
  scene->apply(*this);
  __mesh_builder.Start(__face);
  pos_att_id = __mesh_builder.AddAttribute(draco::GeometryAttribute::POSITION, 3, draco::DT_FLOAT32);
  col_att_id = __mesh_builder.AddAttribute(draco::GeometryAttribute::COLOR, 3, draco::DT_UINT8);
  __pass = 2;
  scene->apply(*this);
  auto mesh = __mesh_builder.Finalize();

  draco::ExpertEncoder encoder(*mesh.get());
  encoder.SetAttributeQuantization(draco::GeometryAttribute::POSITION, 11);
  encoder.SetAttributeQuantization(draco::GeometryAttribute::TEX_COORD, 10);
  encoder.SetAttributeQuantization(draco::GeometryAttribute::NORMAL, 7);
  encoder.SetAttributeQuantization(draco::GeometryAttribute::GENERIC, 8);
  encoder.SetSpeedOptions(__speed, __speed);

  auto status = encoder.EncodeToBuffer(&__buffer);

  return status.ok();
}


/* ----------------------------------------------------------------------- */

#undef stream

/* ----------------------------------------------------------------------- */


bool
DrcPrinter::print(ScenePtr scene,string filename,const char * comment, int speed)
{

    ofstream stream(filename.c_str());
    if(!stream)return false;
    else {
      Discretizer discretizer;
      draco::TriangleSoupMeshBuilder mesh_builder;
      draco::EncoderBuffer buffer;
      DrcPrinter printer(stream, discretizer, mesh_builder, buffer, speed);
      bool status = printer.process(scene);
      if (status)
        stream << buffer.data();
      return status;
    }
}



bool
DrcPrinter::print(ScenePtr scene, Discretizer & discretizer,
      string filename,const char * comment, int speed)
{
  ofstream stream(filename.c_str());
  if (!stream)
    return false;
  else
  {
    draco::TriangleSoupMeshBuilder mesh_builder;
    draco::EncoderBuffer buffer;
    DrcPrinter printer(stream, discretizer, mesh_builder, buffer, speed);
    bool status = printer.process(scene);
    if (status)
      stream << buffer.data();
    return status;
  }
}

char*
DrcPrinter::print(ScenePtr scene, size_t *size, int speed)
{

  Tesselator t;
  draco::TriangleSoupMeshBuilder mesh_builder;
  draco::EncoderBuffer buffer;
  DrcPrinter printer(t, mesh_builder, buffer, speed);

  bool ok = printer.process(scene);
  if (!ok)
  {
    *size = 0;
    return NULL;
  }

  char *data = static_cast<char*>(malloc(buffer.size() * sizeof(char)));
  memcpy(data, buffer.data(), buffer.size() * sizeof(char));

  *size = buffer.size();

  return data;

}
