#/bin/bash

ADRESS="FE:8D:E4:E3:19:69"

#https://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.sdk5.v14.1.0%2Fiot_sdk_user_guides_ipv6_address_creation.html
#Create a modified EUI-64 address.
FLIPPED=$( printf "%X" $(( "0x${ADRESS:0:2}" ^ 0x00 )))   
LINK_LOCAL_ADRESS=${FLIPPED}${ADRESS:3:2}:${ADRESS:6:2}FF:FE${ADRESS:9:2}:${ADRESS:12:2}${ADRESS:15}

echo ${ADRESS:3:2}

echo "$ADRESS -> $LINK_LOCAL_ADRESS"
echo "ping6 -I bt0 FE80::$LINK_LOCAL_ADRESS"
