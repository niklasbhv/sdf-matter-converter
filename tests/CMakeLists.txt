set(TEST_DATA ${CMAKE_CURRENT_SOURCE_DIR}/data)
set(TEST_OUT ${CMAKE_CURRENT_BINARY_DIR}/out)
file(MAKE_DIRECTORY ${TEST_OUT})

#! Registers a test that runs the converter with ARGS and expects the files matching EXPECT to be generated
function(add_converter_test name)
    cmake_parse_arguments(PARSE_ARGV 1 TEST "" "" "ARGS;EXPECT;SETUP;REQUIRES")
    add_test(NAME ${name}
            COMMAND ${CMAKE_COMMAND}
            -DCONVERTER=$<TARGET_FILE:sdf_matter_converter>
            "-DARGS=${TEST_ARGS}"
            "-DEXPECT=${TEST_EXPECT}"
            -P ${CMAKE_CURRENT_SOURCE_DIR}/run_converter.cmake)
    if(TEST_SETUP)
        set_tests_properties(${name} PROPERTIES FIXTURES_SETUP ${TEST_SETUP})
    endif()
    if(TEST_REQUIRES)
        set_tests_properties(${name} PROPERTIES FIXTURES_REQUIRED ${TEST_REQUIRES})
    endif()
endfunction()

# Matter -> SDF
add_converter_test(matter_to_sdf_cluster
        ARGS --matter-to-sdf -cluster-xml ${TEST_DATA}/clusters/ExampleToggle.xml -o ${TEST_OUT}/cluster.json
        EXPECT ${TEST_OUT}/cluster-model.json ${TEST_OUT}/cluster-mapping.json
        SETUP sdf_object)

add_converter_test(matter_to_sdf_device
        ARGS --matter-to-sdf -device-xml ${TEST_DATA}/device_types/ExampleToggleDevice.xml
             -cluster-xml ${TEST_DATA}/clusters -o ${TEST_OUT}/device.json
        EXPECT ${TEST_OUT}/device-model.json ${TEST_OUT}/device-mapping.json
        SETUP sdf_thing)

# SDF -> Matter, using the output of the Matter -> SDF tests
add_converter_test(sdf_to_matter_object
        ARGS --sdf-to-matter -sdf-model ${TEST_OUT}/cluster-model.json -sdf-mapping ${TEST_OUT}/cluster-mapping.json
             -o ${TEST_OUT}/object.xml
        EXPECT ${TEST_OUT}/object-cluster*.xml
        REQUIRES sdf_object)

add_converter_test(sdf_to_matter_thing
        ARGS --sdf-to-matter -sdf-model ${TEST_OUT}/device-model.json -sdf-mapping ${TEST_OUT}/device-mapping.json
             -o ${TEST_OUT}/thing.xml
        EXPECT ${TEST_OUT}/thing-device.xml ${TEST_OUT}/thing-cluster*.xml
        REQUIRES sdf_thing)

# Round-tripping
add_converter_test(roundtrip_matter
        ARGS --matter-to-sdf --roundtrip -device-xml ${TEST_DATA}/device_types/ExampleToggleDevice.xml
             -cluster-xml ${TEST_DATA}/clusters -o ${TEST_OUT}/roundtrip.xml
        EXPECT ${TEST_OUT}/roundtrip-device.xml ${TEST_OUT}/roundtrip-cluster*.xml)

add_converter_test(roundtrip_sdf
        ARGS --sdf-to-matter --roundtrip -sdf-model ${TEST_OUT}/device-model.json
             -sdf-mapping ${TEST_OUT}/device-mapping.json -o ${TEST_OUT}/roundtrip.json
        EXPECT ${TEST_OUT}/roundtrip-model.json ${TEST_OUT}/roundtrip-mapping.json
        REQUIRES sdf_thing)
