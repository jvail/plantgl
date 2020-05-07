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

// #include <fstream>

PGL_USING_NAMESPACE
TOOLS_USING_NAMESPACE


#define GEOM_ASSERT_OBJ(obj)

#define GEOM_SERIALIZER_CHECK_APPEARANCE(app) \
  if (__appearance.get() == (Appearance *)app) return true;

#define GEOM_SERIALIZER_UPDATE_APPEARANCE(app) \
  __appearance = AppearancePtr(app);


template<class T>
bool Serializer::discretize(T *geom) {
  GEOM_ASSERT_OBJ(geom);
  bool b = false;

  DiscretizerCache::Iterator it = __discretizer_cache.find((uint_t)geom->getObjectId());
  if (it != __discretizer_cache.end()) {
    b = it->second->apply(*this);
  } else {
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
  if (__appearance && __appearance->isTexture())
    __tesselator.computeTexCoord(true);
  else __tesselator.computeTexCoord(false);
  b = geom->apply(__tesselator);
  if (b && (b = (__tesselator.getDiscretization()))) {
    b = __tesselator.getDiscretization()->apply(*this);
  }
  return b;
}

#define SERIALIZER_DISCRETIZE(geom) \
  return discretize(geom); \

#define SERIALIZER_TESSELATE(geom) \
  return tesselate(geom) \

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
    __triangle_soup_cache.clear();
    __data.clear();
}

bool Serializer::addMesh(const std::vector<TriangleSetInstances> &soups)
{
  auto mesh = std::unique_ptr<draco::Mesh>(new draco::Mesh());
  size_t numFaces = 0;

  for (int i = 0; i < soups.size(); i++) {
      numFaces += soups.at(i).triangles->getIndexListSize();
  }

  if (numFaces > 0) {
    mesh->SetNumFaces(numFaces);
    mesh->set_num_points(numFaces * 3);
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

    size_t index = 0;
    draco::PointAttribute *const pos_attr = mesh->attribute(pos_attr_id);
    draco::PointAttribute *const col_attr = mesh->attribute(col_attr_id);

    for (int i = 0; i < soups.size(); i++) {
      auto soup = soups.at(i);
      auto instance = soup.instances.at(0);
      TriangleSetPtr triangles;
      if (instance.matrix != Matrix4()) {
        Transformation3DPtr transform(new Transform4(instance.matrix));
        triangles = dynamic_pointer_cast<TriangleSet>(soup.triangles->transform(transform)).get();
      } else {
        triangles = soup.triangles;
      }
      auto points = triangles->getPointList();
      for (auto it2 = triangles->getIndexList()->begin(); it2 != triangles->getIndexList()->end(); it2++)
      {
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
          mesh->SetFace(draco::FaceIndex(index),
              {{draco::PointIndex(start_index), draco::PointIndex(start_index + 1), draco::PointIndex(start_index + 2)}});

          uchar_t colors[3] = { (uchar_t)instance.red, (uchar_t)instance.green, (uchar_t)instance.blue };
          col_attr->SetAttributeValue(draco::AttributeValueIndex(start_index), &colors);
          col_attr->SetAttributeValue(draco::AttributeValueIndex(start_index + 1), &colors);
          col_attr->SetAttributeValue(draco::AttributeValueIndex(start_index + 2), &colors);

          index++;
      }
    }

    auto metadata = std::unique_ptr<draco::GeometryMetadata>(new draco::GeometryMetadata());
    metadata->AddEntryInt("id", 0);
    metadata->AddEntryString("type", "mesh");
    mesh->AddMetadata(std::move(metadata));

    mesh->DeduplicateAttributeValues();
    mesh->DeduplicatePointIds();

    draco::EncoderBuffer buffer;
    draco::ExpertEncoder encoder(*mesh.get());
    encoder.SetAttributeQuantization(draco::GeometryAttribute::POSITION, 12);
    encoder.SetAttributeQuantization(draco::GeometryAttribute::TEX_COORD, 10);
    encoder.SetAttributeQuantization(draco::GeometryAttribute::NORMAL, 7);
    encoder.SetAttributeQuantization(draco::GeometryAttribute::GENERIC, 8);
    encoder.SetSpeedOptions(__speed, __speed);
    auto status = encoder.EncodeToBuffer(&buffer);
    if (!status.ok()) {
      return false;
    }

    auto bufferData = *buffer.buffer();
    __data.insert(__data.end(), bufferData.begin(), bufferData.end());
    __offsets.push_back(__data.size() - bufferData.size());
  }

  return true;
}

