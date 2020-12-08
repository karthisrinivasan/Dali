//
// Created by Yihang Yang on 2019-05-14.
//

#include <ctime>
#include <unistd.h>

#include <iostream>
#include <vector>

#include "circuit.h"
#include "placer.h"

#define TEST_LG 0
#define TEST_PO 0
#define TEST_WELL 0
#define TEST_CLUSTER_WELL 0
#define TEST_STDCLUSTER_WELL 1

using namespace dali;

int main() {
  //PrintSoftwareStatement();
  Circuit circuit;

  //int num_of_thread_galois = 6;
  //galois::SharedMemSys G;
  //galois::preAlloc(num_of_thread * 2);
  //galois::setActiveThreads(num_of_thread_galois);

  //BOOST_LOG_TRIVIAL(info)  <<"Galois thread %d\n", num_of_thread);

  int num_of_thread_openmp = 6;
  omp_set_num_threads(num_of_thread_openmp);

  Eigen::initParallel();
  //Eigen::setNbThreads(1);
  BOOST_LOG_TRIVIAL(info)  <<"Eigen thread " << Eigen::nbThreads() << "\n";

  double wall_time = get_wall_time();

  std::string lef_file_name = "processor100.lef";
  std::string def_file_name = "processor100.def";

  // read LEF/DEF

  BOOST_LOG_TRIVIAL(info)   << "File loading complete, time: " << double(get_wall_time() - wall_time)  << " s" << std::endl;
  BOOST_LOG_TRIVIAL(info)  <<"  Average white space utility: " << circuit.WhiteSpaceUsage() << "\n";
  circuit.ReportBriefSummary();
  //circuit.ReportBlockType();
  //circuit.ReportIOPin();
  circuit.ReportHPWL();
  circuit.BuildBlkPairNets();

  Placer *gb_placer = new GPSimPL;
  gb_placer->SetInputCircuit(&circuit);

  gb_placer->SetBoundaryDef();
  gb_placer->SetFillingRate(0.67);
  gb_placer->ReportBoundaries();
  gb_placer->StartPlacement();
  //gb_placer->SaveDEFFile("benchmark_1K_dali.def", def_file_name);
  gb_placer->GenMATLABTable("gb_result.txt");
  circuit.GenLongNetTable("gb_longnet.txt");
  //gb_placer->GenMATLABWellTable("gb_result");
  //circuit.ReportNetFanoutHisto();

  /*Placer *d_placer = new MDPlacer;
  d_placer->TakeOver(gb_placer);
  d_placer->StartPlacement();
  d_placer->GenMATLABScript("dp_result.txt");*/

  Placer *legalizer = new LGTetrisEx;
  legalizer->TakeOver(gb_placer);
  legalizer->StartPlacement();
  legalizer->GenMATLABTable("lg_result.txt");
  circuit.GenLongNetTable("lg_longnet.txt");
  //legalizer->SaveDEFFile("circuit.def", def_file);

#if TEST_PO
  Placer *post_optimizer = new PLOSlide;
  post_optimizer->TakeOver(legalizer);
  post_optimizer->StartPlacement();
  post_optimizer->GenMATLABTable("po_result.txt");
  delete post_optimizer;
#endif

#if TEST_WELL
  std::string cell_file_name("benchmark_1K.cell");
  circuit.ReadCellFile(cell_file_name);
  Placer *well_legalizer = new WellLegalizer;
  well_legalizer->TakeOver(gb_placer);
  well_legalizer->StartPlacement();
  circuit.GenMATLABWellTable("lg_result");
  delete well_legalizer;
#endif

#if TEST_CLUSTER_WELL
  Placer *cluster_well_legalizer = new ClusterWellLegalizer;
  std::string cell_file_name("benchmark_1K.cell");
  circuit.ReadCellFile(cell_file_name);
  cluster_well_legalizer->TakeOver(gb_placer);
  cluster_well_legalizer->StartPlacement();
  delete cluster_well_legalizer;
#endif

#if TEST_STDCLUSTER_WELL
  StdClusterWellLegalizer std_cluster_well_legalizer;
  std::string cell_file_name("processor100.cell");
  circuit.ReadCellFile(cell_file_name);
  std_cluster_well_legalizer.TakeOver(gb_placer);
  std_cluster_well_legalizer.StartPlacement();
  //std_cluster_well_legalizer.GenMatlabClusterTable("sc_result");
  std_cluster_well_legalizer.GenMATLABTable("sc_result.txt");
  circuit.GenLongNetTable("sc_longnet.txt");
  std_cluster_well_legalizer.GenMatlabClusterTable("sc_result");
  std_cluster_well_legalizer.GenMATLABWellTable("scw", 0);

  std_cluster_well_legalizer.SimpleIOPinPlacement(0);
  std_cluster_well_legalizer.EmitDEFWellFile("circuit", 1);
#endif

  circuit.SaveDefFile("circuit", "", def_file_name, 1, 1, 2, 1);
  circuit.SaveDefFile("circuit", "_io", def_file_name, 1, 1, 1, 1);
  circuit.SaveDefFile("circuit", "_filling", def_file_name, 1, 4, 2, 1);
  circuit.InitNetFanoutHistogram();
  circuit.ReportNetFanoutHistogram();
  circuit.ReportHPWLHistogramLinear();
  circuit.ReportHPWLHistogramLogarithm();

  delete gb_placer;
  delete legalizer;

  circuit.SaveOptimalRegionDistance();

/*#ifdef USE_OPENDB
  odb::dbDatabase::destroy(db);
#endif*/

  wall_time = get_wall_time() - wall_time;
  BOOST_LOG_TRIVIAL(info)   << "Execution time " << wall_time << "s.\n";

  return 0;
}