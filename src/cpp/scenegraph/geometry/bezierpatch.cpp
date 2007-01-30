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

#include "bezierpatch.h"
#include <scenegraph/core/pgl_messages.h>
#include <tool/util_string.h>
#include <math/util_math.h>
#include <scenegraph/container/pointmatrix.h>

PGL_USING_NAMESPACE
TOOLS_USING_NAMESPACE
using namespace std;

/* ----------------------------------------------------------------------- */


const uint32_t BezierPatch::DEFAULT_STRIDE(30);


/* ----------------------------------------------------------------------- */

BezierPatch::Builder::Builder( ) :
  Patch::Builder(),
         CtrlPointMatrix(0),
         UDegree(0),
         UStride(0),
         VDegree(0),
         VStride(0){
}


BezierPatch::Builder::~Builder( ) {
  // nothing to do
}


SceneObjectPtr BezierPatch::Builder::build( ) const {
    if (isValid()){
#ifdef GEOM_DEBUG
        if(!UDegree)cout << "UDegree value assign to " << ((*CtrlPointMatrix)->getColsNb() - 1) << endl;
        if(!VDegree)cout << "VDegree value assign to " << ((*CtrlPointMatrix)->getRowsNb() - 1) << endl;
#endif
        return SceneObjectPtr(
            new BezierPatch(*CtrlPointMatrix,(UStride ? *UStride : DEFAULT_STRIDE),
                            (VStride ? *VStride : DEFAULT_STRIDE),(CCW ? *CCW : DEFAULT_CCW)));
    }
    return SceneObjectPtr();
}


void BezierPatch::Builder::destroy( ) {
  if (CtrlPointMatrix) delete CtrlPointMatrix;
  if (UDegree) delete UDegree;
  if (VDegree) delete VDegree;
  if (UStride) delete UStride;
  if (VStride) delete VStride;
  if (CCW) delete CCW;
}


bool BezierPatch::Builder::isValid( ) const {

  // CtrlPointList field
    if ( !CtrlPointMatrix ) {
        genMessage(WARNINGMSG(UNINITIALIZED_FIELD_ss),"Bezier Patch","CtrlPointMatrix");
        return false;
    }
    uint32_t _usize = (*CtrlPointMatrix)->getColsNb();
    uint32_t _vsize = (*CtrlPointMatrix)->getRowsNb();

    if (_usize < 2 ) {
        genMessage(WARNINGMSG(INVALID_FIELD_SIZE_sss),"Bezier Patch","CtrlPointMatrix","Rows must be greater than 1.");
        return false;

    }    if (_vsize < 2 ) {
        genMessage(WARNINGMSG(INVALID_FIELD_SIZE_sss),"Bezier Patch","CtrlPointMatrix","Columns must be greater than 1.");
        return false;
    }

    for(uint32_t i=0; i < _usize;i++)
      for(uint32_t j=0; j < _vsize;j++)
        {
// getAt( rows, columns ) => getAt(j,i) !!!
//        if(fabs((*CtrlPointMatrix)->getAt(i,j).w()) < GEOM_TOLERANCE) {
        if(fabs((*CtrlPointMatrix)->getAt(j,i).w()) < GEOM_TOLERANCE) {
            string _ith =  "( " + number(i + 1) + " , " + number(j+1) + ")";
            genMessage
                (WARNINGMSG(INVALID_FIELD_ITH_VALUE_ssss),"Bezier Patch","CtrlPointMatrix",_ith.c_str(),"Weight must be not null.");
          return false;
        }
        }

    // UDegree field
    uint32_t _udegree = (UDegree ? *UDegree : _usize - 1);
    if (_udegree < 1 ) {
        genMessage(WARNINGMSG(INVALID_FIELD_VALUE_sss),"Bezier Patch","UDegree","Must be greater than 0.");
        return false;
    };

    // VDegree field
    uint32_t _vdegree = (VDegree ? *VDegree : _vsize - 1);
    if (_vdegree < 1 ) {
        genMessage(WARNINGMSG(INVALID_FIELD_VALUE_sss),"Bezier Patch","VDegree","Must be greater than 0.");
        return false;
    };

    // UStride field
    if ( UStride && *UStride < 1 ) {
        genMessage(WARNINGMSG(INVALID_FIELD_VALUE_sss),"Bezier Patch","UStride","Must be greater than 0.");
        return false;
    };

    // VStride field
    if ( VStride && *VStride < 1 ) {
        genMessage(WARNINGMSG(INVALID_FIELD_VALUE_sss),"Bezier Patch","VStride","Must be greater than 0.");
        return false;
    };

    // Fields dependency
    if ( (_usize != (_udegree + 1)) || (_vsize !=(_vdegree +1)) ) {
        string _mess = "Must be coherent with UDegree and VDegree : wait for " + number(_udegree + 1) + "x" + number(_vdegree+1) + " Matrix";
        genMessage(WARNINGMSG(INVALID_FIELD_SIZE_sss),"Bezier Patch","CtrlPointMatrix",_mess.c_str());
        return false;
    };

    return true;
}


/* ----------------------------------------------------------------------- */

