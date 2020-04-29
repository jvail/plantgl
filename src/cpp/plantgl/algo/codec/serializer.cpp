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




#include "serializer.h"

#include <plantgl/pgl_appearance.h>
#include <plantgl/pgl_geometry.h>
#include <plantgl/pgl_transformation.h>
#include <plantgl/pgl_scene.h>

#include <plantgl/scenegraph/container/indexarray.h>
#include <plantgl/scenegraph/container/pointarray.h>
#include <plantgl/scenegraph/container/geometryarray2.h>

#include <fstream>


PGL_USING_NAMESPACE
TOOLS_USING_NAMESPACE


#define GEOM_ASSERT_OBJ(obj)

#define GEOM_SERIALIZER_CHECK_APPEARANCE(app) \
  if (__appearance.get() == (Appearance *)app) return true;

#define GEOM_GLRENDERER_UPDATE_APPEARANCE(app) \
  __appearance = AppearancePtr(app);


template<class T>
bool Serializer::discretize(T *geom) {
  GEOM_ASSERT_OBJ(geom);
  bool b = false;
  DiscretizerCache::Iterator it = __discretizer_cache.find((uint_t)geom->getObjectId());
  if (it != __discretizer_cache.end()) {
    std::cout << "discretizer cached" << " " << (uint_t)geom->getObjectId() << std::endl;
    b = it->second->apply(*this);
  }
  else {
       std::cout << "discretizer not cached" << " " << (uint_t)geom->getObjectId() << std::endl;
      if (__appearance && __appearance->isTexture())
        __discretizer.computeTexCoord(true);
      else __discretizer.computeTexCoord(false);
      b = geom->apply(__discretizer);
      if (b && (b = (__discretizer.getDiscretization()))) {
        __discretizer_cache.insert((uint_t)geom->getObjectId(), __discretizer.getDiscretization());
        b = __discretizer.getDiscretization()->apply(*this);
      }
  }
  return b;
}

template<class T>
bool Serializer::tesselate(T *geom) {
  GEOM_ASSERT_OBJ(geom);
  bool b = false;
  TesselatorCache::Iterator it = __tesselator_cache.find((uint_t)geom->getObjectId());
  if (it != __tesselator_cache.end()) {
    std::cout << "tesselator cached" << " " << (uint_t)geom->getObjectId() << std::endl;
    b = it->second->apply(*this);
  }
  else {
       std::cout << "tesselator not cached" << " " << (uint_t)geom->getObjectId() << std::endl;
      b = geom->apply(__tesselator);
      if (b && (b = (__tesselator.getTriangulation()))) {
        __tesselator_cache.insert((uint_t)geom->getObjectId(), __tesselator.getTriangulation());
        b = __tesselator.getTriangulation()->apply(*this);
      }
  }
  return b;
}



#define SERIALIZER_DISCRETIZE(geom) \
  return discretize(geom); \

#define SERIALIZER_TESSELATE(geom) \
  return tesselate(geom); \

#define  PUSH_MODELMATRIX __modelmatrix.push();

#define  POP_MODELMATRIX __modelmatrix.pop();

/* ----------------------------------------------------------------------- */


Serializer::Serializer() :
    Action(),
    __discretizer(),
    __tesselator(),
    __appearance() {}

Serializer::~Serializer() {
    __discretizer_cache.clear();
    __tesselator_cache.clear();
    __triangle_soup_cache.clear();
    __data.clear();
}

bool Serializer::beginProcess()
{
  std::cout << "beginn" << std::endl;
  __discretizer_cache.clear();
  __tesselator_cache.clear();
  __triangle_soup_cache.clear();
  __data.clear();
  __offsets.clear();
  return true;
}

