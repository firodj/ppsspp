cmake_minimum_required(VERSION 3.6)
project(psp-disasm)

if(WIN32)
	add_definitions(-D_UNICODE -DUNICODE)
endif()
if(NOT ANDROID)
	set(CMAKE_CXX_STANDARD 11)
endif()

set(DisasmSource
	stub.cpp
	Memory.cpp
	go_bridge.cpp
	include/go_bridge.h

	../Core/Debugger/SymbolMap.cpp
	../Core/Debugger/SymbolMap.h
	../Core/MemmapFunctions.cpp

	../ext/xxhash.c
	../ext/xxhash.h

	../Core/MIPS/MIPS.cpp
	../Core/MIPS/MIPS.h
	../Core/MIPS/MIPSAnalyst.cpp
	../Core/MIPS/MIPSAnalyst.h
	../Core/MIPS/MIPSAsm.cpp
	../Core/MIPS/MIPSAsm.h
	../Core/MIPS/MIPSCodeUtils.cpp
	../Core/MIPS/MIPSCodeUtils.h
	../Core/MIPS/MIPSDebugInterface.cpp
	../Core/MIPS/MIPSDebugInterface.h
	../Core/MIPS/MIPSDis.cpp
	../Core/MIPS/MIPSDis.h
	../Core/MIPS/MIPSDisVFPU.cpp
	../Core/MIPS/MIPSDisVFPU.h
	../Core/MIPS/MIPSInt.cpp
	../Core/MIPS/MIPSInt.h
	../Core/MIPS/MIPSIntVFPU.cpp
	../Core/MIPS/MIPSIntVFPU.h
	../Core/MIPS/MIPSStackWalk.cpp
	../Core/MIPS/MIPSStackWalk.h
	../Core/MIPS/MIPSTables.cpp
	../Core/MIPS/MIPSTables.h
	../Core/MIPS/MIPSVFPUUtils.cpp
	../Core/MIPS/MIPSVFPUUtils.h
)

add_library(cityhash STATIC
	../ext/cityhash/city.cpp
	../ext/cityhash/city.h
	../ext/cityhash/citycrc.h
)
if(NOT MSVC)
	# These can be fast even for debug.
	set_target_properties(cityhash PROPERTIES COMPILE_FLAGS "-O2")
endif()

add_library(disasmlib STATIC ${DisasmSource})
target_compile_definitions(disasmlib PUBLIC BUILD_DISASM)
target_compile_features(disasmlib PUBLIC cxx_std_11)
target_include_directories(disasmlib
	PUBLIC ${PROJECT_SOURCE_DIR}/..
	PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(disasmlib PUBLIC cityhash)
set(GOLIB $<TARGET_FILE:disasmlib>)

# was: Common cityhash armips libzstd_static disasmlib
set(DISASM_DEPEND_LIBS cityhash disasmlib)

if (NOT MSVC)
	set(DISASM_COMBINED_LIB ${CMAKE_BINARY_DIR}/libdisasm_combined.a)
	set(GOLIB ${DISASM_COMBINED_LIB})

	# TODO: libtool is bash script, it is troublesome on MSYS, may be on macos using same aproach?
	if(APPLE AND (NOT IOS))
		add_custom_target(disasm_combined
			COMMAND libtool -static -o ${DISASM_COMBINED_LIB}
				$<TARGET_FILE:disasmlib>
				$<TARGET_FILE:cityhash>
			WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
			DEPENDS ${DISASM_DEPEND_LIBS}
		)
	elseif (MINGW)
		file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/CombinedTemp)
		add_custom_target(disasm_combined
			COMMAND
				${CMAKE_AR} x $<TARGET_FILE:disasmlib>
			COMMAND
				${CMAKE_AR} x $<TARGET_FILE:cityhash>
			COMMAND
				${CMAKE_AR} rv ${DISASM_COMBINED_LIB} *.obj
			WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/CombinedTemp
			DEPENDS ${DISASM_DEPEND_LIBS}
		)
	endif()

	add_library(libdisasm_combined STATIC IMPORTED GLOBAL)
	add_dependencies(libdisasm_combined disasm_combined)
	set_target_properties(libdisasm_combined
		PROPERTIES
		IMPORTED_LOCATION ${DISASM_COMBINED_LIB}
	)

	# Generate cgo wrapper
	add_custom_command(TARGET disasm_combined
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -P ${PROJECT_SOURCE_DIR}/BindConfig.cmake ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/include ${GOLIB}
	)
endif(NOT MSVC)

# Deprecated: converted to golang
#
# add_executable(disasm main.cpp)
# target_compile_features(disasm PUBLIC cxx_std_11)
# target_link_libraries(disasm PUBLIC ${DISASM_DEPEND_LIBS})

