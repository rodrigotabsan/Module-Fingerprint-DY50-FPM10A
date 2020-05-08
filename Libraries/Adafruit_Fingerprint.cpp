/***************************************************
  This is a library for our optical Fingerprint sensor

  Designed specifically to work with the Adafruit Fingerprint sensor
  ----> http://www.adafruit.com/products/751

  These displays use TTL Serial to communicate, 2 pins are required to
  interface
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include "Adafruit_Fingerprint.h"

//#define FINGERPRINT_DEBUG

#if ARDUINO >= 100
  #define SERIAL_WRITE(...) mySerial->write(__VA_ARGS__)
#else
  #define SERIAL_WRITE(...) mySerial->write(__VA_ARGS__, BYTE)
#endif

#define SERIAL_WRITE_U16(v) SERIAL_WRITE((uint8_t)(v>>8)); SERIAL_WRITE((uint8_t)(v & 0xFF));

#define GET_CMD_PACKET(...) \
  uint8_t data[] = {__VA_ARGS__}; \
  Adafruit_Fingerprint_Packet packet(FINGERPRINT_COMMANDPACKET, sizeof(data), data); \
  writeStructuredPacket(packet); \
  if (getStructuredPacket(&packet) != FINGERPRINT_OK) return FINGERPRINT_PACKETRECIEVEERR; \
  if (packet.type != FINGERPRINT_ACKPACKET) return FINGERPRINT_PACKETRECIEVEERR;

#define SEND_CMD_PACKET(...) GET_CMD_PACKET(__VA_ARGS__); return packet.data[0];

/***************************************************************************
 PUBLIC FUNCTIONS
			FINGERPRINT_COMMANDPACKET, sizeof(data), data); \  Oxo1 , N , data
			!= FINGERPRINT_OK) return FINGERPRINT_PACKETRECIEVEERR; \ // != 0x00 return 0x01
			!= FINGERPRINT_ACKPACKET) return FINGERPRINT_PACKETRECIEVEERR; // != 0x07 return 0x01
 ***************************************************************************/


#if defined(__AVR__) || defined(ESP8266) || defined(FREEDOM_E300_HIFIVE1)
/**************************************************************************/
/*!
    @brief  Instantiates sensor with Software Serial
    @param  ss Pointer to SoftwareSerial object
    @param  password 32-bit integer password (default is 0)
*/
/**************************************************************************/
Adafruit_Fingerprint::Adafruit_Fingerprint(SoftwareSerial *ss, uint32_t password) {
  thePassword = password;
  theAddress = 0xFFFFFFFF;

  hwSerial = NULL;
  swSerial = ss;
  mySerial = swSerial;
}
#endif

/**************************************************************************/
/*!
    @brief  Instantiates sensor with Hardware Serial
    @param  hs Pointer to HardwareSerial object
    @param  password 32-bit integer password (default is 0)

*/
/**************************************************************************/
Adafruit_Fingerprint::Adafruit_Fingerprint(HardwareSerial *hs, uint32_t password) {
  thePassword = password;
  theAddress = 0xFFFFFFFF;

#if defined(__AVR__) || defined(ESP8266) || defined(FREEDOM_E300_HIFIVE1)
  swSerial = NULL;
#endif
  hwSerial = hs;
  mySerial = hwSerial;
}

/**************************************************************************/
/*!
    @brief  Initializes serial interface and baud rate
    @param  baudrate Sensor's UART baud rate (usually 57600, 9600 or 115200)
*/
/**************************************************************************/
void Adafruit_Fingerprint::begin(uint32_t baudrate) {
  delay(1000);  // one second delay to let the sensor 'boot up'

  if (hwSerial) hwSerial->begin(baudrate);
#if defined(__AVR__) || defined(ESP8266) || defined(FREEDOM_E300_HIFIVE1)
  if (swSerial) swSerial->begin(baudrate);
#endif
}

/**************************************************************************/
/*!
    @brief  Verifies the sensors' access password (default password is 0x0000000). A good way to also check if the sensors is active and responding
    @returns True if password is correct
*/
/**************************************************************************/
boolean Adafruit_Fingerprint::verifyPassword(void) {
  return checkPassword() == FINGERPRINT_OK;
}

