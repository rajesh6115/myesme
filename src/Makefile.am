ACLOCAL_AMFLAGS = -I m4
lib_LIBRARIES = libsmpp.a libsmppapputil.a
libsmpp_a_SOURCES =  \
alert_notification.cpp     broadcast_sm.cpp              data_sm_resp.cpp       outbind.cpp                  submit_multi_resp.cpp \
aux_types.cpp              broadcast_sm_resp.cpp         deliver_sm.cpp         query_broadcast_sm.cpp       submit_sm.cpp \
bind_receiver.cpp          buffer.cpp                    deliver_sm_resp.cpp    query_broadcast_sm_resp.cpp  submit_sm_resp.cpp \
bind_receiver_resp.cpp     cancel_broadcast_sm.cpp       enquire_link.cpp       query_sm.cpp                 unbind.cpp \
bind_transceiver.cpp       cancel_broadcast_sm_resp.cpp  enquire_link_resp.cpp  query_sm_resp.cpp            unbind_resp.cpp \
bind_transceiver_resp.cpp  cancel_sm.cpp                 error.cpp              replace_sm.cpp \
bind_transmitter.cpp       cancel_sm_resp.cpp            generic_nack.cpp       replace_sm_resp.cpp \
bind_transmitter_resp.cpp  data_sm.cpp                   header.cpp             submit_multi.cpp
libsmpp_a_CPPFLAGS = -I ${top_srcdir}/include

libsmppapputil_a_SOURCES = clogger.cpp appconfig.cpp NetBuffer.cpp 
libsmppapputil_a_CPPFLAGS = -I ${top_srcdir}/include ${libxml2_CFLAGS}

lib_LTLIBRARIES = libsmpp.la libsmppapputil.la
libsmpp_la_SOURCES =	\
alert_notification.cpp     broadcast_sm.cpp              data_sm_resp.cpp       outbind.cpp                  submit_multi_resp.cpp \
aux_types.cpp              broadcast_sm_resp.cpp         deliver_sm.cpp         query_broadcast_sm.cpp       submit_sm.cpp \
bind_receiver.cpp          buffer.cpp                    deliver_sm_resp.cpp    query_broadcast_sm_resp.cpp  submit_sm_resp.cpp \
bind_receiver_resp.cpp     cancel_broadcast_sm.cpp       enquire_link.cpp       query_sm.cpp                 unbind.cpp \
bind_transceiver.cpp       cancel_broadcast_sm_resp.cpp  enquire_link_resp.cpp  query_sm_resp.cpp            unbind_resp.cpp \
bind_transceiver_resp.cpp  cancel_sm.cpp                 error.cpp              replace_sm.cpp \
bind_transmitter.cpp       cancel_sm_resp.cpp            generic_nack.cpp       replace_sm_resp.cpp \
bind_transmitter_resp.cpp  data_sm.cpp                   header.cpp             submit_multi.cpp 
libsmpp_la_CPPFLAGS=-fPIC -I ${top_srcdir}/include
libsmpp_la_LDFLAGS= -shared -fPIC

libsmppapputil_la_SOURCES = clogger.cpp appconfig.cpp NetBuffer.cpp MySqlWrapper.cpp
libsmppapputil_la_CPPFLAGS = -fPIC -I ${top_srcdir}/include ${libxml2_CFLAGS} -std=c++11 `mysql_config --cflags`
libsmppapputil_la_LDFLAGS = -shared -fPIC ${libxml2_LIBS} `mysql_config --libs`

bin_PROGRAMS = myesme esmedbins
myesme_SOURCES = Main.cpp Esme.cpp EsmePduProcessThread.cpp EsmeSenderThread.cpp EsmeConfig.cpp SmsAppConfig.cpp Externs.cpp SmsCampaign.cpp EsmeGetterSetter.cpp MessageQueue.cpp
myesme_CPPFLAGS = -I ${top_srcdir}/include ${libxml2_CFLAGS} ${libevent_CFLAGS} -std=c++11 `mysql_config --cflags` -D_MESSAGE_QUEUE_UPDATE_ -DWITH_THREAD_POOL 
myesme_LDFLAGS = ${libxml2_LIBS} ${libevent_LIBS} libsmpp.la libsmppapputil.la -lpthread `mysql_config --libs` -lrt -levent_pthreads

esmedbins_SOURCES = SmsDbInsertConfig.cpp DbInsertApp.cpp MessageQueue.cpp
esmedbins_CPPFLAGS = -I ${top_srcdir}/include ${libxml2_CFLAGS} -std=c++11 `mysql_config --cflags` -D_MESSAGE_QUEUE_UPDATE_
esmedbins_LDFLAGS = ${libxml2_LIBS} libsmppapputil.la -lpthread `mysql_config --libs` -lrt

sysconf_DATA = MySmsAppConfig.xml SmppConnection.xml SmsDbInsert.xml
