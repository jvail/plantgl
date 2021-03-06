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



#include <fstream>
#include <iomanip>
#include <string>

#include "linetreeprinter.h"
#include <plantgl/pgl_appearance.h>
#include <plantgl/pgl_geometry.h>
#include <plantgl/pgl_transformation.h>
#include <plantgl/scenegraph/scene/shape.h>
#include <plantgl/scenegraph/container/geometryarray2.h>
#include <plantgl/math/util_math.h>
#include <plantgl/tool/util_string.h>
#include <plantgl/tool/dirnames.h>
#include <plantgl/tool/bfstream.h>

PGL_USING_NAMESPACE

using namespace std;
using namespace STDEXT;

/* ----------------------------------------------------------------------- */
GeometryPtr LinetreePrinter::UNIT_CYLINDER;
GeometryPtr LinetreePrinter::UNIT_SOLID_CYLINDER;
GeometryPtr LinetreePrinter::UNIT_SPHERE;
GeometryPtr LinetreePrinter::UNIT_DISC;
GeometryPtr LinetreePrinter::UNIT_BOX;

void LinetreePrinter::initUnitShape(){
  if(!UNIT_CYLINDER){
          UNIT_CYLINDER = GeometryPtr(new Cylinder(Cylinder::DEFAULT_RADIUS,
                                                                                           Cylinder::DEFAULT_HEIGHT,
                                                                                           false,2*Cylinder::DEFAULT_SLICES));

          UNIT_CYLINDER->setName("ucylinder");
  }
  if(!UNIT_SOLID_CYLINDER){
          UNIT_SOLID_CYLINDER = GeometryPtr(new Cylinder(Cylinder::DEFAULT_RADIUS,
                                                                                           Cylinder::DEFAULT_HEIGHT,
                                                                                           true,2*Cylinder::DEFAULT_SLICES));

          UNIT_SOLID_CYLINDER->setName("usolidcylinder");
  }
  if(!UNIT_SPHERE){
          UNIT_SPHERE = GeometryPtr(new Sphere(Sphere::DEFAULT_RADIUS,
                  2*Sphere::DEFAULT_SLICES,2*Sphere::DEFAULT_STACKS));

          UNIT_SPHERE->setName("usphere");
  }
  if(!UNIT_DISC){
          UNIT_DISC = GeometryPtr(new Disc(Disc::DEFAULT_RADIUS,2*SOR::DEFAULT_SLICES));
          UNIT_DISC->setName("udisc");
  }
  if(!UNIT_BOX){
          UNIT_BOX = GeometryPtr(new Box());
          UNIT_BOX->setName("ubox");
  }
}

string LinetreePrinter::PROTECTION = "!&@V3@aFFv5</p*kRE=fCUSB9-;/Q[RS9@nq=9PL22^RE[%<pTIsMvS6mUoL8?RJmQWrM%a.Sj@rl8cO";

/* ----------------------------------------------------------------------- */

LinetreePrinter::LinetreePrinter( bofstream& stream,
                                  ostream& dtastream,
                                  const string& smbpath,
                                  AmapTranslator& translator) :
  MatrixComputer(),
  __ligstream(stream),
  __dtastream(dtastream),
  __smbpath(smbpath),
  __base_radius(1),
  __top_radius(1),
  __translator(translator),
  __smbNumber(0){
          initUnitShape();
}


/* ----------------------------------------------------------------------- */

bool LinetreePrinter::header(const std::string& key){
        if(key.empty()){
                __ligstream << PROTECTION;
                return true;
        }
        else if(key.size() == 80){
                __ligstream << key;
                return true;
        }
    else __ligstream << PROTECTION;
    return false;
}

void
LinetreePrinter::printInf( ostream& inf,
                                                   const Vector3& _max ,
                                                   const Vector3& _min ) const{

        inf << "File : 1geom" << std::endl;
    inf << "Age : 5   1  pattern(s) number of branches 1"  << std::endl;
    inf << "Random_seed 0 Simplification 0 " << std::endl;

    inf.precision(6);

    inf << _max.x() << " " << _max.y() << " " << _max.z() << std::endl;
    inf << _min.x() << " " << _min.y() << " " << _min.z() << std::endl;
    inf << "entre-noeud 1        nentn105  1" << endl;

}

