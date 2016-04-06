add_executable(generator-Bs2tautau generator-Bs2tautau.cpp)

target_link_libraries(generator-Bs2tautau datamodel podio datamodelDict boost_program_options ${ROOT_LIBRARIES} ${PYTHIA8_LIBRARIES} ${HEPMC_LIB} ${EVTGEN_LIBRARIES} ${PHOTOS_LIBRARIES} ${TAUOLA_LIBRARIES})

install(TARGETS generator-Bs2tautau DESTINATION bin)
