cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(gc_wvcp)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
SET(CMAKE_AR "gcc-ar")

# -Wsign-conversion -Weffc++
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_ARCH_FLAGS} -DDEBUG -O0 -g -ldl -lm -isystem dir -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wunused -Woverloaded-virtual -Wpedantic -Wconversion -Wmisleading-indentation -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wnull-dereference -Wuseless-cast -Wdouble-promotion -Wformat=2")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_ARCH_FLAGS} -O3 -flto=auto -DNDEBUG -ldl -lm -Wall -s -isystem dir -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wunused -Woverloaded-virtual -Wpedantic -Wconversion -Wmisleading-indentation -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wnull-dereference -Wuseless-cast -Wdouble-promotion -Wformat=2")

list(APPEND CMAKE_PREFIX_PATH "$PWD/../thirdparty/libtorch")
find_package(Torch REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

add_executable(${CMAKE_PROJECT_NAME}
    src/main.cpp

    # utils
    src/utils/random_generator.cpp src/utils/random_generator.hpp
    src/utils/utils.cpp src/utils/utils.hpp

    # representation
    src/representation/enum_types.cpp src/representation/enum_types.hpp
    src/representation/Graph.cpp src/representation/Graph.hpp
    src/representation/Method.hpp
    src/representation/Node.cpp src/representation/Node.hpp
    src/representation/Parameters.cpp src/representation/Parameters.hpp
    src/representation/ProxiSolutionILSTS.cpp src/representation/ProxiSolutionILSTS.hpp
    src/representation/ProxiSolutionRedLS.cpp src/representation/ProxiSolutionRedLS.hpp
    src/representation/Solution.cpp src/representation/Solution.hpp

    # methods
    src/methods/adaptive.cpp src/methods/adaptive.hpp
    src/methods/afisa.cpp src/methods/afisa.hpp
    src/methods/afisa_original.cpp src/methods/afisa_original.hpp
    src/methods/greedy.cpp src/methods/greedy.hpp
    src/methods/hill_climbing.cpp src/methods/hill_climbing.hpp
    src/methods/ilsts.cpp src/methods/ilsts.hpp
    src/methods/LocalSearch.cpp src/methods/LocalSearch.hpp
    src/methods/MCTS.cpp src/methods/MCTS.hpp
    src/methods/neural_network.cpp src/methods/neural_network.hpp
    src/methods/none_ls.cpp src/methods/none_ls.hpp
    src/methods/redls.cpp src/methods/redls.hpp
    src/methods/redls_freeze.cpp src/methods/redls_freeze.hpp
    src/methods/partial_col.cpp src/methods/partial_col.hpp
    src/methods/tabu_col.cpp src/methods/tabu_col.hpp
    src/methods/SimulationHelper.cpp src/methods/SimulationHelper.hpp
    src/methods/tabu_weight.cpp src/methods/tabu_weight.hpp
    src/methods/useless_ls.cpp src/methods/useless_ls.hpp
    src/methods/worst_ls.cpp src/methods/worst_ls.hpp
)

set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY CXX_STANDARD 17)

# add dependencies
set(CPM_DOWNLOAD_VERSION 0.35.5)

if(CPM_SOURCE_CACHE)
    set(CPM_DOWNLOAD_LOCATION "${CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
elseif(DEFINED ENV{CPM_SOURCE_CACHE})
    set(CPM_DOWNLOAD_LOCATION "$ENV{CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
else()
    set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
endif()

if(NOT(EXISTS ${CPM_DOWNLOAD_LOCATION}))
    message(STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}")
    file(DOWNLOAD
        https://github.com/TheLartians/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
        ${CPM_DOWNLOAD_LOCATION}
    )
endif()

include(${CPM_DOWNLOAD_LOCATION})

CPMAddPackage(
    GITHUB_REPOSITORY jarro2783/cxxopts
    VERSION 2.2.1
    OPTIONS "CXXOPTS_BUILD_EXAMPLES NO" "CXXOPTS_BUILD_TESTS NO" "CXXOPTS_ENABLE_INSTALL YES"
)

CPMAddPackage(
    NAME fmt
    GIT_TAG 9.0.0
    GITHUB_REPOSITORY fmtlib/fmt
)

# link dependencies
target_link_libraries(${CMAKE_PROJECT_NAME} fmt cxxopts "${TORCH_LIBRARIES}")
