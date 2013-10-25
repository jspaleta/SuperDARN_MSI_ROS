struct DEV_reg
{
        unsigned long timereq;
        unsigned long eventreq;
        unsigned long unlock1;
        unsigned long unlock2;
        unsigned long control;
        unsigned long ack;
        unsigned long mask;
        unsigned long intstat;
        unsigned long minstrb;
        unsigned long majstrb;
        unsigned long event2_0;
        unsigned long event2_1;
        unsigned long time0;
        unsigned long time1;
        unsigned long event0;
        unsigned long event1;
} ;

struct DP_ram_ofs
{
        char *input;
        char *output;
        char *GPS;
        char *year;
};


/* PCI data */
#define DATUM_PCI_VENDOR_ID     0x12E2
#define DATUM_PCI_DEVICE_ID     0x4013
#define DATUM_DPRAM_TOP         0x800

/* Device register control field bit definitions */
#define DATUM_CONTROL_BIT_LOCKEN        0x01
#define DATUM_CONTROL_BIT_EVSOURCE      0x02
#define DATUM_CONTROL_BIT_EVSENSE       0x04
#define DATUM_CONTROL_BIT_EVENTEN       0x08
#define DATUM_CONTROL_BIT_STREN         0x10
#define DATUM_CONTROL_BIT_STRMODE       0x20
#define DATUM_CONTROL_BIT_FREQSEL0      0x40
#define DATUM_CONTROL_BIT_FREQSEL1      0x80

/* Device register interrupt mask and status field bit definitions */
#define DATUM_INT_BIT_EVENT             0x01
#define DATUM_INT_BIT_PERIOD    0x02
#define DATUM_INT_BIT_STROBE    0x04
#define DATUM_INT_BIT_1PPS              0x08
#define DATUM_INT_BIT_GPS               0x10
#define DATUM_INT_BIT_ALL               ( DATUM_INT_BIT_EVENT |\
                                                                  DATUM_INT_BIT_PERIOD|\
                                                                  DATUM_INT_BIT_STROBE|\
                                                                  DATUM_INT_BIT_1PPS  |\
                                                                  DATUM_INT_BIT_GPS )

/* Time data status bit field definitions */
#define DATUM_STATUS_BIT_FLY    0x01
#define DATUM_STATUS_BIT_TOFS   0x02
#define DATUM_STATUS_BIT_FOFS   0x04

/* Device register acknowledge field bit definitions */
#define DATUM_ACK_BIT_REC               0x01
#define DATUM_ACK_BIT_GPS               0x02
#define DATUM_ACK_BIT_CMD               0x80

/* Dual port ram commands */
#define DATUM_SET_TIMING_MODE_CMD       0x10
#       define DATUM_MODE_TIMECODE              0x00
#       define DATUM_MODE_FREERUN               0x01
#       define DATUM_MODE_EXT1PPS               0x02
#       define DATUM_MODE_RTC                   0x03
#       define DATUM_MODE_GPS                   0x06

#define DATUM_SET_TIME_FORMAT_CMD       0x11
#       define DATUM_TIME_FMT_BCD               0x00
#       define DATUM_TIME_FMT_UNIX              0x01

#define DATUM_SET_TIME_CMD                      0x12

#define DATUM_SET_YEAR_CMD                      0x13

#define DATUM_SET_PERIODIC_CMD          0x14
#       define DATUM_NO_SYNC_PPS                0x00
#       define DATUM_SYNC_PPS                   0x01

#define DATUM_SET_TC_FORMAT_CMD         0x15
#       define DATUM_TIME_CODE_IRIGA    0x41
#       define DATUM_TIME_CODE_IRIGB    0x42
#       define DATUM_TIME_CODE_1344             0x49

#define DATUM_SET_TC_MOD_CMD            0x16
#       define DATUM_MODE_AMP                   0x4D
#       define DATUM_MODE_PULSE                 0x44

#define DATUM_SET_TIMING_OFFSET_CMD     0x17

#define DATUM_REQ_UTC_TM_DATA_CMD       0x18
#       define DATUM_GPS_TIME_UTC               0x00
#       define DATUM_GPS_TIME_GPS               0x01
#       define DATUM_LEAP_DELETE                0xFF
#       define DATUM_LEAP_NONE                  0x00
#       define DATUM_LEAP_ADD                   0x01

#define DATUM_GET_DATA_CMD                      0x19
#       define DATUM_GET_DATA_MODE              0x10
#       define DATUM_GET_DATA_TFORMAT   0x11
#       define DATUM_GET_DATA_YEAR              0x13
#       define DATUM_GET_DATA_PER               0x14
#       define DATUM_GET_DATA_TCFMT             0x15
#       define DATUM_GET_DATA_TCMOD             0x16
#       define DATUM_GET_DATA_TOFF              0x17
#       define DATUM_GET_DATA_UTCINFO   0x18

