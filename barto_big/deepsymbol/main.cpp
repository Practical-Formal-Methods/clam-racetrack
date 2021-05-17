/*********************                                                        */
/*! \file absEngine.h
 ** Top contributors (to current version):
 **   Jiangchao Liu
 ** This file is part of the AI & SMTReluplex project.
 ** Copyright (c) 2018-2100 by the authors listed in the file AUTHORS
 ** (in the top-level source directory) and their institutional affiliations.
 ** All rights reserved. See the file COPYING in the top-level source
 ** directory for licensing information.\endverbatim
 **/
#include <boost/date_time.hpp>
#include <csignal>
#include <iostream>
#include <string>
#include <sys/resource.h>
#include <boost/program_options.hpp>

#include "absExe.h"
#include "middleware.h"

namespace po = boost::program_options;

std::string inFileName      = "";
std::string summaryFileName = "";
std::string ans             = "";

void got_signal(int) {
    printf("Got signal\n");

    if (summaryFileName != "") {

        //        auto current = boost::posix_time::microsec_clock::local_time();
        //        auto totalTime = current - problem.startTime;
        //
        //        long long initialTime = problem.initialWorkTime.total_milliseconds();
        //        long long solveTime   = problem.solveTime.total_milliseconds();
        //        long long total = totalTime.total_milliseconds() ;
        //
        //
        //        std::ofstream summary(summaryFileName,std::ios::app);
        //        auto p = inFileName.find_last_of('/');
        //        summary << inFileName.substr(p+1,inFileName.length()) << " ,"
        //                << "TIMEOUT , "
        //                << totalTime << " ,"
        //                << total << " ,"
        //                << initialTime << " ,"
        //                << solveTime << " ,"
        //                << problem.cacheSize()<< std::endl;
        //        summary.close();
    }

    exit(0);
}

//
// int acas(){
//  std::string path("/home/jiangchao/AIProjects/reluplex/ReluplexCav2017/nnet/ACASXU_run2a_1_2_batch_2000.nnet");
//  //std::string path("/home/jiangchao/AIProjects/AIsmt/ReluplexCav2017/nnet/test.nnet");
//  Log_info("Build an Acas network");
//  Dnn *adnn = new Dnn(Acas, path);
//  Log_info("Get the dims in the input layer");
//  int inputDim = adnn->get_inputdim();
//  std::cout<<"dim is"<<inputDim<<std::endl;
//  Log_info("Build an engine based on abstract interpretation with an Acas network");
//  AbsEngine *aeng = new AbsEngine(adnn);
//  std::vector<std::string> name(inputDim);
//  int i;
//  for(i = 0; i < inputDim; i++){
//    name[i] =  "dim" + std::to_string(i);
//  }
//  Log_info("Introduce contriants on inputs");
//  double *mins = adnn->get_mins();
//  double *maxes = adnn->get_maxes();
//  Matrix amatrix(inputDim);
//  // for(int i = 0; i < inputDim; i++){
//  //   amatrix.set_uperbound(i,maxes[i]);
//  //    amatrix.set_lowerbound(i,mins[i]);
//  // }
//  // amatrix.set_lowerbound(0,55947);
//  // amatrix.set_lowerbound(3,1145);
//  // amatrix.set_uperbound(4,60);
//
//  // amatrix.set_lowerbound(0,-0.2000);
//  // amatrix.set_uperbound(0,0.00000);
//  // amatrix.set_lowerbound(1,0.027465);
//  // amatrix.set_uperbound(1,0.227465);
//  // amatrix.set_lowerbound(2,-0.200703);
//  // amatrix.set_uperbound(2,-0.000703);
//  // amatrix.set_lowerbound(3,0.053843);
//  // amatrix.set_uperbound(3,0.253843);
//  // amatrix.set_lowerbound(4,-0.20000);
//  // amatrix.set_uperbound(4,-0.00000);
//
//
//
//  // amatrix.set_lowerbound(0,-0.02000);
//  // amatrix.set_uperbound(0,0.00000);
//  // amatrix.set_lowerbound(1,0.0);
//  // amatrix.set_uperbound(1,0.02);
//  // amatrix.set_lowerbound(2,-0.0200);
//  // amatrix.set_uperbound(2,-0.000);
//  // amatrix.set_lowerbound(3,0.0);
//  // amatrix.set_uperbound(3,0.02);
//  // amatrix.set_lowerbound(4,-0.220000);
//  // amatrix.set_uperbound(4,-0.20000);
//
//
//  amatrix.set_lowerbound(0,-0.1000);
//  amatrix.set_uperbound(0,-0.10000);
//  amatrix.set_lowerbound(1,0.127465);
//  amatrix.set_uperbound(1,0.127465);
//  amatrix.set_lowerbound(2,-0.100703);
//  amatrix.set_uperbound(2,-0.100703);
//  amatrix.set_lowerbound(3,0.153843);
//  amatrix.set_uperbound(3,0.153843);
//  amatrix.set_lowerbound(4,-0.10000);
//  amatrix.set_uperbound(4,-0.10000);
//
//  double **coffs = amatrix.get_coff();
//  double **bias = amatrix.get_bias();
//  LogicOP *lop =  amatrix.get_lop();
//  int num = amatrix.get_num();
//  std::cout<<"num of cons is "<<num<<std::endl;
//  std::cout<<"before building an absele"<<std::endl;
//  AbsEle *aele = new AbsEle(inputDim,name,BOX,num,coffs,lop,bias);
//  //  std::cout<<"after building an absele, the ales is "<<*aele<<std::endl<<std::flush;
//
//  aeng->iterateWholeNet(aele);
//  std::cout<<"computation finished"<<std::endl;
//  //  std::cout<<*aele<<std::endl;
//  std::cout<<"unsplit is "<<unsplit<<std::endl;
//  std::cout<<"split is "<<split<<std::endl;
//  return 0;
//}

