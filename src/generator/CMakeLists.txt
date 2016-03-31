add_executable(generator generator.cpp)

target_link_libraries(generator datamodel podio datamodelDict boost_program_options ${ROOT_LIBRARIES} ${PYTHIA8_LIBRARIES} ${HEPMC_LIB} ${EVTGEN_LIBRARIES} ${PHOTOS_LIBRARIES} ${TAUOLA_LIBRARIES})

install(TARGETS generator DESTINATION bin)
