cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_TOOLCHAIN_FILE prime-practice-native/PrimeAPI2/PrimeToolchain.cmake)
project(pwe-prime1-quickplay)

include_directories(prime-practice-native/PrimeAPI/include)

set(SOURCE_FILES
        prime-practice-native/PrimeAPI/include/PrimeAPI.h
        prime-practice-native/PrimeAPI/include/rstl/string.h
        prime-practice-native/PrimeAPI/include/rstl/rstl.h
        prime-practice-native/PrimeAPI/include/rstl/vector.h
        prime-practice-native/src/include/prime/fnAddr.cpp
        prime-practice-native/src/include/prime/CStateManager.h
        prime-practice-native/src/include/dvd.h
        prime-practice-native/src/include/os.h
        src/QuickplayModule.cpp
        )

patch_function("CGraphics::EndScene()"
        "RenderHook()")
patch_function("CPauseScreen::Draw()"
        "PauseScreenDrawReplacement(CPauseScreen *)")
patch_function("CPauseScreen::ProcessControllerInput(const CStateManager &, const CFinalInput &)"
        "PauseControllerInputHandler(CPauseScreen *, CStateManager &, const CFinalInput &)")
patch_function("CMainFlow::OnMessage(const CArchitectureMessage&, CArchitectureQueue&)"
        "IOWinMessageHook(CMainFlow *, const CArchitectureMessage &, CArchitectureQueue &)")
patch_function("CStateManager::DrawDebugStuff() const"
        "drawDebugStuff(CStateManager *)")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  patch_function("CMainFlow::AdvanceGameState(CArchitectureQueue&)"
          "Hook_CMainFlow_AdvanceGameState(CMainFlow*, CArchitectureQueue&)")
endif()

add_prime_library(default-prime-practice default.dol ${SOURCE_FILES})