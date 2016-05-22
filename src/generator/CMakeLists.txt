add_executable(generator generator.cpp)

target_link_libraries(generator datamodel podio datamodelDict boost_program_options ${ROOT_LIBRARIES} ${PYTHIA8_LIBRARIES} ${HEPMC_LIBRARIES} ${EVTGEN_LIBRARIES} ${PHOTOS_LIBRARIES})

install(TARGETS generator DESTINATION bin)
