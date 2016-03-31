/**
 * PYTHIAFCC event generator
 * Andrii Semkiv
 * 15.03.2016
 */

// Configuration
#include "GeneratorConfig.h"

// Data model
#include "datamodel/EventInfo.h"
#include "datamodel/EventInfoCollection.h"
#include "datamodel/MCParticle.h"
#include "datamodel/MCParticleCollection.h"
#include "datamodel/GenVertex.h"
#include "datamodel/GenVertexCollection.h"

// STL
// IO
#include <iostream>
#include <string>
// EXIT_SUCCESS & EXIT_FAILURE
#include <cstdlib>
// exceptions handling
#include <stdexcept>
// time
#include <chrono>
// containers
#include <unordered_set>
#include <unordered_map>
// math
#include <cmath>

// PODIO
#include "podio/EventStore.h"
#include "podio/ROOTWriter.h"

// PYTHIA and HepMC
#include "Pythia8/Pythia.h"
#include "Pythia8Plugins/HepMC2.h"

// #include "Pythia8Plugins/EvtGen.h"

#ifdef USE_BOOST
	// Boost
	#include "boost/program_options.hpp"
#endif

bool isBAtProduction(HepMC::GenParticle const * thePart);

inline bool is_charged_track(HepMC::GenParticle const * const ptc_ptr) {
	return std::abs(ptc_ptr->pdg_id()) == 211 /* pions */ || std::abs(ptc_ptr->pdg_id()) == 321 /* kaons */ || std::abs(ptc_ptr->pdg_id()) == 2212 /* protons */ || std::abs(ptc_ptr->pdg_id()) == 11 /* electrons */ || std::abs(ptc_ptr->pdg_id()) == 13 /* muons */;
}