bool LinetreePrinter::printCfg( const std::string& path,
                                                                const std::string& ligne ,
                                                                const std::string& smb,
                                                                std::string& cfg) const
{
        cfg = path;
        if((*(cfg.end()-1)) != '/' && (*(cfg.end()-1)) != '\\' )cfg+='/';
        cfg+=".cfg";
        ofstream stream(cfg.c_str());
        if(stream){
                stream << "# Pathnames to AMAP directories" << endl;
                stream << "LIGNE = " << ligne << endl;
                stream << "SYMBOLES = " << smb << endl;
                return true;
        }
        else return false;
}

/* ----------------------------------------------------------------------- */
void LinetreePrinter::printMaterial(const MaterialPtr& mat){
        __dtastream << setprecision(3);
        __dtastream << "EMISSION  "
                << mat->getEmission().getRedClamped() << ' '
                << mat->getEmission().getGreenClamped() << ' '
                << mat->getEmission().getBlueClamped() << endl;

        __dtastream << "AMBIENT   "
                << mat->getAmbient().getRedClamped() << ' '
                << mat->getAmbient().getGreenClamped() << ' '
                << mat->getAmbient().getBlueClamped() << endl;

        __dtastream << "DIFFUSE   "
                << mat->getAmbient().getRedClamped()*mat->getDiffuse() << ' '
                << mat->getAmbient().getGreenClamped()*mat->getDiffuse() << ' '
                << mat->getAmbient().getBlueClamped()*mat->getDiffuse() << endl;

        __dtastream << "SPECULAR  "
                << mat->getSpecular().getRedClamped() << ' '
                << mat->getSpecular().getGreenClamped() << ' '
                << mat->getSpecular().getBlueClamped() << endl;

        __dtastream << "SHININESS "
                << mat->getShininess()*20 << endl;

}

#define PRINT_COORD(coord) \
        __ligstream << (float) c.coord(); \
        __ligstream << (float) a.coord(); \
        __ligstream << (float) b.coord(); \
        __ligstream << (float) t.coord();

void LinetreePrinter::printTransformation(){

  // val1 = 1 ; val2 = 1 ; range = 1;
  __ligstream << long(1) << long(1) << long(1);

  Vector3 a(getMatrix()(0,0),getMatrix()(1,0),getMatrix()(2,0));
  Vector3 b(getMatrix()(0,1),getMatrix()(1,1),getMatrix()(2,1));
  Vector3 c(getMatrix()(0,2),getMatrix()(1,2),getMatrix()(2,2));
  Vector3 t(getMatrix()(0,3),getMatrix()(1,3),getMatrix()(2,3));

  //  real_t scale_z = c.normalize();
  float scale_x = norm(a);
  float scale_y = norm(b);

  if(scale_x == scale_y){
        a.normalize();
        b.normalize();

        PRINT_COORD(x);
        PRINT_COORD(y);
        PRINT_COORD(z);

        __ligstream << __base_radius*scale_x << __top_radius*scale_x ;
  }
  else {
        PRINT_COORD(x);
        PRINT_COORD(y);
        PRINT_COORD(z);

        __ligstream << __base_radius << __top_radius ;
  }

  __ligstream << __entity_number;
  if(__entity_number != 0)
          __entity_number = 0;
}

/* ----------------------------------------------------------------------- */

bool LinetreePrinter::symbolProcess( Geometry* object )
{
        GEOM_ASSERT(object);
        if(!object)return false;
        AmapSymbolPtr _symbol;
        Vector3 translation;
        Vector3 scaling;

        SmbMap::const_iterator
                _it = __smbcache.find(object->getObjectId());
        if(_it == __smbcache.end()){
                if(!object->apply(__translator))return false;
                _symbol = __translator.getSymbol();
                if(_symbol){
                        if(object->isNamed())_symbol->setName(object->getName());
                        translation = __translator.getTranslation();
                        scaling = __translator.getNormalizationFactors();
                        pair<Vector3,Vector3> t(translation,scaling);
                        __smbcache[object->getObjectId()] = pair<AmapSymbolPtr,pair<Vector3,Vector3> > (_symbol,t);
                }
                else return false;
        }
        else {
                _symbol = _it->second.first;
                translation = _it->second.second.first;
                scaling = _it->second.second.second;
        }

        __matrix.push();
        __matrix.translate(translation);
        __matrix.scale(scaling);
        bool result = _symbol->apply(*this);
        __matrix.pop();
        return result;
}

