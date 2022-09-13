#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <ncurses.h>


/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port;



int fisier(int sd, char msg2[100]){

	char msg[100], msg3[100];
	char buffer[10000];
	FILE *file;



	int yMax, x, xMax;
	int c;
	int i,j,n;

  	getmaxyx(stdscr, yMax, xMax);





  	msg2[strlen(msg2)] = '\0';

	strcpy(msg3, "~ File: ");
	strcat(msg3, msg2);
	strcat(msg3, " ~");



	mvprintw(0, (xMax - strlen(msg3) ) / 2, "%s", msg3);
	move(2, 0);


	bzero(buffer, 10000);
	fflush(stdout);
	/* citirea raspunsului dat de server 
	 (apel blocant pina cind serverul raspunde) */
	if (read (sd, buffer, 10000) <= 0) return -1;

	/* afisam mesajul primit */
	printw("%s", buffer);
	
      
	nodelay(stdscr, true);

      /*fisier*/
      while(1){

      	if( (c = getch()) == ERR ) c = KEY_UP;


		bzero (msg, 100);
		msg[0] = 'f';
		msg[1] = 'i';
		msg[2] = 's';
		msg[3] = 'i';
		msg[4] = 'e';
		msg[5] = 'r';
		msg[6] = '\0';




		fflush (stdout);

		if (write (sd, msg, sizeof(msg)) <= 0) return -1;

		fflush (stdout);

		if (write (sd, msg2, 100) <= 0) return -1;

		fflush(stdout);


		if (write (sd, &c, sizeof(int)) <= 0) return -1;


		if( c == 27 ) break;



		bzero(buffer, 10000);
		fflush(stdout);
		/* citirea raspunsului dat de server 
		 (apel blocant pina cind serverul raspunde) */
		if (read (sd, buffer, 10000) <= 0) return -1;

		fflush(stdout);

		if( read(sd, &i, sizeof(int)) <= 0) return -1;




        clear();
        mvprintw(0, (xMax - strlen(msg3) ) / 2, "%s", msg3);
        move(2, 0);
        printw("%s", buffer);
        move(2 + i/xMax, i%xMax);



      }
      
      nodelay(stdscr, false);

      clear();


	return 0;
}





int comanda(int sd){


  char msg[100], msg2[100], msg3[100];
  char buffer[10000];
  FILE *file;


  int yMax, x, xMax;
  int c;
  int i,j,n;

  getmaxyx(stdscr, yMax, xMax);



  strcpy(msg2, "~ Welcome ! ~");


  while(1){


    bzero (msg, 100);
    i=0;
    n=0;
    mvprintw(0, (xMax - strlen(msg2) ) / 2, "%s", msg2);
    move(2, 0);
    printw("Enter a command: ");


    while( ( c = getch() ) != '\n'){



      if( c >= 32 && c <= 126 && c != '\'' ){
        for(j = n-1; j >= i; j--) msg[j+1] = msg[j];
        msg[i] = (char)c;
        n++;
        msg[n] = '\0';
        i++;
      }
      else if( c == KEY_BACKSPACE && i > 0 ){
              for(j = i; j < n; j++) msg[j-1] = msg[j];
              n--;
              msg[n] = '\0';
              i--;
            }
            else if( c == KEY_LEFT && i > 0 ) i--; 
                 else if( c == KEY_RIGHT && i < n ) i++;
                      else if( c == 27 ) break;


      clear();
      mvprintw(0, (xMax - strlen(msg2) ) / 2, "%s", msg2);
      move(2, 0);
      printw("Enter a command: %s", msg);
      move(2, i+17);
    }

    if( c == 27 ) break;

 	bzero (msg3, 100);
	msg3[0] = 'c';
	msg3[1] = 'o';
	msg3[2] = 'm';
	msg3[3] = 'a';
	msg3[4] = 'n';
	msg3[5] = 'd';
	msg3[6] = 'a';
	msg3[7] = '\0';

	fflush (stdout);

	if (write (sd, msg3, sizeof(msg3)) <= 0)
	{
	  
	  return -1;
	} 


    fflush (stdout);


    if (write (sd, msg, 100) <= 0)
    {
      return -1;
    }


    if(strncmp(msg, "create ", 7) == 0){
      bzero (msg, 100);
      /* citirea raspunsului dat de server 
         (apel blocant pina cind serverul raspunde) */
      if (read (sd, msg, 100) < 0)
        {
          return errno;
        }
      clear();
      mvprintw(4, 0, "%s", msg);
    }
    else if(strncmp(msg, "download ", 9) == 0){

            strcpy(msg3, "./");
            strcat(msg3, msg+9);
            bzero (msg, 100);
            /* citirea raspunsului dat de server 
               (apel blocant pina cind serverul raspunde) */
            if (read (sd, msg, 100) < 0)
              {
                return -1;
              }
            clear();
            mvprintw(4, 0, "%s", msg);

            if(strcmp(msg,"Fisierul a fost descarcat.") == 0){
              bzero (buffer, 100);
              fflush(stdout);
              /* citirea raspunsului dat de server 
                 (apel blocant pina cind serverul raspunde) */
              if (read (sd, buffer, 10000) < 0)
                {
                  return -1;
                }


              fflush (stdout);
              file = fopen(msg3, "w");
              
              if(strlen(buffer) != 0) if (fprintf(file, buffer, sizeof(buffer)) <= 0) return -1;

              fclose(file);

            }

    }
    else{

    bzero (msg, 100);
    /* citirea raspunsului dat de server 
       (apel blocant pina cind serverul raspunde) */
    if (read (sd, msg, 100) < 0)
      {
        return -1;
      }


    clear();



    /*clientul a primit acces de la server la fisierul specificat*/
    if(strncmp(msg, "Acces permis", 12) == 0){ 

      bzero(msg3, 100);
      strcat(msg3, msg + 13);


      fisier(sd, msg3);

      }
    else mvprintw (4, 0, "%s", msg);

  }
  }




	return 0;
}