bool Serializer::addInstanceMesh(const TriangleSetInstances &soup)
{

  auto mesh = std::unique_ptr<draco::Mesh>(new draco::Mesh());
  size_t num_faces = soup.triangles->getIndexListSize();

  mesh->SetNumFaces(num_faces);
  mesh->set_num_points(num_faces * 3);
  draco::GeometryAttribute position_attr;
  position_attr.Init(draco::GeometryAttribute::POSITION, nullptr, 3, draco::DT_FLOAT32, false,
            DataTypeLength(draco::DT_FLOAT32) * 3, 0);

  int pos_attr_id = mesh->AddAttribute(position_attr, true, mesh->num_points());
  // colors are added per instance in the instance pointcloud
  mesh->SetAttributeElementType(pos_attr_id, draco::MeshAttributeElementType::MESH_CORNER_ATTRIBUTE);

  size_t index = 0;
  draco::PointAttribute *const pos_attr = mesh->attribute(pos_attr_id);

  auto triangles = soup.triangles;
  auto instance = soup.instances[0];
  auto points = triangles->getPointList();

  for (auto it2 = triangles->getIndexList()->begin(); it2 != triangles->getIndexList()->end(); it2++)
  {
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

    mesh->SetFace(draco::FaceIndex(index),
        {{draco::PointIndex(start_index), draco::PointIndex(start_index + 1), draco::PointIndex(start_index + 2)}});

    index++;
  }

  auto metadata = std::unique_ptr<draco::GeometryMetadata>(new draco::GeometryMetadata());
  metadata->AddEntryInt("id", (int)soup.id);
  metadata->AddEntryString("type", "instanced_mesh");

  // instances are a draco pointcloud added to the mesh's metadata. Then we get at least some compression (~1/3) of matrices.
  // The compression was tested against zfp and bzip2. zfp is not suitable (bad compression) for this type of data and bzip2
  // has a better compression if we have a lot of 1 and 0 in the matrices.
  if (!addInstances(metadata.get(), soup)) return false;

  mesh->AddMetadata(std::move(metadata));

  mesh->DeduplicateAttributeValues();
  mesh->DeduplicatePointIds();

  draco::EncoderBuffer buffer;
  draco::ExpertEncoder encoder(*mesh.get());
  encoder.SetAttributeQuantization(draco::GeometryAttribute::POSITION, 12);
  encoder.SetAttributeQuantization(draco::GeometryAttribute::TEX_COORD, 10);
  encoder.SetAttributeQuantization(draco::GeometryAttribute::NORMAL, 7);
  encoder.SetAttributeQuantization(draco::GeometryAttribute::GENERIC, 8);
  encoder.SetSpeedOptions(__speed, __speed);
  auto status = encoder.EncodeToBuffer(&buffer);
  if (!status.ok()) {
    return false;
  }

  auto bufferData = *buffer.buffer();
  __data.insert(__data.end(), bufferData.begin(), bufferData.end());
  __offsets.push_back(__data.size() - bufferData.size());

  return true;
}