/* ----------------------------------------------------------------------- */

bool LinetreePrinter::endProcess(){
        __dtastream << "  " << __dta.size() << "  symbole(s)" << endl;
        for(DtaList::const_iterator
                _it = __dta.begin(); _it != __dta.end(); _it++){
                string file = __smbfilecache[_it->second.first->getObjectId()];
                if(!file.empty()){
                        file = get_filename(file);
                        size_t pos = file.find_last_of('.');
                        if(pos != string::npos && pos < file.size()){
                                file = string(file.begin(),file.begin()+pos);
                        }
                        __dtastream << "smb" << _it->first << "               " << _it->first
                                << "   " << file << "   1" << endl;
                }
        }
        for(DtaList::const_iterator
                _it2 = __dta.begin(); _it2 != __dta.end(); _it2++){
                __dtastream << "Symbole   " << _it2->first << endl;
                printMaterial(_it2->second.second);
        }
        return true;
}


/* ----------------------------------------------------------------------- */

bool LinetreePrinter::process( AmapSymbol * amapSymbol ) {

        Cache::const_iterator
                _it = __cache.find(amapSymbol->getObjectId());
        if(_it != __cache.end()){
                CacheUnit::const_iterator _it2 = _it->second.find(__mat->getObjectId());
                if(_it2 != _it->second.end()){
                        __ligstream << _it2->second;
                        printTransformation();
                        return true;
                }
        }
        __smbNumber++;
        __ligstream << __smbNumber;
        printTransformation();

        if(amapSymbol->getFileName().empty()){
                string filename;
                if(*(__smbpath.end()-1) != '/' && *(__smbpath.end()-1) != '\\')
                        __smbpath += '/';
                if(amapSymbol->isNamed())
                        filename = __smbpath + amapSymbol->getName() + ".smb";
                else filename = __smbpath +"smb" + number(amapSymbol->getObjectId()) + ".smb";
                beofstream stream(filename);
                if(stream){
                        amapSymbol->write(stream);
                        __smbfilecache[amapSymbol->getObjectId()] = filename;
                }
                else return false;
        }
        else {
                string filename;
                if(*(__smbpath.end()-1) != '/' && *(__smbpath.end()-1) != '\\')
                        __smbpath += '/';
                filename = __smbpath + get_filename(amapSymbol->getFileName());
                if(absolute_dirname(filename) !=
                        absolute_dirname(amapSymbol->getFileName())){
                        if(exists(amapSymbol->getFileName())){
                                if(!copy(amapSymbol->getFileName(),filename))return false;
                                __smbfilecache[amapSymbol->getObjectId()] = filename;
                        }
                        else {
                                beofstream stream(filename);
                                if(stream){
                                        amapSymbol->write(stream);
                                        __smbfilecache[amapSymbol->getObjectId()] = filename;
                                }
                                else return false;
                        }
                }
                else __smbfilecache[amapSymbol->getObjectId()] = filename;

        }
        __cache[amapSymbol->getObjectId()][__mat->getObjectId()] = __smbNumber;
        __dta.push_back(pair<long, pair<AmapSymbolPtr,MaterialPtr> >(__smbNumber,
                pair<AmapSymbolPtr,MaterialPtr> (AmapSymbolPtr(amapSymbol),__mat)));
        return true;
}


/* ----------------------------------------------------------------------- */


bool LinetreePrinter::process(Shape * Shape) {
  GEOM_ASSERT(Shape);
  if(Shape->getId() != Shape::NOID)
        __entity_number = Shape->getId();
  else __entity_number = Shape->getObjectId();
  bool b=Shape->appearance->apply(*this);
  return ( b && (Shape->geometry->apply(*this)));
}


/* ----------------------------------------------------------------------- */


bool LinetreePrinter::process( Material * material ) {
  GEOM_ASSERT(material);
  __mat = MaterialPtr(material);
  return true;
}