uint8_t Adafruit_Fingerprint::checkPassword(void) {
  GET_CMD_PACKET(FINGERPRINT_VERIFYPASSWORD,
                  (uint8_t)(thePassword >> 24), (uint8_t)(thePassword >> 16),
                  (uint8_t)(thePassword >> 8), (uint8_t)(thePassword & 0xFF));
  if (packet.data[0] == FINGERPRINT_OK)
    return FINGERPRINT_OK;
  else
    return FINGERPRINT_PACKETRECIEVEERR;
}

uint8_t Adafruit_Fingerprint::ReadSysPara(void) {
  SEND_CMD_PACKET(FINGERPRINT_READSYSPARA); 	
}

uint8_t Adafruit_Fingerprint::SetSysPara(uint8_t parameter, uint8_t content) {

	setPara(theAddress, FINGERPRINT_COMMANDPACKET, 0x0005, FINGERPRINT_SETSYSPARA, parameter, content);
	 delay(25);
}

void Adafruit_Fingerprint::setPara(uint32_t addr, uint8_t packettype, uint16_t len, uint8_t instruct, uint8_t para, uint8_t content) {

#if ARDUINO >= 100
  mySerial->write((uint8_t)(FINGERPRINT_STARTCODE >> 8));
  mySerial->write((uint8_t)FINGERPRINT_STARTCODE);
  mySerial->write((uint8_t)(addr >> 24));
  mySerial->write((uint8_t)(addr >> 16));
  mySerial->write((uint8_t)(addr >> 8));
  mySerial->write((uint8_t)(addr));
  mySerial->write((uint8_t)packettype);
  mySerial->write((uint8_t)(len >> 8));
  mySerial->write((uint8_t)(len));
    mySerial->write((uint8_t)(instruct));
	mySerial->write((uint8_t)(para));
	mySerial->write((uint8_t)(content));
#else
  mySerial->print((uint8_t)(FINGERPRINT_STARTCODE >> 8), BYTE);
  mySerial->print((uint8_t)FINGERPRINT_STARTCODE, BYTE);
  mySerial->print((uint8_t)(addr >> 24), BYTE);
  mySerial->print((uint8_t)(addr >> 16), BYTE);
  mySerial->print((uint8_t)(addr >> 8), BYTE);
  mySerial->print((uint8_t)(addr), BYTE);
  mySerial->print((uint8_t)packettype, BYTE);
  mySerial->print((uint8_t)(len >> 8), BYTE);
  mySerial->print((uint8_t)(len), BYTE);
	mySerial->write((uint8_t)(instruct), BYTE);
	mySerial->write((uint8_t)(para), BYTE);
	mySerial->write((uint8_t)(content), BYTE);
#endif
  uint16_t sum = (len>>8) + (len&0xFF) + packettype + instruct + para + content;
#if ARDUINO >= 100
  mySerial->write((uint8_t)(sum>>8));
  mySerial->write((uint8_t)sum);
#else
  mySerial->print((uint8_t)(sum>>8), BYTE);
  mySerial->print((uint8_t)sum, BYTE);
#endif
}

/**************************************************************************/
/*!
    @brief   Ask the sensor to take an image of the finger pressed on surface
    @returns <code>FINGERPRINT_OK</code> on success
    @returns <code>FINGERPRINT_NOFINGER</code> if no finger detected
    @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
    @returns <code>FINGERPRINT_IMAGEFAIL</code> on imaging error
*/
/**************************************************************************/

uint8_t Adafruit_Fingerprint::getImage(void) {
  SEND_CMD_PACKET(FINGERPRINT_GETIMAGE); 	
}

uint8_t Adafruit_Fingerprint::downImage(void) {
  SEND_CMD_PACKET(FINGERPRINT_DOWNIMAGE); 	
}