static void show_usage(std::string name) {
    std::cerr << "Usage: " << name << std::endl
              << "Options:\n"
              << "\t-h,--help  \t\tShow this help message\n"
              << "\t-d,--delta  \t\tDelta value, a integer (delta %%) \n"
              << "\t-t,--type   \t\tRobustness type : AI2 / PLANET\n"
              //              << "\t-m,--domain \t\tAbstract domain : BOX / TOPO\n"
              << "\t-n,--net    \t\tInputFile, RLV format file\n"
              << "\t-p,--point  \t\tFix input point: a 28*28 MNIST img in integer format\n"
              << "\t-b,--bound  \t\tOutput file, stores the bounds\n"
              << "\t-s,--summary\t\tSummary file: store the statistics\n"
              << std::endl;
}

std::istream& operator>>(std::istream& in, ROBUST& robust)
{
    std::string token;
    in >> token;
    if (token == "BRIGHTNESS")
        robust = BRIGHTNESS;
    else if (token == "LINFBALL")
        robust = LINFBALL;
    else if (token == "CUSTOMBOX")
        robust = CUSTOMBOX;
    else
        in.setstate(std::ios_base::failbit);
    return in;
}

int main(int argc, char* argv[]) {

    // ./deepsymbol --robust LINFBALL --network ../../AIsmt/caffeprototxt/AI2_MNIST_FNN_1/testNetworkB.rlv --image ../../AIsmt/scripts/input/3/1746.txt --delta 0.01 --dumpJSON ./tmp.json
    std::ios::sync_with_stdio(false);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = got_signal;
    sigfillset(&sa.sa_mask);
    sigaction(SIGTERM, &sa, NULL);
    const rlim_t  kStackSize = 128L * 1024L * 1024L;
    struct rlimit rl;
    int           result = getrlimit(RLIMIT_STACK, &rl);
    if (result == 0) {
        if (rl.rlim_cur < kStackSize) {
            rl.rlim_cur = kStackSize;
            result      = setrlimit(RLIMIT_STACK, &rl);
            if (result != 0) {
                std::cout << "stack limit reset faild" << std::endl;
            }
        }
    }

    double      delta       = 0.0;
    ROBUST      rob_type    = LINFBALL;
    Domain      domain_type = BOX;
    std::string net_file_name;
    std::string img_file_name;
    std::string bound_file_name;
    std::string summary_file_name;
    std::string dumpJSON_file_name;
#ifdef BDOMAIN
    domain_type = BOX;
#endif

#ifdef TDOMAIN
    domain_type = TOPO;
#endif

#ifdef EZDOMAIN
    domain_type = EZONO;
#endif

    try {

        po::options_description desc("Options");
        desc.add_options()
            ("help", "produce help message")
            ("robust",po::value<ROBUST>(),"robustness type, valid value : BRIGHTNESS, LINFBALL or CUSTOMBOX")
            ("network", po::value<std::string>(), "path to rlv format network")
            ("image", po::value<std::string>(), "input image of the network")
            ("box", po::value<std::string>(), "normalized box constraints of the input of the network")
            ("delta", po::value<double>(), "float between 0 and 1")
            ("dumpJSON", po::value<std::string>(), "path to dump the results in JSON format")
            ;
            // TODO
            //  mean : specifies mean used to normalize the data. If the data has multiple channels the mean for every channel has to be provided (e.g. for cifar10 --mean 0.485, 0.456, 0.406) (default is 0 for non-geometric mnist and 0.5 0.5 0.5 otherwise)
            //  std : specifies standard deviation used to normalize the data. If the data has multiple channels the standard deviaton for every channel has to be provided (e.g. for cifar10 --std 0.2 0.3 0.2) (default is 1 1 1)

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            std::cout << desc << "\n";
            return 0;
        }

        if (vm.count("robust")) {
            rob_type = vm["robust"].as<ROBUST>();
        } else {
            std::cout << "robustness type undefined.\n";
            return 0;
        }

        if (vm.count("network")) {
            net_file_name = vm["network"].as<std::string>();
        } else {
            std::cout << "network path undefined.\n";
            return 0;
        }

        if (vm.count("dumpJSON")) {
            dumpJSON_file_name = vm["dumpJSON"].as<std::string>();
        }

        switch (rob_type) {
        case LINFBALL : case BRIGHTNESS : {
                if (vm.count("image")) {
                    img_file_name = vm["image"].as<std::string>();
                } else {
                    std::cout << "image path undefined.\n";
                    return 0;
                }
                if (vm.count("delta")) {
                    delta = vm["delta"].as<double>();
                } else {
                    std::cout << "delta undefined.\n";
                    return 0;
                }
                middleware::run(net_file_name,rob_type,img_file_name,delta,dumpJSON_file_name);
                break;
            }
            case CUSTOMBOX : {
                if (vm.count("box")) {
                    std::string box_file_name = vm["box"].as<std::string>();
                    std::ifstream ibox(box_file_name);
                    darguments box;
                    double a,b;
                    while (ibox >> a >> b) {
                        box.push_back(PII(a,b));
                    }
                    middleware::run(net_file_name,box,dumpJSON_file_name);
                } else {
                    std::cout << "box file path undefined.\n";
                    return 0;
                }
                break;
            }
            default: {
                return 0;
            }
        }

    }
    catch(std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    }
    catch(...) {
        std::cerr << "Exception of unknown type!\n";
    }
