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
#include "Memory.h"

/* Private variables ---------------------------------------------------------*/

UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart2;

/* Exported Variables -----------------------------------------------------------------------------*/

uint8_t Rxdata[750];
char Txdata[750];
char Ligne_GPRMC[110];
uint8_t Flag=0;
static int indice;
char * p;
float Temps, Longitude, Latitude, Vitesse, dist=0.0;
int Date, Jour, Mois, Annee, Heures, Minutes, Secondes;
uint32_t Epoch_Time;
char ch[6];
char *x;
int ind;
char Data[64];
Data_from_GPS mydata;
int compteur=0;
int led1,led2,led3,led4,niv;




/* Private functions ----------------------------------------------------------*/

static void Format_data(int Date,float Temps,float Latitude,float Longitude,float Vitesse,uint8_t buffer[15],uint8_t buff[15], int * compteur_vit_null);
static uint32_t Get_Epoch_Time(int jour,int mois,int annee,int heures,int minutes,int secondes);



/* Exported Functions -----------------------------------------------------------------------------*/

void Get_Data(uint8_t buffer[15],uint8_t buff[15], int  * compteur_vit_null)
{

	if (Flag == 1)
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
		  sscanf(Ligne_GPRMC,"GGPRMC,%f,A,%f,N,%f,E,%f,,%d",&Temps,&Latitude,&Longitude,&Vitesse,&Date);

		  // Format des données
		  Format_data(Date, Temps, Latitude, Longitude, Vitesse,buffer,buff, compteur_vit_null);
		  Flag=0;

		}
	  }

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

  HAL_UART_Receive_IT(&huart4, (uint8_t*)Rxdata,750);
}

/* Private Functions --------------------------------------------------------------------------*/

static void Format_data(int Date, float Temps, float Latitude, float Longitude, float Vitesse,uint8_t buffer[15],uint8_t buff[15],int * compteur_vit_null)
{

  // Date  :  ddmmaa ==> dd/mm/aa

  Jour = (int)(Date/10000);
  Mois = (int)((Date-(Jour*10000))/100);
  Annee = (int)(Date-((Jour*10000)+(Mois*100)));

  // Temps :  hhmmss ==> hh:mm:ss
  Heures = (int)(Temps/10000);
  Minutes = (int)((Temps-(Heures*10000))/100);
  Secondes = (int)(Temps-((Heures*10000)+(Minutes*100)));

  // Timestamp en secondes :

  Epoch_Time = Get_Epoch_Time(Jour,Mois,2000+Annee,Heures,Minutes,Secondes);

  // Vitesse :  vitesse en noeuds ==> vitesse en km/h
  Vitesse = Vitesse * 1.852;

  if (Vitesse <= 5)
	  (*compteur_vit_null)++;
  else
	  (*compteur_vit_null)=0;

  if ((*compteur_vit_null) == 30)
	  (*compteur_vit_null)=0;

  //Distance parcourue en km:

  dist = dist + Vitesse * (1.5/3600);

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
     .Distance = dist,
  };


  memcpy(buffer, &mydata, sizeof(mydata));

  // niveau de charge de la batterie :

  led1 = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2);
  	  led2 = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_8);
  	  led3 = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3);
  	  led4 = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5);

  	  if (led4==1)
  		  niv=100;
  	  else if ((led4==0)&&(led3==1))
  		  niv=75;
  	  else if ((led3==0)&&(led2==1))
  		  niv=50;
  	  else if ((led2==0)&&(led1==1))
  		  niv=25;
  	  else
  		  niv=0;
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
void Inversion(uint8_t buffer[15],uint8_t buff[15])
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
	  buff[13] = buffer[13];
	  buff[14]= niv;
}



/**
 * la fonction ci-dessous est appellé après la réception de chaque donnée du module GPS
 */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef * huart)
{

	 Flag=1;
     HAL_UART_Receive_IT(&huart4, (uint8_t*)Rxdata,750);


}
