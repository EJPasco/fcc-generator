/// Generator of inclusive decays
/// Uses PYTHIA to generate initial collision and to decay produced particles
/// Then examines the generated event and looks for decays of B0 into K, pi, tau and at least 3 additional charged tracks among daughters, granddaughters and grandgranddaughters of B. If such events is found it is stored
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
#include <string>
#include <cstdlib>
#include <stdexcept>
#include <chrono>
#include <unordered_set>
#include <unordered_map>

// PYTHIA and HepMC
#include "Pythia8/Pythia.h"
#include "Pythia8Plugins/HepMC2.h"

#ifdef USE_BOOST
	// Boost
	#include "boost/program_options.hpp"
#endif

bool isBAtProduction(HepMC::GenParticle const * thePart); // utility function to determine whether the particle is NOT a B oscillation. Stolen from https://lhcb-release-area.web.cern.ch/LHCb-release-area/DOC/rec/latest_doxygen/da/db4/_hep_m_c_utils_8h_source.html


// utility function to determine whether a particle leaves a charged track
inline bool is_charged_track(HepMC::GenParticle const * const ptc_ptr) {
	return std::abs(ptc_ptr->pdg_id()) == 211 /* pions */ || std::abs(ptc_ptr->pdg_id()) == 321 /* kaons */ || std::abs(ptc_ptr->pdg_id()) == 2212 /* protons */ || std::abs(ptc_ptr->pdg_id()) == 11 /* electrons */ || std::abs(ptc_ptr->pdg_id()) == 13 /* muons */;
}