/**************************************************************************/
/*!
    @brief   Ask the sensor to convert image to feature template
    @param slot Location to place feature template (put one in 1 and another in 2 for verification to create model)
    @returns <code>FINGERPRINT_OK</code> on success
    @returns <code>FINGERPRINT_IMAGEMESS</code> if image is too messy
    @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
    @returns <code>FINGERPRINT_FEATUREFAIL</code> on failure to identify fingerprint features
    @returns <code>FINGERPRINT_INVALIDIMAGE</code> on failure to identify fingerprint features
*/

uint8_t Adafruit_Fingerprint::image2Tz(uint8_t slot) {
  SEND_CMD_PACKET(FINGERPRINT_IMAGE2TZ,slot);
}

/**************************************************************************/
/*!
    @brief   Ask the sensor to take two print feature template and create a model
    @returns <code>FINGERPRINT_OK</code> on success
    @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
    @returns <code>FINGERPRINT_ENROLLMISMATCH</code> on mismatch of fingerprints
*/
uint8_t Adafruit_Fingerprint::createModel(void) {
  SEND_CMD_PACKET(FINGERPRINT_REGMODEL);
}


/**************************************************************************/
/*!
    @brief   Ask the sensor to store the calculated model for later matching
    @param   location The model location #
    @returns <code>FINGERPRINT_OK</code> on success
    @returns <code>FINGERPRINT_BADLOCATION</code> if the location is invalid
    @returns <code>FINGERPRINT_FLASHERR</code> if the model couldn't be written to flash memory
    @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
*/
uint8_t Adafruit_Fingerprint::storeModel(uint16_t location) {
  SEND_CMD_PACKET(FINGERPRINT_STORE, 0x01, (uint8_t)(location >> 8), (uint8_t)(location & 0xFF));
}

/**************************************************************************/
/*!
    @brief   Ask the sensor to load a fingerprint model from flash into buffer 1
    @param   location The model location #
    @returns <code>FINGERPRINT_OK</code> on success
    @returns <code>FINGERPRINT_BADLOCATION</code> if the location is invalid
    @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
*/

uint8_t Adafruit_Fingerprint::loadModel(uint16_t location) {
  SEND_CMD_PACKET(FINGERPRINT_LOAD, 0x01, (uint8_t)(location >> 8), (uint8_t)(location & 0xFF));
}

/**************************************************************************/
/*!
    @brief   Ask the sensor to transfer 256-byte fingerprint template from the buffer to the UART
    @returns <code>FINGE(RPRINT_OK</code> on success
        @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
    */
    
uint8_t Adafruit_Fingerprint::loadModeltwo(uint16_t location) {
  SEND_CMD_PACKET(FINGERPRINT_LOAD, 0x02, (uint8_t)(location >> 8), (uint8_t)(location & 0xFF));
}

uint8_t Adafruit_Fingerprint::getModel(void) {
  SEND_CMD_PACKET(FINGERPRINT_UPLOAD, 0x01);
}

uint8_t Adafruit_Fingerprint::getModel2(void) {
  SEND_CMD_PACKET(FINGERPRINT_UPLOAD, 0x02);
}

/**************************************************************************/
/*!
    @brief   Ask the sensor to delete a model in memory
    @param   location The model location #
    @returns <code>FINGERPRINT_OK</code> on success
    @returns <code>FINGERPRINT_BADLOCATION</code> if the location is invalid
    @returns <code>FINGERPRINT_FLASHERR</code> if the model couldn't be written to flash memory
    @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
*/
uint8_t Adafruit_Fingerprint::deleteModel(uint16_t location) {
  SEND_CMD_PACKET(FINGERPRINT_DELETE, (uint8_t)(location >> 8), (uint8_t)(location & 0xFF), 0x00, 0x01);
}

/**************************************************************************/
/*!
    @brief   Ask the sensor to delete ALL models in memory
    @returns <code>FINGERPRINT_OK</code> on success
    @returns <code>FINGERPRINT_BADLOCATION</code> if the location is invalid
    @returns <code>FINGERPRINT_FLASHERR</code> if the model couldn't be written to flash memory
    @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
*/
uint8_t Adafruit_Fingerprint::emptyDatabase(void) {
  SEND_CMD_PACKET(FINGERPRINT_EMPTY);
}