#       define DATUM_GET_DATA_TCOUTFMT  0x1B
#       define DATUM_GET_DATA_TCGENOFF  0x1C
#       define DATUM_GET_DATA_LOCTMOFF  0x1D
#       define DATUM_GET_DATA_LEAPSEC   0x1E

#       define DATUM_GET_DATA_FWVER             0x1F
#       define DATUM_GET_DATA_CLKSRC    0x20
#       define DATUM_GET_DATA_JAMSC             0x21
#       define DATUM_GET_DATA_OSCCTL    0x23
#       define DATUM_GET_DATA_DAVAL             0x24

#       define DATUM_GET_DATA_BATTSTAT  0x26
#       define DATUM_GET_DATA_CLKVAL    0x29
#       define DATUM_GET_DATA_DTFW              0x4F
#       define DATUM_GET_DATA_ASMB              0xF4
#       define DATUM_GET_DATA_HWFAB             0xF5
#       define DATUM_GET_DATA_TFPMODEL  0xF6
#       define DATUM_GET_DATA_SERIAL    0xFE

#define DATUM_SOFT_RESET_CMD            0x1A

#define DATUM_SET_TC_OUT_FMT_CMD        0x1B

#define DATUM_SET_GEN_TM_OFF_CMD        0x1C
#       define DATUM_HALF_NO                    0x00
#       define DATUM_HALF_YES                   0x01

#define DATUM_SET_LOC_TM_OFF_CMD        0x1D

#define DATUM_SET_LEAP_SEC_CMD          0x1E

#define DATUM_SEL_CLOCK_CMD                     0x20
#       define DATUM_SEL_CLOCK_INT              0x49
#       define DATUM_SEL_CLOCK_EXT              0x45

#define DATUM_JAM_SYNC_ENABLE_CMD       0x21
#       define DATUM_JAMSYNC_DIS                0x00
#       define DATUM_JAMSYNC_EN                 0x01

#define DATUM_FORCEJAMSYNC_CMD          0x22

#define DATUM_DISC_ENABLE_CMD           0x23
#       define DATUM_DISC_DIS                   0x00
#       define DATUM_DISC_EN                    0x01

#define DATUM_SET_DAC_CMD                       0x24

#define DATUM_SET_DISC_GAIN_CMD         0x25

#define DATUM_REQ_BATT_STAT_CMD         0x26
#       define DATUM_BATT_FAILED                0x00
#       define DATUM_BATT_OK                    0x10

#define DATUM_SYNC_RTC_CMD                      0x27

#define DATUM_DISC_BATT_RTC_CMD         0x28

#define DATUM_SET_CLK_VAL_CMD           0x29

#define DATUM_SEND_GPS_PKT_CMD          0x30

#define DATUM_REQ_GPS_PKT_CMD           0x31

#define DATUM_MAN_REQ_GPS_PKT_CMD       0x32

#define DATUM_GPS_TIME_FORMAT_CMD       0x33

#define DATUM_SET_GPS_MD_FLG_CMD        0x34
#       define DATUM_DISABLE                    0x00
#       define DATUM_ENABLE                             0x01

#define DATUM_SET_LOC_TM_FLG_CMD        0x40

#define DATUM_REQ_DAYLT_FLG_CMD         0x41
#       define DATUM_LOCAL_ENABLE               0x08

#define DATUM_SET_YR_INC_FLG_CMD        0x42

#define DATUM_SELECT_PER_DDS_CMD        0x43
#       define DATUM_SELECT_PER             0x00
#       define DATUM_SELECT_DDS             0x01

#define DATUM_ENABLE_PER_DDS_CMD        0x44
#       define DATUM_DISABLE_PER            0x00 
#       define DATUM_ENABLE_PER             0x01 

#define DATUM_REQ_DTFW_NUM_CMD          0x4F
#       define DATUM_ID_DTFW                    "DT6000"

#define DATUM_REQ_ASSEMB_CMD            0xF4
#       define DATUM_OLD_ASM_PART               12043
#       define DATUM_NEW_ASM_PART               12083

#define DATUM_REQ_HW_FAB_CMD            0xF5
#       define DATUM_OLD_FAB_PART               12042
#       define DATUM_NEW_FAB_PART               12083

#define DATUM_REQ_TFP_MODEL_CMD         0xF6
#       define DATUM_ID_IRIG                    "BC635PCI"
#       define DATUM_ID_GPS                             "BC637PCI"

#define DATUM_REQ_SER_NUM_CMD           0xFE

