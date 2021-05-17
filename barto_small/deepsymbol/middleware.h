#include <cassert>
#include <vector>
#include <utility>
#include <cmath>
#include <string>
#include <fstream>
#include <numeric>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "absExe.h"

//#define PII std::make_pair
typedef std::vector<std::pair<int, int>> relation;
typedef std::vector<std::pair<int, int>> iarguments;
typedef std::vector<std::pair<double, double>> darguments;

class middleware {
public:

    static const int map_height = 12;
    static const int map_width = 35;
    static const int vx_limit = 5;
    static const int vy_limit = 5;
    static const int map_short;
    static const std::vector<std::pair<int,int>> wall_directions;
    static const std::vector<std::pair<int, int>> directions;

    static std::pair<double, double> div(std::pair<int, int> a, int b) {
        //TODO : check over approximation rounding
        return PII(double(a.first) / b, double(a.second) / b);
    }

    /**
     * The inputs of the network are :
     *   - the position of the car (2 floats for x and y coordinate),
     *   - the velocity of the car (2 floats),
     *   - distance to the walls in 8 directions (8 floats) and
     *   - distance to the goal (2 floats).
     * @param raw is the integer input without normalization.
     * @return normalized float input
     */
    static darguments read_out_bounds(std::string JSON, std::string name) {
        boost::property_tree::ptree root;
        try {
            boost::property_tree::read_json<boost::property_tree::ptree>(JSON,root);
        }
        catch(std::exception & e){
            printf("Read JSON file failed\n");
            return darguments();
        }
        std::vector<std::string> names;
        for (auto val : root.get_child("graph.names")) {
            assert(val.first.empty());
            std::string name = val.second.data();
            names.push_back(name);
            //std::cout << name << " ";
        }
        //std::cout << std::endl;
        darguments res;
        for (auto pitem : root.get_child("layers"))
        {
            assert(pitem.first.empty());
            auto player = pitem.second;
            std::string pname = player.get<std::string>("name");
            if (pname == name) {
                for (auto pbound : player.get_child("bound_of_neurons"))
                {
                    std::string key = pbound.first;
                    auto val = pbound.second;
                    double inf = val.get<double>("inf");
                    double sup = val.get<double>("sup");
                    //std::cout << "key:\t" << key << "\tinf:\t" << inf << "\tsup:\t" << sup << std::endl;
                    assert(names[res.size()] == key);
                    res.push_back(std::make_pair(inf,sup));
                }
            }
        }
        return res;
    }
    static darguments read_out_bounds_eran(std::string path) {
        darguments res;
        std::ifstream file(path);
        int n;
        file >> n;
        for (int i = 0; i < n; ++i) {
            double l, u;
            file >> l >> u;
            res.push_back(PII(l,u));
        }
        return res;

    }

    static std::pair<double, double> interval_division(double a, double b, double c, double d){
        double p1 = a*c;
        double p2 = a*d;
        double p3 = b*c;
        double p4 = b*d;

        double low = std::min( p1, std::min(p2, std::min(p3, p4)));
        double high = std::max( p1, std::max(p2, std::max(p3, p4)));
        return std::make_pair(low, high);
    }


    static darguments normalize_output(std::string network_path, darguments bounds) {
        //Apply Temperature for big NNs
        //Ref: http://www.kasimte.com/2020/02/14/how-does-temperature-affect-softmax-in-machine-learning.html
        //Expects bigNN networks to be present in /agentsB
        if(network_path.rfind("/agentsB", 0) == 0){
            for( auto &it: bounds){
                it.first /= 75;
                it.second /= 75;
            }
        }
        const double sum_up = std::accumulate(
            bounds.begin(),
            bounds.end(),
            0.0,
            [](double res,std::pair<double, double> bound) -> double {
                return double(res + std::exp(bound.second));
        });
        const double sum_low = std::accumulate(
            bounds.begin(),
            bounds.end(),
            0.0,
            [](double res,std::pair<double, double> bound) -> double {
                return double(res + std::exp(bound.first));
            });
        //std::cout << sum_up << "===" << sum_low << std::endl;
        darguments res;
        std::transform(
            bounds.begin(),
            bounds.end(),
            std::back_inserter(res),
            [sum_low,sum_up](std::pair<double, double> bound) {
                return interval_division(std::exp(bound.first), std::exp(bound.second), 1/sum_low, 1/sum_up);//std::make_pair(std::exp(bound.first) / sum_low, std::exp(bound.second) / sum_up);
            });
        return res;
    }


