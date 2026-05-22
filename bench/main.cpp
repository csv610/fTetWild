#include <floattetwild/FloatTetwild.h>
#include <floattetwild/Logger.hpp>
#include <floattetwild/MeshIO.hpp>
#include <geogram/basic/common.h>
#include <chrono>
#include <iostream>
#include <vector>
#include <thread>
#include <tbb/global_control.h>
#include <igl/parallel_for.h>

using namespace floatTetWild;

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_mesh.off> [max_threads]" << std::endl;
        return 1;
    }

    std::string input_file = argv[1];
    unsigned int max_threads = std::thread::hardware_concurrency();
    if (argc >= 3) {
        max_threads = std::stoi(argv[2]);
    }

    GEO::initialize();
    Logger::init(true, "");
    spdlog::set_level(spdlog::level::warn);

    // TBB Parallelism Control
    tbb::global_control parallelism_limit(tbb::global_control::max_allowed_parallelism, max_threads);
    igl::default_num_threads(std::ceil(std::sqrt(max_threads)));

    Parameters params;
    params.is_quiet = true;
    params.input_path = input_file;
    params.num_threads = max_threads;

    GEO::Mesh sf_mesh;
    std::vector<Vector3> input_vertices;
    std::vector<Vector3i> input_faces;
    std::vector<int> input_tags;

    if (!MeshIO::load_mesh(input_file, input_vertices, input_faces, sf_mesh, input_tags)) {
        std::cerr << "Failed to load mesh: " << input_file << std::endl;
        return 1;
    }

    std::cout << "Benchmarking with " << max_threads << " threads on " << input_file << "..." << std::endl;

    Eigen::MatrixXd VO;
    Eigen::MatrixXi TO;

    auto start = std::chrono::high_resolution_clock::now();
    tetrahedralization(sf_mesh, params, VO, TO);
    auto end = std::chrono::high_resolution_clock::now();

    double duration = std::chrono::duration<double>(end - start).count();
    std::cout << "TIME: " << duration << "s" << std::endl;

    return 0;
}