bool Serializer::endProcess()
{
  std::cout << "end" << std::endl;

  auto mesh_no_instances = std::unique_ptr<draco::Mesh>(new draco::Mesh());
  size_t numFaces = 0;

  TriangleSoupCache::Iterator it;
  // collect all with zero instances
  for (it = __triangle_soup_cache.begin(); it != __triangle_soup_cache.end(); it++) {
      if (it->second.instances.size() == 1) {
        numFaces += it->second.triangles->getIndexListSize();
      }
  }

  if (numFaces > 0) {
    mesh_no_instances->SetNumFaces(numFaces);
    mesh_no_instances->set_num_points(numFaces * 3);
    draco::GeometryAttribute position_attr;
    position_attr.Init(draco::GeometryAttribute::POSITION, nullptr, 3, draco::DT_FLOAT32, false,
              DataTypeLength(draco::DT_FLOAT32) * 3, 0);
    draco::GeometryAttribute color_attr;
    color_attr.Init(draco::GeometryAttribute::COLOR, nullptr, 3, draco::DT_UINT8, false,
          DataTypeLength(draco::DT_UINT8) * 3, 0);

    int32_t pos_attr_id = mesh_no_instances->AddAttribute(position_attr, true, mesh_no_instances->num_points());
    int32_t col_attr_id = mesh_no_instances->AddAttribute(color_attr, true, mesh_no_instances->num_points());
    mesh_no_instances->SetAttributeElementType(pos_attr_id, draco::MeshAttributeElementType::MESH_CORNER_ATTRIBUTE);
    mesh_no_instances->SetAttributeElementType(col_attr_id, draco::MeshAttributeElementType::MESH_FACE_ATTRIBUTE);
    size_t index = 0;
    draco::PointAttribute *const pos_attr = mesh_no_instances->attribute(pos_attr_id);
    draco::PointAttribute *const col_attr = mesh_no_instances->attribute(col_attr_id);

    for (it = __triangle_soup_cache.begin(); it != __triangle_soup_cache.end(); it++) {
        if (it->second.instances.size() == 1) {
          auto triangles = it->second.triangles;
          auto points = triangles->getPointList();
          for (auto it2 = triangles->getIndexList()->begin(); it2 != triangles->getIndexList()->end(); it2++)
          {
              std::cout << "index " << index << std::endl;
              auto it3 = it2->begin();
              Vector3 p1 = points->getAt(*(it3));
              Vector3 p2 = points->getAt(*(++it3));
              Vector3 p3 = points->getAt(*(++it3));

              const size_t start_index = 3 * index;

              pos_attr->SetAttributeValue(draco::AttributeValueIndex(start_index),
                  draco::Vector3f(p1.x(), p1.y(), p1.z()).data());
              pos_attr->SetAttributeValue(draco::AttributeValueIndex(start_index + 1),
                  draco::Vector3f(p2.x(), p2.y(), p2.z()).data());
              pos_attr->SetAttributeValue(draco::AttributeValueIndex(start_index + 2),
                  draco::Vector3f(p3.x(), p3.y(), p3.z()).data());
              mesh_no_instances->SetFace(draco::FaceIndex(index),
                  {{draco::PointIndex(start_index), draco::PointIndex(start_index + 1), draco::PointIndex(start_index + 2)}});

              uchar_t colors[3] = { (uchar_t)it->second.red, (uchar_t)it->second.green, (uchar_t)it->second.blue };
              col_attr->SetAttributeValue(draco::AttributeValueIndex(start_index), &colors);
              col_attr->SetAttributeValue(draco::AttributeValueIndex(start_index + 1), &colors);
              col_attr->SetAttributeValue(draco::AttributeValueIndex(start_index + 2), &colors);

              index++;
        }
      }
    }

    mesh_no_instances->DeduplicateAttributeValues();
    mesh_no_instances->DeduplicatePointIds();

    draco::EncoderBuffer buffer;
    draco::ExpertEncoder encoder(*mesh_no_instances.get());
    encoder.SetSpeedOptions(10, 10);
    auto status = encoder.EncodeToBuffer(&buffer);
    if (!status.ok()) {
      return false;
    }

    auto bufferData = *buffer.buffer();
    __data.insert(__data.end(), bufferData.begin(), bufferData.end());
    __offsets.push_back(0);
  }

  // collect all with instances
  for (it = __triangle_soup_cache.begin(); it != __triangle_soup_cache.end(); it++) {
      if (it->second.instances.size() > 1) {
          std::cout << "getIndexListSize " << it->second.triangles->getIndexListSize() << std::endl;
        auto triangles = it->second.triangles;
        auto mesh = std::unique_ptr<draco::Mesh>(new draco::Mesh());
        mesh->SetNumFaces(triangles->getIndexListSize());
        mesh->set_num_points(triangles->getIndexListSize() * 3);
        draco::GeometryAttribute position_attr;
        position_attr.Init(draco::GeometryAttribute::POSITION, nullptr, 3, draco::DT_FLOAT32, false,
            DataTypeLength(draco::DT_FLOAT32) * 3, 0);
        draco::GeometryAttribute color_attr;
        color_attr.Init(draco::GeometryAttribute::COLOR, nullptr, 3, draco::DT_UINT8, false,
          DataTypeLength(draco::DT_UINT8) * 3, 0);

        int32_t pos_attr_id = mesh->AddAttribute(position_attr, true, mesh->num_points());
        int32_t col_attr_id = mesh->AddAttribute(color_attr, true, mesh->num_points());

        mesh->SetAttributeElementType(pos_attr_id, draco::MeshAttributeElementType::MESH_CORNER_ATTRIBUTE);
        mesh->SetAttributeElementType(col_attr_id, draco::MeshAttributeElementType::MESH_FACE_ATTRIBUTE);

        auto points = triangles->getPointList();
        size_t index = 0;
        for (auto it2 = triangles->getIndexList()->begin(); it2 != triangles->getIndexList()->end(); it2++)
        {
            std::cout << "index " << index << std::endl;
            auto it3 = it2->begin();
            Vector3 p1 = points->getAt(*(it3));
            Vector3 p2 = points->getAt(*(++it3));
            Vector3 p3 = points->getAt(*(++it3));

            const size_t start_index = 3 * index;
            draco::PointAttribute *const pos_attr = mesh->attribute(pos_attr_id);
            draco::PointAttribute *const col_attr = mesh->attribute(col_attr_id);

            pos_attr->SetAttributeValue(draco::AttributeValueIndex(start_index),
                draco::Vector3f(p1.x(), p1.y(), p1.z()).data());
            pos_attr->SetAttributeValue(draco::AttributeValueIndex(start_index + 1),
                draco::Vector3f(p2.x(), p2.y(), p2.z()).data());
            pos_attr->SetAttributeValue(draco::AttributeValueIndex(start_index + 2),
                draco::Vector3f(p3.x(), p3.y(), p3.z()).data());
            mesh->SetFace(draco::FaceIndex(index),
                {{draco::PointIndex(start_index), draco::PointIndex(start_index + 1), draco::PointIndex(start_index + 2)}});

            uchar_t colors[3] = { (uchar_t)it->second.red, (uchar_t)it->second.green, (uchar_t)it->second.blue };
            col_attr->SetAttributeValue(draco::AttributeValueIndex(start_index), &colors);
            col_attr->SetAttributeValue(draco::AttributeValueIndex(start_index + 1), &colors);
            col_attr->SetAttributeValue(draco::AttributeValueIndex(start_index + 2), &colors);

            index++;
        }

        std::cout << "num_attributes " << mesh->num_attributes() << std::endl;
        std::cout << "num_faces " << mesh->num_faces() << std::endl;
        std::cout << "num_points " << mesh->num_points() << std::endl;

        std::unique_ptr<draco::AttributeMetadata> instance_attr_metadata =
            std::unique_ptr<draco::AttributeMetadata>(new draco::AttributeMetadata());
        instance_attr_metadata->set_att_unique_id(1000);

        std::vector<real_t> instances;
        for (size_t i = 0; i < it->second.instances.size(); ++i) {
            Matrix4 matrix = it->second.instances.at(i);
            Matrix4::iterator it2;
            for (it2 = matrix.begin(); it2 != matrix.end(); ++it2) {
                instances.push_back(*it2);
            }
        }

        instance_attr_metadata->AddEntryDoubleArray("instances", instances);
        instance_attr_metadata->AddEntryString("test", "hallo");
        mesh->AddAttributeMetadata(pos_attr_id, std::move(instance_attr_metadata));

        mesh->DeduplicateAttributeValues();
        mesh->DeduplicatePointIds();

        draco::EncoderBuffer buffer;
        draco::ExpertEncoder encoder(*mesh.get());
        encoder.SetSpeedOptions(10, 10);
        auto status = encoder.EncodeToBuffer(&buffer);
        if (!status.ok()) {
          return false;
        }

        auto bufferData = *buffer.buffer();
        __data.insert(__data.end(), bufferData.begin(), bufferData.end());
        __offsets.push_back(__data.size() - bufferData.size());

      }
      std::cout << it->first << " " << it->second.instances.size() << std::endl;

  }

  // auto myfile = std::fstream("mesh_cpp.drc", std::ios::out | std::ios::binary);
  // myfile.write(__data.data(), __data.size());
  // myfile.close();

  return true;
}

