
#-------------------------------------------------------------------------------
# @Brief function AddPythonTest
# @ NAME
# @ FILE
# @ PYTHONPATH
#-------------------------------------------------------------------------------
function(AddPythonTest)
  set(options )
  set(oneValueArgs NAME FILE)
  set(multiValueArgs PYTHONPATH)
  cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if(SIMPL_USE_ANACONDA_PYTHON)
    if(WIN32)
      add_test(NAME ${ARGS_NAME}
        COMMAND ${SIMPLProj_SOURCE_DIR}/Wrapping/Python/Binding/anaconda_test.bat
      )

      set_property(TEST ${ARGS_NAME}
        PROPERTY
          ENVIRONMENT
            "SIMPL_CONDA_EXECUTABLE=${SIMPL_CONDA_EXECUTABLE}"
            "SIMPL_CONDA_ENV=${ANACONDA_ENVIRONMENT_NAME}"
            "PYTHON_TEST_FILE=${ARGS_FILE}"
      )
    else()
      add_test(NAME ${ARGS_NAME}
        COMMAND ${SIMPLProj_SOURCE_DIR}/Wrapping/Python/Binding/anaconda_test.sh
      )

      set_property(TEST ${ARGS_NAME}
        PROPERTY
          ENVIRONMENT
            "SIMPL_ANACONDA_DIR=${ANACONDA_DIR}"
            "SIMPL_CONDA_ENV=${ANACONDA_ENVIRONMENT_NAME}"
            "PYTHON_TEST_FILE=${ARGS_FILE}"
      )
    endif()
  else()
    add_test(NAME ${ARGS_NAME}
      COMMAND ${PYTHON_EXECUTABLE} ${ARGS_FILE}
    )
  endif()

  if(WIN32)
    string(REPLACE ";" "\\;" ARGS_PYTHONPATH "${ARGS_PYTHONPATH}")
  else()
    string(REPLACE ";" ":" ARGS_PYTHONPATH "${ARGS_PYTHONPATH}")
  endif()

  set_property(TEST ${ARGS_NAME}
    APPEND
    PROPERTY
      ENVIRONMENT
      "PYTHONPATH=${ARGS_PYTHONPATH}"
      "${SIMPL_PYTHON_TEST_ENV}"
  )
endfunction()

