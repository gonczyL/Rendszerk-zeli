#include "feladat.h"

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<signal.h>
#define BUFSIZE 1024                 // Max length of buffer
#define PORT_NO 3333                 // The port on which the server is listening
int s;                               // socket ID

char*  nev = "Gönczy László";
char* ido = "2022.04.25";
char* vezio = "varsion_0.9";

void SendViaFile(int *Values, int NumValues)
{	
	FILE *fp = fopen("Measurement.txt", "w");
	
	if(fp == NULL)
	{
		printf("Nem sikerult megnyitni Olvasasra a fajlt!\n");
		exit(1);
	}
	
	for(int i = 0;  i < NumValues; i++)
	{
		fprintf(fp,"%d\n", Values[i]);
	}
	
	fclose(fp);
	
	int asd;
	
	asd = FindPID();
	
	if(asd != -1)
	{
		kill(asd, SIGUSR1);
	}else
	{
		printf("\n");
	}
}

void ReceiveViaFile(int sig)
{
	FILE *fp = fopen("Measurement.txt", "r");
	int *tomb = malloc(0 * sizeof(int));
	int tmp;
	
	if(fp == NULL)
	{
		printf("Nem sikerult megnyitni Irasra a fajlt!\n");
		exit(1);
	}
	
	int count = 0;
	while((fscanf(fp, "%d", &tmp)) != EOF)
	{
		tomb = realloc(tomb, (count + 1) * sizeof(int));
		tomb[count] = tmp;
		count++;
	}
	
	BMPcreator(tomb,count);
	
	free(tomb);
		
}


/*       - UDP server for Linux -        */
/*             by Imre Varga             */


void stop(int sig){ // signal handler CLOSING
   close(s);
   printf("\n Server is terminated.\n");
   exit(0);
   }


void ReceiveViaSocket()
{
   //************************ Declarations **********************
   int bytes;                        // received/sent bytes
   int err;                          // error code
   int flag;                         // transmission flag
   char on;                          // sockopt option
   char buffer[BUFSIZE];             // datagram buffer area
   unsigned int server_size;         // length of the sockaddr_in server
   unsigned int client_size;         // length of the sockaddr_in client
   struct sockaddr_in server;        // address of server
   struct sockaddr_in client;        // address of client

   //************************ Initialization ********************
   on   = 1;
   flag = 0;
   server.sin_family      = AF_INET;
   server.sin_addr.s_addr = INADDR_ANY;
   server.sin_port        = htons(PORT_NO);
   server_size = sizeof server;
   client_size = sizeof client;
   signal(SIGINT, stop);
   signal(SIGTERM, stop);

   //************************ Creating socket *******************
   s = socket(AF_INET, SOCK_DGRAM, 0 );
   if ( s < 0 ) {
      fprintf(stderr, "Socket creation error.\n");
      exit(2);
      }
   setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);
   setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof on);

   //************************ Binding socket ********************
   err = bind(s, (struct sockaddr *) &server, server_size);
   if ( err < 0 ) {
      fprintf(stderr,"Binding error.\n");
      exit(3);
      }

   while(1){ // Continuous server operation
      //************************ Receive data **********************
      int numValues;
	 
      bytes = recvfrom(s, &numValues, sizeof(int), flag, (struct sockaddr *) &client, &client_size );

      //************************ Sending data **********************
      int *seged = malloc(numValues * sizeof(int));
      bytes = sendto(s, &numValues, sizeof(int), flag, (struct sockaddr *) &client, client_size);
	 
	  bytes = recvfrom(s, seged, sizeof(int) * numValues, flag, (struct sockaddr *) &client, &client_size );
	
	  bytes = sendto(s, &bytes, sizeof(int), flag, (struct sockaddr *) &client, client_size);

	
	BMPcreator(seged,numValues);
	/*
		  for(int i = 0; i< numValues; i++)
		{
			printf("%d\t", seged[i]);
		}
		*/
	free(seged);  
    }

   return EXIT_SUCCESS;
	
}



/*       - UDP client for Linux -        */
/*             by Imre Varga             */