BezierPatch::BezierPatch( const Point4MatrixPtr& ctrlPoints,
                          uint32_t ustride ,
                          uint32_t vstride ,
                          bool ccw) :
    Patch(ccw),
    __ctrlPointMatrix(ctrlPoints),
    __ustride(ustride),
    __vstride(vstride){
    GEOM_ASSERT(isValid());
}

BezierPatch::BezierPatch() :
    Patch(),
    __ctrlPointMatrix(0),
    __ustride(DEFAULT_STRIDE),
    __vstride(DEFAULT_STRIDE){
}

BezierPatch::~BezierPatch( ) {
}

bool
BezierPatch::apply( Action& action ) {
  return action.process(this);
}


/* ----------------------------------------------------------------------- */


Point4MatrixPtr
BezierPatch::getCtrlPoints( ) const {
  return __ctrlPointMatrix;
}

const Point4MatrixPtr&
BezierPatch::getCtrlPointMatrix( ) const {
  return __ctrlPointMatrix;
}

Point4MatrixPtr&
BezierPatch::getCtrlPointMatrix( ) {
  return __ctrlPointMatrix;
}

const uint32_t&
BezierPatch::getUStride( ) const {
  return __ustride;
}

uint32_t&
BezierPatch::getUStride( ){
  return __ustride;
}

const uint32_t&
BezierPatch::getVStride( ) const {
  return __vstride;
}

uint32_t&
BezierPatch::getVStride( ) {
  return __vstride;
}

bool
BezierPatch::isUStrideToDefault( ) const{
  return ( __ustride == DEFAULT_STRIDE );
}

bool
BezierPatch::isVStrideToDefault( ) const{
  return ( __vstride == DEFAULT_STRIDE );
}

const uint32_t
BezierPatch::getUDegree( ) const {
  return __ctrlPointMatrix->getColsNb() - 1;
}

const uint32_t
BezierPatch::getVDegree( ) const {
  return __ctrlPointMatrix->getRowsNb() - 1;
}



/* ----------------------------------------------------------------------- */





Vector3 BezierPatch::getPointAt(real_t u, real_t v) const{
    GEOM_ASSERT( u >= 0.0 && u <= 1.0 && v>= 0.0 && v<=1.0);

    uint32_t _udeg = getUDegree();
    uint32_t _vdeg = getVDegree();
    Vector3 myPoint( 0 , 0 , 0 );

    real_t u1 = real_t(1.0) - u;
    real_t v1 = real_t(1.0) - v;


    if( _udeg <= _vdeg ){


        vector<Vector4> Q(_vdeg+1);
        for (uint32_t j =0 ; j <= _vdeg ; j ++){
          vector<Vector4> T=__ctrlPointMatrix->getRow(j);
          for (uint32_t k = 1 ; k <= _udeg ; k++)
              for (uint32_t i = 0 ; i <= ( _udeg - k ) ; i++)
                  T[i] = (T[i] * u1) + (T[i+1] * u);
          Q[j]=T[0];
        }
        for (uint32_t k = 1 ; k <= _vdeg ; k++)
            for (uint32_t j = 0 ; j <= ( _vdeg - k ) ; j++)
                Q[j] = (Q[j] * v1) + (Q[j+1] * v);

        if (fabs(Q[0].w()) < GEOM_TOLERANCE)
            return Vector3(Q[0].x(),Q[0].y(),Q[0].z());
        return Q[0].project();
    }
    else{
        vector<Vector4> Q(_udeg+1);
        for (uint32_t i =0 ; i <= _udeg ; i ++){
            vector<Vector4> T=__ctrlPointMatrix->getColumn(i);

            for (uint32_t k = 1 ; k <= _vdeg ; k++)
                for (uint32_t j = 0 ; j <= ( _vdeg - k ) ; j++)
                    T[j] = (T[j] * u1) + (T[j+1] * u);
            Q[i]=T[0];
        }
        for (uint32_t k = 1 ; k <= _udeg ; k++)
            for (uint32_t i = 0 ; i <= ( _udeg - k ) ; i++)
                Q[i] = (Q[i] * v1) + (Q[i+1] * v);

        if (fabs(Q[0].w()) < GEOM_TOLERANCE)
            return Vector3(Q[0].x(),Q[0].y(),Q[0].z());
        return Q[0].project();
    }
}


bool BezierPatch::isValid( ) const {
  Builder _builder;
  _builder.CtrlPointMatrix = const_cast<Point4MatrixPtr *>(&__ctrlPointMatrix);
  _builder.UStride = const_cast<uint32_t *>(&__ustride);
  _builder.VStride = const_cast<uint32_t *>(&__vstride);
  _builder.CCW= const_cast<bool *>(&__ccw);
  return _builder.isValid();
}

SceneObjectPtr BezierPatch::copy() const {
  BezierPatch * ptr = new BezierPatch(*this);
  if(__ctrlPointMatrix)ptr->getCtrlPointMatrix( ) = Point4MatrixPtr(new Point4Matrix(*__ctrlPointMatrix));
  return SceneObjectPtr(ptr);
}




/* ----------------------------------------------------------------------- */


