#include <cstdlib>
#include <iostream>
#include <vector>

#include "middleware.h"

static std::string python_exec = "/home/lijianlin/.conda/envs/tensor/bin/python";
static std::string eran_path = "/home/lijianlin/projects/github/ljlin/eran/tf_verify";
static std::string network_path =
    "/home/lijianlin/projects/github/ljlin/eran/tf_verify/racetrack-fnn-14-64-64-9_20191121.tf";

int main(int argc, char **argv) {
    iarguments input_box_int = {{0, 19},
                                {7,  7},
                                {-2, -2},
                                {-4, -4},
                                {5,  5},
                                {5,  5},
                                {5,  5},
                                {7,  7},
                                {0, 16},
                                {4,  4},
                                {4,  4},
                                {4,  4},
                                {0,  0},
                                {7,  7}};

    darguments input_box_double = middleware::normalize(input_box_int);

    std::string dir_name = get_current_dir_name();
    std::string input_box_path = dir_name + "/input_box.txt";
    std::string relation_diagram_path = dir_name + "/relation.txt";

    std::vector<int> res = middleware::run_eran(python_exec, eran_path, input_box_path, relation_diagram_path,
                                                network_path, input_box_double);

    std::cout << "Possible output :";
    for (int x : res) {
        std::cout << " " << x;
    }
    std::cout << std::endl;
}