void SendViaSocket(int *Values, int NumValues){

   //************************ Declarations **********************
   int d;                            // socket ID
   int bytes;                        // received/sent bytes
   int flag;                         // transmission flag
   char on;                          // sockopt option
   char buffer[BUFSIZE];             // datagram buffer area
   unsigned int server_size;         // length of the sockaddr_in server
   struct sockaddr_in server;        // address of server

   //************************ Initialization ********************
   on   = 1;
   flag = 0;
   server.sin_family      = AF_INET;
   server.sin_addr.s_addr = inet_addr("127.0.0.1");
   server.sin_port        = htons(PORT_NO);
   server_size = sizeof server;

   //************************ Creating socket *******************
   d = socket(AF_INET, SOCK_DGRAM, 0 );
   if ( d < 0 ) {
      fprintf(stderr, "Socket creation error.\n");
      exit(2);
      }
   setsockopt(d, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);
   setsockopt(d, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof on);

   //************************ Sending data **********************
   
	
   bytes = sendto(d, &NumValues, sizeof(int), flag, (struct sockaddr *) &server, server_size);
     //************************ Receive data **********************
   int vartNumValues;
   bytes = recvfrom(d, &vartNumValues, sizeof(int), flag, (struct sockaddr *) &server, &server_size);
   
   if(NumValues != vartNumValues)
   {
	   printf("A ket ertek nem egyezik!\n");
	   exit(3);
   }
   
    bytes = sendto(d, Values, sizeof(int) * NumValues , flag, (struct sockaddr *) &server, server_size);
	
	/*for(int i = 0; i< NumValues; i++)
	{
		printf("%d\t", Values[i]);
	}*/
	
	int meret;
	int ajj;
	 ajj = recvfrom(d, &meret, sizeof(int), flag, (struct sockaddr *) &server, &server_size);
	 
	 if(meret != bytes)
   {
	   printf("A ket ertek nem egyezik!\n");
	   exit(4);
   }

   //************************ Closing ***************************
   close(d);

   return EXIT_SUCCESS;
   }



void version()
{
	printf("Verzio: %s\ndátum: %s\nKeszitette: %s\n", vezio, ido, nev);
	exit(0);
}

void help()
{

	printf("--help:\nkilistázza a  prancssori argumentumokat, mit mire lehet használni.\n");
	printf("--version:\nkiirja a program verzióját, hogy ki irta a programot, a program befejezésének dátumát\n");
	printf("-send:\nmeghivja a SendViaFile()-eljárást;\n");
	printf("-receive:\nmeghivja a ReceiveViaFile()-eljárást;\n");
	printf("-send -file vagy -file -send\nmeghivja a SendViaFile()-eljárást\n");
	printf("-receive -file vagy -file -receive\nmeghivja a ReceiveViaFile()-eljárást;n");
	printf("-send -socket vagy -socket -send\nmeghivja a SendViaSocket()-eljárást;\n");
	printf("-receive -socket vagy -socket -receive\nmeghivja a ReceiveViaSocket()-eljárást;\n");
	exit(0);
}

int main(int argc, char* argv[])
{
  srand(time(0));

  int *tomb;
  int size = Measurement(&tomb);


	if(argc == 2)
	{
		if(strcmp(argv[1], "--version") == 0)
		{
			version();
		}
		else if(strcmp(argv[1], "--help") == 0)
		{
			help();
		}
		if(strcmp(argv[1], "-send") == 0)
		{
			SendViaFile(tomb,size);

		}
		if(strcmp(argv[1], "-receive") == 0)
		{
			
			ReceiveViaFile(1);
			
		}else
		{
			SendViaFile(tomb,size);
			//ez is send
		}
		
	}else
	if(strcmp(argv[1], "-send") == 0 && strcmp(argv[2], "-file") == 0 || strcmp(argv[1], "-file") == 0 && strcmp(argv[2], "-send") == 0)
	{
		SendViaFile(tomb,size);
		//file send
	}
	else 
	if(strcmp(argv[1], "-receive") == 0 && strcmp(argv[2], "-file") == 0 || strcmp(argv[1], "-file") == 0 && strcmp(argv[2], "-receive") == 0)
	{
		
		ReceiveViaFile(1);
		
			//file receive
	}else
	if(strcmp(argv[1], "-send") == 0 && strcmp(argv[2], "-socket") == 0 || strcmp(argv[1], "-socket") == 0 && strcmp(argv[2], "-send") == 0)
	{
		//socket send
		SendViaSocket(tomb, size);
	}
	else
	if(strcmp(argv[1], "-receive") == 0 && strcmp(argv[2], "-socket") == 0 || strcmp(argv[1], "-socket") == 0 && strcmp(argv[2], "-receive") == 0)
	{
		//socket receive
		ReceiveViaSocket();
	}else
	{
		help();
	}
	
	
  free(tomb);
  return 0;
}
