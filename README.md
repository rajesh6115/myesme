[![Build Status](https://travis-ci.org/rajesh6115/myesme.svg?branch=master)](https://travis-ci.org/rajesh6115/myesme)
This is an ESME Implementation Using SMPP protocol.
Learing Project Where you can find steps in developing this BULKSMS project.

STEPS
------
1. Just Simple Sending SMS usin submitsm pdu
2. Made Application Recive Deliversm pdu for Delivery report
3. Made Application to support unicode and flash sms concept
4. Made Application to fetch smses from mysql database
5. Changed logic of updating sms status directly to mysql
6. Made Application to support multiple campaign
7. Made Application to support multiple SMPP clients/multibind
8. used std::map for storing intermediate states
9. added thereadpool concept to get high performance

TESTS AND RESULTS
------------------
1. mysql is performing low (AS I am not good in mysql concepts)
2. POSIX message queues (Good But Required to change OS limits and permissions)

NOTE:
-----
1. In my test setup i am able to get 100 sms/sec with single connection to my test setup (no network latency)
2. with 2 tx binds i am able to get 200 sms/sec 
3. with 5 tx binds I am able to get 500 sms/sec but memory usage touching 800Mb almost, If I am not getting delivery reports from SMSC Application is crashing after 25,00,000 submitsm. which is not good.

IN ALL SENARIO CPU USAGE IS VERY LESS BELOW 10%. (I have 4 core i5 4th gen CPU with 8Gb RAM)

HOW TO INSTALL
-----------------------------------------------
1. INSTALL REQUIRED PACKAGES (UBUNTU)
	# sudo apt-get install g++ libtool autoconf automake libmysqlclient-dev exuberant-ctags libxml2-dev pkg-config make
2. FORTESTING 
	# sudo apt-get install openjdk-7-jre
3. Generate Configure
	# autoreconf -i
	# ./configure --prefix=/usr
4. Make Project
	# make
5. Install Project
	# make install or sudo make install

HOW TO START APPLICATION
------------------------
Application need configuration file from where application will get important informations. Sample configuration files also included.
1. Main Applicatio config file (src/MySmsAppConfig.xml)
2. SMPP Connection config file (src/SmppConnection.xml)

STEP 1: MODIFY Configfiles according to your testsetup 
STEP 2: myesme <main configuration file>
	ex: sudo /usr/bin/myesme /usr/etc/MySmsAppConfig.xml&


APPLICATION LOG
----------------
PATH: you can set path in main app config file
it can be like /var/log/myesme/xxxxxx.log