bool LinetreePrinter::process( ImageTexture * texture ) {
  GEOM_ASSERT(texture);
  return true;
}

bool LinetreePrinter::process( Texture2DTransformation * texturetransfo ) {
  GEOM_ASSERT(texturetransfo);
  return true;
}


bool LinetreePrinter::process( Texture2D * texture ) {
  GEOM_ASSERT(texture);
  __mat = dynamic_pointer_cast<Material>(Material::DEFAULT_MATERIAL);
  return true;
}


/* ----------------------------------------------------------------------- */


bool LinetreePrinter::process( MonoSpectral * monoSpectral ) {
  GEOM_ASSERT(monoSpectral);
  __mat = dynamic_pointer_cast<Material>(Material::DEFAULT_MATERIAL);
  return true;
}


/* ----------------------------------------------------------------------- */


bool LinetreePrinter::process( MultiSpectral * multiSpectral ) {
  GEOM_ASSERT(multiSpectral);
  __mat = dynamic_pointer_cast<Material>(Material::DEFAULT_MATERIAL);
  return true;
}


/* ----------------------------------------------------------------------- */


bool LinetreePrinter::process( AsymmetricHull * asymmetricHull ) {
  return symbolProcess(asymmetricHull);
}


/* ----------------------------------------------------------------------- */


bool LinetreePrinter::process( BezierCurve * bezierCurve ) {
  GEOM_ASSERT(bezierCurve);
  // nothing to do
  return false;
}


/* ----------------------------------------------------------------------- */


bool LinetreePrinter::process( BezierPatch * bezierPatch ) {
  return symbolProcess(bezierPatch);
}


/* ----------------------------------------------------------------------- */


bool LinetreePrinter::process( Box * box ) {
  __matrix.push();
  __matrix.scale(box->getSize()*2);
  symbolProcess(UNIT_BOX.get());
  __matrix.pop();
  return true;
}


/* ----------------------------------------------------------------------- */


bool LinetreePrinter::process( Cone * cone ) {
  GEOM_ASSERT(cone);

  __matrix.push();
  real_t top = __top_radius;
  __top_radius = 0.001f;

  __matrix.scale(Vector3(cone->getRadius()*2,
                                cone->getRadius()*2,
                                cone->getHeight()));
  if(cone->getSolid())symbolProcess(UNIT_SOLID_CYLINDER.get());
  else symbolProcess(UNIT_CYLINDER.get());

  __top_radius = top;
  __matrix.pop();
  return true;
}


/* ----------------------------------------------------------------------- */


bool LinetreePrinter::process( Cylinder * cylinder ) {
//  return symbolProcess(cylinder);
  GEOM_ASSERT(cylinder);

  __matrix.push();

  __matrix.scale(Vector3(cylinder->getRadius()*2,
                            cylinder->getRadius()*2,
                                cylinder->getHeight()));
  if(cylinder->getSolid())symbolProcess(UNIT_SOLID_CYLINDER.get());
  else symbolProcess(UNIT_CYLINDER.get());

  __matrix.pop();

  return true;
}


/* ----------------------------------------------------------------------- */


bool LinetreePrinter::process( ElevationGrid * elevationGrid ) {
  return symbolProcess(elevationGrid);
}


/* ----------------------------------------------------------------------- */


bool LinetreePrinter::process( ExtrudedHull * extrudedHull ) {
  return symbolProcess(extrudedHull);
}


/* ----------------------------------------------------------------------- */


bool LinetreePrinter::process( FaceSet * faceSet ) {
  return symbolProcess(faceSet);
}


/* ----------------------------------------------------------------------- */


bool LinetreePrinter::process( Frustum * cylinder) {
  GEOM_ASSERT(cylinder);


  __matrix.push();
  real_t top = __top_radius;
  __top_radius *= cylinder->getTaper();

  __matrix.scale(Vector3(cylinder->getRadius()*2,
                                cylinder->getRadius()*2,
                                cylinder->getHeight()));
  if(cylinder->getSolid())symbolProcess(UNIT_SOLID_CYLINDER.get());
  else symbolProcess(UNIT_CYLINDER.get());

  __top_radius = top;
  __matrix.pop();

  return true;
}


