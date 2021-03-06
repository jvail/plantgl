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



/*! \file geom_profile.h
    \brief Definition of the geometry class ProfileTransformation and ProfileInterpolation.
*/


#ifndef __geom_profile_h__
#define __geom_profile_h__

/* ----------------------------------------------------------------------- */

#include <plantgl/tool/rcobject.h>

#include "../sg_config.h"
#include <plantgl/scenegraph/container/geometryarray2.h>

#ifndef GEOM_FWDEF
#include <plantgl/scenegraph/container/pointarray.h>
#include <plantgl/scenegraph/transformation/transformed.h>
#endif
/* ----------------------------------------------------------------------- */

PGL_BEGIN_NAMESPACE

#ifdef GEOM_FWDEF
class RealArray;
typedef RCPtr<RealArray> RealArrayPtr;
#endif

PGL_END_NAMESPACE

/* ----------------------------------------------------------------------- */

PGL_BEGIN_NAMESPACE

#ifdef GEOM_FWDEF
// class Curve2DArray;
// class CurveArray;
class Point2Array;
class Point3Array;
class Transformation2D;
// typedef RCPtr<Curve2DArray> Curve2DArrayPtr;
// typedef RCPtr<CurveArray> CurveArrayPtr;
typedef RCPtr<Point2Array> Point2ArrayPtr;
typedef RCPtr<Point3Array> Point3ArrayPtr;
typedef RCPtr<Transformation2D> Transformation2DPtr;
#endif

class ProfileTransformation;
typedef RCPtr<ProfileTransformation> ProfileTransformationPtr;
/* ----------------------------------------------------------------------- */

/**
    \class ProfileTransformation
    \brief Class for profile transformations functions.
*/

/* ----------------------------------------------------------------------- */

class SG_API ProfileTransformation : public RefCountObject {

    public :

  /// The Default ScalingList Field Value.
  static const Point2ArrayPtr DEFAULT_SCALE_LIST;

  /// The Default OrientationList Field Value.
  static const RealArrayPtr DEFAULT_ORIENTATION_LIST;

    /// Constructs a ProfileScaling.
    ProfileTransformation(Point2ArrayPtr _scalingList = DEFAULT_SCALE_LIST,
                          RealArrayPtr _orientationList = DEFAULT_ORIENTATION_LIST,
                          RealArrayPtr _knotList =  TOOLS(RealArrayPtr(0)));

    /// Destructor
    virtual ~ProfileTransformation( ) ;

    /// Return the minimal \e u value.
    virtual real_t getUMin() const;

    /// Return the maximal \e u value.
    virtual real_t getUMax() const;

    /// Return the tranformation at u = \e u
    virtual Transformation2DPtr operator() (real_t u) const ;

    /// Return the Scaling Factor List value.
    const Point2ArrayPtr& getScale() const;

    /// Return the Orientation Factor List value.
    const RealArrayPtr& getOrientation() const;

    /// Return the Scaling Factor List field.
    Point2ArrayPtr& getScale();

    /// Return the Orientation Factor List field.
    RealArrayPtr& getOrientation();

    /// Return the KnotList Factor List value.
    const RealArrayPtr getKnotList() const;

    /// Return the KnotList Factor List field.
    RealArrayPtr& getKnotList();

    /// return whether KnotList is set to its default value.
    const bool isKnotListToDefault() const;

    /// Returns whether \e self is valid.
    virtual bool isValid( ) const;

    virtual ProfileTransformationPtr deepcopy(DeepCopier& copier) const;

  protected:

    /// The Scaling array list.
    Point2ArrayPtr __scalingList;

    /// A pointee to the \b Orientation \b List field.
    RealArrayPtr  __orientationList;

    /// The \b knotsList field.
    RealArrayPtr __knotList;

};

/// ProfileTransformation Pointer
typedef RCPtr<ProfileTransformation> ProfileTransformationPtr;

/* ----------------------------------------------------------------------- */

// forward declaration
class ProfileInterpolation;
typedef RCPtr<ProfileInterpolation> ProfileInterpolationPtr;