bool Serializer::addInstances(draco::GeometryMetadata *metadata, const TriangleSetInstances & soup)
{
  auto cloud = std::unique_ptr<draco::PointCloud>(new draco::PointCloud());
  auto instances = soup.instances;
  cloud->set_num_points(instances.size());
  draco::GeometryAttribute position_attr;
  position_attr.Init(draco::GeometryAttribute::POSITION, nullptr, 16, draco::DT_FLOAT32, false,
            DataTypeLength(draco::DT_FLOAT32) * 16, 0);
  draco::GeometryAttribute color_id_attr;
  color_id_attr.Init(draco::GeometryAttribute::COLOR, nullptr, 3, draco::DT_UINT8, false,
    DataTypeLength(draco::DT_UINT8) * 3, 0);

  int pos_attr_id = cloud->AddAttribute(position_attr, true, instances.size());
  int col_attr_id = cloud->AddAttribute(color_id_attr, true, instances.size());

  uint32_t index = 0;
  draco::PointAttribute *const pos_attr = cloud->attribute(pos_attr_id);
  draco::PointAttribute *const col_attr = cloud->attribute(col_attr_id);

  for (auto it = instances.begin(); it != instances.end(); it++)
  {
    std::vector<float_t> data;
    auto instance = *it;
    for (uchar_t i=0; i<4; i++) {
      auto row = instance.matrix.getRow(i);
      for (uchar_t j=0; j<4; j++) {
        data.push_back(row.getAt(j));
      }
    }
    pos_attr->SetAttributeValue(draco::AttributeValueIndex(index), data.data());

    uchar_t colors[3] = { (uchar_t)instance.red, (uchar_t)instance.green, (uchar_t)instance.blue };
    col_attr->SetAttributeValue(draco::AttributeValueIndex(index), &colors);
    index++;
  }

  auto cloud_metadata = std::unique_ptr<draco::GeometryMetadata>(new draco::GeometryMetadata());
  cloud_metadata->AddEntryInt("id", (int)soup.id);
  cloud_metadata->AddEntryString("type", "instances");
  cloud->AddMetadata(std::move(cloud_metadata));

  cloud->DeduplicateAttributeValues();
  cloud->DeduplicatePointIds();

  draco::EncoderBuffer buffer;
  draco::ExpertEncoder encoder(*cloud.get());
  encoder.SetAttributeQuantization(draco::GeometryAttribute::POSITION, 10);
  encoder.SetSpeedOptions(__speed, __speed);
  auto status = encoder.EncodeToBuffer(&buffer);
  if (!status.ok()) {
    return false;
  }

  auto bufferData = buffer.buffer();
  std::vector<uint8_t> blob(bufferData->begin(), bufferData->end());
  metadata->AddEntryBinary("instances", blob);

  return true;

}

bool Serializer::beginProcess()
{
  __discretizer_cache.clear();
  __triangle_soup_cache.clear();
  __data.clear();
  __offsets.clear();
  return true;
}

bool Serializer::endProcess()
{

  if (__single_mesh) {
    draco::TriangleSoupMeshBuilder mesh_builder;
    uint32_t index = 0;

    mesh_builder.Start(__triangleSoup.numFaces);

    int pos_att_id = mesh_builder.AddAttribute(draco::GeometryAttribute::POSITION, 3, draco::DT_FLOAT32);
    int col_att_id = mesh_builder.AddAttribute(draco::GeometryAttribute::COLOR, 3, draco::DT_UINT8);

    for (int i = 0; i < __triangleSoup.meshs.size(); i++) {
      auto mesh = &__triangleSoup.meshs.at(i);
      auto color = __triangleSoup.colors.at(i);
      auto points = mesh->getPointList();

      for (Index3Array::const_iterator it = mesh->getIndexList()->begin(); it != mesh->getIndexList()->end(); it++)
      {
        auto it2 = it->begin();
        Vector3 p1 = points->getAt(*(it2));
        Vector3 p2 = points->getAt(*(++it2));
        Vector3 p3 = points->getAt(*(++it2));

        mesh_builder.SetAttributeValuesForFace(
          pos_att_id, draco::FaceIndex(index),
          draco::Vector3f(p1.x(), p1.y(), p1.z()).data(),
          draco::Vector3f(p2.x(), p2.y(), p2.z()).data(),
          draco::Vector3f(p3.x(), p3.y(), p3.z()).data()
        );

        mesh_builder.SetPerFaceAttributeValueForFace(
          col_att_id, draco::FaceIndex(index), color.data()
        );

        index++;
      }
    }


    auto mesh = mesh_builder.Finalize();
    if (!mesh) {
      return false;
    }
    auto metadata = std::unique_ptr<draco::GeometryMetadata>(new draco::GeometryMetadata());
    metadata->AddEntryInt("id", 0);
    metadata->AddEntryString("type", "mesh");
    mesh->AddMetadata(std::move(metadata));
    draco::ExpertEncoder encoder(*mesh.get());
    encoder.SetAttributeQuantization(draco::GeometryAttribute::POSITION, 12);
    encoder.SetAttributeQuantization(draco::GeometryAttribute::TEX_COORD, 10);
    encoder.SetAttributeQuantization(draco::GeometryAttribute::NORMAL, 7);
    encoder.SetAttributeQuantization(draco::GeometryAttribute::GENERIC, 8);
    encoder.SetSpeedOptions(__speed, __speed);
    draco::EncoderBuffer buffer;
    auto status = encoder.EncodeToBuffer(&buffer);
    if (!status.ok()) {
      return false;
    }
    auto bufferData = *buffer.buffer();
    __data.insert(__data.end(), bufferData.begin(), bufferData.end());
    __offsets.push_back(__data.size() - bufferData.size());

  } else {

    std::vector<TriangleSetInstances> soups;
    for (auto it = __triangle_soup_cache.begin(); it != __triangle_soup_cache.end(); it++) {
      if (it->second.instances.size() > 1) {
        bool mesh_ok = addInstanceMesh(it->second);
        if (!mesh_ok) return false;
      } else {
        soups.push_back(it->second);
      }
    }
    if (soups.size() > 0) {
      if (!addMesh(soups)) return false;
    }
    soups.clear();
  }

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
  if (__single_mesh) {
    SERIALIZER_TESSELATE(geomshape);
  } else {
    return geomshape->geometry->apply(*this);
  }
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
  SERIALIZER_TESSELATE(amapSymbol);
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
  return group->getGeometryList()->apply(*this);
}