bool Serializer::process(Shape *geomshape) {
  GEOM_ASSERT_OBJ(geomshape);
  processAppereance(geomshape);
  return processGeometry(geomshape);
}

bool Serializer::processAppereance(Shape *geomshape) {
  GEOM_ASSERT(geomshape);
  if (geomshape->appearance) {
    return geomshape->appearance->apply(*this);
  } else return Material::DEFAULT_MATERIAL->apply(*this);
}

bool Serializer::processGeometry(Shape *geomshape)
{
  GEOM_ASSERT(geomshape);
  return geomshape->geometry->apply(*this);
}

bool Serializer::process(Inline *geomInline) {
  GEOM_ASSERT_OBJ(geomInline);
  if (geomInline->getScene()) {
    if (!geomInline->isTranslationToDefault() || !geomInline->isScaleToDefault()) {
        PUSH_MODELMATRIX;
        const Vector3 _trans = geomInline->getTranslation();
        __modelmatrix.translate(_trans);
        const Vector3 _scale = geomInline->getScale();
        __modelmatrix.scale(_scale);
    }

    bool _result = true;
    for (Scene::iterator _i = geomInline->getScene()->begin();
         _i != geomInline->getScene()->end();
         _i++) {
      if (!(*_i)->applyAppearanceFirst(*this)) _result = false;
    };

    if (!geomInline->isTranslationToDefault() || !geomInline->isScaleToDefault()) {
      POP_MODELMATRIX;
    }

    return _result;
  }
  else
    return false;
}

