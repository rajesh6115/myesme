bin_PROGRAMS = createmq deletemq writemq readmq
createmq_SOURCES = CreateMQ.cpp
createmq_CPPFLAGS = -I ${top_srcdir}/include 
createmq_LDFLAGS = -lrt

deletemq_SOURCES = DeleteMQ.cpp
deletemq_CPPFLAGS = -I ${top_srcdir}/include 
deletemq_LDFLAGS = -lrt

writemq_SOURCES = writetoqueue.cpp ${top_srcdir}/src/MessageQueue.cpp
writemq_CPPFLAGS = -I ${top_srcdir}/include -std=c++11
writemq_LDFLAGS = -lrt

readmq_SOURCES = readmessage.cpp ${top_srcdir}/src/MessageQueue.cpp
readmq_CPPFLAGS = -I ${top_srcdir}/include -std=c++11
readmq_LDFLAGS = -lrt

