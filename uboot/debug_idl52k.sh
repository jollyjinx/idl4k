echo ""
echo ""
echo "+--------------------------------------------------------------------------+"
echo "| make sure that the all the environment variables for stsdk build are set |"
echo "+--------------------------------------------------------------------------+"
echo ""
echo ""

if [ "$AXE_JTAG_KEY_FILE" = "" ] ; then
  echo "No key provided (export AXE_JTAG_KEY_FILE to unlock the device)"
  EXTRA_ARG="post_reset_unlock=1"
else
  EXTRA_ARG="post_reset_unlock=1,tmcreglength=130,keyfile=$AXE_JTAG_KEY_FILE"
fi

if [[ "$AXE_MICROCONNECT_IP_ADDRESS" =~ "^([0-9]{1,3}\.){3}[0-9]{1,3}$" ]] ; then
  echo "running u-boot with gdb... "
  sh4-linux-gdb \
  --eval-command="set pagination off" \
  --eval-command="sh4tp $AXE_MICROCONNECT_IP_ADDRESS:idl52k:st40,se=1,overclk=2,$EXTRA_ARG" \
  --eval-command="symbol-file u-boot" \
  --eval-command="load u-boot" \
  --eval-command="set pagination on"
else
  echo "You have to define AXE_MICROCONNECT_IP_ADDRESS with valid IP address!!!"
fi

