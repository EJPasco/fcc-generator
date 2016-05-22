add_executable(generator-inclusive generator-inclusive.cpp)

target_link_libraries(generator-inclusive datamodel datamodelDict podio boost_program_options ${ROOT_LIBRARIES} ${PYTHIA8_LIBRARIES} ${HEPMC_LIBRARIES})

install(TARGETS generator-inclusive DESTINATION bin)
