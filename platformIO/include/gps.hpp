#include <Wire.h>
//#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include <SparkFun_u-blox_GNSS_v3.h>

#define ZED_F9P_I2C_ADDRESS 0x42 // Adresse I2C par défaut
//#define ZED_F9P_I2C_ADDRESS 0x21 // Adresse I2C trouvée par scan I2C

class GNSS
{
    public:
        SFE_UBLOX_GNSS myGNSS; // Objet GNSS pour communiquer avec le ZED-F9P

        GNSS();
        ~GNSS();

        void scanI2C();
        void activeUBX_RTK();
        void configurerTrameNAV_PVT_I2C();
        void lireFluxGPS();
        void gpsInit();
};
