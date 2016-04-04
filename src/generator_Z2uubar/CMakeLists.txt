add_executable(generator_Z2uubar generator_Z2uubar.cpp)

target_link_libraries(generator_Z2uubar datamodel datamodelDict podio boost_program_options ${ROOT_LIBRARIES} ${PYTHIA8_LIBRARIES} ${HEPMC_LIB})

install(TARGETS generator_Z2uubar DESTINATION bin)
