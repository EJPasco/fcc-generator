/// Generator of user defined decays
/// Uses PYTHIA to generate initial collision and then EvtGen to decay produced particles in user defined way
/// Uses FCC-ee data model and HepMC event model as intermediate layer to transfer data from PYTHIA to PODIO (that takes care of storing data)
/// Stores data in a ROOT file

// Configuration
#include "GeneratorConfig.h"

// PODIO
#include "podio/EventStore.h"
#include "podio/ROOTWriter.h"

// Data model
#include "datamodel/EventInfo.h"
#include "datamodel/EventInfoCollection.h"
#include "datamodel/MCParticle.h"
#include "datamodel/MCParticleCollection.h"
#include "datamodel/GenVertex.h"
#include "datamodel/GenVertexCollection.h"

// STL
#include <iostream>
#include <iomanip>
#include <string>
#include <cstdlib>
#include <stdexcept>
#include <unordered_map>
#include <cmath>
#include <chrono>
#include <algorithm>

// PYTHIA, EvtGen and HepMC
#include "Pythia8/Pythia.h"
#include "Pythia8Plugins/HepMC2.h"
#include "Pythia8Plugins/EvtGen.h"

#ifdef USE_BOOST
	// Boost
	#include "boost/program_options.hpp"
#endif

std::unordered_map<int, std::string> const particle_names = {{511, "B_d^0"},
												   {-511, "Anti-B_d^0"},
												   {531, "B_s^0"},
												   {-531, "Anti-B_s^0"},
												   {313, "K^*0"},
												   {313, "Anti-K^*0"},
											       {15, "tau-"},
											       {-15, "tau+"},
											       {321, "K^+"},
												   {-321, "K^-"},
												   {211, "pi^+"},
												   {-211, "pi^-"},
										    	   {16, "nu_tau"},
									   			   {-16, "Anti-nu_tau"},
								   				   {431, "D_s^+"},
							   					   {-431, "D_s^-"}};

bool isBAtProduction(HepMC::GenParticle const * thePart); // utility function to determine whether the particle is NOT a B oscillation. Stolen from https://lhcb-release-area.web.cern.ch/LHCb-release-area/DOC/rec/latest_doxygen/da/db4/_hep_m_c_utils_8h_source.html

