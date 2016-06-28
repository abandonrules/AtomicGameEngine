include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_SOURCE_DIR}/Source/ThirdParty
                    ${CMAKE_SOURCE_DIR}/Source/ThirdParty/rapidjson/include
                    ${CMAKE_SOURCE_DIR}/Source/ThirdParty/kNet/include
                    ${CMAKE_SOURCE_DIR}/Source/ThirdParty/FreeType/include
                    ${CMAKE_SOURCE_DIR}/Source/ThirdParty/Box2D)

set (CSATOMICNATIVEDIR "${CMAKE_SOURCE_DIR}/Artifacts/Build/Source/Generated/${JAVASCRIPT_BINDINGS_PLATFORM}/CSharp/Packages/Atomic/Native")
set (CSATOMICNETNATIVEDIR "${CMAKE_SOURCE_DIR}/Artifacts/Build/Source/Generated/${JAVASCRIPT_BINDINGS_PLATFORM}/CSharp/Packages/AtomicNET/Native")

file (GLOB CSHARP_BINDINGS_SOURCE ${CSATOMICNATIVEDIR}/*.cpp ${CSATOMICNATIVEDIR}/*.h ${CSATOMICNETNATIVEDIR}/*.cpp ${CSATOMICNETNATIVEDIR}/*.h)

file (GLOB_RECURSE SOURCE_FILES *.cpp *.h)

add_library(AtomicNETNative SHARED ${SOURCE_FILES} ${CSHARP_BINDINGS_SOURCE})

target_link_libraries(AtomicNETNative AtomicJS AtomicPlayerLib AtomicPlayerJS ${ATOMIC_LINK_LIBRARIES})

if (APPLE)

    if (NOT IOS)

        target_link_libraries( AtomicNETNative "-stdlib=libc++ -framework AudioUnit -framework Carbon -framework Cocoa -framework CoreAudio -framework ForceFeedback -framework IOKit -framework OpenGL -framework CoreServices -framework Security")

    endif()

endif()