/* ----------------------------------------------------------------------- */

/**
    \class ProfileInterpolation
    \brief Class for profiles Interpolation.
*/

/* ----------------------------------------------------------------------- */

class SG_API ProfileInterpolation : public RefCountObject
{
  public :

  /// The Default Degree Field Value.
  static const uint_t DEFAULT_DEGREE;

  /// The \b Slices field default value.
  static const uint_t DEFAULT_STRIDE;

  /// A structure which helps to build a Profile Interpolation when parsing.
  struct Builder
    {

    /// A pointee to the \b profile list.
    Curve2DArrayPtr* ProfileList;

    /// The \b knotsList field.
    RealArrayPtr* KnotList;

    /// The \b degree field
    uint_t* Degree;

    /// The \b stride field
    uint_t* Stride;

    /// Constructor.
    Builder( );

    /// Destructor.
    virtual ~Builder( );

    virtual ProfileInterpolationPtr build( ) const;

    virtual void destroy( );

    virtual bool isValid( ) const;

    };

  /// Default Constructor. Build object is invalid.
  ProfileInterpolation();

  /// Constructs Profiles Interpolant.
  ProfileInterpolation( Curve2DArrayPtr _profileList,
                        RealArrayPtr _knotList,
                        uint_t _degree= DEFAULT_DEGREE,
                        uint_t _stride= DEFAULT_STRIDE );

  /// Destructor
  virtual ~ProfileInterpolation( );

  /// Return the minimal \e u value.
  virtual real_t getUMin() const;

  /// Return the maximal \e u value.
  virtual real_t getUMax() const;

  /// Return the section at u = \e u
  virtual const Point2ArrayPtr& getSection2DAt(real_t u) const;
  virtual const Point3ArrayPtr& getSection3DAt(real_t u) const;

  /// Return the Profile List value.
  virtual const Curve2DArrayPtr& getProfileList( ) const;

  /// Return the Profile List field.
  virtual Curve2DArrayPtr& getProfileList( );

  /// Return the Knot List value.
  virtual const RealArrayPtr& getKnotList() const;

  /// Return the KnotList field.
  RealArrayPtr& getKnotList();

  /// Return whether KnotList is set to its default value.
  virtual bool isKnotListToDefault() const;

  /// Return whether KnotList is set to its default value.
  virtual bool isDegreeToDefault() const;

  /// Return whether KnotList is set to its default value.
  virtual bool isStrideToDefault() const;

  /// Returns \e Stride value.
  virtual const uint_t& getStride( ) const;

  /// Returns \e Stride field.
  uint_t& getStride( );

  /// Returns \e Interpolation Degree value.
  virtual const uint_t& getDegree( ) const;

  /// Returns \e Interpolation Degree field.
  uint_t& getDegree( );

  /// Interpolate 2D pts (r,z) or 3D pts (r cos,r sin,z)
  /// Returns \e Interpolation Mode value.
  virtual const bool& is2DInterpolMode() const;

  /// Returns whether \e self is valid.
  virtual bool isValid( ) const;

  // Compute the interpolation
  virtual bool interpol();

  // check if interpolation has been made
  virtual bool check_interpolation();

  virtual ProfileInterpolationPtr deepcopy(DeepCopier& copier) const;

  protected:


  /// A pointee to the \b profile list.
  Curve2DArrayPtr __profileList;

  /// The \b knotsList field.
  RealArrayPtr __knotList;

  /// The stride field
  uint_t __stride;

  /// The degree value
  uint_t __degree;

  private:

  Point2ArrayPtr __evalPt2D;
  Curve2DArrayPtr __fctList2D; // Nurbs Curve 2D [Stride]
                             // Profiles are discretize and points are interpolled.
                             // __fctList are the functions of interpollation
  Point3ArrayPtr __evalPt3D;
  CurveArrayPtr __fctList3D;

  bool __is2D;
};


/* ----------------------------------------------------------------------- */

// __geom_profile_h__
/* ----------------------------------------------------------------------- */

PGL_END_NAMESPACE

/* ----------------------------------------------------------------------- */
#endif

