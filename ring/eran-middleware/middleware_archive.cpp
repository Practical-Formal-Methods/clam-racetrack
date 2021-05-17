#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include "unistd.h"
#include "middleware.h"

static std::string python_exec = "/usr/bin/python3";
static std::string eran_path = "/eran/tf_verify";
static std::string network_path = "/agents/barto-small_20k_saved_model.tf";

int main(int argc, char *argv[]){

    int pid;
    pid = getpid();
    std::ostringstream jsonFileName;
    jsonFileName << "./info_" << pid << ".json";

    darguments out = {
        {63.668, 63.668},
        {2.5238, 2.5238},
        {89.0969, 89.0969},
        {80.997, 80.997},
        {14.5207, 14.5207},
        {86.1086, 86.1086},
        {26.829, 26.829},
        {100.242, 100.242},
        {9.85262, 9.85262}
    };

    darguments pros = middleware::normalize_output(out);
    for (auto p : pros) {
        std::cout << p.first << "   " << p.second << std::endl;
    }
    //Read file descriptor
    int write_fd;
    sscanf(argv[1], "%d", &write_fd);

    //Read bounds
    int lbound, ubound;
    iarguments input_box_int;

    for(int i=2;i<30;i+=2){
        sscanf(argv[i], "%d", &lbound);
        sscanf(argv[i+1], "%d", &ubound);
        std::cout << "Bounds read for feature " << (i/2) << " are : " << lbound << " " << ubound << std::endl;
        input_box_int.push_back(std::pair<int, int>(lbound, ubound));
    }

    darguments input_box_double = middleware::normalize(input_box_int);
    std::vector<int> res = middleware::run("./barto-small_20k_saved_model.rlv",input_box_double,jsonFileName.str());

    darguments bounds = middleware::read_out_bounds(jsonFileName.str(),"Accuracy");
    darguments prob_bounds = middleware::normalize_output(bounds);
    for (int i = 0; i < prob_bounds.size(); ++i) {
        std::cout << prob_bounds[i].first << "\t" << prob_bounds[i].second << std::endl;
    }

    //Compute actions based on upper bounds of prob
    std::vector<int> new_res;
    for(int i=0; i < prob_bounds.size(); i++){
        if(prob_bounds[i].second > 0.1){
            new_res.push_back(i);
        }
    }

    int size = new_res.size();
    std::cout << "Number of actions returned " << size << std::endl;
    write(write_fd, (const void *)(&size), sizeof(int));

    for (int x : new_res) {
        std::cout << "Action returned " << x << std::endl;
        write(write_fd, (const void *)(&middleware::directions[x].first), sizeof(int));
        write(write_fd, (const void *)(&middleware::directions[x].second), sizeof(int));
    }

    std::cout << std::endl;
    exit(0);
}
