#include <iostream>  // NOLINT

#include <voxblox/core/esdf_map.h>
#include <voxblox/integrator/esdf_integrator.h>

#include "voxblox_tango_interface/io/tango_layer_io.h"
#include "voxblox_tango_interface/core/tango_layer_interface.h"

using namespace voxblox;  // NOLINT

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);

  if (argc != 3) {
    throw std::runtime_error("Args: filename to load, followed by filename to save to");
  }

  const std::string file = argv[1];

  TangoLayerInterface::Ptr layer_from_file;
  io::TangoLoadLayer(file, &layer_from_file);

  std::cout << "Layer memory size: " << layer_from_file->getMemorySize() << "\n";

  // ESDF maps.
  EsdfMap::Config esdf_config;
  std::shared_ptr<EsdfMap> esdf_map_;
  std::unique_ptr<EsdfIntegrator> esdf_integrator_;

  // Same number of voxels per side for ESDF as with TSDF
  esdf_config.esdf_voxels_per_side = layer_from_file->voxels_per_side();
  // Same voxel size for ESDF as with TSDF
  esdf_config.esdf_voxel_size = layer_from_file->voxel_size();
  esdf_map_.reset(new EsdfMap(esdf_config));
  EsdfIntegrator::Config esdf_integrator_config;
  // Make sure that this is the same as the truncation distance OR SMALLER!
  esdf_integrator_config.min_distance_m = esdf_config.esdf_voxel_size;
  esdf_integrator_.reset(new EsdfIntegrator(esdf_integrator_config,
                                                 layer_from_file.get(),
                                                 esdf_map_->getEsdfLayerPtr()));
  esdf_integrator_->updateFromTsdfLayerBatch();

  bool esdfSuccess = io::SaveLayer(esdf_map_->getEsdfLayer(), argv[2]);

  if (esdfSuccess == false) {
    throw std::runtime_error("Failed to save ESDF");
  }

  return 0;
}