    static darguments normalize(iarguments raw) {
        assert(raw.size() == 14);
        darguments res;
        // x coordinate
        res.push_back(div(raw[0], map_width));
        // y coordinate
        res.push_back(div(raw[1], map_height));
        // vx
        res.push_back(div(raw[2], vx_limit * 2));
        // vy
        res.push_back(div(raw[3], vy_limit * 2));

        //distance to the wall
        for (unsigned i = 0; i < wall_directions.size(); ++i) {
            std::pair<int, int> dir = wall_directions[i];
            int j = 4 + i;

            if ((dir.first != 0) && (dir.second != 0)) {
                res.push_back(div(raw[j], map_short));
            } else if (dir.first != 0) { // and dir.second == 0
                res.push_back(div(raw[j], map_width));
            } else { // dir.first == 0 and dir.second != 0
                res.push_back(div(raw[j], map_height));
            }
        }

        //x-distance to the goal
        res.push_back(div(raw[12], map_width));
        //y-distance to the goal
        res.push_back(div(raw[13], map_height));
        assert(res.size() == 14);
        return res;
    }

    static void write_box_to_file(darguments box, std::string path) {
        std::ofstream file(path);
        for (unsigned i = 0; i < box.size(); ++i) {
            file << box[i].first << " " << box[i].second << std::endl;
        }
        file.close();
    }

    static std::pair<int, relation> read_graph_from(std::string path) {
        int num, s, t;
        relation r;
        std::ifstream file(path);
        file >> num;

        while (file >> s >> t) {
            r.push_back(PII(s, t));
        }

        return PII(num, r);
    }

    static std::vector<int> maximum_of(std::pair<int, relation> graph) {
        int num = graph.first;
        relation &edges = graph.second;
        bool max[num];
        for (int i = 0; i < num; i++) max[i] = true;
        for (auto edge : edges) {
            max[edge.first] = false;
        }

        std::vector<int> possibleOutput;
        for (int i = 0; i < num; i++) {
            if (max[i]) {
                possibleOutput.push_back(i);
            }
        }
        return possibleOutput;
    }

    static void run(std::string network, ROBUST robust, std::string image, double delta, std::string JSON) {
        int pid = getpid();
        std::ostringstream boundFileName;
        boundFileName << "/deepsymbol/bound_" << pid << ".log";
        std::ostringstream summaryFileName;
        summaryFileName << "/deepsymbol/summary_" << pid << ".txt";
        robust_rlv(BOX, robust, network,
                   image, delta,
                   std::vector<std::pair<double, double>>(),
                   false, boundFileName.str(), summaryFileName.str(),
                   true, JSON);

        std::stringstream ssr2;
        ssr2 << "rm " << boundFileName.str();
        system(ssr2.str().c_str());
        std::stringstream ssr3;
        ssr3 << "rm " << summaryFileName.str();
        system(ssr3.str().c_str());
    }

    static std::vector<int> run(std::string network, darguments input_box,std::string JSON) {
        bool dump = JSON.length() > 0;
        int pid = getpid();
        std::ostringstream boundFileName;
        boundFileName << "/deepsymbol/bound_" << pid << ".log";
        std::ostringstream summaryFileName;
        summaryFileName << "/deepsymbol/summary_" << pid << ".txt";
        std::pair<int, relation> graph = robust_rlv(BOX, CUSTOMBOX, network,
                                                    "", 0,
                                                    input_box,
                                                    true, boundFileName.str(), summaryFileName.str(),
                                                    dump, JSON);

        std::stringstream ssr2;
        ssr2 << "rm " << boundFileName.str();
        system(ssr2.str().c_str());
        std::stringstream ssr3;
        ssr3 << "rm " << summaryFileName.str();
        system(ssr3.str().c_str());

        return maximum_of(graph);
    }

};

const std::vector<std::pair<int,int>> middleware::directions = {
    {-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 0}, {0, 1}, {1, -1}, {1, 0}, {1, 1}
};

const std::vector<std::pair<int,int>> middleware::wall_directions = {
    {-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}
};


const int middleware::map_short = std::min(middleware::map_height, middleware::map_width);
