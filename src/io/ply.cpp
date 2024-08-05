#include <fmt/os.h>
#include <occ/io/ply.h>
#include <occ/io/tinyply.h>

namespace occ::io {

void write_ply_file(const std::string &filename,
                    const Eigen::Matrix3Xf &vertices,
                    const Eigen::Matrix3Xi &faces) {
  auto file = fmt::output_file(filename);
  file.print("ply\n");
  file.print("format ascii 1.0\n");
  file.print("comment exported from OCC\n");
  file.print("element vertex {}\n", vertices.size() / 3);
  file.print("property float x\n");
  file.print("property float y\n");
  file.print("property float z\n");
  file.print("element face {}\n", faces.size() / 3);
  file.print("property list uchar int vertex_index\n");
  file.print("end_header\n");
  for (size_t idx = 0; idx < vertices.cols(); idx++) {
    file.print("{} {} {}\n", vertices(0, idx), vertices(1, idx),
               vertices(2, idx));
  }
  for (size_t idx = 0; idx < faces.cols(); idx++) {
    file.print("3 {} {} {}\n", faces(0, idx), faces(1, idx), faces(2, idx));
  }
}

void write_ply_mesh(const std::string &filename, const IsosurfaceMesh &mesh,
                    const VertexProperties &properties, bool binary) {

  tinyply::PlyFile ply_file;

  ply_file.add_properties_to_element(
      "vertex", {"x", "y", "z"}, tinyply::Type::FLOAT32,
      mesh.vertices.size() / 3,
      reinterpret_cast<const uint8_t *>(mesh.vertices.data()),
      tinyply::Type::INVALID, 0);

  if (mesh.normals.size() > 0) {
    ply_file.add_properties_to_element(
        "vertex", {"nx", "ny", "nz"}, tinyply::Type::FLOAT32,
        mesh.normals.size() / 3,
        reinterpret_cast<const uint8_t *>(mesh.normals.data()),
        tinyply::Type::INVALID, 0);
  }

  ply_file.add_properties_to_element(
      "face", {"vertex_indices"}, tinyply::Type::UINT32, mesh.faces.size() / 3,
      reinterpret_cast<const uint8_t *>(mesh.faces.data()),
      tinyply::Type::UINT32, 3);

  for (const auto &[k, v] : properties.fprops) {
    ply_file.add_properties_to_element(
        "vertex", {k}, tinyply::Type::FLOAT32, v.size(),
        reinterpret_cast<const uint8_t *>(v.data()), tinyply::Type::INVALID, 0);
  }

  for (const auto &[k, v] : properties.iprops) {
    ply_file.add_properties_to_element(
        "vertex", {k}, tinyply::Type::INT32, v.size(),
        reinterpret_cast<const uint8_t *>(v.data()), tinyply::Type::INVALID, 0);
  }

  ply_file.get_comments().push_back("Generated by OCC");

  std::filebuf out_fb;
  if (binary) {
    out_fb.open(filename, std::ios::out | std::ios::binary);
  } else {
    out_fb.open(filename, std::ios::out);
  }
  std::ostream out(&out_fb);
  if (out.fail())
    throw std::runtime_error("Could not open file for writing: " + filename);

  ply_file.write(out, binary);
}

} // namespace occ::io
