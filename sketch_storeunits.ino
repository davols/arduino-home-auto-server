#include <HomeAutomationUnit.h>
#include <RemoteTransmitter.h>
#include <string>


#include <EEPROMEx.h>

#include <SPI.h>
#include <WiFly.h>

#include "Credentials.h"

WiFlyServer server(80);

#define MAX_COMMAND_LENGTH 50
#define MAX_STRING_LEN  50


const char delimiters[] = "=&";
const int maxAllowedWrites = 80;
const int memBase          = 350;

int adressNumberofUnits=0;
int numberOfUnits; 

int addressChannel;
int addressChar;
int addressCharArray;
int addressBoolean;

HomeAutomationUnit myAutoUnits[10];


HomeAutomationUnit my("test567891",'B',4,false);

void setup()
{
  Serial.begin(9600);
  //EEPROM STUFF

  // start reading from position memBase (address 0) of the EEPROM. Set maximumSize to EEPROMSizeUno 
  // Writes before membase or beyond EEPROMSizeUno will only give errors when _EEPROMEX_DEBUG is set
  EEPROM.setMemPool(memBase, EEPROMSizeMega);

  // Set maximum allowed writes to maxAllowedWrites. 
  // More writes will only give errors when _EEPROMEX_DEBUG is set
  EEPROM.setMaxAllowedWrites(maxAllowedWrites);
  delay(100);
  Serial.println("");       

  // Always get the adresses first and in the same order

  WiFly.begin();

  if (!WiFly.join(ssid, passphrase)) {
    while (1) {
      // Hang on failure.
    }
  }

  Serial.begin(9600);
  Serial.print("IP: ");
  Serial.println(WiFly.ip());

  server.begin();

  adressNumberofUnits = 0;
//  writeZeroFirstTime();
  loadFromSettings();




}

void loadFromSettings() 
{
  numberOfUnits = EEPROM.readInt(adressNumberofUnits);
  Serial.println("LoadFromSettings. Number of units:");
  Serial.println(numberOfUnits);
  //Fetches how many bytes that we need. 
  int bytesAllNeed = getNeededBytes();
  Serial.println("Bytes we need");
  Serial.println(bytesAllNeed);

  for(int i=0;i<numberOfUnits;i++) {
    char output[] = "          ";
    char name;
    int channel;
    int mStatus;

    addressCharArray=adressNumberofUnits+sizeof(int)+bytesAllNeed*i+1;
    Serial.println("addressCharArray");
    Serial.println(addressCharArray);
    EEPROM.readBlock<char>(addressCharArray, output, 10);
    Serial.println("name:");
    Serial.println(output);
    addressChar=addressCharArray+sizeof(char)*10;
    name = (char) EEPROM.readByte(addressChar);
    Serial.println("channel:");
    Serial.println(name);
    addressChannel=addressChar+sizeof(char);
    channel = EEPROM.readInt(addressChannel);
    Serial.println("Number:");
    Serial.println(channel);
    addressBoolean = addressChannel+sizeof(int);
    mStatus = EEPROM.readInt(addressBoolean);
    Serial.println("Status:");
    Serial.println(mStatus);
    myAutoUnits[i] = HomeAutomationUnit(output,name,channel,mStatus);
    myAutoUnits[i].setAddress(addressCharArray);
  
  }

}

void writeZeroFirstTime() 
{

  Serial.println(adressNumberofUnits);
  EEPROM.writeInt(adressNumberofUnits,0);
}

void addNewUnit(HomeAutomationUnit myUnit) 
{
  // Always get the adresses first and in the same order
  // myAutoUnits[numberOfUnits] = myUnit;
  Serial.println("--------------------------");     
  Serial.println("Adress on addNew");
  Serial.println(adressNumberofUnits);
  Serial.println("End of adress");

  Serial.println("asdasdretreiving number of sunits");     
  Serial.println("--------------------------");    
  numberOfUnits = EEPROM.readInt(adressNumberofUnits);
  Serial.println(numberOfUnits);
  if(numberOfUnits<0)
  {
    numberOfUnits=0;
  }

  addressCharArray=adressNumberofUnits+numberOfUnits*getNeededBytes()+1+sizeof(int);
  int tmpAdr = addressCharArray+sizeof(char)*10;

  Serial.println("Trying to write at");
  Serial.println(addressCharArray);
  Serial.println("Trying to write Name");
  char tmpName[10];
  strcpy(tmpName, myUnit.getName().c_str());
  Serial.println(tmpName);
  bool ans =     EEPROM.writeBlock<char>(addressCharArray, tmpName, 10);

  Serial.println(ans);

  ans =  EEPROM.writeByte(tmpAdr,(byte) myUnit.getDeviceNumber());
  Serial.println("Trying to write device number");
  Serial.println(ans);
  tmpAdr=tmpAdr+sizeof(char);
  ans = EEPROM.writeInt(tmpAdr,myUnit.getDeviceChannel());
  Serial.println("Trying to write channel");
  Serial.println(ans);
  tmpAdr=tmpAdr+sizeof(int);
  ans= EEPROM.writeInt(tmpAdr,myUnit.getStatus());
  Serial.println("Trying to write int");
  Serial.println(ans);

  numberOfUnits++;
  ans =    EEPROM.writeInt(adressNumberofUnits,numberOfUnits);
  Serial.println("Trying to write numberOfUnits");

  Serial.println("Adress in writing");
  Serial.println(adressNumberofUnits);
  Serial.println("End of adress");
  Serial.println(ans);
  Serial.println(numberOfUnits);
  Serial.println("Wrote stuff");

}


