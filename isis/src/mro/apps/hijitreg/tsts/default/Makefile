APPNAME = hijitreg

include $(ISISROOT)/make/isismake.tsts

PSP_001446_1790_RED.flat.txt.IGNORELINES = RunDate: FROM MATCH RegFile

commands:
	$(APPNAME) from=$(INPUT)/PSP_001446_1790_RED2.balance.cub \
	           match=$(INPUT)/PSP_001446_1790_RED3.balance.cub \
	           flatfile=$(OUTPUT)/PSP_001446_1790_RED.flat.txt \
	           cnetfile=$(OUTPUT)/PSP_001446_1790_RED.net > /dev/null;