int main(int argc, char * argv[]){
	std::string evtgen_root = std::getenv("EVTGEN_ROOT_DIR"); // path to EvtGen installation directory

	// declaring and initializing some variables. Most of them will be set according to command line options passed to the program after parsing of command line arguments. However, if Boost is not used, the only available command line option is the number of events to generate; other variables will use the values set below
	std::size_t nevents = 0; // number of events to generate
	std::string pythia_cfgfile = "pythia.cmnd"; // name of PYTHIA cofiguration file
	int keyptc = 511; // "key" particle
	std::string evtgen_decfile = evtgen_root + "/share/DECAY_2010.DEC"; // EvtGen decay file
	std::string evtgen_pdlfile = evtgen_root + "/share/evt.pdl"; // EvtGen PDL file
	std::string evtgen_user_decfile = "user.dec"; // user defined decays
	std::string output_filename = "output.root"; // name of the output file
	std::size_t verbosity = 0; // verbosity level

	#ifdef USE_BOOST
		try {
			boost::program_options::options_description desc("Usage");

			// defining command line options. See boost::program_options documentation for more details
			desc.add_options()
							("help", "produce this help message")
							("nevents,n", boost::program_options::value<std::size_t>(&nevents), "number of events to generate")
							("keyparticle,k", boost::program_options::value<int>(&keyptc)->default_value(511), "PDG ID of \"key\" particle (the one the redefined decay chain starts with)")
							("pythiacfg,P", boost::program_options::value<std::string>(&pythia_cfgfile)->default_value("pythia.cmnd"), "PYTHIA config file")
							("customdec,E", boost::program_options::value<std::string>(&evtgen_user_decfile)->default_value("user.dec"), "EvtGen user decay file")
							("evtgendec", boost::program_options::value<std::string>(&evtgen_decfile)->default_value(evtgen_root + "/share/DECAY_2010.DEC"), "EvtGen decay file")
							("evtgenpdl", boost::program_options::value<std::string>(&evtgen_pdlfile)->default_value(evtgen_root + "/share/evt.pdl"), "EvtGen PDL file")
							("outfile,o", boost::program_options::value<std::string>(&output_filename)->default_value("output.root"), "Output file")
							("verbosity,v", boost::program_options::value<std::size_t>(&verbosity)->implicit_value(1), "Set verbosity level (0, 1, 2)")
			;
			boost::program_options::variables_map vm;
			boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
			boost::program_options::notify(vm);

			if(vm.find("help") != vm.end() || argc < 2) {
				std::cout << "Generator of forced user-defined decays. Version " << Generator_VERSION_MAJOR << '.' << Generator_VERSION_MINOR << std::endl;
				std::cout << desc << std::endl;

				return EXIT_SUCCESS;
			}

			if(vm.find("verbosity") != vm.end()) {
				verbosity = vm.at("verbosity").as<size_t>();
			}
		} catch(std::exception const & e) {
			std::cerr << "Exception thrown during options parsing:" << std::endl << e.what() << std::endl;

			return EXIT_FAILURE;
		}
	#else
		if(argc < 2) {
			std::cout << "Generator of forced user-defined decays. Version " << Generator_VERSION_MAJOR << '.' << Generator_VERSION_MINOR << std::endl;
			std::cout << "Usage: " << argv[0] << " n, where \"n\" is a number of events to generate" << std::endl;
			std::cout << "WARNING! This version of the generator does not use program options parser, which means that you are personally responsible for providing correct options to this program." << std::endl;

			return EXIT_SUCCESS;
		} else {
			nevents = std::stoull(argv[1]);
		}
	#endif

	if(verbosity >= 1) {
			std::cout << "PYTHIA config file: \"" << pythia_cfgfile << "\"" << std::endl
					<< "EvtGen user decay file: \"" << evtgen_user_decfile << "\"" << std:: endl
					<< "EvtGen decay file: \"" << evtgen_decfile << "\"" << std:: endl
					<< "EvtGen PDL file: \"" << evtgen_pdlfile << "\"" << std:: endl
					<< nevents << " events will be generated." << std:: endl;
	}

	if(verbosity >= 1) {
		std::cout << "Prepairing data store" << std::endl;
	}

	// prepairing event store
	podio::EventStore store;
	podio::ROOTWriter writer(output_filename, &store);

	// creating collections
	auto & evinfocoll = store.create<fcc::EventInfoCollection>("EventInfo");
	auto & pcoll = store.create<fcc::MCParticleCollection>("GenParticle");
	auto & vcoll = store.create<fcc::GenVertexCollection>("GenVertex");

	// registering collections
	writer.registerForWrite<fcc::EventInfoCollection>("EventInfo");
	writer.registerForWrite<fcc::MCParticleCollection>("GenParticle");
	writer.registerForWrite<fcc::GenVertexCollection>("GenVertex");

	if(verbosity >= 1) {
		std::cout << "Initializing PYTHIA" << std::endl;
	}

	// initializing PYTHIA
	Pythia8::Pythia pythia; // creating PYTHIA generator object
	pythia.readFile(pythia_cfgfile); // reading settings from file

	pythia.init(); // initializing PYTHIA generator

	if(verbosity >= 1) {
		std::cout << "Initializing EvtGen" << std::endl;
	}

	//
	auto evtgen = new EvtGenDecays(&pythia, evtgen_decfile.c_str(), evtgen_pdlfile.c_str(), nullptr, nullptr, 1, false, true, true, false); // creating EvtGen generator
	if (evtgen) {
		evtgen->readDecayFile(evtgen_user_decfile.c_str()); // reading user defined decays
		evtgen->exclude(23); // make PYTHIA itself (not EvtGen) decay Z
	} else {
		std::cerr << "Unable to initialize EvtGen. Program stopped." << std::endl;
		return EXIT_FAILURE;
	}

	// interface for conversion from Pythia8::Event to HepMC event.
	HepMC::Pythia8ToHepMC ToHepMC;

	auto generation_start_time = std::chrono::system_clock::now(); // time of beginning of the generation
	auto last_timestamp = generation_start_time; // time of last time check

	std::size_t keyptc_counter = 0; // number of events containing "key" particle generated so far
	std::size_t total = 0; // total number of events generated so far

	while(keyptc_counter < nevents) {
		if(pythia.next()) {
			++total;

			evtgen->decay(); // performing user defined decays in EvtGen

			// creating HepMC event storage
			HepMC::GenEvent * hepmcevt = new HepMC::GenEvent(HepMC::Units::GEV, HepMC::Units::MM);

			// converting generated event to HepMC format
			ToHepMC.fill_next_event(pythia, hepmcevt);

			auto keyptc_in_event = std::count_if(hepmcevt->particles_begin(), hepmcevt->particles_end(), [keyptc](HepMC::GenParticle const * const ptc_ptr) {return std::abs(ptc_ptr->pdg_id()) == keyptc && isBAtProduction(ptc_ptr);});
			if(keyptc_in_event > 0) {
				keyptc_counter += keyptc_in_event;

				if(verbosity >= 2) {
					hepmcevt->print();

					std::cout << keyptc_counter << " events with " << ((particle_names.find(keyptc) != particle_names.end()) ? particle_names.at(keyptc) : std::to_string(keyptc)) << " production have been generated (" << total << " total)" << std::endl;
					auto time_taken = std::chrono::duration<double>(std::chrono::system_clock::now() - last_timestamp).count();
					std::cout << "Time taken: " << time_taken << " s. Current rate: " << 1. / time_taken << " ev / s" << std::endl;

					last_timestamp = std::chrono::system_clock::now();
				} else {
					if(verbosity >= 1 && keyptc_counter % 100 == 0) {
						std::cout << keyptc_counter << " events with " << ((particle_names.find(keyptc) != particle_names.end()) ? particle_names.at(keyptc) : std::to_string(keyptc)) << " production have been generated (" << total << " total)" << std::endl;
						auto time_taken = std::chrono::duration<double>(std::chrono::system_clock::now() - last_timestamp).count();
						std::cout << "Time taken: " << time_taken << " s. Current rate: " << 100. / time_taken << " ev / s" << std::endl;

						last_timestamp = std::chrono::system_clock::now();
					}
				}

				// filling event info
				auto evinfo = fcc::EventInfo();
				evinfo.Number(keyptc_counter); // Number takes int as its parameter, so here's a narrowing conversion (std::size_t to int). Should be safe unless we get 2^32 events or more. Then undefined behaviour
				evinfocoll.push_back(evinfo);

				// filling vertices
				std::unordered_map<HepMC::GenVertex *, fcc::GenVertex> vtx_map;
				for(auto iv = hepmcevt->vertices_begin(), endv = hepmcevt->vertices_end(); iv != endv; ++iv) {
					auto vtx = fcc::GenVertex();
					vtx.Position().X = (*iv)->position().x();
					vtx.Position().Y = (*iv)->position().y();
					vtx.Position().Z = (*iv)->position().z();
					vtx.Ctau((*iv)->position().t());
					vtx_map.emplace(*iv, vtx);

					vcoll.push_back(vtx);
				}

				// filling particles
				for(auto ip = hepmcevt->particles_begin(), endp = hepmcevt->particles_end(); ip != endp; ++ip) {
					auto ptc = fcc::MCParticle();
					auto & core = ptc.Core();
					core.Type = (*ip)->pdg_id();
					core.Status = (*ip)->status();

					core.Charge = pythia.particleData.charge(core.Type); // PYTHIA returns charge as a double value (in case it's quark), so here's a narrowing conversion (double to int), but here it's safe
					core.P4.Mass = (*ip)->momentum().m();
					core.P4.Px = (*ip)->momentum().px();
					core.P4.Py = (*ip)->momentum().py();
					core.P4.Pz = (*ip)->momentum().pz();

					auto prodvtx = vtx_map.find((*ip)->production_vertex());
					if(prodvtx != vtx_map.end()) {
						ptc.StartVertex(prodvtx->second);
					}
					auto endvtx = vtx_map.find((*ip)->end_vertex());
					if(endvtx != vtx_map.end()) {
						ptc.EndVertex(endvtx->second);
					}

					if(verbosity >= 2) {
						auto const & pdg_id = ptc.Core().Type;
						std::cout << "Stored particle: " << pdg_id << (particle_names.find(pdg_id) != particle_names.end() ? std::string(" (") + particle_names.at(pdg_id) + ")" : "") << std::endl;

						auto const & p4 = ptc.Core().P4;
						std::cout << std::setprecision(12) << "\tP4: (Px = " << p4.Px << ", Py = " << p4.Py << ", Pz = " << p4.Pz << ", Mass = " << p4.Mass << ")" << std::endl;

						if(ptc.StartVertex().isAvailable()) {
							auto const & svtx = ptc.StartVertex().Position();
							std::cout << std::setprecision(12) << "\tProduction vertex: (X = " << svtx.X << ", Y = " << svtx.Y << ", Z = " << svtx.Z << ")" << std::endl;
						} else {
							std::cout << "\tProduction vertex is not valid" << std::endl;
						}

						if(ptc.EndVertex().isAvailable()) {
							auto const & evtx = ptc.EndVertex().Position();
							std::cout << std::setprecision(12) << "\tDecay vertex: (X = " << evtx.X << ", Y = " << evtx.Y << ", Z = " << evtx.Z << ")" << std::endl;
						} else {
							std::cout << "\tDecay vertex is not valid" << std::endl;
						}

						if(ptc.StartVertex().isAvailable() && ptc.EndVertex().isAvailable()) {
							auto const & svtx = ptc.StartVertex().Position(), evtx = ptc.EndVertex().Position();

							std::cout << std::setprecision(12) << "\tFlight distance: " << std::sqrt((evtx.X - svtx.X) * (evtx.X - svtx.X) + (evtx.Y - svtx.Y) * (evtx.Y - svtx.Y) + (evtx.Z - svtx.Z) * (evtx.Z - svtx.Z)) << "mm" << std::endl;
						}

					}
					pcoll.push_back(ptc);
				}

				writer.writeEvent();
				store.clearCollections();
			}

			// freeing resources
			if(hepmcevt) {
				delete hepmcevt;
				hepmcevt = nullptr;
			}
		}
	}

	auto elapsed_time = std::chrono::duration<double>(std::chrono::system_clock::now() - generation_start_time).count();

	writer.finish();

	// freeing resources
	if(evtgen) {
		delete evtgen;
		evtgen = nullptr;
	}

	std::cout << keyptc_counter << " events with production of " << ((particle_names.find(keyptc) != particle_names.end()) ? particle_names.at(keyptc) : std::to_string(keyptc)) << " have been generated (" << total << " total)." << std::endl;
	std::cout << "Elapsed time: " << elapsed_time << " s. Mean rate: " << static_cast<long double>(keyptc_counter) / static_cast<long double>(elapsed_time) << " ev / s." << std::endl;

	return EXIT_SUCCESS;
}

// utility function to determine whether the particle is NOT a B oscillation. Stolen from https://lhcb-release-area.web.cern.ch/LHCb-release-area/DOC/rec/latest_doxygen/da/db4/_hep_m_c_utils_8h_source.html
bool isBAtProduction(HepMC::GenParticle const * thePart) {
	if((std::abs(thePart->pdg_id()) != 511) && (std::abs(thePart->pdg_id()) != 531)) {
		return true;
	}
	if(thePart->production_vertex() == nullptr) {
		return true;
	}
	auto theVertex = thePart->production_vertex();
	if(theVertex -> particles_in_size() != 1) {
		return true;
	}
	HepMC::GenParticle * theMother = (*theVertex->particles_in_const_begin()) ;
	if(theMother->pdg_id() == - thePart->pdg_id()) {
		return false;
	}

	return true;
}