int getNeededBytes() 
{
  return ((sizeof(char)*10)+sizeof(byte)+sizeof(int)+sizeof(int));
}

// Ten chars for each name, then one char for each (byte) for each, then one int for all (channel) then one int for status. 
int getAddressOfAll(int antal) 
{ 
  return (((sizeof(char)*10)+(sizeof(byte))+(sizeof(int))+(sizeof(int)))*antal);
}


void loop()
{
  WiFlyClient client = server.available();
  if (client) {
    char buffer[512];
    char get_url[MAX_COMMAND_LENGTH];
    char house[MAX_COMMAND_LENGTH];
    char command[MAX_COMMAND_LENGTH];
    int buffPos = 0;

    // an http request ends with a blank line
    boolean current_line_is_blank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        buffer[buffPos++] = c;
        Serial.print(c);
        // if we've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so we can send a reply
        if (c == '\n' && current_line_is_blank) {

          sscanf(buffer, "GET %s HTTP/1.1", get_url);
          Serial.println("Test");
          Serial.println(get_url);
          Serial.println("#");
          char *cmd = subStr(get_url, 1);
          Serial.println("cmd!");
          Serial.print(cmd);
          Serial.println("End of cmd");

          if(strcmp(cmd,"/favicon.ico")==0) 
          {
            Serial.println("Favicon");

          }
          else if(strcmp(cmd,"/add")==0)
          {
            Serial.println("Winning");
            char *name = subStr(get_url, 2);
            char *deviceChar = subStr(get_url, 4);
            char myChar[1];
            char myName[10];
            strcpy(myName,name);
            strcpy(myChar,deviceChar);
            int deviceNumber = atoi(subStr(get_url,6));
            int mStatus = atoi(subStr(get_url,8));
            Serial.println("Name");
            Serial.println(name);
            Serial.println("MyName");
            Serial.println(myName);
            Serial.println("device char");
            Serial.println(myChar);
            Serial.println("ChannelNr");
            Serial.println(deviceNumber);
            Serial.println("Status");
            Serial.println(mStatus);
            HomeAutomationUnit my2(name,myChar[0],deviceNumber,mStatus);
            addNewUnit(my2);
            loadFromSettings();
          }
          else if(strcmp(cmd,"/set")==0) {
            Serial.println("Set!");
            int id = atoi(subStr(get_url, 2));
            Serial.println("id");
            Serial.println(id);
            int newValue = atoi(subStr(get_url,4));
            Serial.println("value");
            Serial.println(newValue);
            if(id<numberOfUnits) 
            {
             myAutoUnits[id].setStatus(newValue); 
              
            }
            
          }
            Serial.println(subStr(get_url, 1));

          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();
          client.print("[");
          for(int i=0;i<numberOfUnits;i++)
          {
            client.print("{\"Name\":\"");
            client.print(myAutoUnits[i].getName());
            client.print("\",\"deviceId\":\"");
            client.print(myAutoUnits[i].getDeviceNumber());
            client.print("\",\"deviceChannel\":");
            client.print(myAutoUnits[i].getDeviceChannel());
            client.print(",\"mStatus\":");
            client.print(myAutoUnits[i].getStatus());  
            client.print("}");
            if(i<(numberOfUnits-1)) {
              client.print(","); 
            }
          }
          client.print("]");
          break;
        }
        if (c == '\n') {
          // we're starting a new line
          current_line_is_blank = true;
        } 
        else if (c != '\r') {
          // we've gotten a character on the current line
          current_line_is_blank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(100);
    client.stop();
  }
}    



// Function to return a substring defined by a delimiter at an index
char* subStr (char* str, int index) {
  char *act, *sub, *ptr;
  static char copy[MAX_STRING_LEN];
  int i;

  // Since strtok consumes the first arg, make a copy
  strcpy(copy, str);

  for (i = 1, act = copy; i <= index; i++, act = NULL) {
    //Serial.print(".");
    sub = strtok_r(act, delimiters, &ptr);
    if (sub == NULL) break;
  }
  return sub;

}












