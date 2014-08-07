echo ""
echo ""
echo "+--------------------------------------------------------------------------+"
echo "| make sure that the all the environment variables for stsdk build are set |"
echo "+--------------------------------------------------------------------------+"
echo ""
echo ""

if [[ "$AXE_MICROCONNECT_IP_ADDRESS" =~ "^([0-9]{1,3}\.){3}[0-9]{1,3}$" ]] ; then
  echo "running u-boot with gdb... "
  sh4-linux-gdb \
  --eval-command="set pagination off" \
  --eval-command="sh4tp $AXE_MICROCONNECT_IP_ADDRESS:idl4kstx7108:host,no_convertor_abort=1,active_cores=host:rt,tapmux_mux=1,dual_cpu=1,boardrev=1,lmi_contig=1,overclk=0" \
  --eval-command="symbol-file u-boot" \
  --eval-command="load u-boot" \
  --eval-command="set pagination on"
else
  echo "You have to define AXE_MICROCONNECT_IP_ADDRESS with valid IP address!!!"
fi

