add_executable(generator-Z2uubar generator-Z2uubar.cpp)

target_link_libraries(generator-Z2uubar datamodel datamodelDict podio boost_program_options ${ROOT_LIBRARIES} ${PYTHIA8_LIBRARIES} ${HEPMC_LIB})

install(TARGETS generator-Z2uubar DESTINATION bin)