/**************************************************************************/
/*!
    @brief   Ask the sensor to search the current slot 1 fingerprint features to match saved templates. The matching location is stored in <b>fingerID</b> and the matching confidence in <b>confidence</b>
    @returns <code>FINGERPRINT_OK</code> on fingerprint match success
    @returns <code>FINGERPRINT_NOTFOUND</code> no match made
    @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
*/
/**************************************************************************/
  // high speed search of slot #1 starting at page 0x0000 and page #0x00A3
uint8_t Adafruit_Fingerprint::fingerFastSearch(void) {
  // high speed search of slot #1 starting at page 0x0000 and page #0x00A3		==> A3 = 163
  GET_CMD_PACKET(FINGERPRINT_HISPEEDSEARCH, 0x01, 0x00, 0x00, 0x00, 0xA3);			// ==> FINGERPRINT_HISPEEDSEARCH = 0x1B		0x01 = CharBuffer 1
  fingerID = 0xFFFF;
  confidence = 0xFFFF;

  fingerID = packet.data[1];
  fingerID <<= 8;
  fingerID |= packet.data[2];

  confidence = packet.data[3];
  confidence <<= 8;
  confidence |= packet.data[4];

  return packet.data[0];
}

uint8_t Adafruit_Fingerprint::matchUpload(void) {
  SEND_CMD_PACKET(FINGERPRINT_MATCH);
}


uint8_t Adafruit_Fingerprint::uploadModel(uint8_t packet2[], uint8_t packet3[], uint8_t packet4[], uint8_t packet5[], uint8_t packet6[], uint8_t packet7[]) {
	
    uint8_t packet[] = {FINGERPRINT_DOWNLOAD, 0x01};
	
	 writePacket(theAddress, FINGERPRINT_COMMANDPACKET, sizeof(packet)+2, packet);
	 delay(25);
	 writePacket(theAddress, FINGERPRINT_DATAPACKET, 128+2, packet2);
	 delay(25);
	 writePacket(theAddress, FINGERPRINT_DATAPACKET, 128+2, packet3);
	 delay(25);
	 writePacket(theAddress, FINGERPRINT_DATAPACKET, 128+2, packet4);
	 delay(25);
	 writePacket(theAddress, FINGERPRINT_DATAPACKET, 128+2, packet5);
	 delay(25);
	 writePacket(theAddress, FINGERPRINT_DATAPACKET, 128+2, packet6);
	 delay(25);
	 writePacket(theAddress, FINGERPRINT_DATAPACKET, 128+2, packet7);
	 delay(25);

    uint8_t len = getReply(packet);
    if ((len != 1) && (packet[0] != FINGERPRINT_ACKPACKET))
	{
		return -1;
	}
	else
	{
		return packet[1];
	}
}

uint8_t Adafruit_Fingerprint::uploadModelPercobaan(uint8_t packet2[], uint8_t packet3[], uint8_t packet4[], uint8_t packet5[]) {
	
    uint8_t packet[] = {FINGERPRINT_DOWNLOAD, 0x01};
	
	 writePacket(theAddress, FINGERPRINT_COMMANDPACKET, sizeof(packet)+2, packet);
	 delay(25);
	 writePacket(theAddress, FINGERPRINT_DATAPACKET, 128+2, packet2);
	 delay(10);
	 writePacket(theAddress, FINGERPRINT_DATAPACKET, 128+2, packet3);
	 writePacket(theAddress, FINGERPRINT_DATAPACKET, 128+2, packet4);
	 writePacket(theAddress, FINGERPRINT_ENDDATAPACKET, 128+2, packet5);

	 uint8_t len = getReply(packet);
    if ((len != 1) && (packet[0] != FINGERPRINT_ACKPACKET))
	{
		return -1;
	}
	else
	{
		return packet[1];
	}
}

/**************************************************************************/
/*!
    @brief   Ask the sensor for the number of templates stored in memory. The number is stored in <b>templateCount</b> on success.
    @returns <code>FINGERPRINT_OK</code> on success
    @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
*/
/**************************************************************************/
uint8_t Adafruit_Fingerprint::getTemplateCount(void) {
  GET_CMD_PACKET(FINGERPRINT_TEMPLATECOUNT);

  templateCount = packet.data[1];
  templateCount <<= 8;
  templateCount |= packet.data[2];

  return packet.data[0];
}

