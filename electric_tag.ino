#include "UART_328p.c"
#include "twi.c"
#include "ssd1306.c"

#include <avr/io.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <avr/interrupt.h>

#define Buffer_Size 150
#define degrees_buffer_size 20

char Latitude_Buffer[15],Longitude_Buffer[15],Time_Buffer[15],Altitude_Buffer[8],Date_Buffer[15];
char LAT[degrees_buffer_size], LONG[degrees_buffer_size];                    /* save latitude or longitude in degree */
char day_buff[2], month_buff[2], year_buff[2];      char date_result[10];
char GGA_Buffer[Buffer_Size], RMC_Buffer[Buffer_Size];       /* save GGA string */
uint8_t GGA_Pointers[20],RMC_Pointers[20];                   /* to store instances of ',' */
char GGA_CODE[3], RMC_CODE[3];
char dis_str[15]; 
volatile uint16_t GGA_Index, CommaCounter, RMC_Index;
bool	IsItGGAString	= false,
		  IsItRMCString			= false;

struct Point_data Current;
struct Point_data Centroid;

uint16_t  btn_pressed_cnt = 0, 
          main_tim_cnt = 0,
          btn_tim_cnt = 0,
          alert_tim_cnt = 0;
bool      alert_flag = false;

void GPIO_Timer_Init(void);
void get_rmcdate(void);
void get_gpstime(void);
void get_latitude(uint16_t lat_pointer, char *result_buffer, float *result);
void get_longitude(uint16_t long_pointer, char *result_buffer, float *result);
void get_altitude(uint16_t alt_pointer);
void convert_time_to_UTC(void);
void convert_to_degrees(char *raw, char *result_buffer, float *result);
float gps_distance_between (float lat1, float long1, float lat2, float long2);
void main_procedure(void);
double roundToFourDecimalPlaces(double a);



void setup() {
  // put your setup code here, to run once:
  GGA_Index=0;
	memset(GGA_Buffer, 0, Buffer_Size);
	memset(LONG,0,degrees_buffer_size);
  memset(LAT,0,degrees_buffer_size);

  GPIO_Timer_Init();               /* Init input button, LED & Timer ISR */
  SSD1306_Init (0x3C);             /* 0x3C = I2C address OLED SSD1306 */
	USART_Init(9600);                /* initialize USART with 9600 baud rate */
	UCSR0B |= RX_COMPLETE_INTERRUPT; /* enable RX interupt */
	sei();

  // Centroid.lat = 10.877558;
  // Centroid.lng = 106.807154;
  
  Centroid.lat = 10.772057;
  Centroid.lng = 106.657856;
  Current = {0.0,0.0,{0,0,0,0,0}};
  
  delay(3000);
}

void loop() {
  // put your main code here, to run repeatedly:
    
}

