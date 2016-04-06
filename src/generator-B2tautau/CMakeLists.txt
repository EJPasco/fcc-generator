add_executable(generator-B2tautau generator-B2tautau.cpp)

target_link_libraries(generator-B2tautau datamodel podio datamodelDict boost_program_options ${ROOT_LIBRARIES} ${PYTHIA8_LIBRARIES} ${HEPMC_LIB} ${EVTGEN_LIBRARIES} ${PHOTOS_LIBRARIES} ${TAUOLA_LIBRARIES})

install(TARGETS generator-B2tautau DESTINATION bin)
