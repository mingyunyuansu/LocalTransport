set(SOURCE
    socket.cc
    sender.cc
    receiver.cc
)

add_library(local_transport STATIC ${SOURCE})