int main(int argc, char * argv[]){
	// declaring and initializing some variables. Most of them will be set according to command line options passed to the program after parsing of command line arguments. However, if Boost is not used, the only available command line option is the number of events to generate; other variables will use the values set below
	std::size_t nevents = 0; // number of events to generate
	std::string pythia_cfgfile = "pythia.cmnd"; // name of PYTHIA cofiguration file
	std::string output_filename = "output.root"; // name of the output file
	bool verbose = false; // increased verbosity switch

	#ifdef USE_BOOST
		try {
			boost::program_options::options_description desc("Usage");

			// defining command line options. See boost::program_options documentation for more details
			desc.add_options()
							("help", "produce this help message")
							("nevents,n", boost::program_options::value<std::size_t>(&nevents), "number of events to generate")
							("pythiacfg,P", boost::program_options::value<std::string>(&pythia_cfgfile)->default_value("pythia.cmnd"), "PYTHIA config file")
							("outfile,o", boost::program_options::value<std::string>(&output_filename)->default_value("output.root"), "Output file")
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

	std::size_t b_counter = 0, tau_counter = 0, tau2pipipi_counter = 0, charged_tracks_counter = 0; // number of events containing B0, tau, tau -> pi pi pi, and >=3 charged tracks respectively

	if(verbose) {
		std::cout << "Starting to generate events" << std::endl;
	}

	while(counter < nevents) {
		if(pythia.next()) {
			++total;

			// creating HepMC event storage
			HepMC::GenEvent * hepmcevt = new HepMC::GenEvent(HepMC::Units::GEV, HepMC::Units::MM);

			// converting generated event to HepMC format
			ToHepMC.fill_next_event(pythia, hepmcevt);

			std::size_t decays_in_event = 0; // counts interesting decays in the event

			// looping through all particles in order to find B that decays into K, pi and tau (and tau in turn decays into 3 pis) and exclude their daughters from charged tracks count
			for(auto ib = hepmcevt->particles_begin(), endp = hepmcevt->particles_end(); ib != endp; ++ib) {
				if(std::abs((*ib)->pdg_id()) == 511 && isBAtProduction(*ib)) {
					++b_counter;

					bool k_found = false, pi_found = false, tau_found = false, tau2pipipi = false; // flags signalazing whether k, pi, tau were found and if tau decays into 3 pions respectively
					std::unordered_set<decltype(*ib)> exclude; // we exclude particles produced in decays of tau from charge tracks count

					// iterating through the particles looking for tau, K and pi that were produced in the B0 decay
					for(auto idaugh = hepmcevt->particles_begin(); idaugh != endp; ++idaugh) {
						// if the paricle is a daughter of B. a check if the production vertex is valid is required in order to avoid null pointer dereferencing
						if((*idaugh)->production_vertex() != nullptr && (*idaugh)->production_vertex()->point3d() == (*ib)->end_vertex()->point3d()) {
							// if it's a tau
							if(std::abs((*idaugh)->pdg_id()) == 15) {
								++tau_counter;
								tau_found = true;

								decltype(exclude) pi_daughters; // container for pions produced in the tau decay
								// iterating through particles loking for pions produced in the tau decay
								for(auto igranddaugh = hepmcevt->particles_begin(); igranddaugh != endp; ++igranddaugh) {
									if((*igranddaugh)->production_vertex() != nullptr && (*idaugh)->end_vertex() != nullptr && (*igranddaugh)->production_vertex()->point3d() == (*idaugh)->end_vertex()->point3d() && std::abs((*igranddaugh)->pdg_id()) == 211) { // if the paricle is a daughter of tau. a check if the vertices are valid is required in order to avoid null pointer dereferencing
										pi_daughters.emplace(*igranddaugh);
									}
								}

								if(pi_daughters.size() == 3) {
									tau2pipipi = true;
									exclude.insert(pi_daughters.cbegin(), pi_daughters.cend()); // exclude them from charged tarcks count

									++tau2pipipi_counter;
								}
							}

							// if it's a pion
							if(std::abs((*idaugh)->pdg_id()) == 211) {
								pi_found = true;
								exclude.emplace(*idaugh);
							}

							// if it's a kaon
							if(std::abs((*idaugh)->pdg_id()) == 321) {
								k_found = true;
								exclude.emplace(*idaugh);
							}
						}
					}

					decltype(exclude) charged_tracks; // container for charget tracks. We can't just count charged tracks in a simple way since there is a double count possible (e.g. daughters of tau are granddaughters of B). std::set alows us to ignore duplicates

					// a new loop is required since we have to fill exclude set first

					//looking for charged_tracks among daughters of B0
					for(auto idaugh = hepmcevt->particles_begin(); idaugh != endp; ++idaugh) {
						if((*idaugh)->production_vertex() != nullptr && (*idaugh)->production_vertex()->point3d() == (*ib)->end_vertex()->point3d() && exclude.find(*idaugh) == exclude.end()) { // a check if the production vertex is valid is required in order to avoid null pointer dereferencing
							if(is_charged_track(*idaugh)) {
								charged_tracks.emplace(*idaugh);
							} else {
								// looking for charged tracks among granddaughters of B0
								for(auto igranddaugh = hepmcevt->particles_begin(); igranddaugh != endp; ++igranddaugh) {
									if((*igranddaugh)->production_vertex() != nullptr && (*idaugh)->end_vertex() != nullptr && (*igranddaugh)->production_vertex()->point3d() == (*idaugh)->end_vertex()->point3d() && exclude.find(*igranddaugh) == exclude.end()) { // a check if the vertices are valid is required in order to avoid null pointer dereferencing
										if(is_charged_track(*igranddaugh)) {
											charged_tracks.emplace(*igranddaugh);
										} else {
											// looking for charged tracks among grandgranddaughters of B0
											for(auto igrandgranddaugh = hepmcevt->particles_begin(); igrandgranddaugh != endp; ++igrandgranddaugh) {
												if(is_charged_track(*igrandgranddaugh) && (*igrandgranddaugh)->production_vertex() != nullptr && (*igranddaugh)->end_vertex() != nullptr && (*igrandgranddaugh)->production_vertex()->point3d() == (*igranddaugh)->end_vertex()->point3d() && exclude.find(*igrandgranddaugh) == exclude.end()) { // a check if the vertices are valid is required in order to avoid null pointer dereferencing
													charged_tracks.emplace(*igrandgranddaugh);
												}
											}
										}
									}
								}
							}
						}
					}

					if(charged_tracks.size() >= 3) {
						++charged_tracks_counter;
					}

					if(tau2pipipi && k_found && pi_found && charged_tracks.size() >= 3) {
						++decays_in_event;

						std::cout << "HURRAY!!! We've got a decay we've been looking for!" << std::endl;
						hepmcevt->print();
					} else {
						if(k_found && pi_found && tau_found) {
							std::cout << "tau" << (tau2pipipi ? "->pipipi" : "") << ", pi and K found and there are " << charged_tracks.size() << " charged tracks" << std::endl;
							hepmcevt->print();
							std::cout << "Excluded particles:" << std::endl;
							for(auto ptc : exclude) {
								ptc->print();
							}
							std::cout << "Charged tracks:" << std::endl;
							for(auto ptc : charged_tracks) {
								ptc->print();
							}
						}
					}
				}
			}

			counter += decays_in_event;

			if(decays_in_event > 0) {
				if(verbose && counter % 100 == 0) {
					std::cout << counter << " events with decay of B0 -> K*0 tau production have been generated (" << total << " total). " << std::chrono::duration<double>(std::chrono::system_clock::now() - last_timestamp).count() / 100 << "events / sec" << std::endl;
					last_timestamp = std::chrono::system_clock::now();
				}

				// filling event info
				auto evinfo = fcc::EventInfo();
				evinfo.Number(counter); // Number takes int as its parameter, so here's a narrowing conversion (std::size_t to int). Should be safe unless we get 2^32 events or more. Then undefined behaviour
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

	std::cout << counter << " events with decay of B0 -> K*0 tau have been generated (" << total << " total)." << std::endl;
	auto elapsed_seconds = std::chrono::duration<double>(std::chrono::system_clock::now() - start_time).count();
	std::cout << "Elapsed time: " << elapsed_seconds << " s (" << static_cast<long double>(counter) / static_cast<long double>(elapsed_seconds) << " events / s)" << std::endl;

	std::cout << "B0: " << b_counter << std::endl << "tau: " << tau_counter << std::endl << "3 tracks: " << charged_tracks_counter << std::endl;

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
