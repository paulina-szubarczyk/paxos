find_package( Protobuf REQUIRED )

PROTOBUF_GENERATE_CPP(ProtoSources ProtoHeaders messages.proto)

add_library(PaxosProtocol STATIC ${ProtoSources} ${ProtoHeaders})

target_include_directories(PaxosProtocol PUBLIC ${PROTOBUF_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(PaxosProtocol PUBLIC ${PROTOBUF_LIBRARIES})