/**************************************************************************/
/*!
    @brief   Set the password on the sensor (future communication will require password verification so don't forget it!!!)
    @param   password 32-bit password code
    @returns <code>FINGERPRINT_OK</code> on success
    @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
*/
/**************************************************************************/
uint8_t Adafruit_Fingerprint::setPassword(uint32_t password) {
  SEND_CMD_PACKET(FINGERPRINT_SETPASSWORD, (password >> 24), (password >> 16), (password >> 8), password);
}

/**************************************************************************/
/*!
    @brief   Helper function to process a packet and send it over UART to the sensor
    @param   packet A structure containing the bytes to transmit
*/
/**************************************************************************/

void Adafruit_Fingerprint::writeStructuredPacket(const Adafruit_Fingerprint_Packet & packet) {
  SERIAL_WRITE_U16(packet.start_code); //Header = 0xEF01
  SERIAL_WRITE(packet.address[0]);		// 0xFF
  SERIAL_WRITE(packet.address[1]);		// 0xFF
  SERIAL_WRITE(packet.address[2]);		// 0xFF
  SERIAL_WRITE(packet.address[3]);		// 0xFF => Module Address
  SERIAL_WRITE(packet.type);	// --?--

  uint16_t wire_length = packet.length + 2; 	// = N + 2
  SERIAL_WRITE_U16(wire_length);

  uint16_t sum = ((wire_length)>>8) + ((wire_length)&0xFF) + packet.type;
  for (uint8_t i=0; i< packet.length; i++) {
    SERIAL_WRITE(packet.data[i]); 
    sum += packet.data[i];
  }

  SERIAL_WRITE_U16(sum);
  return;
}
//------------------------------------------------------------------------------------------
void Adafruit_Fingerprint::writePacket(uint32_t addr, uint8_t packettype,
               uint16_t len, uint8_t *packet) {
			   /*
#ifdef FINGERPRINT_DEBUG
  Serial.println("\n writePacket");
  Serial.print("---> 0x");
  Serial.print((uint8_t)(FINGERPRINT_STARTCODE >> 8), HEX);
  Serial.print(" 0x");
  Serial.print((uint8_t)FINGERPRINT_STARTCODE, HEX);
  Serial.print(" 0x");
  Serial.print((uint8_t)(addr >> 24), HEX);
  Serial.print(" 0x");
  Serial.print((uint8_t)(addr >> 16), HEX);
  Serial.print(" 0x");
  Serial.print((uint8_t)(addr >> 8), HEX);
  Serial.print(" 0x");
  Serial.print((uint8_t)(addr), HEX);
  Serial.print(" 0x");
  Serial.print((uint8_t)packettype, HEX);
  Serial.print(" 0x");
  Serial.print((uint8_t)(len >> 8), HEX);
  Serial.print(" 0x");
  Serial.println((uint8_t)(len), HEX);
#endif*/

#if ARDUINO >= 100
  mySerial->write((uint8_t)(FINGERPRINT_STARTCODE >> 8));
  mySerial->write((uint8_t)FINGERPRINT_STARTCODE);
  mySerial->write((uint8_t)(addr >> 24));
  mySerial->write((uint8_t)(addr >> 16));
  mySerial->write((uint8_t)(addr >> 8));
  mySerial->write((uint8_t)(addr));
  mySerial->write((uint8_t)packettype);
  mySerial->write((uint8_t)(len >> 8));
  mySerial->write((uint8_t)(len));
#else
  mySerial->print((uint8_t)(FINGERPRINT_STARTCODE >> 8), BYTE);
  mySerial->print((uint8_t)FINGERPRINT_STARTCODE, BYTE);
  mySerial->print((uint8_t)(addr >> 24), BYTE);
  mySerial->print((uint8_t)(addr >> 16), BYTE);
  mySerial->print((uint8_t)(addr >> 8), BYTE);
  mySerial->print((uint8_t)(addr), BYTE);
  mySerial->print((uint8_t)packettype, BYTE);
  mySerial->print((uint8_t)(len >> 8), BYTE);
  mySerial->print((uint8_t)(len), BYTE);
#endif
	
#ifdef FINGERPRINT_DEBUG
    Serial.print(" ISI PAKETNYA =");
#endif
  uint16_t sum = (len>>8) + (len&0xFF) + packettype;
  for (uint8_t i=0; i< len-2; i++) {
#if ARDUINO >= 100
    mySerial->write((uint8_t)(packet[i]));
#else
    mySerial->print((uint8_t)(packet[i]), BYTE);
#endif

    sum += packet[i];
  }
#if ARDUINO >= 100
  mySerial->write((uint8_t)(sum>>8));
  mySerial->write((uint8_t)sum);
#else
  mySerial->print((uint8_t)(sum>>8), BYTE);
  mySerial->print((uint8_t)sum, BYTE);
#endif
}


