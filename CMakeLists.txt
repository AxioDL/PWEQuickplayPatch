cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_STANDARD 20)
include("prime-practice-native/PrimeAPI2/PrimeAPI.cmake")

project(pwe-prime1-quickplay C CXX)

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

add_echoes_library(quickplay prime-practice-native/PrimeAPI2/python/symbols/v1.028.lst default.dol ${SOURCE_FILES})