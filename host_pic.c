#define PIC_ADDR 0x40
#define CLK_FREQ 100000

#include <Headers/host/timelib.h>

// <DEBUG>
unsigned int debugLogCtr = 0;
char ctrStr[10];
// </DEBUG>


// <RTC>
unsigned char sec, min1, hr, week_day, day, mn, year;
char *txt, tnum[4];
TimeStruct ts;
unsigned long int epoch;
char epochStr[24];
// </RTC>


// <SESSION>
unsigned long int currEpoch = 1000000000;
unsigned long int userExpEpoch = 1000000060;

char userExpEpochStr[24];
char currEpochStr[24];
// </SESSION>


// <INTERRUPT>
char j;
char uartRcv;
char uartRcvBuff[64];

unsigned int uartCount = 0;
unsigned int intFromUart = 0;
unsigned int dataReceived = 0;
unsigned int intFromTimer = 0;
unsigned int timerCounter = 0;
unsigned int timerThirtySecCounter = 0;
// </INTERRUPT>

// <TEMPORARY>
char intFromUartStr[7];
// </TEMPORARY>


#include <Headers/lcd.h>
#include <Headers/connection.h>
#include <Headers/output_strings.h>
#include <Headers/coreSession.h>
#include <Headers/debug.h>
#include <Headers/host/modules.h>
#include <Headers/rtc.h>

void interrupt(){
  host_isr();
}

void main(){
  ADCON1 = 0x0F;
  
  PIE1.SSPIE = 1;
  PIE1.RCIE = 1;
  PIR1.RCIF = 0;
  
  UART1_Init(9600);
  
  I2C_Master_Init(CLK_FREQ);
  
  INTCON.GIE = 1;
  INTCON.PEIE = 1;
  
  debug("reading time");
  
  while(1){
    while(dataReceived == 0){
      debug("reading time");
      Read_Time(&sec,&min1,&hr,&week_day,&day,&mn,&year);
      Transform_Time(&sec,&min1,&hr,&week_day,&day,&mn,&year); // format date and time
      ts.ss = sec;
      ts.mn = min1;
      ts.hh = hr;
      ts.md = day;
      ts.mo = mn;
      ts.yy = year + 2000;
      epoch = Time_dateToEpoch(&ts);
      LongToStr(epoch, epochStr);

      debug(Ltrim(epochStr));
      Delay_ms(5000);
    }
    if(intFromUart == 1){
       debug(uartRcvBuff);
    }
    if(intFromTimer){
      intFromTimer = 0;

      logSessionHead("Time Sync");
      LongToStr(currEpoch, currEpochStr);
      debug(currEpochStr);
      clearSession();
      strcpy(sessionMode, "TIME");
      debug(sessionMode);
      strcpy(sessionData[0], Ltrim(currEpochStr));
      strcpy(sessionBlockData, deMapSession(1, 0));
      debug(sessionBlockData);

      i2cSend(0x44, Ltrim(sessionBlockData));

      logSessionFoot("Time Sync");
    }

    dataReceived = 0;
  }
}