void main_procedure(void){
  if (alert_flag == false)
  {
    // PORTB ^= (1 << PB3);

    // USART_SendString("\r\nGGA_Buffer:\r\n");
    // USART_SendString(GGA_Buffer);
    // USART_SendString("\r\nRMC_Buffer:\r\n");
    // USART_SendString(RMC_Buffer);

    get_rmcdate(GGA_Pointers[7]);
        sprintf(day_buff, "%c%c",Date_Buffer[0],Date_Buffer[1]);
        Current.timeStp.day = atoi(day_buff);
        sprintf(month_buff, "%c%c",Date_Buffer[2],Date_Buffer[3]);
        Current.timeStp.month = atoi(month_buff);
        sprintf(year_buff, "%c%c",Date_Buffer[4],Date_Buffer[5]);
        Current.timeStp.year = atoi(year_buff)+2000;
        sprintf(date_result, "%02d-%02d-%04d",Current.timeStp.day,
                                              Current.timeStp.month,
                                              Current.timeStp.year);
    USART_SendString("\r\nDate(dd-mm-yy): ");
    USART_SendString(date_result);

    get_gpstime();                         /* Extract Time in UTC */
    USART_SendString("\r\nTime(GMT+7): ");
    USART_SendString(Time_Buffer);
    
    USART_SendString("\r\nCurrent : ");
    get_latitude(GGA_Pointers[0],LAT, &(Current.lat));         /* Extract Latitude */
    USART_SendString(LAT);            /* display latitude in degree */
    
    USART_SendString("; ");
    get_longitude(GGA_Pointers[2],LONG, &(Current.lng));        /* Extract Longitude */
    USART_SendString(LONG);            /* display longitude in degree */

              USART_SendString("\r\nCentroid: ");
              char Cen_lat[8];
              dtostrf(Centroid.lat, 8, 4, Cen_lat);
              USART_SendString(Cen_lat);           
              
              USART_SendString("; ");
              char Cen_lng[8];
              dtostrf(Centroid.lng, 8, 4, Cen_lng);
              USART_SendString(Cen_lng);            

    USART_SendString("\r\nDistance(m): ");
    float dis = gps_distance_between(Centroid.lat, Centroid.lng, Current.lat, Current.lng);
    // float dis = gps_distance_between(10.8812, 106.8097,10.8811, 106.8096);

    dtostrf(dis, 7, 4, dis_str);
    USART_SendString(dis_str);

    USART_SendString("\r\nAlt: ");
    get_altitude(GGA_Pointers[7]);         /* Extract Altitude in meters*/
    USART_SendString(Altitude_Buffer);
    USART_SendString("\r\n___________________________________*");

    // SSD1306_ClearScreen();
    SSD1306_SetPosition(35, 6);
    SSD1306_DrawString(date_result);
    SSD1306_SetPosition(42, 7);
    SSD1306_DrawString(Time_Buffer);
    SSD1306_SetPosition(2, 1);
    SSD1306_DrawString("Latitude:  ");
    SSD1306_DrawString(LAT);
    SSD1306_SetPosition(2, 2);
    SSD1306_DrawString("Longitude: ");
    SSD1306_DrawString(LONG);

    SSD1306_UpdateScreen (0x3C);
  }
    
}

void GPIO_Timer_Init(void){
  cli();

  DDRB |= (1 << PB4)|(1 << PB3);  //PB4 (pin12) as output

  DDRD &= ~(1 << DDD2);  // PD2 (pin 2) as input
  PORTD |= (1 << PORTD2); // Enable pull-up resistor on PD2 => HIGH

  /* Reset Timer/Counter1 */
  TCCR2A = 0;
  TCCR2B = 0;
  TIMSK2 = 0;
  
  /* Setup Timer/Counter1 */
  TCCR2B |= (1 << CS22) ;    // prescale = 64
  TCNT2 = 5;                 // timeout = 1ms
  TIMSK2 = (1 << TOIE2);     // Overflow interrupt enable 
  sei();                     // Enable global interupts
}

ISR (TIMER2_OVF_vect){
  main_tim_cnt++;
  btn_tim_cnt++;
  alert_tim_cnt++;
  TCNT2 = 5;

  if (main_tim_cnt == 1000)
  {
    main_procedure();
    main_tim_cnt = 0;
  }

  if (btn_tim_cnt == 300)     //Check button every 300ms
  {
    btn_tim_cnt = 0;
    if (!(PIND & (1 << PIND2))){
      delay(10);
      if(!(PIND & (1 << PIND2)))
      {
          btn_pressed_cnt++;
          if (btn_pressed_cnt == 6)    // Pressed 3s
          { 
            PORTB ^= (1 << PB4);
            alert_flag = true;    
            alert_tim_cnt = 0;      
            Centroid.lat = (Current.lat);
            Centroid.lng = (Current.lng);

              SSD1306_ClearScreen();
              SSD1306_SetPosition (5, 1);
              SSD1306_DrawString("CENTROID LOCKED !!!");

              SSD1306_SetPosition (30, 3);
              SSD1306_DrawString("Lat: ");
              SSD1306_DrawString  (LAT);

              SSD1306_SetPosition (20, 5);
              SSD1306_DrawString("Long: ");
              SSD1306_DrawString  (LONG);
              SSD1306_UpdateScreen (0x3C);
                       
            USART_SendString("\r\nCentroid Locked !!!");
            btn_pressed_cnt = 0;
              
          }
      }
    }
    else if (PIND & (1 << PIND2)){
      delay(10);
      if (PIND & (1 << PIND2))
      {
        btn_pressed_cnt =0;
      }
    }
  }

  if (alert_tim_cnt == 1000 && alert_flag)
  {
    SSD1306_ClearScreen();
    alert_flag = false;
    alert_tim_cnt = 0;
  }
}