bool Serializer::process(AmapSymbol *amapSymbol) {
  GEOM_ASSERT_OBJ(amapSymbol);

  return true;
}

bool Serializer::process(AsymmetricHull *asymmetricHull) {
  SERIALIZER_DISCRETIZE(asymmetricHull);
}

bool Serializer::process(AxisRotated *axisRotated) {
  GEOM_ASSERT_OBJ(axisRotated);


  const Vector3 &_axis = axisRotated->getAxis();
  const real_t &_angle = axisRotated->getAngle();

  PUSH_MODELMATRIX;

  __modelmatrix.axisRotation(_axis, _angle);
  axisRotated->getGeometry()->apply(*this);

  POP_MODELMATRIX;

  return true;
}

bool Serializer::process(BezierCurve *bezierCurve) {
  SERIALIZER_DISCRETIZE(bezierCurve);
}

bool Serializer::process(BezierPatch *bezierPatch) {
  SERIALIZER_DISCRETIZE(bezierPatch);
}

bool Serializer::process(Box *box) {
  SERIALIZER_DISCRETIZE(box);
}

bool Serializer::process(Cone *cone) {
  SERIALIZER_DISCRETIZE(cone);
}

bool Serializer::process(Cylinder *cylinder) {
  SERIALIZER_DISCRETIZE(cylinder);
}

bool Serializer::process(ElevationGrid *elevationGrid) {
  SERIALIZER_DISCRETIZE(elevationGrid);
}

bool Serializer::process(EulerRotated *eulerRotated) {
  GEOM_ASSERT_OBJ(eulerRotated);

  PUSH_MODELMATRIX;

  __modelmatrix.eulerRotationZYX(eulerRotated->getAzimuth(),
                            eulerRotated->getElevation(),
                            eulerRotated->getRoll());

  eulerRotated->getGeometry()->apply(*this);
  POP_MODELMATRIX;

  return true;
}

bool Serializer::process(ExtrudedHull *extrudedHull) {
  SERIALIZER_DISCRETIZE(extrudedHull);
}

bool Serializer::process(FaceSet *faceSet) {
  GEOM_ASSERT_OBJ(faceSet);

  std::cout << "faceSet" << " " << (uint_t)faceSet->getObjectId()  << std::endl;
  SERIALIZER_TESSELATE(faceSet);

}

bool Serializer::process(Frustum *frustum) {
  SERIALIZER_DISCRETIZE(frustum);
}

bool Serializer::process(Extrusion *extrusion) {
  SERIALIZER_DISCRETIZE(extrusion);
}

