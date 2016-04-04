/// Generator of Z -> u ubar decays
/// Uses PYTHIA to generate initial collision and to decay produced particles
/// Stores only events with less than 7 particles in the final state
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
#include <chrono>
#include <map>
#include <unordered_map>

// PYTHIA and HepMC
#include "Pythia8/Pythia.h"
#include "Pythia8Plugins/HepMC2.h"

#ifdef USE_BOOST
	// Boost
	#include "boost/program_options.hpp"
#endif

std::size_t n_stable_in_event(HepMC::GenEvent const * const event);

// utility function to determine whether a particle leaves a charged track
inline bool is_charged_track(HepMC::GenParticle const * const ptc_ptr) {
	return std::abs(ptc_ptr->pdg_id()) == 211 /* pions */ || std::abs(ptc_ptr->pdg_id()) == 321 /* kaons */ || std::abs(ptc_ptr->pdg_id()) == 2212 /* protons */ || std::abs(ptc_ptr->pdg_id()) == 11 /* electrons */ || std::abs(ptc_ptr->pdg_id()) == 13 /* muons */;
}

int main(int argc, char * argv[]){
	// declaring and initializing some variables. Most of them will be set according to command line options passed to the program after parsing of command line arguments. However, if Boost is not used, the only available command line option is the number of events to generate; other variables will use the values set below
	std::size_t nevents = 0; // number of events to generate
	std::string pythia_cfgfile = "Z2uubar.cmnd"; // name of PYTHIA cofiguration file
	std::string output_filename = "Z2uubar.root"; // name of the output file
	bool verbose = false; // increased verbosity switch

	#ifdef USE_BOOST
		try {
			boost::program_options::options_description desc("Usage");

			// defining command line options. See boost::program_options documentation for more details
			desc.add_options()
							("help", "produce this help message")
							("nevents,n", boost::program_options::value<std::size_t>(&nevents), "number of events to generate")
							("pythiacfg,P", boost::program_options::value<std::string>(&pythia_cfgfile)->default_value("Z2uubar.cmnd"), "PYTHIA config file")
							("outfile,o", boost::program_options::value<std::string>(&output_filename)->default_value("Z2uubar.root"), "Output file")
							("verbose,v", boost::program_options::bool_switch()->default_value(false), "Run with increased verbosity")
			;
			boost::program_options::variables_map vm;
			boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
			boost::program_options::notify(vm);

			if(vm.find("help") != vm.end() || argc < 2) {
				std::cout << "Generator of inclusive events. Version " << Generator_VERSION_MAJOR << '.' << Generator_VERSION_MINOR << std::endl;
				std::cout << desc << std::endl;

				return EXIT_SUCCESS;
			}

			verbose = vm.at("verbose").as<bool>();
		} catch(std::exception const & e) {
			std::cout << e.what() << std::endl;

			return EXIT_FAILURE;
		}
	#else
		if(argc < 2) {
			std::cout << "Generator of inclusive events. Version " << Generator_VERSION_MAJOR << '.' << Generator_VERSION_MINOR << std::endl;
			std::cout << "Usage: " << argv[0] << " n, where \"n\" is a number of events to generate" << std::endl;
			std::cout << "WARNING! This version of the generator does not use program options parser, which means that you are personally responsible for providing correct options to this program." << std::endl;

			return EXIT_SUCCESS;
		} else {
			nevents = std::stoull(argv[1]);
		}
	#endif

	auto start_time = std::chrono::system_clock::now();
	auto last_timestamp = start_time;

	if(verbose) {
			std::cout << "PYTHIA config file: \"" << pythia_cfgfile << "\"" << std::endl << nevents << " events will be generated." << std:: endl;
	}

	if(verbose) {
		std::cout << "Prepairing data store" << std::endl;
	}

	// prepairing event store
	podio::EventStore store;
	podio::ROOTWriter writer(output_filename, &store);

	// registering collections
	auto & evinfocoll = store.create<fcc::EventInfoCollection>("EventInfo");
	auto & pcoll = store.create<fcc::MCParticleCollection>("GenParticle");
	auto & vcoll = store.create<fcc::GenVertexCollection>("GenVertex");

	writer.registerForWrite<fcc::EventInfoCollection>("EventInfo");
	writer.registerForWrite<fcc::MCParticleCollection>("GenParticle");
	writer.registerForWrite<fcc::GenVertexCollection>("GenVertex");

	if(verbose) {
		std::cout << "Initializing PYTHIA" << std::endl;
	}

	// initializing PYTHIA
	Pythia8::Pythia pythia; // creating PYTHIA generator object
	pythia.readFile(pythia_cfgfile); // reading settings from file

	pythia.init(); // initializing PYTHIA generator

	// Interface for conversion from Pythia8::Event to HepMC event.
	HepMC::Pythia8ToHepMC ToHepMC;

	std::size_t counter = 0; // number of "interesting" (that satisfy all the requirements) events generated so far
	std::size_t total = 0; // total number of events generated so far

	if(verbose) {
		std::cout << "Starting to generate events" << std::endl;
	}

	std::map<std::size_t, std::size_t> stable_ptcs_count;

	while(counter < nevents) {
		if(pythia.next()) {
			++total;

			// creating HepMC event storage
			HepMC::GenEvent * hepmcevt = new HepMC::GenEvent(HepMC::Units::GEV, HepMC::Units::MM);

			// converting generated event to HepMC format
			ToHepMC.fill_next_event(pythia, hepmcevt);

			auto nstable = n_stable_in_event(hepmcevt);

			if(nstable <= 7) {
				stable_ptcs_count[nstable]++;
				++counter;

				if(verbose && counter % 100 == 0) {
					std::cout << counter << " events with with 7 or less particles in the final state have been generated (" << total << " total). " << std::chrono::duration<double>(std::chrono::system_clock::now() - last_timestamp).count() / 100 << "events / sec" << std::endl;
					last_timestamp = std::chrono::system_clock::now();
				}

				// filling event info
				auto evinfo = fcc::EventInfo();
				evinfo.Number(counter);
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

					core.Charge = pythia.particleData.charge(core.Type);
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

	writer.finish();

	std::cout << counter << " events with 7 or less particles in the final state have been generated (" << total << " total)." << std::endl;
	for(auto const & nv : stable_ptcs_count) {
		std::cout << std::setw(4) << std::right << nv.first << std::setw(4) << std::right << nv.second << "(" << nv.second * 100. / total << "%)" << std::endl;
	}
	auto elapsed_seconds = std::chrono::duration<double>(std::chrono::system_clock::now() - start_time).count();
	std::cout << "Elapsed time: " << elapsed_seconds << " s (" << counter / elapsed_seconds << " events / s)" << std::endl;

	return EXIT_SUCCESS;
}

std::size_t n_stable_in_event(HepMC::GenEvent const * const event_ptr) {
	std::size_t counter = 0;
	for(auto it = event_ptr->particles_begin(), endit = event_ptr->particles_end(); it != endit; ++it) {
		if((*it)->status() == 1) {
			++counter;
		}
	}

	return counter;
}

// utility function to determine whether the particle is NOT a B oscillation. Stolen from https://lhcb-release-area.web.cern.ch/LHCb-release-area/DOC/rec/latest_doxygen/da/db4/_hep_m_c_utils_8h_source.html
bool isBAtProduction(HepMC::GenParticle const * thePart) {
	if((abs(thePart->pdg_id()) != 511) && (abs(thePart->pdg_id()) != 531)) {
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