bool Serializer::process(IFS *ifs) {
  GEOM_ASSERT_OBJ(ifs);

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

  GEOM_SERIALIZER_UPDATE_APPEARANCE(texture);

  return true;
}

bool Serializer::process(Texture2DTransformation *texturetransfo) {
  GEOM_ASSERT_OBJ(texturetransfo);

  return true;
}

bool Serializer::process(MonoSpectral *monoSpectral) {
  GEOM_ASSERT_OBJ(monoSpectral);

  GEOM_SERIALIZER_CHECK_APPEARANCE(monoSpectral);
  GEOM_SERIALIZER_UPDATE_APPEARANCE(monoSpectral);

  return true;
}

bool Serializer::process(MultiSpectral *multiSpectral) {
  GEOM_ASSERT_OBJ(multiSpectral);

  GEOM_SERIALIZER_CHECK_APPEARANCE(multiSpectral);
  GEOM_SERIALIZER_UPDATE_APPEARANCE(multiSpectral);

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
  // TODO ?
  return false;
}

bool Serializer::process(PGL(Polyline) *polyline) {
  GEOM_ASSERT_OBJ(polyline);
  // TODO ?
  return true;
}

bool Serializer::process(QuadSet *quadSet) {
  GEOM_ASSERT_OBJ(quadSet);
  SERIALIZER_TESSELATE(quadSet);
}

bool Serializer::process(Revolution *revolution) {
  GEOM_ASSERT_OBJ(revolution);
  SERIALIZER_DISCRETIZE(revolution);
}

bool Serializer::process(Swung *swung) {
  SERIALIZER_DISCRETIZE(swung);
}

bool Serializer::process(Scaled *scaled) {
  GEOM_ASSERT_OBJ(scaled);

  PUSH_MODELMATRIX;

  __modelmatrix.scale(scaled->getScale());
  scaled->getGeometry()->apply(*this);

  POP_MODELMATRIX;

  return true;
}

bool Serializer::process(ScreenProjected *scp) {
  GEOM_ASSERT_OBJ(scp);
  return false;
}

bool Serializer::process(Sphere *sphere) {
  SERIALIZER_DISCRETIZE(sphere);
}

bool Serializer::process(Tapered *tapered) {
  GEOM_ASSERT_OBJ(tapered);

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

  PUSH_MODELMATRIX;

  __modelmatrix.translate(translated->getTranslation());
  translated->getGeometry()->apply(*this);

  POP_MODELMATRIX;

  return true;
}

bool Serializer::process(TriangleSet *triangleSet)
{

  GEOM_ASSERT_OBJ(triangleSet);

  uint_t id = (uint_t)triangleSet->getObjectId();
  if (__single_mesh) {
      __triangleSoup.colors.push_back(Color3(__red, __green, __blue));
      __triangleSoup.numFaces += triangleSet->getIndexListSize();
      __triangleSoup.meshs.push_back(*triangleSet);
  } else {
    TriangleSoupCache::Iterator it = __triangle_soup_cache.find((uint_t)triangleSet->getObjectId());
    auto instance = Instance(__modelmatrix.getMatrix(), __red, __green, __blue);
    if (it != __triangle_soup_cache.end()) {
      it->second.instances.push_back(instance);
    } else {
      auto soup = TriangleSetInstances((uint_t)id, triangleSet, instance);
      __triangle_soup_cache.insert((uint_t)triangleSet->getObjectId(), soup);
    }
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
  return false;
}

bool Serializer::process(Font *font) {
  GEOM_ASSERT_OBJ(font);
  return false;
}