bool Serializer::process(Group *group) {
  GEOM_ASSERT_OBJ(group);

  group->getGeometryList()->apply(*this);

  return true;
}

bool Serializer::process(IFS *ifs) {
  GEOM_ASSERT_OBJ(ifs);

  std::cout << "ifs" << std::endl;

  ITPtr transfos;
  transfos = dynamic_pointer_cast<IT>(ifs->getTransformation());
  GEOM_ASSERT(transfos);
  const Matrix4ArrayPtr &matrixList = transfos->getAllTransfo();
  GEOM_ASSERT(matrixList);

  Matrix4Array::const_iterator matrix = matrixList->begin();
  while (matrix != matrixList->end()) {
    PUSH_MODELMATRIX;

    __modelmatrix.transform(*matrix);
    ifs->getGeometry()->apply(*this);

    POP_MODELMATRIX;
    matrix++;
  }

  return true;
}

bool Serializer::process(Material *material) {
  GEOM_ASSERT_OBJ(material);

  __red = int(material->getAmbient().getRed());
  __green = int(material->getAmbient().getGreen());
  __blue = int(material->getAmbient().getBlue());

  return true;
}

bool Serializer::process(ImageTexture *texture) {
  GEOM_ASSERT_OBJ(texture);

  GEOM_SERIALIZER_CHECK_APPEARANCE(texture);

  return true;
}

bool Serializer::process(Texture2D *texture) {
  GEOM_ASSERT_OBJ(texture);
  GEOM_SERIALIZER_CHECK_APPEARANCE(texture);

  const Color4 color = texture->getBaseColor();
  __red = int(color.getRed());
  __green = int(color.getGreen());
  __blue = int(color.getBlue());
/*
  GLfloat _rgba[4];

  _rgba[0] = (GLfloat) _color.getRedClamped();
  _rgba[1] = (GLfloat) _color.getGreenClamped();
  _rgba[2] = (GLfloat) _color.getBlueClamped();
  _rgba[3] = 1.0f - _color.getAlphaClamped();

  /// We set the current color in the case of disabling the lighting
  glColor4fv(_rgba);
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, _rgba);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, _rgba);

  if (texture->getImage()) {
    // load image
    texture->getImage()->apply(*this);

    // apply texture transformation
    if (texture->getTransformation())
      texture->getTransformation()->apply(*this);
    else {
      // Set Texture transformation to Id if no transformation is specified
      glMatrixMode(GL_TEXTURE);
      glLoadIdentity();
      glMatrixMode(GL_MODELVIEW);
    }
  }
  */

  GEOM_GLRENDERER_UPDATE_APPEARANCE(texture);

  return true;
}

bool Serializer::process(Texture2DTransformation *texturetransfo) {
  GEOM_ASSERT_OBJ(texturetransfo);

  return true;
}

bool Serializer::process(MonoSpectral *monoSpectral) {
  GEOM_ASSERT_OBJ(monoSpectral);

  GEOM_SERIALIZER_CHECK_APPEARANCE(monoSpectral);

  GEOM_GLRENDERER_UPDATE_APPEARANCE(monoSpectral);

  return true;
}

bool Serializer::process(MultiSpectral *multiSpectral) {
  GEOM_ASSERT_OBJ(multiSpectral);

  GEOM_SERIALIZER_CHECK_APPEARANCE(multiSpectral);
  GEOM_GLRENDERER_UPDATE_APPEARANCE(multiSpectral);

  return true;
}

bool Serializer::process(NurbsCurve *nurbsCurve) {
  SERIALIZER_DISCRETIZE(nurbsCurve);
}

bool Serializer::process(NurbsPatch *nurbsPatch) {
  SERIALIZER_DISCRETIZE(nurbsPatch);
}

bool Serializer::process(Oriented *oriented) {
  GEOM_ASSERT_OBJ(oriented);

  std::cout << "oriented" << std::endl;

  PUSH_MODELMATRIX;

  Matrix4TransformationPtr _basis;
  _basis = dynamic_pointer_cast<Matrix4Transformation>(oriented->getTransformation());
  GEOM_ASSERT(_basis);

  __modelmatrix.transform(_basis->getMatrix());

  oriented->getGeometry()->apply(*this);

  POP_MODELMATRIX;

  return true;
}