ISR (USART_RX_vect){
	uint8_t oldsrg = SREG;
	cli();
	char received_char = USART_RxChar();   
  // if (received_char != 0)
  //   USART_TxChar(received_char); 
  /*---------------------------------------------------------------------------*/
	if(received_char =='$'){                   /* check for '$' */
		GGA_Index = 0;
		CommaCounter = 0;
		IsItGGAString = false;
	}
	else if(IsItGGAString == true){                                             /* if true save GGA info. into buffer */
		if(received_char == ',' ) GGA_Pointers[CommaCounter++] = GGA_Index;     /* store instances of ',' in buffer */
		GGA_Buffer[GGA_Index++] = received_char;
	}
	else if(GGA_CODE[0] == 'G' && GGA_CODE[1] == 'G' && GGA_CODE[2] == 'A'){    /* check for GGA string */
		IsItGGAString = true;
		GGA_CODE[0] = 0; GGA_CODE[1] = 0; GGA_CODE[2] = 0;
	}
	else{
		GGA_CODE[0] = GGA_CODE[1];  GGA_CODE[1] = GGA_CODE[2]; GGA_CODE[2] = received_char;
	}
  /*---------------------------------------------------------------------------*/
  if(received_char =='$'){                  /* check for '$' */
		RMC_Index = 0;
		CommaCounter = 0;
		IsItRMCString = false;
	}
	else if(IsItRMCString == true){                                             /* if true save RMC info. into buffer */
		if(received_char == ',' ) RMC_Pointers[CommaCounter++] = RMC_Index;     /* store instances of ',' in buffer */
		RMC_Buffer[RMC_Index++] = received_char;
	}
	else if(RMC_CODE[0] == 'R' && RMC_CODE[1] == 'M' && RMC_CODE[2] == 'C'){    /* check for RMC string */
		IsItRMCString = true;
		RMC_CODE[0] = 0; RMC_CODE[1] = 0; RMC_CODE[2] = 0;
	}
	else{
		RMC_CODE[0] = RMC_CODE[1];  RMC_CODE[1] = RMC_CODE[2]; RMC_CODE[2] = received_char;
	}
  /*---------------------------------------------------------------------------*/
  
  
  
  sei();
	SREG = oldsrg;
  
}

void get_rmcdate(uint16_t date_pointer){
	cli();
	uint8_t date_index;
	uint8_t index = date_pointer;
  
	date_index=0;
	
	/* parse Date in RMC string stored in buffer */
	for(;RMC_Buffer[index]!=',';index++){
		Date_Buffer[date_index]= RMC_Buffer[index];
		date_index++;
	}
	sei();
}

void get_gpstime(){
	cli();
	uint8_t time_index=0;
	/* parse Time in GGA string stored in buffer */
	for(uint8_t index = 0;GGA_Buffer[index]!=','; index++){
		
		Time_Buffer[time_index] = GGA_Buffer[index];
		time_index++;
	}
	convert_time_to_UTC();
	sei();
}

