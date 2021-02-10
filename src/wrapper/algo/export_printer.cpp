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



#include "export_printer.h"
#include <plantgl/algo/codec/printer.h>
#include <plantgl/algo/codec/binaryprinter.h>
#include <plantgl/algo/codec/scne_binaryparser.h>
#include <plantgl/scenegraph/scene/scene.h>
#include <plantgl/scenegraph/geometry/triangleset.h>
#include <plantgl/algo/base/tesselator.h>
#include <plantgl/tool/bfstream.h>
#include <boost/python.hpp>

#include <draco/mesh/triangle_soup_mesh_builder.h>
#include <draco/compression/expert_encode.h>

/* ----------------------------------------------------------------------- */

PGL_USING_NAMESPACE
using namespace boost::python;

/* ----------------------------------------------------------------------- */

PyStrPrinter::~PyStrPrinter()   { _mystream << std::endl;  }
PyFilePrinter::~PyFilePrinter() { _mystream << std::endl; _mystream.close(); }

/* ----------------------------------------------------------------------- */

void export_StrPrinter()
{
  class_< PyStrPrinter, boost::noncopyable > ("StrPrinter", no_init  )
    .def("str",    &PyStrPrinter::str,  "str() : return string")
    .def("__str__",&PyStrPrinter::str,  "__str__() : return string")
    .add_property("result", &PyStrPrinter::str)
    ;
}

void export_FilePrinter()
{
    class_< PyFilePrinter, boost::noncopyable >
        ("FilePrinter", no_init )
    ;
}


/* ----------------------------------------------------------------------- */

class PyStrPGLPrinter : public PyStrPrinter, public Printer {
public:
    PyStrPGLPrinter() :  Printer(_mystream,_mystream,_mystream) {}
};

class PyFilePGLPrinter : public PyFilePrinter, public Printer {
public:
    PyFilePGLPrinter(const std::string& fname) : PyFilePrinter(fname), Printer(_mystream,_mystream,_mystream) {}
};

/* ----------------------------------------------------------------------- */


void print_header0(Printer * p)
{ p->header(); }

void print_header(Printer * p, const std::string comment)
{ p->header(comment.c_str()); }

void export_PglPrinter()
{
 class_< Printer, bases< Action >, boost::noncopyable > ( "PglPrinter" , no_init )
    .def("clear",&Printer::clear)
    .def("incrementIndentation",&Printer::addIndent)
    .def("isPrinted",&Printer::isPrinted)
    .def("flush",&Printer::flush)
    .def("header",&print_header0)
    .def("header",&print_header)
    ;

  class_< PyStrPGLPrinter , bases< PyStrPrinter, Printer > , boost::noncopyable>
      ("PglStrPrinter",init<>("String Printer in PGL format" ))
      .def(str_printer_clear<>());

  class_< PyFilePGLPrinter , bases< PyFilePrinter, Printer > , boost::noncopyable>
      ("PglFilePrinter",init<const std::string&>("File Printer in PGL format",args("filename")) );
    ;
}

/* ----------------------------------------------------------------------- */

class PyFileBinaryPrinter : public BinaryPrinter {
    public:
        PyFileBinaryPrinter(const std::string& fname) :
          BinaryPrinter(_mystream), _mystream(fname.c_str(), std::ios::out | std::ios::binary)  { }

        ~PyFileBinaryPrinter(){}
        std::ofstream _mystream;
};

template<class Printer>
bool abp_print(Printer* printer, ScenePtr scene)
{
  return printer->print(scene);
}


class PyStrBinaryPrinter : public BinaryPrinter {
public:
    PyStrBinaryPrinter() :
        BinaryPrinter(_mystream) { }

        /// resulting string
        boost::python::object result() {
            std::string res = _mystream.str().c_str();
            return object( handle<>( PyBytes_FromStringAndSize(res.c_str(), res.size())));
        }
        /// clear the buffer
        void clear() {  this->BinaryPrinter::clear(); _mystream.str(""); }

protected:
    std::ostringstream _mystream;
};

boost::python::object py_tobinarystring(ScenePtr scene, bool double_precision = true,  const char * comment = NULL) {
    std::string res = BinaryPrinter::tobinarystring(scene, double_precision, comment);
    return object( handle<>( PyBytes_FromStringAndSize(res.c_str(), res.size())));
}

ScenePtr py_frombinarystring(boost::python::object bytes) {
    return BinaryParser::frombinarystring(extract<std::string>(bytes)());
}