//
//    if (argc == 1) {
//        // std::string img("fixinput.txt");
//        // std::string net("net.rlv");
//
//        std::string img("767.txt");
//        std::string net("../caffeprototxt/AI2_MNIST_FNN_7/testNetworkB.rlv");
//
//        //  std::string net("test.rlv");
//        // std::string net("testNetworkB.rlv");
//
//        //  std::string net("MNISTAI2_new.rlv");
////        robust_rlv(img, BRIGHTNESS, 0.6, net, domain_type, "bounds.txt", "summary.txt");
//        //  acas();
//        return 0;
//    }
//
//    for (int i = 1; i < argc; ++i) {
//        std::string arg = argv[i];
//        if ((arg == "-h") || (arg == "--help")) {
//            show_usage(argv[0]);
//            return 0;
//        } else if ((arg == "-d") || (arg == "--delta")) {
//            if (i + 1 < argc) {
//                int d = std::atol(argv[++i]);
//                //                if (delta > 1) {
//                delta = d / 1000.0;
//                //                }
//                //                cout << delta << endl;
//            } else {
//                std::cerr << "--delta option requires one argument." << std::endl;
//                return 1;
//            }
//        } else if ((arg == "-t") || (arg == "--type")) {
//            if (i + 1 < argc) {
//                std::string type_str = std::string(argv[++i]);
//                // cout << type_str << endl;
//                if (type_str == "PLANET") {
//                    rob_type = LINFBALL;
//                } else if (type_str == "AI2") {
//                    rob_type = BRIGHTNESS;
//                } else if (type_str == "RACE") {
//                    rob_type = RACE;
//                } else {
//                    std::cerr << "--type option wrong argument." << std::endl;
//                    return 1;
//                }
//            } else {
//                std::cerr << "--type option requires one argument." << std::endl;
//                return 1;
//            }
//            //        } else if ((arg == "-m") || (arg == "--domain")) {
//            //            if (i + 1 < argc) {
//            //                string type_str = string(argv[++i]);
//            //                //cout << type_str << endl;
//            //                if (type_str == "BOX") {
//            //                    domain_type = BOX;
//            //                }
//            //                else if (type_str == "TOPO") {
//            //                    domain_type == TOPO;
//            //                }
//            //                else {
//            //                    std::cerr << "--domain option wrong argument." << std::endl;
//            //                    return 1;
//            //                }
//            //            } else {
//            //                std::cerr << "--type option requires one argument." << std::endl;
//            //                return 1;
//            //            }
//        } else if ((arg == "-n") || (arg == "--net")) {
//            if (i + 1 < argc) {
//                net_file_name = std::string(argv[++i]);
//            } else {
//                std::cerr << "--net option requires one argument." << std::endl;
//                return 1;
//            }
//        } else if ((arg == "-p") || (arg == "--point")) {
//            if (i + 1 < argc) {
//                img_file_name = std::string(argv[++i]);
//            } else {
//                std::cerr << "--point option requires one argument." << std::endl;
//                return 1;
//            }
//        } else if ((arg == "-b") || (arg == "--bound")) {
//            if (i + 1 < argc) {
//                bound_file_name = std::string(argv[++i]);
//            } else {
//                std::cerr << "--bound option requires one argument." << std::endl;
//                return 1;
//            }
//        } else if ((arg == "-s") || (arg == "--summary")) {
//            if (i + 1 < argc) {
//                summary_file_name = std::string(argv[++i]);
//            } else {
//                std::cerr << "--summary option requires one argument." << std::endl;
//                return 1;
//            }
//        } else {
//            return 1;
//        }
//    }
//
////    robust_rlv(img_file_name, rob_type, delta, net_file_name, domain_type, bound_file_name, summary_file_name);
}
