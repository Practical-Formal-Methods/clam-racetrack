#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include "unistd.h"
#include "middleware.h"

std::string network_path = "/agentsB/barto-big_4x64_good.rlv";
int NN_Noise = 1;

int main(int argc, char *argv[]){

    int pid;
    pid = getpid();
    std::ostringstream jsonFileName;
    jsonFileName << "/deepsymbol/info_" << pid << ".json";

    int write_fd;
    sscanf(argv[1], "%d", &write_fd);

    //Read bounds
    int lbound, ubound;
    iarguments input_box_int;

    for(int i=2;i<30;i+=2){
        sscanf(argv[i], "%d", &lbound);
        sscanf(argv[i+1], "%d", &ubound);
        input_box_int.push_back(std::pair<int, int>(lbound, ubound));
    }

    darguments input_box_double = middleware::normalize(input_box_int);
    std::vector<int> res = middleware::run(network_path, input_box_double,jsonFileName.str());

    darguments bounds = middleware::read_out_bounds(jsonFileName.str(),"Accuracy");
    darguments prob_bounds = middleware::normalize_output(network_path, bounds);
    //for (int i = 0; i < prob_bounds.size(); ++i) {
    //    std::cout << prob_bounds[i].first << "\t" << prob_bounds[i].second << std::endl;
   // }

    if(NN_Noise){
        //Compute actions based on upper bounds of prob
        std::vector<int> new_res;
        for(int i=0; i < prob_bounds.size(); i++){
            if(prob_bounds[i].second >= 0.25){
                new_res.push_back(i);
            }
        }

        if(new_res.size() == 0){
            new_res =res;
        }

        //USE new_res if NN Noise is ON
        int size = new_res.size();
        write(write_fd, (const void *)(&size), sizeof(int));

        for (int x : new_res) {
            write(write_fd, (const void *)(&middleware::directions[x].first), sizeof(int));
            write(write_fd, (const void *)(&middleware::directions[x].second), sizeof(int));
        }

        std::cout << std::endl;
        exit(0);
    }
    else{
        int size = res.size();
        write(write_fd, (const void *)(&size), sizeof(int));

        for (int x : res) {
            write(write_fd, (const void *)(&middleware::directions[x].first), sizeof(int));
            write(write_fd, (const void *)(&middleware::directions[x].second), sizeof(int));
        }

        std::cout << std::endl;
        exit(0);
    }
}