void export_PglBinaryPrinter()
{
  class_< PyFileBinaryPrinter, bases< Printer >, boost::noncopyable>
      ("PglBinaryPrinter",init<const std::string&>("Binary Pgl Printer",args("filename")))
    .def("print",abp_print<PyFileBinaryPrinter>)
    .def("getCanonicalFilename",BinaryPrinter::getCanonicalFilename,args("filename"))
    .staticmethod("getCanonicalFilename");
    ;

  class_< PyStrBinaryPrinter , bases< Printer > , boost::noncopyable>
      ("PglStrBinaryPrinter",init<>("String Printer in PGL binary format" ))
      .def("print",abp_print<PyStrBinaryPrinter>)
      .def( "clear", &PyStrBinaryPrinter::clear)
      .add_property("result", &PyStrBinaryPrinter::result)
      ;
    def("tobinarystring", &py_tobinarystring,(bp::arg("scene"),bp::arg("double_precision")=true,bp::arg("comment")=""));
    def("frombinarystring", &py_frombinarystring);
}

struct DracoScene {
  bool status = true;
  boost::python::object data = boost::python::object();
};

DracoScene draco_encode(ScenePtr scene, int compression = 2) {

    DracoScene ret;

    draco::TriangleSoupMeshBuilder mesh_builder;
    draco::EncoderBuffer buffer;
    uint_t count_f = 0;
    uint_t count_v = 0;
    uint_t index_f = 0;
    std::vector<TriangleSetPtr> tris;
    std::vector<Material*> mats;

    if (scene && !scene->empty()) {

        for (Scene::const_iterator it = scene->begin(); it != scene->end(); ++it) {
            Shape* shp = dynamic_cast<Shape*>(it->get());
            Tesselator tes;
            if (shp && shp->appearance && !shp->appearance->isTexture()) {
                Material* mat =  static_cast<Material*>(shp->appearance.get());
                mats.push_back(mat);
            } else {
                mats.push_back((Material*)&Material::DEFAULT_MATERIAL);
            }
            (*it)->apply(tes);
            TriangleSetPtr tri = tes.getTriangulation();
            tris.push_back(tri);
            count_f += tri->getIndexListSize();
            count_v += tri->getPointListSize();
        }

        mesh_builder.Start(count_f);
        int_t pos_att_id = mesh_builder.AddAttribute(draco::GeometryAttribute::POSITION, 3, draco::DT_FLOAT32);
        int_t col_att_id = mesh_builder.AddAttribute(draco::GeometryAttribute::COLOR, 3, draco::DT_UINT8);

        for (std::vector<TriangleSetPtr>::const_iterator it = tris.begin(); it != tris.end(); ++it) {

            auto tri = *it;
            auto vs = tri->getPointList();
            auto fs = tri->getIndexList();

            auto mat = mats.at(it - tris.begin());
            auto amb = mat->getAmbient();
            real_t dif = mat->getDiffuse();
            u_int8_t color[3] = {
                (u_int8_t)(amb.getRedClamped() * dif * 255.),
                (u_int8_t)(amb.getGreenClamped() * dif * 255.),
                (u_int8_t)(amb.getBlueClamped() * dif * 255.)
            };

            for (std::vector<Index3>::const_iterator it = fs->begin(); it != fs->end(); ++it) {

                Index3 face = *it;
                Vector3 p1 = vs->getAt(face[0]);
                Vector3 p2 = vs->getAt(face[1]);
                Vector3 p3 = vs->getAt(face[2]);

                mesh_builder.SetAttributeValuesForFace(
                    pos_att_id,
                    draco::FaceIndex(index_f),
                    draco::Vector3f(p1.x(), p1.y(), p1.z()).data(),
                    draco::Vector3f(p2.x(), p2.y(), p2.z()).data(),
                    draco::Vector3f(p3.x(), p3.y(), p3.z()).data()
                );

                mesh_builder.SetPerFaceAttributeValueForFace(
                    col_att_id,
                    draco::FaceIndex(index_f),
                    &color
                );

                index_f++;
            }
        }

        auto mesh = mesh_builder.Finalize();

        mesh->DeduplicatePointIds();
        draco::ExpertEncoder encoder(*mesh.get());
        encoder.SetAttributeQuantization(draco::GeometryAttribute::POSITION, 14 - std::max(0, std::min(5, compression)));
        encoder.SetAttributeQuantization(draco::GeometryAttribute::TEX_COORD, 10);
        encoder.SetAttributeQuantization(draco::GeometryAttribute::NORMAL, 7);
        encoder.SetAttributeQuantization(draco::GeometryAttribute::GENERIC, 8);
        encoder.SetSpeedOptions(9, 9);
        auto status = encoder.EncodeToBuffer(&buffer);
        ret.status = status.ok();

        char *c = new char[buffer.size()];
        std::copy(buffer.buffer()->begin(), buffer.buffer()->end(), c);

        if (ret.status) {
            bp::object mem(bp::handle<>(PyMemoryView_FromMemory(c, buffer.size(), PyBUF_READ)));
            ret.data = mem;
        }
    }

    tris.clear();
    mats.clear();

    return ret;

}

void export_Draco()
{
    boost::python::class_< DracoScene > ("DracoScene", boost::python::no_init )
        .add_property("status", &DracoScene::status)
        .add_property("data", &DracoScene::data);

    def("draco_encode", &draco_encode, (bp::arg("scene"), bp::arg("compression")=2));
}
