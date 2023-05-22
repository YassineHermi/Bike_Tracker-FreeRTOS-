/*
 * GY-NEO6MV2.c
 *
 *  Created on: 5 avr. 2023
 *      Author: Yassine HERMI
 */
/* Includes ------------------------------------------------------------------------------*/

#include "GPS.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdint.h"
#include "time.h"
#include "stm32l4xx_hal.h"

/* Private variables ---------------------------------------------------------*/

UART_HandleTypeDef huart4;
UART_HandleTypeDef huart2;

/* Exported Variables -----------------------------------------------------------------------------*/

uint8_t Rxdata[750];
char Txdata[750];
char Ligne_GPRMC[110];
uint8_t Flag=0;
static int indice;
char * p;
float Temps, Longitude, Latitude, Vitesse;
int Date, Jour, Mois, Annee, Heures, Minutes, Secondes;
uint32_t Epoch_Time;
char ch[6];
char *x;
int ind;
char Data[64];
Data_from_GPS mydata;

/* Private functions ----------------------------------------------------------*/

static void Format_data(int Date,float Temps,float Latitude,float Longitude,float Vitesse,uint8_t buffer[13],uint8_t buff[13]);
static uint32_t Get_Epoch_Time(int jour,int mois,int annee,int heures,int minutes,int secondes);
static void Inversion(uint8_t buffer[13], uint8_t buff[13]);
static void USART2_UART_Init(void);



/* Exported Functions -----------------------------------------------------------------------------*/

void Get_Data(uint8_t buffer[13],uint8_t buff[13])
{

	if (Flag != 0)
	  {

		// Extraction de la ligne GPRMC
		indice=0;
		strcpy(Txdata,(char*)Rxdata);
		p= strstr(Txdata,"GPRMC");
		// chercher l'indice de '\n'(retour à la ligne) dans la ligne de GPRMC
		x = strchr(p, '\n');
		ind = x - p;
		// continuer l'extraction
		if (*p == 'G')
		 {
			for( indice =0;indice<=ind;indice++)
			        {

				     	 Ligne_GPRMC[indice]=*p;
				     	 *p=*(p+indice);

			        }

			Ligne_GPRMC[indice]= '\0';
		  // Extraction de chaque information dans la ligne
		  HAL_UART_Transmit(&huart2, (uint8_t*)Ligne_GPRMC, sizeof(Ligne_GPRMC), 100);
		  sscanf(Ligne_GPRMC,"GGPRMC,%f,A,%f,N,%f,E,%f,,%d",&Temps,&Latitude,&Longitude,&Vitesse,&Date);

          HAL_UART_Transmit(&huart2, (uint8_t*)"\n", sizeof("\n"), 50);
		  // Format des données
		  Format_data(Date, Temps, Latitude, Longitude, Vitesse,buffer,buff);
		  Flag=0;

		}
	  }
	//return (uint8_t *)buff;
}

void GPS_Init(void)
{

  huart4.Instance = UART4;
  huart4.Init.BaudRate = 9600;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  huart4.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart4.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  HAL_UART_Init(&huart4);

  USART2_UART_Init();
  HAL_UART_Receive_IT(&huart4, (uint8_t*)Rxdata,750);
}

/* Private Functions --------------------------------------------------------------------------*/

static void Format_data(int Date, float Temps, float Latitude, float Longitude, float Vitesse,uint8_t buffer[13],uint8_t buff[13])
{

  // Date  :  ddmmaa ==> dd/mm/aa

  Jour = (int)(Date/10000);
  Mois = (int)((Date-(Jour*10000))/100);
  Annee = (int)(Date-((Jour*10000)+(Mois*100)));

  // Temps :  hhmmss ==> hh:mm:ss
  Heures = (int)(Temps/10000);
  Minutes = (int)((Temps-(Heures*10000))/100);
  Secondes = (int)(Temps-((Heures*10000)+(Minutes*100)));

  // Timestamp :

  Epoch_Time = Get_Epoch_Time(Jour,Mois,2000+Annee,Heures,Minutes,Secondes);

  // Vitesse :  vitesse en noeuds ==> vitesse en km/h
  Vitesse = Vitesse * 1.852;

  // Latitude : ddmm.mmmm ==> dd + mm.mmmm/60

  int degre_lat = (int)(Latitude/100);
  Latitude = degre_lat + ((Latitude- (degre_lat*100))/60);

  // Longitude :   ddmm.mmmm ==> dd + mm.mmmm/60

  int degre_long = (int)(Longitude/100);
  Longitude = degre_long + ((Longitude- (degre_long*100))/60);


	  Data_from_GPS mydata = {
		  .epoch_time = Epoch_Time,
	      .latitude = Latitude,
		  .longitude= Longitude,
		  .speed= Vitesse,
	  };


  memcpy(buffer, &mydata, sizeof(mydata));
  Inversion(buffer,buff);
  HAL_UART_Transmit_IT(&huart2, (uint8_t*)"==> Ready to store\r\n",22);

}


/**
 * la fonction ci-dessous a pour but de convertir date et temps en une valeur en secondes (epoch_time)
 */
static uint32_t Get_Epoch_Time(int jour,int mois,int annee,int heures,int minutes,int secondes)
{
	    struct tm t;
	    time_t t_of_day;

	    t.tm_year = annee-1900;  // Year - 1900
	    t.tm_mon = mois-1;           // Month, where 0 = jan
	    t.tm_mday = jour;          // Day of the month
	    t.tm_hour = heures+1;
	    t.tm_min = minutes;
	    t.tm_sec = secondes;
	    t_of_day = mktime(&t);
	    return (t_of_day);
}

/**
 * la fonction ci-dessous a pour but d'inverser le contenu d'un buffer de la façon suivante : "abcdefghijklp" ==> "dcbahgfelkjip"
 */
static void Inversion(uint8_t buffer[13],uint8_t buff[13])
{
	int k =0;
	  for (int i=0; i<=8; i=i+4)
	  {
	      for (int j =i+3; j>=i;j--)
	      {
	          buff[k]=buffer[j];
	          k++;
	      }
	  }

	  buff[12]=buffer[12];
}

/**
 * la fonction ci-dessous permet d'initialiser l'usart2 de STLINK pour afficher des messages
 */
static void USART2_UART_Init(void)
{

  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  HAL_UART_Init(&huart2);

}

/**
 * la fonction ci-dessous est appellé après la réception de chaque donnée du module GPS
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef * huart)
{
  Flag=1;
  HAL_UART_Receive_IT(&huart4, (uint8_t*)Rxdata,750);

}
