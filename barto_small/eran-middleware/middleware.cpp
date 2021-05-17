#include <cstdlib>
#include <iostream>
#include <vector>
#include <sstream>
#include "unistd.h"
#include "middleware.h"

static std::string python_exec = "/usr/bin/python3";
static std::string eran_path = "/eran/tf_verify";
static std::string network_path = "/agentsB/barto-small_4x64_good.tf";
static int NN_Noise = 1;

int main(int argc, char **argv) {

	std::ostringstream oss;
	int p_id;
	p_id = getpid();
	oss << p_id;
    
    //Read file descriptor
    int write_fd;
    sscanf(argv[1], "%d", &write_fd);

    //Read bounds
    int lbound, ubound;
    iarguments input_box_int;

    for(int i=2;i<30;i+=2){
        sscanf(argv[i], "%d", &lbound);
        sscanf(argv[i+1], "%d", &ubound);
        //std::cout << "Bounds read for feature " << (i/2) << " are : " << lbound << " " << ubound << std::endl;
        input_box_int.push_back(std::pair<int, int>(lbound, ubound));
    }

    darguments input_box_double = middleware::normalize(input_box_int);

    std::string dir_name = get_current_dir_name();
    std::string input_box_path = dir_name + "/input_box_" + oss.str() + ".txt";
    std::string relation_diagram_path = dir_name + "/relation_" + oss.str() + ".txt";
    std::string output_bound_path = dir_name + "/eran_bound_" + oss.str() + ".txt";

    std::vector<int> res = middleware::run_eran(
        python_exec, eran_path,
        input_box_path, relation_diagram_path,output_bound_path,
        network_path, input_box_double
    );

	darguments scores = middleware::read_out_bounds_eran(output_bound_path);
    darguments probs = middleware::normalize_output(network_path, scores);

    std::stringstream ssr1;
    ssr1 << "rm " << input_box_path;
    system(ssr1.str().c_str());
    std::stringstream ssr2;
    ssr2 << "rm " << relation_diagram_path;
    system(ssr2.str().c_str());
    std::stringstream ssr3;
    ssr3 << "rm " << output_bound_path;
    system(ssr3.str().c_str());

    if(NN_Noise){
        //Compute actions based on upper bounds of prob
        std::vector<int> new_res;
        for(int i=0; i < probs.size(); i++){
            if(probs[i].second >= 0.25){
                new_res.push_back(i);
            }
        }

        if(new_res.size() == 0){
            new_res = res;
        }

        int size = new_res.size();
        //std::cout << "Number of actions returned " << size << std::endl;
        write(write_fd, (const void *)(&size), sizeof(int));

        for (int x : new_res) {
            //std::cout << "Action returned " << x << std::endl;
            write(write_fd, (const void *)(&middleware::directions[x].first), sizeof(int));
            write(write_fd, (const void *)(&middleware::directions[x].second), sizeof(int));
        }

        std::cout << std::endl;
        exit(0);
    }
    else{
        int size = res.size();
        //std::cout << "Number of actions returned " << size << std::endl;
        write(write_fd, (const void *)(&size), sizeof(int));

        for (int x : res) {
            //std::cout << "Action returned " << x << std::endl;
            write(write_fd, (const void *)(&middleware::directions[x].first), sizeof(int));
            write(write_fd, (const void *)(&middleware::directions[x].second), sizeof(int));
        }

        std::cout << std::endl;
        exit(0);
    }
}
