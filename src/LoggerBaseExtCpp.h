// 
// Private function extensions
//
#if defined USE_USB_MSC_SD1 
//--------------------------------------------------------------------+
// SD Card callbacks
//--------------------------------------------------------------------+

int32_t Logger::sd1_card_read_cb (uint32_t lba, void* buffer, uint32_t bufsize)
{
  return Logger::sd1_card_phy.card()->readBlocks(lba, (uint8_t*) buffer, bufsize/512) ? bufsize : -1;
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and 
// return number of written bytes (must be multiple of block size)
int32_t Logger::sd1_card_write_cb (uint32_t lba, uint8_t* buffer, uint32_t bufsize)
{
  //digitalWrite(LED_BUILTIN, HIGH);

  return sd1_card_phy.card()->writeBlocks(lba, buffer, bufsize/512) ? bufsize : -1;
}

// Callback invoked when WRITE10 command is completed (status received and accepted by host).
// used to flush any pending cache.
void Logger::sd1_card_flush_cb (void)
{
  sd1_card_phy.card()->syncBlocks();

  // clear file system's cache to force refresh
  sd1_card_phy.cacheClear();

  sd1_card_changed = true;

  //digitalWrite(LED_BUILTIN, LOW);
}
#endif // USE_USB_MSC_SD1 
#if defined USE_USB_MSC_SD0 
//--------------------------------------------------------------------+
// External Flash callbacks
//--------------------------------------------------------------------+

// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and
// return number of copied bytes (must be multiple of block size)
int32_t Logger::sdq_flashspi_read_cb (uint32_t lba, void* buffer, uint32_t bufsize)
{
  // Note: SPIFLash Bock API: readBlocks/writeBlocks/syncBlocks
  // already include 4K sector caching internally. We don't need to cache it, yahhhh!!
  return sdq_flashspi_phy.readBlocks(lba, (uint8_t*) buffer, bufsize/512) ? bufsize : -1;
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and
// return number of written bytes (must be multiple of block size)
int32_t Logger::sdq_flashspi_write_cb (uint32_t lba, uint8_t* buffer, uint32_t bufsize)
{
  //digitalWrite(LED_BUILTIN, HIGH);

  // Note: SPIFLash Bock API: readBlocks/writeBlocks/syncBlocks
  // already include 4K sector caching internally. We don't need to cache it, yahhhh!!
  return sdq_flashspi_phy.writeBlocks(lba, buffer, bufsize/512) ? bufsize : -1;
}

// Callback invoked when WRITE10 command is completed (status received and accepted by host).
// used to flush any pending cache.
void Logger::sdq_flashspi_flush_cb (void)
{
  sdq_flashspi_phy.syncBlocks();

  // clear file system's cache to force refresh
  sd0_card_fatfs.cacheClear();

  sd0_card_changed = true;

  //digitalWrite(LED_BUILTIN, LOW);
}

// Callback invoked when USB system checking if available
bool Logger::sdq_ready (void)
{
    PRINTOUT("USB check for sdq");
    usbDriveStatus = true;
    return true; //unless sleeping in which case false
}
bool Logger::usbDriveActive(void){
    bool retVal = usbDriveStatus;
    usbDriveStatus = false;
    return retVal;
}


void print_rootdir(File* rdir);
void print_rootdir(File* rdir)
{
  File file;

  // Open next file in root.
  // Warning, openNext starts at the current directory position
  // so a rewind of the directory may be required.
  while ( file.openNext(rdir, O_RDONLY) )
  {
    file.printFileSize(&STANDARD_SERIAL_OUTPUT);
    //PRINTOUT(" ");
    STANDARD_SERIAL_OUTPUT.print(' ');
    file.printName(&STANDARD_SERIAL_OUTPUT);
    if ( file.isDir() )
    {
      // Indicate a directory.
      //PRINTOUT('/');
      STANDARD_SERIAL_OUTPUT.print('/');
    }
    //PRINTOUT('\n');
    STANDARD_SERIAL_OUTPUT.print('\n');
    file.close();
  }
}//print_rootdir()
#endif // USE_USB_MSC 
bool Logger::SDextendedInit(bool sd1Success) {
    bool retVal = sd1Success;

#if defined BOARD_SDQ_QSPI_FLASH
    #if !defined USE_TINYUSB
    #error need -DUSE_TINYUSB
    #endif//
    usbDriveStatus = false;
    //If defined need to initiliaze else turns off ints
    usb_msc.setMaxLun(2);

    usb_msc.setID(0, "Adafruit", "External Flash", "1.0");
    usb_msc.setID(1, "Adafruit", "SD Card", "1.0");

    // Since initialize both external flash and SD card can take time.
    // If it takes too long, our board could be enumerated as CDC device only
    // i.e without Mass Storage. To prevent this, we call Mass Storage begin first
    // LUN readiness will always be set later on
    usb_msc.begin();

  //------------- Lun 0 for external flash -------------//
    sdq_flashspi_phy.begin();// should have assigned &sdq_flashspi_transport_QSPI);
    sd0_card_fatfs.begin(&sdq_flashspi_phy);
    MS_DBG(F("Successfully setup SD0"));

#if defined USE_USB_MSC_SD0
    usb_msc.setCapacity(0, sdq_flashspi_phy.pageSize()*sdq_flashspi_phy.numPages()/512, 512);
    usb_msc.setReadWriteCallback(0, sdq_flashspi_read_cb, sdq_flashspi_write_cb, sdq_flashspi_flush_cb);
    usb_msc.setReadyCallback(0,sdq_ready);
    usb_msc.setUnitReady(0, true);

    sd0_card_changed = true; // to print contents initially
    MS_DBG(F("SD0 Supported on USB"));
#if defined USE_USB_MSC_SD1 
    //------------- Lun 1 for SD card -------------//
    if ( sd1Success )
    {
        uint32_t block_count = sd1_card_phy.card()->cardSize();
        usb_msc.setCapacity(1, block_count, 512);
        usb_msc.setReadWriteCallback(1, sd1_card_read_cb, sd1_card_write_cb, sd1_card_flush_cb);
        usb_msc.setUnitReady(1, true);

        sd1_card_changed = true; // to print contents initially
        SerialTty.println("SD1 Card suppored on USB");
    }
#endif //USE_USB_MSC_SD1 
#endif //USE_USB_MSC_SD0
#endif //BOARD_SDQ_QSPI_FLASH
    return retVal;
}

void Logger::SDusbPoll(uint8_t sdActions) {
#if defined USE_USB_MSC_SD0 
  if ( sd0_card_changed )
  {
    File root_fs;
    root_fs = sd0_card_fatfs.open("/");

    PRINTOUT("Flash contents:");
    print_rootdir(&root_fs);
    PRINTOUT("\n");

    root_fs.close();

    sd0_card_changed = false;
  }
#if defined USE_USB_MSC_SD1 
  if ( sd1_card_changed )
  {
    File root_fs;
    root_fs = sd1_card_phy.open("/");

    PRINTOUT("SD contents:\n");
    print_rootdir(&root_fs);
    PRINTOUT("\n");

    root_fs.close();

    sd1_card_changed = false;
  }  
#endif // USE_USB_MSC_SD1 
#endif // USE_USB_MSC_SD0 
}
// ===================================================================== //
// Public functions extensions
// ===================================================================== //
#if 0
void Logger::setLoggingInterval_atl485(uint16_t loggingIntervalMinutes)
{
    _loggingIntervalMinutes = loggingIntervalMinutes;
    #ifdef DEBUGGING_SERIAL_OUTPUT
        const char* prtout1 = "Logging interval:";
        PRINTOUT(prtout1, _loggingIntervalMinutes);
    #endif
}
#endif
void Logger::setLoggerId(const char *newLoggerId,bool copyId,uint8_t LoggerIdMaxSize)
{
    uint8_t LoggerIdSize = strlen(newLoggerId)+2;
    _loggerID =  newLoggerId;

    if (copyId) {
        /* Do size checks, allocate memory for the LoggerID, copy it there
        *  then set assignment.
        */
        if (LoggerIdSize > LoggerIdMaxSize) {
            char *newLoggerId2 = (char *)newLoggerId;
            PRINTOUT(F("\n\r   Logger:setLoggerId too long: Trimmed to "),LoggerIdMaxSize);
            newLoggerId2[LoggerIdMaxSize] = 0; //Trim max size
            LoggerIdSize=LoggerIdMaxSize; 
        }
        if (NULL==_LoggerId_buf) {
            _LoggerId_buf = new char[LoggerIdSize+2]; //Allow for trailing 0
        } else {
            PRINTOUT(F("\nLogger:setLoggerId error - expected NULL ptr"));
        }
        if (NULL==_LoggerId_buf) {
            //Major problem
            PRINTOUT(F("\nLogger::setLoggerId error -no buffer "), newLoggerId);
        } else {
            strcpy((char *)_LoggerId_buf, newLoggerId);
            _loggerID =  _LoggerId_buf;
        }
        MS_DBG(F("\nsetLoggerId cp "),  _loggerID," sz: ",LoggerIdSize);
    } 
}

// ===================================================================== //
// Parse an ini file for customization
// ===================================================================== //

#define INI_USE_STACK 1
/* Maximum line length for any line in INI file (stack or heap). Note that
   this must be 3 more than the longest line (due to '\r', '\n', and '\0'). */
//#define INI_MAX_LINE 200
#define INI_MAX_LINE 100
#define MAX_SECTION 50
#define MAX_NAME 50
/* Nonzero to allow multi-line value parsing, in the style of Python's
   configparser. If allowed, ini_parse() will call the handler with the same
   name for each subsequent line parsed. */
#ifndef INI_ALLOW_MULTILINE
#define INI_ALLOW_MULTILINE 1
#endif

/* Nonzero to allow a UTF-8 BOM sequence (0xEF 0xBB 0xBF) at the start of
   the file. See https://github.com/benhoyt/inih/issues/21 */
#ifndef INI_ALLOW_BOM
#define INI_ALLOW_BOM 1
#endif

/* Chars that begin a start-of-line comment. Per Python configparser, allow
   both ; and # comments at the start of a line by default. */
#ifndef INI_START_COMMENT_PREFIXES
#define INI_START_COMMENT_PREFIXES ";#"
#endif

/* Nonzero to allow inline comments (with valid inline comment characters
   specified by INI_INLINE_COMMENT_PREFIXES). Set to 0 to turn off and match
   Python 3.2+ configparser behaviour. */
#ifndef INI_ALLOW_INLINE_COMMENTS
#define INI_ALLOW_INLINE_COMMENTS 1
#endif
#ifndef INI_INLINE_COMMENT_PREFIXES
#define INI_INLINE_COMMENT_PREFIXES ";"
#endif

/* Strip whitespace chars off end of given string, in place. Return s. */
static char* rstrip(char* s)
{
    char* p = s + strlen(s);
    while (p > s && isspace((unsigned char)(*--p)))
        *p = '\0';
    return s;
}

/* Return pointer to first non-whitespace char in given string. */
static char* lskip(const char* s)
{
    while (*s && isspace((unsigned char)(*s)))
        s++;
    return (char*)s;
}

/* Return pointer to first char (of chars) or inline comment in given string,
   or pointer to null at end of string if neither found. Inline comment must
   be prefixed by a whitespace character to register as a comment. */
static char* find_chars_or_comment(const char* s, const char* chars)
{
#if INI_ALLOW_INLINE_COMMENTS
    int was_space = 0;
    while (*s && (!chars || !strchr(chars, *s)) &&
           !(was_space && strchr(INI_INLINE_COMMENT_PREFIXES, *s))) {
        was_space = isspace((unsigned char)(*s));
        s++;
    }
#else
    while (*s && (!chars || !strchr(chars, *s))) {
        s++;
    }
#endif
    return (char*)s;
}

/* Version of strncpy that ensures dest (size bytes) is null-terminated. */
static char* strncpy0(char* dest, const char* src, size_t size)
{
    strncpy(dest, src, size - 1);
    dest[size - 1] = '\0';
    return dest;
}

//////// this is pretty challenging 
#define PROCESS_LOCAL_INIH 0
#if PROCESS_LOCAL_INIH
/* Process sections of INI file. 
   May have [section]s, 
   name=value pairs (whitespace stripped)
   if [Section] not found then pass on to callers  unhandledFn1()

   */
ini_handler unhandledFn1; //Address of handler if local parsing can't find it
//expect to be in near space
  //#define EDIY_PROGMEM PROGMEM
#define mCONST_UNI(p1) const char p1##_pm[] PROGMEM = #p1
const char BOOT_pm[] EDIY_PROGMEM = "BOOT";
const char VER_pm[] EDIY_PROGMEM = "VER";
const char MAYFLY_SN_pm[] EDIY_PROGMEM = "MAYFLY_SN"; 
const char MAYFLY_REV_pm[] EDIY_PROGMEM = "MAYFLY_REV";
const char MAYFLY_INIT_ID_pm[] EDIY_PROGMEM = "MAYFLY_INIT_ID";

const char COMMON_pm[] EDIY_PROGMEM = "COMMON";
const char LOGGER_ID_pm[] EDIY_PROGMEM = "LOGGER_ID";
//mCONST_UNI(LOGGER_ID);// = "nh07k" ;
const char LOGGING_INTERVAL_MIN_pm[] EDIY_PROGMEM = "LOGGING_INTERVAL_MIN";
const char LIION_TYPE_pm[] EDIY_PROGMEM = "LIION_TYPE";
const char TIME_ZONE_pm[] EDIY_PROGMEM = "TIME_ZONE";
const char GEOGRAPHICAL_ID_pm[] EDIY_PROGMEM = "GEOGRAPHICAL_ID";

const char NETWORK_pm[] EDIY_PROGMEM = "NETWORK";
const char apn_pm[] EDIY_PROGMEM = "apn";
const char WiFiId_pm[] EDIY_PROGMEM = "WiFiId";
const char WiFiPwd_pm[] EDIY_PROGMEM = "WiFiPwd";

const char PROVIDER_pm[] EDIY_PROGMEM = "PROVIDER";
const char CLOUD_ID_pm[] EDIY_PROGMEM = "CLOUD_ID";
const char REGISTRATION_TOKEN_pm[] EDIY_PROGMEM = "REGISTRATION_TOKEN";
const char SAMPLING_FEATURE_pm[] EDIY_PROGMEM = "SAMPLING_FEATURE";

const char UUIDs_pm[] EDIY_PROGMEM = "UUIDs";
const char index_pm[] EDIY_PROGMEM = "index";
static uint8_t uuid_index =0;
static int inihandlerFn( const char* section, const char* name, const char* value)
{
    int retVal=0
    #if 0
    if (strcmp_P(section,PROVIDER_pm)== 0)
    {
        if        (strcmp_P(name,REGISTRATION_TOKEN_pm)== 0) {
            strcpy(ps.provider.s.registration_token, value);
            Serial.print(F("PROVIDER Setting registration token: "));
            Serial.println(ps.provider.s.registration_token );
            EnviroDIYPOST.setToken(ps.provider.s.registration_token);
        } else if (strcmp_P(name,CLOUD_ID_pm)== 0) {
            strcpy(ps.provider.s.cloudId, value);
            Serial.print(F("PROVIDER Setting cloudId: "));
            Serial.println(ps.provider.s.cloudId );
        } else if (strcmp_P(name,SAMPLING_FEATURE_pm)== 0) {
            strcpy(ps.provider.s.sampling_feature, value);
            Serial.print(F("PROVIDER Setting SamplingFeature: "));
            Serial.println(ps.provider.s.sampling_feature );
            dataLogger.setSamplingFeatureUUID(ps.provider.s.sampling_feature);
        } else {
            Serial.print(F("PROVIDER not supported:"));
            Serial.print(name);
            Serial.print("=");
            Serial.println(value);
        }
    } else 
    #endif //0

    if (strcmp_P(section,UUIDs_pm)== 0)
    {
        /*FUT:Add easier method to mng 
        perhaps1) "UUID_label"="UUID"
        then search variableList till find UUID_label
        perhaps2) "dummyDefaultUID"="UUID" 
            "ASQ212_PAR"="UUID"
            then search variablList till find dummyDefaultUID and replace with UUID
        perhaps3) "SensorName"="UUID" 
            "ApogeeSQ212_PAR"="UUID"
            then search variablList till fin SensorsName

        */
        if (strcmp_P(name,index_pm)== 0) {
            PRINTOUT(F("["));
            PRINTOUT(uuid_index);
            PRINTOUT(F("]={"));
            PRINTOUT(value);
            if (uuid_index < variableCount ) 
            {
                PRINTOUT(F("} replacing {"));
                PRINTOUT(variableList[uuid_index]->getVarUUID() );
                PRINTOUT(F("}\n\r"));

                strcpy(&ps.provider.s.uuid[uuid_index][0], value);
                //variableList[uuid_index]->setVarUUID(&ps.provider.s.uuid[uuid_index][0]);
            
            } else {
                Serial.println(F("} out of range. Notused"));
            }
            uuid_index++;
        } else 
        {
            PRINTOUT(F("UUIDs not supported:"));
           PRINTOUT(name);
            PRINTOUT("=");
            PRINTOUT(value,"\n\r");
        } 
    //} else if (strcmp_P(section,COMMON_pm)== 0) {
    } else 
    {
        MS_DBG(F("LBE: Nothandled Sect:"),section);
        retVal = unhandledFn1( section, name, value);
    }

    return retVal;
}
#endif // PROCESS_LOCAL_INIH

/* Parse given INI-style file. 
   May have [section]s, 
   name=value pairs (whitespace stripped), and 
   comments starting with ';' (semicolon). 
   Section  is "" if name=value pair parsed before any section heading. 
   name:value     pairs are also supported as a concession to Python's configparser.

   For each name=value pair parsed, call handler function with given user
   pointer as well as section, name, and value (data only valid for duration
   of handler call). 
   Handler should return nonzero on success, zero on error.

   Returns 0 on success, 
     line number of last error on parse error (doesn't stop on first error), 

   https://en.wikipedia.org/wiki/INI_file
   https://github.com/benhoyt/inih
*/
int8_t Logger::inihParseFile(ini_handler_atl485 handler_fn)
{
    /* Uses a fair bit of stack (use heap instead if you need to) */
#if INI_USE_STACK
    char line[INI_MAX_LINE];
    int max_line = INI_MAX_LINE;
#else
    char* line;
    int max_line = INI_INITIAL_ALLOC;
#endif
#if INI_ALLOW_REALLOC && !INI_USE_STACK
    char* new_line;
    int offset;
#endif
    char section[MAX_SECTION] = "";
    char prev_name[MAX_NAME] = "";

    char* start;
    char* end;
    char* name;
    char* value;
    int lineno = 0;
    int error = 0;

#if !INI_USE_STACK
    line = (char*)malloc(INI_INITIAL_ALLOC);
    if (!line) {
        return -2;
    }
#endif

    /* Scan through stream line by line */
#define reader_fn(line1,max_line1) logFile.fgets(line1,max_line1)
    while (reader_fn(line, max_line) != 0) 
    {
#if INI_ALLOW_REALLOC && !INI_USE_STACK
        offset = strlen(line);
        while (offset == max_line - 1 && line[offset - 1] != '\n') {
            max_line *= 2;
            if (max_line > INI_MAX_LINE)
                max_line = INI_MAX_LINE;
            new_line = realloc(line, max_line);
            if (!new_line) {
                free(line);
                return -2;
            }
            line = new_line;
            if (reader_fn(line + offset, max_line - offset) == NULL)
                break;
            if (max_line >= INI_MAX_LINE)
                break;
            offset += strlen(line + offset);
        }
#endif

        lineno++;

        start = line;
#if INI_ALLOW_BOM
        if (lineno == 1 && (unsigned char)start[0] == 0xEF &&
                           (unsigned char)start[1] == 0xBB &&
                           (unsigned char)start[2] == 0xBF) {
            start += 3;
        }
#endif
        start = lskip(rstrip(start));

        if (strchr(INI_START_COMMENT_PREFIXES, *start)) {
            /* Start-of-line comment */
        }
#if INI_ALLOW_MULTILINE
        else if (*prev_name && *start && start > line) {
            /* Non-blank line with leading whitespace, treat as continuation
               of previous name's value (as per Python configparser). */
            if (!handler_fn( section, prev_name, start) && !error)
                error = lineno;
        }
#endif
        else if (*start == '[') {
            /* A "[section]" line */
            end = find_chars_or_comment(start + 1, "]");
            if (*end == ']') {
                *end = '\0';
                strncpy0(section, start + 1, sizeof(section));
                *prev_name = '\0';
            }
            else if (!error) {
                /* No ']' found on section line */
                error = lineno;
            }
        }
        else if (*start) {
            /* Not a comment, must be a name[=:]value pair */
            end = find_chars_or_comment(start, "=:");
            if (*end == '=' || *end == ':') {
                *end = '\0';
                name = rstrip(start);
                value = end + 1;
#if INI_ALLOW_INLINE_COMMENTS
                end = find_chars_or_comment(value, NULL);
                if (*end)
                    *end = '\0';
#endif
                value = lskip(value);
                rstrip(value);

                /* Valid name[=:]value pair found, call handler */
                strncpy0(prev_name, name, sizeof(prev_name));
                if (!handler_fn(section, name, value) && !error)
                    error = lineno;
            }
            else if (!error) {
                /* No '=' or ':' found on name[=:]value line */
                error = lineno;
            }
        }

#if INI_STOP_ON_FIRST_ERROR
        if (error)
            break;
#endif
    }

#if !INI_USE_STACK
    free(line);
#endif

    return error;
}

bool Logger::parseIniSd(const char *ini_fn,ini_handler_atl485 unhandledFnReq)
{
    uint8_t ini_err;
    // Initialise the SD card
    // skip everything else if there's no SD card, otherwise it might hang
    if (!initializeSDCard()) return false;

    if (logFile.open(ini_fn))
    {
        #if PROCESS_LOCAL_INIH
        unhandledFn1 = unhandledFnReq;
        ini_err = inihParseFile(inihandlerFn); //handle found sections locall first
        #else //PROCESS_LOCAL_INIH
        ini_err = inihParseFile(unhandledFnReq); //handle found sections
        #endif //
        logFile.close();
        PRINTOUT(F("Parse ini; "),ini_fn);
        if (ini_err) {
            PRINTOUT(F("Error on line :"),ini_err);
        } else {
            PRINTOUT(F("Completed."));
        }
    } else
    {
        PRINTOUT(F("Parse ini; No file "),ini_fn);
        return false;
    }
    return true;
}
#ifdef USE_MS_SD_INI
void Logger::setPs_cache(persistent_store_t *ps_ram) 
{
    ps_cache=ps_ram;
}
void Logger::printFileHeaderExtra(Stream *stream) 
{
    if (NULL == ps_cache) return;
    stream->print(F("Board: "));
    stream->print((char *)ps_cache->hw_boot.board_name);
    stream->print(F(" rev:'"));
    stream->print((char *)ps_cache->hw_boot.rev);
    stream->print(F("' sn:'"));
    stream->print((char *)ps_cache->hw_boot.serial_num);      
    stream->println(F("'"));      

    stream->print(F("Tz: "));
    stream->print(ps_cache->app.msc.s.time_zone);
    stream->print(F(" BatteryType: "));
    stream->println(ps_cache->app.msc.s.battery_type);

    stream->print(F("Location: "));
    stream->println((char *)ps_cache->app.msc.s.geolocation_id);
}
#endif //USE_MS_SD_INI

#if defined USE_RTCLIB
USE_RTCLIB * Logger::rtcExtPhyObj()
   { 
       return &rtcExtPhy;
   }
#endif //USE_RTCLIB

// End parse.ini