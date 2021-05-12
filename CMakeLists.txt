cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_STANDARD 20)
include("prime-practice-native/PrimeAPI2/PrimeAPI.cmake")

project(pwe-quickplay C CXX)

include_directories(prime-practice-native/PrimeAPI/include)
include_directories(prime-practice-native/src/include)

set(SOURCE_FILES
    prime-practice-native/PrimeAPI/include/PrimeAPI.h
    prime-practice-native/PrimeAPI/include/rstl/string.h
    prime-practice-native/PrimeAPI/include/rstl/rstl.h
    prime-practice-native/PrimeAPI/include/rstl/vector.h
    prime-practice-native/src/include/dvd.h
    prime-practice-native/src/include/os.h
    Source/QuickplayModule.cpp
)

patch_function("CMainFlow::AdvanceGameState(CArchitectureQueue&)" "Hook_CMainFlow_AdvanceGameState(CMainFlow*, CArchitectureQueue&)")
patch_function("CStateManager::InitializeState(uint, TAreaId, uint)" "Hook_CStateManager_InitializeState(CStateManager&, uint, TAreaId, uint)")
patch_function("CGameArea::StartStreamIn(CStateManager&)" "Hook_CGameArea_StartStreamIn(CGameArea*, CStateManager&)")

set(DEFAULT_QUICKPLAY_PRIME1 OFF)
set(DEFAULT_QUICKPLAY_PRIME2 OFF)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/prime1.dol")
    set(DEFAULT_QUICKPLAY_PRIME1 ON)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/echoes.dol")
    set(DEFAULT_QUICKPLAY_PRIME2 ON)
endif()

option(QUICKPLAY_PRIME1 "Build the quickplay patch for Prime 1" ${DEFAULT_QUICKPLAY_PRIME1})
option(QUICKPLAY_PRIME2 "Build the quickplay patch for Prime 2" ${DEFAULT_QUICKPLAY_PRIME2})

if(${QUICKPLAY_PRIME1})
    add_prime_library(quickplay-prime1 symbols/prime1.lst prime1.dol ${SOURCE_FILES})
endif()

if(${QUICKPLAY_PRIME2})
    add_echoes_library(quickplay-prime2 symbols/echoes.lst echoes.dol ${SOURCE_FILES})
endif()

if(NOT (${QUICKPLAY_PRIME1} OR ${QUICKPLAY_PRIME2}))
    message(FATAL_ERROR "One of QUICKPLAY_PRIME1 or QUICKPLAY_PRIME2 must be enabled.")
endif()