CREATE TABLE `CampaignMaster` (
  `iSNo` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `vcCampaignName` varchar(64) NOT NULL,
  `vcCampaignFile` varchar(256) NOT NULL,
  `dtCreatedDatetime` datetime NOT NULL,
  `dtStartDatetime` datetime DEFAULT NULL,
  `dtCompleteDatetime` datetime DEFAULT NULL,
  `vcTableName` varchar(128) DEFAULT NULL,
  `iDestTon` tinyint(4) unsigned DEFAULT NULL,
  `iDestNpi` tinyint(4) unsigned DEFAULT NULL,
  `iSrcTon` tinyint(4) unsigned DEFAULT NULL,
  `iSrcNpi` tinyint(4) unsigned DEFAULT NULL,
  `vcSource` varchar(21) NOT NULL,
  `iActive` tinyint(1) NOT NULL,
  `iCampaignType` tinyint(4) unsigned NOT NULL,
  `iStatus` tinyint(4) unsigned NOT NULL,
  PRIMARY KEY (`iSNo`),
  UNIQUE KEY `vcCampaignName` (`vcCampaignName`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;
