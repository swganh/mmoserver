#Build a SWGANH Python binding.
#Original File: BuildBinding.cmake from delta3d project
#Expects files to be compiled in TARGET_SRC and TARGET_H.
#Supply the target name as the first parameter.  Additional
#parameters will be passed to TARGET_LINK_LIBRARIES()
INCLUDE(CMakeMacroParseArguments)
FUNCTION (BUILD_PYTHON_BINDING TGTNAME)
  
  #With Cmake2.6.x and CMP003 NEW, if user sets BOOST_PYTHON_LIBRARY
  #to be the non-versioned file ("boost_python-vc90-mt.lib"), the compiler
  #will try to link with the versioned file ("boost_python-vc90-mt-1_35.lib")
  #so we need an additional search path to find it.
  LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

  ADD_LIBRARY( ${TGTNAME} SHARED ${TARGET_SRC} ${TARGET_H} )
  INCLUDE_DIRECTORIES( ${PYTHON_INCLUDE_PATH} 
                       ${Boost_INCLUDE_DIR} )

  FOREACH(varname ${ARGN})
      TARGET_LINK_LIBRARIES( ${TGTNAME} ${varname} )
  ENDFOREACH(varname)
  
 
  TARGET_LINK_LIBRARIES( ${TGTNAME}
                         ${Boost_PYTHON_LIBRARY}
                        )
                        
  #Use the debug python library, should it exist, otherwise, use the release version
  SET (PYTHON_DEBUG ${PYTHON_DEBUG_LIBRARY})

  IF (NOT PYTHON_DEBUG)
    SET (PYTHON_DEBUG ${PYTHON_LIBRARY})
  ENDIF (NOT PYTHON_DEBUG)

  TARGET_LINK_LIBRARIES( ${TGTNAME}
                         optimized ${PYTHON_LIBRARY} debug ${PYTHON_DEBUG}
                       )

  IF (WIN32)
    SET_TARGET_PROPERTIES( ${TGTNAME} PROPERTIES SUFFIX ".pyd")
    SET_TARGET_PROPERTIES( ${TGTNAME} PROPERTIES DEBUG_POSTFIX "")
  ENDIF (WIN32)
  
  #little hack to get the .dll into /bin and .lib into /lib
  IF (MSVC)
    SET_TARGET_PROPERTIES( ${TGTNAME} PROPERTIES PREFIX "/../../bin/$(ConfigurationName)/")
  ENDIF (MSVC)

ENDFUNCTION()