#-------------------------------------------------------------------------------
# @Brief function CreatePybind11Module
# @ MODULE_NAME
# @ OUTPUT_DIR
# @ FILE_LIST_PATH
# @ SOURCE_DIR
# @ HEADER_PATH
# @ BODY_PATH
# @ INCLUDE_DIR
# @ PYTHON_OUTPUT_DIR
#-------------------------------------------------------------------------------
function(CreatePybind11Module)
  set(options PLUGIN)
  set(oneValueArgs MODULE_NAME OUTPUT_DIR FILE_LIST_PATH SOURCE_DIR HEADER_PATH BODY_PATH BODY_TOP_PATH POST_TYPES_PATH INCLUDE_DIR PYTHON_OUTPUT_DIR)
  set(multiValueArgs LINK_LIBRARIES INCLUDE_DIRS)
  cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  set(pybind11_FIND_QUIETLY TRUE)
  find_package(pybind11 REQUIRED)

  set(PLUGIN_ARG "")
  if(${ARGS_PLUGIN})
    set(PLUGIN_ARG "--plugin")
  endif()

  string(TOLOWER ${ARGS_MODULE_NAME} MODULE_NAME_lower)

  get_property(PY_ALL_MODULES_FILE GLOBAL PROPERTY PY_ALL_MODULES_FILE)

  file(APPEND ${PY_ALL_MODULES_FILE} "${MODULE_NAME_lower}\n")

  set(CREATE_PYTHON_BINDINGS_TARGET ${MODULE_NAME_lower}CreatePythonBindings)
  set(PYTHON_MODULE_SOURCE_FILE ${ARGS_OUTPUT_DIR}/py_${MODULE_NAME_lower}.cpp)
  set(PYTHON_UNIT_TEST_FILE ${ARGS_PYTHON_OUTPUT_DIR}/${MODULE_NAME_lower}_UnitTest.py)

  file(MAKE_DIRECTORY ${ARGS_OUTPUT_DIR})

  set(PY_GENERATOR ${SIMPLProj_SOURCE_DIR}/Wrapping/Python/Binding/generate_python_bindings.py)

  set(NO_TESTS_ARG "")
  if(DREAM3D_ANACONDA)
    set(NO_TESTS_ARG "--no_tests")
  endif()

  set(RELATIVE_IMPORTS_ARGS "")
  if(DREAM3D_ANACONDA)
    set(RELATIVE_IMPORTS_ARGS "--relative_imports")
  endif()

  add_custom_target(${CREATE_PYTHON_BINDINGS_TARGET} ALL
      COMMAND ${PYTHON_EXECUTABLE} ${PY_GENERATOR}
      "${ARGS_OUTPUT_DIR}"
      "${ARGS_FILE_LIST_PATH}"
      "${ARGS_SOURCE_DIR}"
      "${PLUGIN_ARG}"
      "${ARGS_PYTHON_OUTPUT_DIR}"
      "--include_dir=${ARGS_INCLUDE_DIR}"
      "--module_name=${MODULE_NAME_lower}"
      "--header_path=${ARGS_HEADER_PATH}"
      "--body_path=${ARGS_BODY_PATH}"
      "--body_top_path=${ARGS_BODY_TOP_PATH}"
      "--post_types_path=${ARGS_POST_TYPES_PATH}"
      "--plugin_name=${ARGS_MODULE_NAME}"
      "${NO_TESTS_ARG}"
      "${RELATIVE_IMPORTS_ARGS}"
      COMMENT "${ARGS_MODULE_NAME}: Generating Python bindings"
      BYPRODUCTS ${PYTHON_MODULE_SOURCE_FILE}
  )

  set_property(TARGET ${CREATE_PYTHON_BINDINGS_TARGET} PROPERTY FOLDER "Python/Generator")

  add_custom_command(TARGET ${CREATE_PYTHON_BINDINGS_TARGET}
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${ARGS_PYTHON_OUTPUT_DIR}
  )

  pybind11_add_module(${MODULE_NAME_lower}
    ${PYTHON_MODULE_SOURCE_FILE}
    ${SIMPLProj_SOURCE_DIR}/Wrapping/Python/Binding/pySupport.h
    ${SIMPLProj_SOURCE_DIR}/Wrapping/Python/Binding/Pybind11CustomTypeCasts.h
  )

  set_property(TARGET ${MODULE_NAME_lower} PROPERTY FOLDER "Python/Bindings")

  set_source_files_properties(${PYTHON_MODULE_SOURCE_FILE}
    PROPERTIES  
      SKIP_AUTOMOC ON
  )

  add_dependencies(${MODULE_NAME_lower} ${CREATE_PYTHON_BINDINGS_TARGET})

  target_link_libraries(${MODULE_NAME_lower}
    PUBLIC
      ${ARGS_LINK_LIBRARIES}
  )

  target_include_directories(${MODULE_NAME_lower}
    PUBLIC
      ${ARGS_INCLUDE_DIRS}
  )

  set(TESTS_PYTHONPATH
    "$<TARGET_FILE_DIR:simpl>"
  )

  AddPythonTest(NAME PY_${MODULE_NAME_lower}_UnitTest
    FILE ${PYTHON_UNIT_TEST_FILE}
    PYTHONPATH ${TESTS_PYTHONPATH}
  )

  set(install_dir ".")
  if(DREAM3D_ANACONDA)
    set(install_dir "bin")
  endif()

  if(NOT DREAM3D_ANACONDA)
    install(TARGETS ${MODULE_NAME_lower}
      LIBRARY DESTINATION ${install_dir}
    )
  endif()

  if(DREAM3D_ANACONDA)
    file(APPEND ${DREAM3D_INIT_PY_FILE} "from . import ${MODULE_NAME_lower}\n")
    file(APPEND ${DREAM3D_INIT_PY_FILE} "from . import ${MODULE_NAME_lower}py\n")
    add_dependencies(CopyPythonPackage ${MODULE_NAME_lower})
    add_dependencies(CreateDREAM3DStubs ${MODULE_NAME_lower})
  endif()

  if(SIMPL_GENERATE_PYI)
    set(CREATE_STUB_FILE_TARGET ${MODULE_NAME_lower}CreateStubFile)
    set(STUB_FILE ${ARGS_OUTPUT_DIR}/${MODULE_NAME_lower}.pyi)
    add_custom_target(${CREATE_STUB_FILE_TARGET} ALL
      COMMAND ${MYPY_STUBGEN_EXE} -m ${MODULE_NAME_lower} -o ${ARGS_PYTHON_OUTPUT_DIR}
      COMMENT "${ARGS_MODULE_NAME}: Generating .pyi files"
      BYPRODUCTS ${STUB_FILE}
      WORKING_DIRECTORY $<TARGET_FILE_DIR:${MODULE_NAME_lower}>
    )
    set_property(TARGET ${CREATE_STUB_FILE_TARGET} PROPERTY FOLDER "Python/Stub")
    add_dependencies(${CREATE_STUB_FILE_TARGET} ${MODULE_NAME_lower})
  endif()
