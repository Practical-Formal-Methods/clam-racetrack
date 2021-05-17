/*********************                                                        */
/*! \file absEle.h
 ** Top contributors (to current version):
 **   Jiangchao Liu
 ** This file is part of the AI & SMTReluplex project.
 ** Copyright (c) 2018-2100 by the authors listed in the file AUTHORS
 ** (in the top-level source directory) and their institutional affiliations.
 ** All rights reserved. See the file COPYING in the top-level source
 ** directory for licensing information.\endverbatim
 **/

#ifndef __ABSEXE_H
#define __ABSEXE_H

#include <boost/date_time.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <fstream>

#include "abst.h"
#include "ap_scalar.h"
#include "apronxx/apronxx.hh"
#include "apronxx/apxx_box.hh"
#include "apronxx/apxx_interval.hh"
#include "apronxx/apxx_oct.hh"
#include "apronxx/apxx_polka.hh"
#include "apronxx/apxx_tcons1.hh"
#include "apronxx/apxx_texpr1.hh"
#include "gmpxx.h"

#define PII std::make_pair

typedef std::vector<std::pair<int,int>> relation;

int maxRelu  = 0;
int inActive = 0;

enum ROBUST { BRIGHTNESS, LINFBALL, RACE, CUSTOMBOX, };


using namespace apron;
using boost::property_tree::ptree;
using boost::property_tree::write_json;

std::pair<unsigned long, std::vector<std::pair<int, int>>> Hasse(const std::pair<unsigned long, std::vector<std::pair<int, int>>>& G);
ptree data2pgraph(const std::pair<unsigned long, std::vector<std::pair<int, int>>>& data, std::vector<var> vars);
std::string dom_name(Domain dom) {
    return (dom == BOX ? "BOX" : (dom == TOPO ? "TOPO" : (dom == OCT ? "OCT" : (dom == EZONO ? "EZONO" : "ERROR"))));
}
std::string robust_string(ROBUST r) {
    switch (r) {
    case LINFBALL:
        return "L_inf Ball";
    case BRIGHTNESS:
        return "Brightness Attack";
    case RACE:
        return "RACE";
    case CUSTOMBOX:
        return "Coustom Box";
    }
    return "IMPOSSIBLE";
}

bool less(ABST* abst, var l, var r) {
    static int cnt = 0;
    var        temp(std::string("temp") + std::to_string(cnt++));
    abst->add_var(temp);
    environment env     = abst->get_environment();
    unsigned    size    = env.get_vars().size();
    coeff*      tcoeff  = new coeff[size];
    tcoeff[env[r]] = get_coeff(1);
    tcoeff[env[l]]  = get_coeff(-1);
    linexpr0 lexpr0(size, tcoeff, get_coeff(0));
    linexpr1 lexpr1(env, lexpr0);
    abst->assign(temp, lexpr1);
    interval bound = abst->get_bound(temp);
    abst->rem_var(temp);
    delete[] tcoeff;
    // cout << std::setprecision(std::numeric_limits<double>::digits10 + 1)
    //     << double(bound.get_inf()) << '\t' << double(bound.get_sup())
    //     << '\t' << std::endl;
    return double(bound.get_inf()) > 0;
}

