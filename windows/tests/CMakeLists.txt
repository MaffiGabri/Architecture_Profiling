# ==============================================================================
# Architecture Profiling Companion - CTest Unit Test Suite Configuration
# Milestone M2: Modern C++20 Verification & Compiler Capability Testing
# ==============================================================================
cmake_minimum_required(VERSION 3.20)

project(ArchitectureProfilingTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# C++20 capability test executable
add_executable(test_cpp20_capabilities test_cpp20_capabilities.cpp)

target_include_directories(test_cpp20_capabilities PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../src"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

# Link domain logic if target exists
if(TARGET architecture_domain)
    target_link_libraries(test_cpp20_capabilities PRIVATE architecture_domain)
endif()

enable_testing()

# Master test runner (executes all 7 capability tests)
add_test(NAME Cpp20_All_Capabilities COMMAND test_cpp20_capabilities --all)

# Granular test targets for detailed CTest reporting
add_test(NAME Cpp20_Concepts           COMMAND test_cpp20_capabilities --test concepts)
add_test(NAME Cpp20_Ranges             COMMAND test_cpp20_capabilities --test ranges)
add_test(NAME Cpp20_StructuredBindings COMMAND test_cpp20_capabilities --test bindings)
add_test(NAME Cpp20_Filesystem         COMMAND test_cpp20_capabilities --test filesystem)
add_test(NAME Cpp20_ThreadsAtomics     COMMAND test_cpp20_capabilities --test threads)
add_test(NAME Cpp20_SpanViews          COMMAND test_cpp20_capabilities --test span)
add_test(NAME Cpp20_SpaceshipOperator  COMMAND test_cpp20_capabilities --test spaceship)

# ==============================================================================
# Domain Engine Unit Test Harness (Milestone M3)
# ==============================================================================
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test_domain_engine.cpp")
    add_executable(test_domain_engine test_domain_engine.cpp)

    target_include_directories(test_domain_engine PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/../include"
        "${CMAKE_CURRENT_SOURCE_DIR}/../src"
        "${CMAKE_CURRENT_SOURCE_DIR}/../third_party"
        "${CMAKE_CURRENT_SOURCE_DIR}"
    )

    target_link_libraries(test_domain_engine PRIVATE
        architecture_domain
    )

    # Convert path to CMake-style forward slashes for compile definition
    get_filename_component(ARCH_PROJ_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
    target_compile_definitions(test_domain_engine PRIVATE
        ARCH_PROJECT_ROOT_DIR="${ARCH_PROJ_ROOT}"
    )

    # CTest Suite Registrations
    add_test(NAME DomainEngine_All              COMMAND test_domain_engine --all)
    add_test(NAME DomainEngine_PairingSchedule  COMMAND test_domain_engine --test pairing)
    add_test(NAME DomainEngine_StateMachine     COMMAND test_domain_engine --test state_machine)
    add_test(NAME DomainEngine_BradleyTerry     COMMAND test_domain_engine --test bradley_terry)
    add_test(NAME DomainEngine_KendallTriads    COMMAND test_domain_engine --test triads)
    add_test(NAME DomainEngine_Confidence       COMMAND test_domain_engine --test confidence)
    add_test(NAME DomainEngine_Archetypes       COMMAND test_domain_engine --test archetypes)
    add_test(NAME DomainEngine_SharedFixtures   COMMAND test_domain_engine --test fixtures)
    add_test(NAME DomainEngine_EmptyTournament  COMMAND test_domain_engine --test empty_tournament)
endif()

# ==============================================================================
# Persistence and Localization Unit Test Harness (Milestone M5)
# ==============================================================================
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/test_persistence_localization.cpp")
    add_executable(test_persistence_localization test_persistence_localization.cpp)

    target_include_directories(test_persistence_localization PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/../include"
        "${CMAKE_CURRENT_SOURCE_DIR}/../src"
        "${CMAKE_CURRENT_SOURCE_DIR}/../third_party"
        "${CMAKE_CURRENT_SOURCE_DIR}"
    )

    target_link_libraries(test_persistence_localization PRIVATE
        architecture_domain
    )

    add_test(NAME Persistence_Localization_All COMMAND test_persistence_localization)
endif()