endfunction()

#-------------------------------------------------------------------------------
# @Brief function CreatePybind11Plugin
# @ PLUGIN_NAME
# @ PLUGIN_TARGET
# @ HEADER_PATH
# @ BODY_PATH
#-------------------------------------------------------------------------------
function(CreatePybind11Plugin)
  set(options)
  set(oneValueArgs PLUGIN_NAME PLUGIN_TARGET HEADER_PATH BODY_PATH BODY_TOP_PATH POST_TYPES_PATH)
  set(multiValueArgs)
  cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  get_property(_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

  CreatePybind11Module(MODULE_NAME ${ARGS_PLUGIN_NAME}
    OUTPUT_DIR "${${ARGS_PLUGIN_NAME}_BINARY_DIR}/Wrapping/PythonCore"
    FILE_LIST_PATH "${SIMPLProj_BINARY_DIR}/Wrapping/${ARGS_PLUGIN_NAME}Filters.txt"
    SOURCE_DIR "${${ARGS_PLUGIN_NAME}_SOURCE_DIR}/${ARGS_PLUGIN_NAME}Filters"
    INCLUDE_DIR "${${ARGS_PLUGIN_NAME}_SOURCE_DIR}"
    PYTHON_OUTPUT_DIR "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}$<${_isMultiConfig}:/$<CONFIG>>"
    HEADER_PATH ${ARGS_HEADER_PATH}
    BODY_PATH ${ARGS_BODY_PATH}
    BODY_TOP_PATH ${ARGS_BODY_TOP_PATH}
    POST_TYPES_PATH ${ARGS_POST_TYPES_PATH}
    LINK_LIBRARIES ${ARGS_PLUGIN_TARGET}
    INCLUDE_DIRS ${SIMPLProj_SOURCE_DIR}/Wrapping/Python
    PLUGIN
  )
endfunction()


#-------------------------------------------------------------------------------
# @Brief function CreatePythonTests
# @ PREFIX
# @ INPUT_DIR
# @ TEST_NAMES
#-------------------------------------------------------------------------------
function(CreatePythonTests)
  set(options)
  set(oneValueArgs PREFIX INPUT_DIR)
  set(multiValueArgs TEST_NAMES)
  cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  set(TESTS_PYTHONPATH
    "$<TARGET_FILE_DIR:simpl>"
  )

  foreach(test ${ARGS_TEST_NAMES})
    string(REPLACE "/" "_" test_name ${test})
    set(PY_TEST_NAME ${ARGS_PREFIX}_${test_name})

    AddPythonTest(NAME ${PY_TEST_NAME}
      FILE ${ARGS_INPUT_DIR}/${test}.py
      PYTHONPATH ${TESTS_PYTHONPATH}
    )
  endforeach()
endfunction()