int login(int sd){

  char msg[100], msg2[100], msg3[100];
  char buffer[10000];
  FILE *file;

  initscr();
  noecho();
  cbreak();


  int yMax, x, xMax;
  int c;
  int i,j,n;

  getmaxyx(stdscr, yMax, xMax);



  keypad(stdscr, true);


  /*log in*/
  while(1){

    bzero (msg, 100);
    i=0;
    n=0;
    mvprintw(0, (xMax - strlen("~ Log in ~")) / 2, "~ Log in ~" );
    move(2, 0);
    printw("Username: ");


    while( ( c = getch() ) != '\n'){



      if( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) || c == '.' || c == '_' ){
        for(j = n-1; j >= i; j--) msg[j+1] = msg[j];
        msg[i] = (char)c;
        n++;
        msg[n] = '\0';
        i++;
      }
      else if( c == KEY_BACKSPACE && i > 0 ){
              for(j = i; j < n; j++) msg[j-1] = msg[j];
              n--;
              msg[n] = '\0';
              i--;
            }
            else if( c == KEY_LEFT && i > 0 ) i--; 
                 else if( c == KEY_RIGHT && i < n ) i++;



      clear();
      mvprintw(0, (xMax - strlen("~ Log in ~")) / 2, "~ Log in ~" );
      move(2, 0);
      printw("Username: %s",msg);
      move(2, i+10);
    }




    bzero (msg2, 100);
    i=0;
    n=0;
    move(3,0);
    printw ("Password: ");

    while( ( c = getch() ) != '\n'){


      if( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c >= '0' && c <= '9' ) || c == '.' || c == '_' ){
        for(j = n-1; j >= i; j--) msg2[j+1] = msg2[j];
        msg2[i] = (char)c;
        n++;
        msg2[n] = '\0';
        i++;
      }
      else if( c == KEY_BACKSPACE && i > 0 ){
              for(j = i; j < n; j++) msg2[j-1] = msg2[j];
              n--;
              msg2[n] = '\0';
              i--;
            }
            else if( c == KEY_LEFT && i > 0 ) i--; 
                 else if( c == KEY_RIGHT && i < n ) i++;


      clear();
      mvprintw(0, (xMax - strlen("~ Log in ~")) / 2, "~ Log in ~" );
      move(2, 0);
      printw("Username: %s",msg);
      move(3, 0);
      printw("Password: %s",msg2);
      move(3, i+10);
    }



	bzero (msg3, 100);
	msg3[0] = 'l';
	msg3[1] = 'o';
	msg3[2] = 'g';
	msg3[3] = 'i';
	msg3[4] = 'n';
	msg3[5] = '\0';

	fflush (stdout);

    if (write (sd, msg3, sizeof(msg3)) <= 0)
    {
      
      return -1;
    }



    fflush (stdout);

    if (write (sd, msg, sizeof(msg)) <= 0)
    {
      
      return -1;
    }

    fflush (stdout);


    if (write (sd, msg2, sizeof(msg2)) <= 0)
    {
      return -1;
    }


    bzero (msg2, 100);
    /* citirea raspunsului dat de server 
       (apel blocant pina cind serverul raspunde) */
    if (read (sd, msg2, 100) < 0)
      {
        return -1;
      }


    clear();

    if(strcmp(msg2,"Validat") == 0) break;
    else mvprintw (5, 0, "%s", msg2);



  }


	return 0;

}



int main (int argc, char *argv[])
{
  int sd;     // descriptorul de socket
  struct sockaddr_in server;  // structura folosita pentru conectare 
  char msg[100], msg2[100], msg3[100];
  char buffer[10000];
  FILE *file;

  /* exista toate argumentele in linia de comanda? */
  if (argc != 3)
    {
      printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
      return -1;
    }

  /* stabilim portul */
  port = atoi (argv[2]);

  /* cream socketul */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("Eroare la socket().\n");
      return errno;
    }

  /* umplem structura folosita pentru realizarea conexiunii cu serverul */
  /* familia socket-ului */
  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  server.sin_addr.s_addr = inet_addr(argv[1]);
  /* portul de conectare */
  server.sin_port = htons (port);
  
  /* ne conectam la server */
  if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
      perror ("[client]Eroare la connect().\n");
      return errno;
    }


   if(login(sd) != 0) return -1;
   else comanda(sd);

   endwin();

  /* inchidem conexiunea, am terminat */
  close (sd);
}