std::pair<int,relation>  robust_rlv(Domain dom, ROBUST trobust, std::string network,
                std::string img, double delta,
                std::vector<std::pair<double, double>> input_box,
                bool log, std::string bound_file_name,std::string summary_file_name,
                bool dump, std::string json_file_name
                ) {

    std::cout << "rlv net file name \t: \t" << network << std::endl;

    if (trobust == LINFBALL || trobust == BRIGHTNESS) {
        std::cout << "img file name     \t: \t" << img << std::endl;
        std::cout << "delta             \t= \t" << delta << std::endl;
    }

    if (log) {
        std::cout << "bound file name   \t: \t" << bound_file_name << std::endl;
        std::cout << "summary file name \t: \t" << summary_file_name << std::endl;
    }

    std::cout << "domain            \t= \t" << dom_name(dom) << std::endl;
    std::cout << "robustness        \t= \t" << robust_string(trobust) << std::endl;

    std::ofstream ofre(bound_file_name, std::ios::app);
    std::ifstream inPic(img);
    std::ifstream rlvNet(network);
    std::string   currentLine;
    ABST*         abst = new ABST(dom);
    std::cout << abst->type_name() << std::endl;
    int                   layer;
    int                   input  = 0;
    unsigned              curNum = 0;
    unsigned              nodeNum;
    bool                  outLayer = false;
    std::vector<interval> boundFinal;
    std::vector<var>      outputvars;
    bool                  Verified   = true;
    auto                  start_time = boost::posix_time::microsec_clock::local_time();
    relation graph;
    ptree * bound_of_neurons = NULL;
    ptree players;
    ptree * player = NULL;

    while (std::getline(rlvNet, currentLine)) {
        while ((currentLine.size() > 0) && (currentLine[0] == ' '))
            currentLine = currentLine.substr(1, std::string::npos);
        while ((currentLine.size() > 0) && (currentLine[currentLine.size() - 1] == ' '))
            currentLine = currentLine.substr(0, currentLine.size() - 1);
        if (currentLine.size() > 0) {
            /* std::cout<<currentLine<<std::endl; */
            /* std::cout<<"apron assign "<<apron_assign_time<<std::endl; */
            /* std::cout<<"apron cons "<<apron_cons_time<<std::endl; */
            /* std::cout<<"apron join "<<apron_join_time<<std::endl; */

            /* std::cout<<"symv assign "<<symv_assign_time<<std::endl; */
            /* std::cout<<"symv cons "<<symv_cons_time<<std::endl; */
            /* std::cout<<"symv join "<<symv_join_time<<std::endl; */
            // Parse this line
            std::istringstream thisLineStream(currentLine);
            std::string        linePrefix;
            thisLineStream >> linePrefix;
            if (linePrefix == "#") {
                // abst->output_bound(abst);
                std::string slayer;
                unsigned    layer, num;
                std::string ty, tn;
                thisLineStream >> slayer >> layer >> nodeNum >> ty >> tn;
                std::cout << slayer << layer << std::endl;
                ofre << "* " << layer << std::endl;
                if (ty == "Linear" && tn == "Accuracy")
                    outLayer = true;
                if (layer > 0) {
                    player->add_child("bound_of_neurons",*bound_of_neurons);
                    players.push_back(PII("",*player));
                }
                bound_of_neurons = new ptree();
                player = new ptree();
                player->put("name",tn);
                player->put("type",ty);
                player->put("index",layer);
                player->put("number_of_neurons",nodeNum);
            } else if (linePrefix == "Input") {
                // Input nodes
                input++;
                std::string nodeName;
                double      value, up, low;
                inPic >> value;
                thisLineStream >> nodeName;
                var dstVar(nodeName);
                abst->add_var(dstVar);
                if (trobust != RACE) {
                    value = value * 0.00390625;
                }
                switch (trobust) {
                case RACE: {
                    if (nodeName == "in_0" || nodeName == "in_1") {
                        up = low = value;
                    } else {
                        up  = value + delta;
                        low = value - delta;
                    }
                    break;
                }
                case BRIGHTNESS:
                    low = value;
                    if (value < 1 - delta)
                        up = value;
                    else
                        up = 1;
                    break;
                case LINFBALL:
                    if (input <= 28 * 3 || input >= 28 * 26 || input - input / 28 * 28 <= 3 ||
                        input - input / 28 * 28 >= 26) {
                        up  = value;
                        low = value;
                    } else {
                        up  = value + delta < 1 ? value + delta : 1;
                        low = value - delta < 0 ? 0 : value - delta;
                    }
                    break;
                case CUSTOMBOX:
                    static int cnt = 0;
                    low            = input_box[cnt].first;
                    up             = input_box[cnt].second;
                    //std::cout << "(" << cnt << ") : [" << low << "," << up << "]" << std::endl;
                    cnt++;
                    break;
                default:
                    throw "unknow robust type";
                }
                ofre << nodeName << " " << low << " " << up << std::endl;
                std::cout << nodeName << " " << low << " " << up << std::endl;

                ptree pbound;
                pbound.put("inf",low);
                pbound.put("sup",up);
                bound_of_neurons->add_child(nodeName,pbound);

                environment env    = abst->get_environment();
                unsigned    dim    = env.get_dim(dstVar);
                unsigned    size   = env.get_vars().size();
                coeff*      tcoeff = new coeff[size];
                tcoeff[dim]        = get_coeff(1);
                linexpr0 t_lexpr0(size, tcoeff, get_coeff(-low));
                lincons0 t_lcons0(AP_CONS_SUPEQ, t_lexpr0);
                lincons1 lcons1(env, t_lcons0);
                abst->meet_lcons(lcons1);
                tcoeff[dim] = get_coeff(-1);
                t_lexpr0    = linexpr0(size, tcoeff, get_coeff(up));
                t_lcons0    = lincons0(AP_CONS_SUPEQ, t_lexpr0);
                lcons1      = lincons1(env, t_lcons0);
                abst->meet_lcons(lcons1);
                delete[] tcoeff;
                if (input == 784 && abst->type_name() == "Abst_ps") {
                    abst->powerset();
                }
            } else if (linePrefix == "MaxPool") {
                std::string nodeName;
                thisLineStream >> nodeName;
                var dstVar = var(nodeName);
                abst->add_var(dstVar);
                std::vector<var> pvars;
                while (!thisLineStream.eof()) {
                    std::string src;
                    thisLineStream >> src;
                    pvars.push_back(var(src));
                }
                std::vector<var>::iterator it;
                std::vector<var>::iterator it2;
                environment                env     = abst->get_environment();
                unsigned                   size    = env.get_vars().size();
                coeff*                     tcoeff  = new coeff[size];
                coeff*                     tcoeffA = new coeff[size];
                ABST*                      ori     = abst;
                for (it = pvars.begin(); it != pvars.end(); it++) {
                    ABST*    tmp    = new ABST(ori);
                    unsigned dim1   = env[*it];
                    tcoeff[dim1]    = get_coeff(1);
                    interval boundO = ori->get_bound(*it);
                    double   infO   = double(boundO.get_inf());
                    double   supO   = double(boundO.get_sup());
                    //		cout << "    " << *it << "[" << infO << "," << supO <<
                    //"]"
                    //<< endl;

                    for (it2 = pvars.begin(); it2 != pvars.end(); it2++) {
                        if (it == it2)
                            continue;
                        interval boundi = ori->get_bound(*it2);
                        double   infi   = double(boundi.get_inf());
                        double   supi   = double(boundi.get_sup());
                        //            cout << "        " << *it2 << "[" << infi
                        //            << "," << supi << "]" << endl;
                        if (infO >= supi)
                            continue;
                        unsigned dim2 = env[*it2];
                        tcoeff[dim2]  = get_coeff(-1);
                        linexpr0 t_lexpr0(size, tcoeff, get_coeff(0));
                        lincons0 t_lcons0(AP_CONS_SUPEQ, t_lexpr0);
                        lincons1 lcon1(env, t_lcons0);
                        //		  cout << "meet lcon "<< endl;
                        tmp->meet_lcons(lcon1);
                        tcoeff[dim2] = get_coeff(0);
                        if (tmp->is_bottom())
                            break;
                    }
                    tcoeff[dim1] = get_coeff(0);
                    if (!(tmp->is_bottom())) {
                        tcoeffA[dim1] = get_coeff(1);
                        linexpr0 a_lexpr0(size, tcoeffA, get_coeff(0));
                        linexpr1 a_lexpr1(env, a_lexpr0);
                        //		  cout << "tmp-> assign" <<endl;
                        tmp->assign(dstVar, a_lexpr1);
                    } else {
                        //		    cout << "tmp->is_bottom" << endl;
                    }
                    if (it != pvars.begin()) {
                        //            cout <<nodeName<<"  before join ";
                        interval fbound = abst->get_bound(dstVar);
                        //            cout << std::setprecision
                        //            (std::numeric_limits<double>::digits10 +
                        //            1)
                        //                 << double(fbound.get_inf()) <<'\t'
                        //                 << double(fbound.get_sup())
                        //                 <<'\t'<<std::endl;
                        abst->join(tmp);
                        //            cout <<nodeName<<" after join ";
                        //            fbound = abst->get_bound(dstVar);
                        //            cout  << std::setprecision
                        //            (std::numeric_limits<double>::digits10 +
                        //            1)
                        //                 << double(fbound.get_inf()) <<'\t'
                        //                 << double(fbound.get_sup())
                        //                 <<'\t'<<std::endl;
                        delete tmp;
                    } else
                        abst = tmp;
                    tcoeffA[dim1] = get_coeff(0);
                }
                delete[] tcoeff;
                delete[] tcoeffA;
                delete ori;
                ofre << nodeName << " ";
                interval fbound = abst->get_bound(dstVar);
                ofre << std::setprecision(std::numeric_limits<double>::digits10 + 1) << double(fbound.get_inf()) << '\t'
                     << double(fbound.get_sup()) << '\t' << std::endl;

                ptree pbound;
                pbound.put("inf",fbound.get_inf());
                pbound.put("sup",fbound.get_sup());
                bound_of_neurons->add_child(nodeName,pbound);

            } else if (linePrefix == "Linear") {
                std::string nodeName;
                thisLineStream >> nodeName;
                var dstVar(nodeName);
                abst->add_var(dstVar);
                double cst;
                thisLineStream >> cst;
                double      weight;
                environment env    = abst->get_environment();
                unsigned    size   = env.get_vars().size();
                coeff*      tcoeff = new coeff[size];
                while (!thisLineStream.eof()) {
                    thisLineStream >> weight;
                    std::string src;
                    thisLineStream >> src;
                    tcoeff[env[var(src)]] = get_coeff(weight);
                }
                linexpr0 a_lexpr0(size, tcoeff, get_coeff(cst));
                linexpr1 a_lexpr1(env, a_lexpr0);
                abst->assign(dstVar, a_lexpr1);
                delete[] tcoeff;
                ofre << nodeName << " ";
                interval bound = abst->get_bound(dstVar);
                ofre << std::setprecision(std::numeric_limits<double>::digits10 + 1) << double(bound.get_inf()) << '\t'
                     << double(bound.get_sup()) << '\t' << std::endl;

                ptree pbound;
                pbound.put("inf",bound.get_inf());
                pbound.put("sup",bound.get_sup());
                bound_of_neurons->add_child(nodeName,pbound);

                if (outLayer) {
                    curNum++;
                    boundFinal.push_back(bound);
                    outputvars.push_back(dstVar);

                    var target("target");
                    if (curNum == nodeNum) {
                        for (unsigned int i = 0; i < outputvars.size(); i++) {
                            Verified = true;
                            for (unsigned int j = 0; j < outputvars.size(); j++) {
                                if (i != j) {
                                    bool res = less(abst,outputvars[i],outputvars[j]);
                                    //std::cout << i << " < " << j << " : " << res << std::endl;
                                    if (res) {
                                        Verified = false;
                                        graph.push_back(PII(i,j));
                                    }
                                }
                            }
                            if(Verified) {
                                target = outputvars[i];
                            }
                        }
                        if (Verified) {
                            ofre << "Verified " << target << std::endl;
                        } else {
                            ofre << "Failed." << std::endl;
                        }
                        //   std::vector<interval>::iterator it =
                        //   boundFinal.begin(); double lb =
                        //   double(it->get_inf()); double ub =
                        //   double(it->get_sup());
                        // //   for(it = it + 1; it != boundFinal.end(); it++){
                        // //     if( Verified && lb <  double(it->get_sup()))
                        // Verified = false;
                        // //     if(! Verified){
                        // //       if (ub < double(it->get_inf())){
                        // // 	ub = double(it->get_sup());
                        // // 	lb = double(it->get_inf());
                        // // 	Verified = true;
                        // //       }
                        // //       else{
                        // // 	ub = ub >  double(it->get_sup())?
                        // ub:double(it->get_sup());
                        // //       }
                        // //     }
                        // //   }
                    }
                }

            } else if (linePrefix == "ReLU") {
                maxRelu++;
                std::string nodeName;
                thisLineStream >> nodeName;
                //	      cout << nodeName << endl;
                var dstVar(nodeName);
                abst->add_var(dstVar);
                double cst;
                thisLineStream >> cst;
                double      weight;
                environment env    = abst->get_environment();
                unsigned    size   = env.get_vars().size();
                coeff*      tcoeff = new coeff[size];
                while (!thisLineStream.eof()) {
                    thisLineStream >> weight;
                    std::string src;
                    thisLineStream >> src;
                    tcoeff[env[var(src)]] = get_coeff(weight);
                }
                linexpr0 a_lexpr0(size, tcoeff, get_coeff(cst));
                linexpr1 a_lexpr1(env, a_lexpr0);

                abst->assign(dstVar, a_lexpr1);
                delete[] tcoeff;
                /* std::string nodeName; */
                /* thisLineStream >> nodeName; */
                /* var dstVar(nodeName); */
                /* double cst; */
                /* std::string src; */
                /* thisLineStream >> cst >> cst >> src; */
                /* var srcVar(src); */
                /* abst->rename(srcVar,dstVar); */
                /* environment env = abst->get_environment(); */
                /* unsigned size = env.get_vars().size(); */
                interval bound = abst->get_bound(dstVar);
                double   inf   = double(bound.get_inf());
                double   sup   = double(bound.get_sup());
                ofre << nodeName << " ";
                ofre << std::setprecision(std::numeric_limits<double>::digits10 + 1) << inf << '\t' << sup << '\t'
                     << std::endl;

                ptree pbound;
                pbound.put("inf",inf);
                pbound.put("sup",sup);
                bound_of_neurons->add_child(nodeName,pbound);

                if (inf >= 0) {
                    inActive++;
                    continue;
                }
                if (sup < 0) {
                    inActive++;
                    coeff*   coff     = new coeff[size];
                    linexpr0 t_lexpr0 = linexpr0(size, coff, get_coeff(0));
                    abst->assign(dstVar, linexpr1(env, t_lexpr0));
                    delete[] coff;
                    continue;
                }
                ABST*  tmp        = new ABST(abst);
                coeff* coff       = new coeff[size];
                coff[env[dstVar]] = get_coeff(1);
                linexpr0 t_lexpr0(size, coff, get_coeff(0));
                lincons0 t_lcons0(AP_CONS_SUPEQ, t_lexpr0);
                abst->meet_lcons(lincons1(env, t_lcons0));
                coff[env[dstVar]] = get_coeff(-1);
                t_lexpr0          = linexpr0(size, coff, get_coeff(0));
                t_lcons0          = lincons0(AP_CONS_SUPEQ, t_lexpr0);
                tmp->meet_lcons(lincons1(env, t_lcons0));
                if (!tmp->is_bottom()) {
                    coff[env[dstVar]] = get_coeff(0);
                    t_lexpr0          = linexpr0(size, coff, get_coeff(0));
                    tmp->assign(dstVar, linexpr1(env, t_lexpr0));
                    abst->join(tmp);
                }
                //	      		ofre << "~~" << std::endl;
                //			interval boundtmp = abst->get_bound(dstVar);
                //	      double inftmp = double(boundtmp.get_inf());
                //	      double suptmp =  double(boundtmp.get_sup());
                //	      ofre<<nodeName<<" ";
                //	      ofre << std::setprecision
                //(std::numeric_limits<double>::digits10 + 1)
                //		   << inftmp <<'\t'
                //		   << suptmp <<'\t'<<std::endl;
                delete tmp;
                delete[] coff;
            } else if (linePrefix == "Assert") {
                std::cout << "assert encountered, but we do nothing" << std::endl;
            } else {
                std::ostringstream err;
                std::cout << "Error: Did not understand line prefix " << linePrefix;
                err << "Error: Did not understand line prefix " << linePrefix;
                throw err.str();
            }
        }
    }
    auto end_time = boost::posix_time::microsec_clock::local_time();
    auto time_d   = end_time - start_time;
    std::cout << time_d << std::endl;

    std::ofstream summary(summary_file_name, std::ios::app);
    summary << network << ",\t" << img << ",\t" << delta << ",\t" << (trobust == LINFBALL ? "PLANET" : "AI2") << ",\t"
            << dom_name(dom) << ",\t"

#ifdef NUM_D
            << "NUM_D,\t"
#endif

#ifdef NUM_MPQ
            << "NUM_MPQ,\t"
#endif

#ifdef ABST
            << ABST::type_name() << ",\t"
#endif
            << time_d << ",\t" << time_d.total_milliseconds() << ",\t" << end_time << ",\t" << maxRelu << ",\t"
            << inActive << ",\t" << Verified << std::endl;

    auto result = PII(outputvars.size(),graph);
    if(dump){
        result = Hasse(result);
        ptree pgraph = data2pgraph(result,outputvars);
        ptree presult;

        presult.add_child("graph",pgraph);

        player->add_child("bound_of_neurons",*bound_of_neurons);
        players.push_back(PII("",*player));
        presult.add_child("layers",players);

        std::ofstream json_file(json_file_name);
        write_json(json_file,presult, true);
        json_file.close();
    }
    return result;
}