uint8_t Adafruit_Fingerprint::getReply(uint8_t packet[], uint16_t timeout)
{																		
  uint8_t reply[20], idx;
  uint16_t timer=0;
	timeout = 0;
  idx = 0;
  while(true)
  {
    while (!mySerial->available()) 
    {
      delay(1);
      timer++;
      if (timer >= timeout){
		  return FINGERPRINT_TIMEOUT;
	  }
    }
      reply[idx] = mySerial->read();
      if ((idx == 0) && (reply[0] != (FINGERPRINT_STARTCODE >> 8)))
        continue;
      idx++;

      // check packet!
      if (idx >= 9) {
        if ((reply[0] != (FINGERPRINT_STARTCODE >> 8)) ||
            (reply[1] != (FINGERPRINT_STARTCODE & 0xFF)))
            return FINGERPRINT_BADPACKET;
        uint8_t packettype = reply[6];
        uint16_t len = reply[7];
        len <<= 8;
        len |= reply[8];
        len -= 2;
        if (idx <= (len+10)) continue;
        packet[0] = packettype;
        for (uint8_t i=0; i<len; i++) {
          packet[1+i] = reply[9+i];
        }
          return len;
        }
  }
}

/**************************************************************************/
/*!
    @brief   Helper function to receive data over UART from the sensor and process it into a packet
    @param   packet A structure containing the bytes received
    @param   timeout how many milliseconds we're willing to wait
    @returns <code>FINGERPRINT_OK</code> on success
    @returns <code>FINGERPRINT_TIMEOUT</code> or <code>FINGERPRINT_BADPACKET</code> on failure
*/
/**************************************************************************/
uint8_t Adafruit_Fingerprint::getStructuredPacket(Adafruit_Fingerprint_Packet * packet, uint16_t timeout) {
  uint8_t byte;
  uint16_t idx=0, timer=0;

  while(true) {
    while(!mySerial->available()) {
      delay(1);
      timer++; 
      if( timer >= timeout) {
#ifdef FINGERPRINT_DEBUG
	Serial.println("Timed out");
#endif
	return FINGERPRINT_TIMEOUT;
      }
    }
    byte = mySerial->read();
#ifdef FINGERPRINT_DEBUG
    Serial.print("<- 0x"); Serial.println(byte, HEX);
#endif
    switch (idx) {
      case 0:
        if (byte != (FINGERPRINT_STARTCODE >> 8)) 
	  continue;
        packet->start_code = (uint16_t)byte << 8;
        break;
      case 1:
        packet->start_code |= byte;
        if (packet->start_code != FINGERPRINT_STARTCODE) 
	  return FINGERPRINT_BADPACKET;
        break;
      case 2:
      case 3:
      case 4:
      case 5:
        packet->address[idx-2] = byte;
        break;
      case 6: 
	packet->type = byte; 
	break;
      case 7: 
	packet->length = (uint16_t)byte << 8; 
	break;
      case 8: 
	packet->length |= byte; 
	break;
      default:
        packet->data[idx-9] = byte;
        if((idx-8) == packet->length)
          return FINGERPRINT_OK;
        break;
    }
    idx++;
  }
  // Shouldn't get here so...
  return FINGERPRINT_BADPACKET;
}
