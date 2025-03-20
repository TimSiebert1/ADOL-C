
# Configure the generated header in the binary path
set(ADOLC_GENERATED_INCLUDE_DIR ${CMAKE_BINARY_DIR}/ADOL-C/include/adolc/internal)
file(MAKE_DIRECTORY ${ADOLC_GENERATED_INCLUDE_DIR})

# Write options into the file adolc_settings.h
configure_file(
  adolc_settings.h.in
  ${ADOLC_GENERATED_INCLUDE_DIR}/adolc_settings.h
  @ONLY
)

# Install headers
install(FILES
  common.h
  usrparms.h
  ${ADOLC_GENERATED_INCLUDE_DIR}/adolc_settings.h
  DESTINATION "include/adolc/internal"
)

# Correct include paths
target_include_directories(adolc
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/ADOL-C/include>
    $<INSTALL_INTERFACE:include>
)