void get_latitude(uint16_t lat_pointer, char *result_buffer, float *result){
	cli();
	uint8_t lat_index;
	uint8_t index = lat_pointer+1;
	lat_index=0;
	
	/* parse Latitude in GGA string stored in buffer */
	for(;GGA_Buffer[index]!=',';index++){
		Latitude_Buffer[lat_index]= GGA_Buffer[index];
		lat_index++;
	}
	
	Latitude_Buffer[lat_index++] = GGA_Buffer[index++];
	Latitude_Buffer[lat_index]= GGA_Buffer[index];		/* get direction */
	convert_to_degrees(Latitude_Buffer, result_buffer, result);
	sei();
}

void get_longitude(uint16_t long_pointer, char *result_buffer, float *result){
	cli();
	uint8_t long_index;
	uint8_t index = long_pointer+1;
	long_index=0;
	
	/* parse Longitude in GGA string stored in buffer */
	for( ; GGA_Buffer[index]!=','; index++){
		Longitude_Buffer[long_index]= GGA_Buffer[index];
		long_index++;
	}
	
	Longitude_Buffer[long_index++] = GGA_Buffer[index++];
	Longitude_Buffer[long_index]   = GGA_Buffer[index]; /* get direction */
	convert_to_degrees(Longitude_Buffer, result_buffer, result);
	sei();
}

void get_altitude(uint16_t alt_pointer){
	cli();
	uint8_t alt_index;
	uint8_t index = alt_pointer+1;
	alt_index=0;
	/* parse Altitude in GGA string stored in buffer */
	for( ; GGA_Buffer[index]!=','; index++){
		Altitude_Buffer[alt_index]= GGA_Buffer[index];
		alt_index++;
	}
	
	Altitude_Buffer[alt_index]   = GGA_Buffer[index+1];
	sei();
}

void convert_time_to_UTC(){
	unsigned int hour, min, sec;
	uint32_t Time_value;
	
	Time_value = atol(Time_Buffer);               /* convert string to integer */
	hour = (Time_value / 10000)+7;                  /* extract hour from integer */

    hour = (hour>24)?(hour-24):hour;
    
	min = (Time_value % 10000) / 100;             /* extract minute from integer */
	sec = (Time_value % 10000) % 100;             /* extract second from integer*/
  Current.timeStp.hour = hour;
  Current.timeStp.minute = min;
  Current.timeStp.sec = sec;
  sprintf(Time_Buffer, "%02d:%02d:%02d", hour,min,sec);
	
}
double roundToFourDecimalPlaces(double a) {
    // Use printf with precision to round to four decimal places
    char buffer[20]; // Assuming the number won't exceed 20 characters
    snprintf(buffer, sizeof(buffer), "%.6f", a);
    double b;
    sscanf(buffer, "%lf", &b);
    return b;
}
void convert_to_degrees(char *raw, char *result_buffer, float *result){
	
	double value;
	float decimal_value,temp;
	
	int32_t degrees;
	
	float position;
	value = atof(raw);                             /* convert string into float for conversion */
	
	/* convert raw latitude/longitude into degree format */
	decimal_value = (value/100);
	degrees = (int)(decimal_value);
	temp = (decimal_value - (int)decimal_value)/0.6;
	position = (float)degrees + temp;
	*result = (position);
	dtostrf(position, 8, 4, result_buffer);       /* convert float value into string */
}

float gps_distance_between (float lat1, float lon1, float lat2, float lon2){
  // Convert latitude and longitude from degrees to radians
    lat1 = lat1* M_PI / 180.0;
    lon1 = lon1* M_PI / 180.0;
    lat2 = lat2* M_PI / 180.0;
    lon2 = lon2* M_PI / 180.0;

    // Calculate differences between latitudes and longitudes
    double dlat = lat2 - lat1;
    double dlon = lon2 - lon1;

    // Haversine formula
    double a = sin(dlat / 2) * sin(dlat / 2) + cos(lat1) * cos(lat2) * sin(dlon / 2) * sin(dlon / 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));

    // Distance in meters
    double distance = 6371000 * c;

    return distance;
}