bool Serializer::process(Paraboloid *paraboloid) {
  SERIALIZER_DISCRETIZE(paraboloid);
}

bool Serializer::process(PointSet *pointSet) {
  GEOM_ASSERT_OBJ(pointSet);

  return true;
}

bool Serializer::process(PGL(Polyline) *polyline) {
  GEOM_ASSERT_OBJ(polyline);

  return true;
}

bool Serializer::process(QuadSet *quadSet) {
  GEOM_ASSERT_OBJ(quadSet);
  std::cout << "quadSet" << " " << (uint_t)quadSet->getObjectId() << std::endl;

  SERIALIZER_TESSELATE(quadSet);

}

bool Serializer::process(Revolution *revolution) {
  SERIALIZER_DISCRETIZE(revolution);
}

bool Serializer::process(Swung *swung) {
  SERIALIZER_DISCRETIZE(swung);
}

bool Serializer::process(Scaled *scaled) {
  GEOM_ASSERT_OBJ(scaled);

  std::cout << "scaled" << std::endl;

  PUSH_MODELMATRIX;

  __modelmatrix.scale(scaled->getScale());

  scaled->getGeometry()->apply(*this);

  POP_MODELMATRIX;

  return true;
}

bool Serializer::process(ScreenProjected *scp) {
  GEOM_ASSERT_OBJ(scp);
  return true;
}

bool Serializer::process(Sphere *sphere) {
  SERIALIZER_DISCRETIZE(sphere);
}

bool Serializer::process(Tapered *tapered) {
  GEOM_ASSERT_OBJ(tapered);

  std::cout << "tapered" << std::endl;

  PrimitivePtr _primitive = tapered->getPrimitive();
  if (_primitive->apply(__discretizer)) {
    ExplicitModelPtr _explicit = __discretizer.getDiscretization();

    Transformation3DPtr _taper(tapered->getTransformation());
    ExplicitModelPtr _tExplicit = _explicit->transform(_taper);
    _tExplicit->apply(*this);

    return true;
  }

  return false;
}

bool Serializer::process(Translated *translated) {
  GEOM_ASSERT_OBJ(translated);

  std::cout << "translated" << " " << (uint_t)translated->getObjectId() << std::endl;

  PUSH_MODELMATRIX;
  __modelmatrix.translate(translated->getTranslation());
  translated->getGeometry()->apply(*this);
  POP_MODELMATRIX;

  return true;
}

bool Serializer::process(TriangleSet *triangleSet)
{
    std::cout << "triangleSet" << " " << (uint_t)triangleSet->getObjectId() << std::endl;
    auto matrix = __modelmatrix.getMatrix();
    Matrix4::iterator i;
    for (i = matrix.begin(); i != matrix.end(); ++i) {
        std::cout << *i << " ";
    }
    uint_t id = (uint_t)triangleSet->getObjectId();
    TriangleSoupCache::Iterator it = __triangle_soup_cache.find(id);
    if (it != __triangle_soup_cache.end()) {
        it->second.instances.push_back(__modelmatrix.getMatrix());
    } else {
        auto soup = TriangleSoup(id, triangleSet, __modelmatrix.getMatrix());
        soup.red = __red;
        soup.green = __green;
        soup.blue = __blue;
        std::cout << "colors " << __red << " "  << __green << " " << __blue << std::endl;
        __triangle_soup_cache.insert(id, soup);
    }

    return true;
}

bool Serializer::process(BezierCurve2D *bezierCurve) {
  SERIALIZER_DISCRETIZE(bezierCurve);
}

bool Serializer::process(Disc *disc) {
  SERIALIZER_DISCRETIZE(disc);
}

bool Serializer::process(NurbsCurve2D *nurbsCurve) {
  SERIALIZER_DISCRETIZE(nurbsCurve);
}

bool Serializer::process(PointSet2D *pointSet) {
  SERIALIZER_DISCRETIZE(pointSet);
}

bool Serializer::process(Polyline2D *polyline) {
  SERIALIZER_DISCRETIZE(polyline);
}

bool Serializer::process(Text *text) {
  GEOM_ASSERT_OBJ(text);
  return true;
}

bool Serializer::process(Font *font) {
  GEOM_ASSERT_OBJ(font);
  return true;
}