int main(int argc, char * argv[]){
	std::size_t nevents = 0;
	std::string pythia_cfgfile = "pythia.cmnd";
	std::string output_filename = "output.root";
	bool verbose = false;

	#ifdef USE_BOOST
		try {
			boost::program_options::options_description desc("Usage");
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
	auto last_lap = start_time;

	if(verbose) {
			std::cout << "PYTHIA config file: \"" << pythia_cfgfile << "\"" << std::endl << nevents << " events will be generated." << std:: endl;
	}

	if(verbose) {
		std::cout << "Prepairing data store" << std::endl;
	}

	podio::EventStore store;
	podio::ROOTWriter writer(output_filename, &store);


	auto & evinfocoll = store.create<fcc::EventInfoCollection>("EventInfo");
	auto & pcoll = store.create<fcc::MCParticleCollection>("GenParticle");
	auto & vcoll = store.create<fcc::GenVertexCollection>("GenVertex");

	writer.registerForWrite<fcc::EventInfoCollection>("EventInfo");
	writer.registerForWrite<fcc::MCParticleCollection>("GenParticle");
	writer.registerForWrite<fcc::GenVertexCollection>("GenVertex");

	if(verbose) {
		std::cout << "Initializing PYTHIA" << std::endl;
	}

	Pythia8::Pythia pythia;
	pythia.readFile(pythia_cfgfile);

	pythia.init();

	// auto evtgen = new EvtGenDecays(&pythia, "/afs/cern.ch/sw/lcg/releases/LCG_80/MCGenerators/evtgen/1.5.0/x86_64-slc6-gcc49-opt/share/DECAY_2010.DEC", "/afs/cern.ch/sw/lcg/releases/LCG_80/MCGenerators/evtgen/1.5.0/x86_64-slc6-gcc49-opt/share/evt.pdl");
	// if (evtgen) {
	// 	evtgen->readDecayFile("background_Bd2DsKTauNu.dec");
	// 	evtgen->exclude(23); // make PYTHIA itself (not EvtGen) decay Z
	// } else {
	// 	std::cerr << "Unable to initialize EvtGen. Program stopped." << std::endl;
	// 	return EXIT_FAILURE;
	// }

	// Interface for conversion from Pythia8::Event to HepMC event.
	HepMC::Pythia8ToHepMC ToHepMC;

	std::size_t counter = 0;
	std::size_t total = 0;

	std::size_t b_counter = 0, tau_counter = 0, tau2pipipi_counter = 0, /*kstar_counter = 0,*/ charged_tracks_counter = 0;

	if(verbose) {
		std::cout << "Starting to generate events" << std::endl;
	}

	// auto generation_start_time = std::chrono::system_clock::now();
	// auto last_b_generated_time = generation_start_time;

	while(counter < nevents) {
		if(pythia.next()) {
			++total;

			// evtgen->decay();

			HepMC::GenEvent * hepmcevt = new HepMC::GenEvent(HepMC::Units::GEV, HepMC::Units::MM);
			ToHepMC.fill_next_event(pythia, hepmcevt);

			std::size_t decays_in_event = 0; // Counts decays particles in the event

			// looping through all particles in order to find B that decays into K* and tau (and tau in turn decays into 3 pis) and exclude their daughters from charged tracks count
			for(auto ib = hepmcevt->particles_begin(), endp = hepmcevt->particles_end(); ib != endp; ++ib) {
				if(std::abs((*ib)->pdg_id()) == 511 && isBAtProduction(*ib)) {
					++b_counter;
					// if(verbose) {
					// 	std::cout << b_counter << " B0 have been produced (" << "time taken to generate the event: " << std::chrono::duration<double>(std::chrono::system_clock::now() - last_b_generated_time).count() << " s; mean rate: " << b_counter / std::chrono::duration<double>(std::chrono::system_clock::now() - generation_start_time).count() << " B0 / s)" << std::endl;
					// 	last_b_generated_time = std::chrono::system_clock::now();
					// }

					bool /*kstar_found = false*,*/ k_found = false, pi_found = false, tau_found = false, /*kstar2kpi = false,*/ tau2pipipi = false;
					std::unordered_set<decltype(*ib)> exclude; // we exclude particles prodeuced in decays of tau and K*0 from charge tracks count

					for(auto idaugh = hepmcevt->particles_begin(); idaugh != endp; ++idaugh) {
						if((*idaugh)->production_vertex() != nullptr && (*idaugh)->production_vertex()->point3d() == (*ib)->end_vertex()->point3d()) { // if the paricle is a daughter of B. a check if the production vertex is valid is required in order to avoid null pointer dereferencing
							if(std::abs((*idaugh)->pdg_id()) == 15) { // if it's a tau
								++tau_counter;
								tau_found = true;

								decltype(exclude) pi_daughters;
								for(auto igranddaugh = hepmcevt->particles_begin(); igranddaugh != endp; ++igranddaugh) { // exclude pi daughters of tau
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

							// if(std::abs((*idaugh)->pdg_id()) == 313) {
							// 	++kstar_counter;
							// 	kstar_found = true;
							//
							// 	bool k_found = false, pi_found = false;
							// 	for(auto igranddaugh = hepmcevt->particles_begin(); igranddaugh != endp; ++igranddaugh) { // exclude daughters of K*0
							// 		if((*igranddaugh)->production_vertex() != nullptr && (*idaugh)->end_vertex() != nullptr && (*igranddaugh)->production_vertex()->point3d() == (*idaugh)->end_vertex()->point3d()) { // a check if the vertices are valid is required in order to avoid null pointer dereferencing
							// 			if(std::abs((*igranddaugh)->pdg_id()) == 211) {
							// 				pi_found = true;
							// 				exclude.emplace(*igranddaugh);
							// 			}
							// 			if(std::abs((*igranddaugh)->pdg_id()) == 321) {
							// 				k_found = true;
							// 				exclude.emplace(*igranddaugh);
							// 			}
							// 		}
							// 	}
							//
							// 	if(k_found && pi_found) {
							// 		kstar2kpi = true;
							// 	}
							// }
							if(std::abs((*idaugh)->pdg_id()) == 211) {
								pi_found = true;
								exclude.emplace(*idaugh);
							}
							if(std::abs((*idaugh)->pdg_id()) == 321) {
								k_found = true;
								exclude.emplace(*idaugh);
							}
						}
					}

					decltype(exclude) charged_tracks;
					// a new loop is required since we have to fill exclude set first
					for(auto idaugh = hepmcevt->particles_begin(); idaugh != endp; ++idaugh) { // looking for charged_tracks among daughters of B0
						if((*idaugh)->production_vertex() != nullptr && (*idaugh)->production_vertex()->point3d() == (*ib)->end_vertex()->point3d() && exclude.find(*idaugh) == exclude.end()) { // a check if the production vertex is valid is required in order to avoid null pointer dereferencing
							if(is_charged_track(*idaugh)) {
								charged_tracks.emplace(*idaugh);
							} else {
								for(auto igranddaugh = hepmcevt->particles_begin(); igranddaugh != endp; ++igranddaugh) { // looking for charged_tracks among granddaughters of B0
									if((*igranddaugh)->production_vertex() != nullptr && (*idaugh)->end_vertex() != nullptr && (*igranddaugh)->production_vertex()->point3d() == (*idaugh)->end_vertex()->point3d() && exclude.find(*igranddaugh) == exclude.end()) { // a check if the vertices are valid is required in order to avoid null pointer dereferencing
										if(is_charged_track(*igranddaugh)) {
											charged_tracks.emplace(*igranddaugh);
										} else {
											for(auto igrandgranddaugh = hepmcevt->particles_begin(); igrandgranddaugh != endp; ++igrandgranddaugh) { // looking for charged_tracks among grandgranddaughters of B0
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
							std::cout << "Tau" << (tau2pipipi ? "->pipipi" : "") << ", pi and K found and there are " << charged_tracks.size() << " charged tracks" << std::endl;
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
					std::cout << counter << " events with decay of B0 -> K*0 tau production have been generated (" << total << " total). " << std::chrono::duration<double>(std::chrono::system_clock::now() - last_lap).count() / 100 << "events / sec" << std::endl;
					last_lap = std::chrono::system_clock::now();
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

	// if(evtgen) {
	// 	delete evtgen;
	// 	evtgen = nullptr;
	// }

	std::cout << counter << " events with decay of B0 -> K*0 tau have been generated (" << total << " total)." << std::endl;
	auto elapsed_seconds = std::chrono::duration<double>(std::chrono::system_clock::now() - start_time).count();
	std::cout << "Elapsed time: " << elapsed_seconds << " s (" << counter / elapsed_seconds << " events / s)" << std::endl;

	std::cout << "B0: " << b_counter << std::endl << "tau: " << tau_counter << std::endl << /*"K*0: " << kstar_counter << std::endl << */"3 tracks: " << charged_tracks_counter << std::endl;

	return EXIT_SUCCESS;
}

bool isBAtProduction(HepMC::GenParticle const * thePart) { // stolen from https://lhcb-release-area.web.cern.ch/LHCb-release-area/DOC/rec/latest_doxygen/da/db4/_hep_m_c_utils_8h_source.html
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
