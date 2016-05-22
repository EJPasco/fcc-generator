add_executable(generator-Z2WW generator-Z2WW.cpp)

target_link_libraries(generator-Z2WW datamodel datamodelDict podio boost_program_options ${ROOT_LIBRARIES} ${PYTHIA8_LIBRARIES} ${HEPMC_LIBRARIES})

install(TARGETS generator-Z2WW DESTINATION bin)
