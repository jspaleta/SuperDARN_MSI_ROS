# default .profile
if test "$(tty)" != "not a tty"; then
echo 'edit the file .profile if you want to change your environment.'
echo 'To start the Photon windowing environment, type "ph".'
fi
export SD_RADAR_NAME="azores"
export MSI_CALDIR=/root/operational_radar_code/$SD_RADAR_NAME/site_data/calibrations/