bool reach_in_two_steps(const std::pair<unsigned long, std::vector<std::pair<int, int>>>& G, int u, int v) {
    const int number_of_nodes = G.first;
    const std::vector<std::pair<int, int>> & edges = G.second;
    std::vector<int> E[number_of_nodes];

    for (auto edge : edges) {
        E[edge.first].push_back(edge.second);
    }

    for (int t1 : E[u]) {
        for (int t2 : E[t1]) {
            if (t2 == v) {
                return true;
            }
        }
    }
    return false;
}

std::pair<unsigned long, std::vector<std::pair<int, int>>> Hasse(const std::pair<unsigned long, std::vector<std::pair<int, int>>>& G) {

    std::vector<std::pair<int, int>> hasse;

    int number_of_nodes = G.first;
    const std::vector<std::pair<int, int>> & edges = G.second;

    for (auto edge : edges) {
        int u = edge.first;
        int v = edge.second;
        if (! reach_in_two_steps(G,u,v)) {
            hasse.push_back(PII(u,v));
        }
    }

    return PII(number_of_nodes,hasse);
}

ptree data2pgraph(const std::pair<unsigned long, std::vector<std::pair<int, int>>>& data, std::vector<var> vars) {

    int number_of_nodes = data.first;
    const std::vector<std::pair<int, int>> & edges = data.second;

    ptree pgraph,pnames, pedges;

    for (int i = 0; i < number_of_nodes; ++i) {
        ptree pname;
        pname.put("",vars[i]);
        pnames.push_back(PII("",pname));
    }

    for (auto edge : edges){
        ptree pedge;
        pedge.put("from",edge.first);
        pedge.put("to",edge.second);
        pedges.push_back(PII("",pedge));
    }

    pgraph.put("number_of_nodes",number_of_nodes);
    pgraph.add_child("names",pnames);
    pgraph.add_child("edges",pedges);
    return pgraph;
}

#endif //__ABSEXE_H