/* ----------------------------------------------------------------------- */


bool LinetreePrinter::process( Extrusion * extrusion   ) {
  return symbolProcess(extrusion);
}

/* ----------------------------------------------------------------------- */


bool LinetreePrinter::process( Group * group ) {
  GEOM_ASSERT(group);
  group->getGeometryList()->apply(*this) ;
  return true;
}


/* ----------------------------------------------------------------------- */


bool LinetreePrinter::process( NurbsCurve * nurbsCurve ) {
  GEOM_ASSERT(nurbsCurve);
  // nothing to do
  return true;
}

/* ----------------------------------------------------------------------- */


bool LinetreePrinter::process( NurbsPatch * nurbsPatch ) {
  return symbolProcess(nurbsPatch);
}

/* ----------------------------------------------------------------------- */


bool LinetreePrinter::process( Paraboloid * paraboloid ) {
  return symbolProcess(paraboloid);
}


/* ----------------------------------------------------------------------- */


bool LinetreePrinter::process( PointSet * pointSet ) {
  GEOM_ASSERT(pointSet);
  // nothing to do
  return true;
}


/* ----------------------------------------------------------------------- */


bool LinetreePrinter::process( Polyline * polyline ) {
  GEOM_ASSERT(polyline);
  // nothing to do
  return true;
}


/* ----------------------------------------------------------------------- */


bool LinetreePrinter::process( Revolution * revolution ) {
  return symbolProcess(revolution);
}


/* ----------------------------------------------------------------------- */


bool LinetreePrinter::process( Swung * swung )
{
  return symbolProcess(swung);
}


/* ----------------------------------------------------------------------- */


bool LinetreePrinter::process( QuadSet * quadSet ) {
  return symbolProcess(quadSet);
}


/* ----------------------------------------------------------------------- */


bool LinetreePrinter::process( Sphere * sphere ) {
  GEOM_ASSERT(sphere);

  __matrix.push();

  real_t r = sphere->getRadius()*2;
  Vector3 scales(r,r,r);
  __matrix.scale(scales);
  symbolProcess(UNIT_SPHERE.get());
  __matrix.pop();

  return true;
}


/* ----------------------------------------------------------------------- */


bool LinetreePrinter::process( Tapered * tapered ) {
  GEOM_ASSERT(tapered);
  real_t base_radius = __base_radius;
  real_t top_radius  = __top_radius;
  __base_radius = tapered->getBaseRadius();
  __top_radius = tapered->getTopRadius();
  bool result = tapered->getPrimitive()->apply(*this);
  __base_radius = base_radius;
  __top_radius = top_radius;
  return result;
}


/* ----------------------------------------------------------------------- */


bool LinetreePrinter::process( TriangleSet * triangleSet ) {
  GEOM_ASSERT(triangleSet);
  return symbolProcess(triangleSet);
}


/* ------------------------------------------------------------------------- */


bool LinetreePrinter::process( BezierCurve2D * bezierCurve ) {
  GEOM_ASSERT(bezierCurve);
  // nothing to do
  return true;
}


/* ----------------------------------------------------------------------- */


bool LinetreePrinter::process( Disc * disc ) {
//  return symbolProcess(disc);

  __matrix.push();

  real_t r = disc->getRadius()*2;
  Vector3 scales(r,r,1);
  __matrix.scale(scales);
  symbolProcess(UNIT_DISC.get());
  __matrix.pop();
  return true;
}


/* ----------------------------------------------------------------------- */


bool LinetreePrinter::process( NurbsCurve2D * nurbsCurve ) {
  GEOM_ASSERT(nurbsCurve);
  // nothing to do
  return true;
}

/* ----------------------------------------------------------------------- */


bool LinetreePrinter::process( PointSet2D * pointSet ) {
  GEOM_ASSERT(pointSet);
  // nothing to do
  return true;
}


/* ----------------------------------------------------------------------- */


bool LinetreePrinter::process( Polyline2D * polyline ) {
  GEOM_ASSERT(polyline);
  // nothing to do
  return true;
}


/* ----------------------------------------------------------------------- */

bool LinetreePrinter::process( ScreenProjected * scp) {
  GEOM_ASSERT(scp);
  // nothing to do
  return true;
}
