#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>

#define ARDUINO_uSD_CS 10
#define LOG_FILE_PREFIX "GPSLOG"
#define LOG_FILE_SUFFIX "csv"
#define MAX_LOG_FILES 100
#define LOG_COLUMN_COUNT 8
#define LOG_RATE 1000
#define GPS_BAUD 115200
#define SERIAL_BAUD 115200
#define ARDUINO_GPS_RX 9
#define ARDUINO_GPS_TX 8
#define GPS_PORT SS_GPS
#define SERIAL_MONITOR Serial

char LOG_FILE_NAME[13];
char * LOG_COLUMN_NAMES[LOG_COLUMN_COUNT] = {"LATITUDE", "LONGITUDE", "ALTITUDE", "VELOCITY", "HEADING", "DATE", "TIME", "SATELLITES"};
unsigned long LAST_LOG = 0;
TinyGPSPlus TINY_GPS;
SoftwareSerial SS_GPS(ARDUINO_GPS_TX, ARDUINO_GPS_RX);

void setup()
{
  SERIAL_MONITOR.begin(SERIAL_BAUD);
  GPS_PORT.begin(GPS_BAUD);

  SERIAL_MONITOR.println("+------------------------------------+");
  SERIAL_MONITOR.println("|   TinyGPS++ Improved Data Logger   |");
  SERIAL_MONITOR.println("|        Created by Ethan Och        |");
  SERIAL_MONITOR.println("+------------------------------------+");
  SERIAL_MONITOR.println();
  SERIAL_MONITOR.println("Checking for SD card...");
  SERIAL_MONITOR.println();
  
  if (!SD.begin(ARDUINO_uSD_CS))
  {
    SERIAL_MONITOR.println("Error initializing SD card:");
    SERIAL_MONITOR.println("Please validate the presence and proper placement of the card in the Arduino.");
    SERIAL_MONITOR.println();
    
    while (!SD.begin(ARDUINO_uSD_CS))
    {
      
    }
  }

  SERIAL_MONITOR.println("SD card found.");
  SERIAL_MONITOR.println();

  UPDATE_FILE_NAME();
  PRINT_HEADER();
    
  SERIAL_MONITOR.println("*** STARTING DATALOGGING ***");
}

void loop()
{
  if ((LAST_LOG + LOG_RATE) <= millis())
  {
    if (TINY_GPS.location.isUpdated())
    {
      SERIAL_MONITOR.print("[");
      SERIAL_MONITOR.print(TINY_GPS.date.value());
      SERIAL_MONITOR.print("][");
      SERIAL_MONITOR.print(TINY_GPS.time.value());
      SERIAL_MONITOR.print("] ");
      
      if (LOG_GPS_DATA())
      {
        SERIAL_MONITOR.println("GPS data logged.");
        LAST_LOG = millis();
      }
      else
      {
        SERIAL_MONITOR.println("Failed to log new GPS data.");
      }
    }
    else
    {
      SERIAL_MONITOR.print("[");
      SERIAL_MONITOR.print(TINY_GPS.date.value());
      SERIAL_MONITOR.print("][");
      SERIAL_MONITOR.print(TINY_GPS.time.value());
      SERIAL_MONITOR.print("] No GPS data found. Number of satellite connections: ");
      SERIAL_MONITOR.println(TINY_GPS.satellites.value());
    }
  }

  while (GPS_PORT.available())
  {
    TINY_GPS.encode(GPS_PORT.read());
  }
}

byte LOG_GPS_DATA()
{
  File LOG_FILE = SD.open(LOG_FILE_NAME, FILE_WRITE);

  if (LOG_FILE)
  {
    LOG_FILE.print(TINY_GPS.location.lat(), 6);
    LOG_FILE.print(',');
    LOG_FILE.print(TINY_GPS.location.lng(), 6);
    LOG_FILE.print(',');
    LOG_FILE.print(TINY_GPS.altitude.feet(), 1);
    LOG_FILE.print(',');
    LOG_FILE.print(TINY_GPS.speed.mph(), 1);
    LOG_FILE.print(',');
    LOG_FILE.print(TINY_GPS.course.deg(), 1);
    LOG_FILE.print(',');
    LOG_FILE.print(TINY_GPS.date.value());
    LOG_FILE.print(',');
    LOG_FILE.print(TINY_GPS.time.value());
    LOG_FILE.print(',');
    LOG_FILE.print(TINY_GPS.satellites.value());
    LOG_FILE.println();
    LOG_FILE.close();

    return 1;
  }

  return 0;
}

void PRINT_HEADER()
{
  File LOG_FILE = SD.open(LOG_FILE_NAME, FILE_WRITE);

  if (LOG_FILE)
  {
    for (int i = 0; i < LOG_COLUMN_COUNT; i++)
    {
      LOG_FILE.print(LOG_COLUMN_NAMES[i]);
      
      if (i < (LOG_COLUMN_COUNT - 1))
      {
        LOG_FILE.print(',');
      }
      else
      {
        LOG_FILE.println();
      }
    }

    LOG_FILE.close();
  }
}

void UPDATE_FILE_NAME()
{
  for (int i = 0; i < MAX_LOG_FILES; i++)
  {
    memset(LOG_FILE_NAME, 0, strlen(LOG_FILE_NAME));

    if (i < 10)
    {
      int z = 0;
      sprintf(LOG_FILE_NAME, "%s%d%d.%s", LOG_FILE_PREFIX, z, i, LOG_FILE_SUFFIX);
    }
    else
    {
      sprintf(LOG_FILE_NAME, "%s%d.%s", LOG_FILE_PREFIX, i, LOG_FILE_SUFFIX);
    }

    if (!SD.exists(LOG_FILE_NAME))
    {
      break;
    }
    else
    {
      SERIAL_MONITOR.print(LOG_FILE_NAME);
      SERIAL_MONITOR.println(" already exists.");
    }
  }

  SERIAL_MONITOR.print("File name for this log: ");
  SERIAL_MONITOR.println(LOG_FILE_NAME);
  SERIAL_MONITOR.println();
}

