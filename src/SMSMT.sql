CREATE TABLE `SMSMT` (
  `iSNo` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `vcMsisdn` varchar(21) NOT NULL,
  `vcSource` varchar(21) DEFAULT NULL,
  `vcMessage` varchar(512) NOT NULL,
  `iSrcTon` tinyint(3) unsigned DEFAULT NULL,
  `iSrcNpi` tinyint(3) unsigned DEFAULT NULL,
  `iDestTon` tinyint(3) unsigned DEFAULT NULL,
  `iDestNpi` tinyint(3) unsigned DEFAULT NULL,
  `iMsgType` tinyint(3) unsigned DEFAULT NULL,
  `iPduId` int(10) unsigned DEFAULT NULL,
  `iStatus` tinyint(3) unsigned NOT NULL,
  `iCampaignId` int(10) unsigned NOT NULL,
  `dtSubmitDatetime` datetime DEFAULT NULL,
  `dtDeliveryDatetime` datetime DEFAULT NULL,
  `vcMsgId` varchar(65) DEFAULT NULL,
  PRIMARY KEY (`iSNo`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;
