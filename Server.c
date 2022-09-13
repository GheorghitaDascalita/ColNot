#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <ncurses.h>


/* portul folosit */
#define PORT 2025

/* codul de eroare returnat de anumite apeluri */
extern int errno;


int x;


static int callback(void *data, int argc, char **argv, char **azColName){
   int i;
   
   for(i = 0; i<argc; i++){
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   
   printf("\n");
   return 0;
}


static int callback1(void *data, int argc, char **argv, char **azColName){
   
  x = 1;

  return 0;
}


static int callback2(void *data, int argc, char **argv, char **azColName){

   if((strcmp(argv[1], "-1") != 0) && (strcmp(argv[2], "-1") != 0)) x = -2;
   else if(strcmp(argv[1], "-1") != 0) x = atoi(argv[1]);
        else if(strcmp(argv[2], "-1") != 0) x = atoi(argv[2]);
             else x = -1;
   
   return 0;
}


static int callback3(void *data, int argc, char **argv, char **azColName){
   
   x=1;

   return 0;
}


static int callback4(void *data, int argc, char **argv, char **azColName){

   x = atoi(argv[0]);
   
   return 0;
}


int login(int fd){

  char msg[100], msg2[100], msgrasp[100];
  char s2[100], s[100];

  sqlite3 *db;
  char *zErrMsg = 0;
  int rc;
  char *sql;


  /* Open database */
  rc = sqlite3_open("Server.db", &db);

  if( rc ) {
    printf("Can't open database: %s\n", sqlite3_errmsg(db));
    return -1;
  }


  
  x=0;
  /* log in */
  while(!x){



      /* se asteapta numele de utilizator*/
      bzero (msg, 100);
      printf ("[server]Asteptam numele de utilizator...\n");
      fflush (stdout);

      /* citirea mesajului */
      if (read (fd, msg, 100) <= 0){
              perror ("[server]Eroare la read() de la client.\n");
              return -1;
      }

      printf ("[server]Mesajul a fost receptionat...%s\n", msg);



      /* se astepta mesajul pentru autentificare*/
      bzero (msgrasp, 100);
      printf ("[server]Asteptam parola...\n");
      fflush (stdout);

      /* citirea mesajului */
      if (read (fd, msgrasp, 100) <= 0){
              perror ("[server]Eroare la read() de la client.\n");
              return -1;
      }

      printf ("[server]Mesajul a fost receptionat...%s\n", msgrasp);


     /* Create SQL statement */
     strcpy(s2,"SELECT * FROM login where nume = '");
     strcat(s2, msg);
     strcat(s2,"' and parola='");
     strcat(s2, msgrasp);
     strcat(s2,"';");
     sql=strdup(s2);

     x=0;

     /* Execute SQL statement */
     rc = sqlite3_exec(db, sql, callback1, 0, &zErrMsg);
     
     if( rc != SQLITE_OK ) {
        printf("SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return -1;
     }




      if(x){

        /* Create SQL statement */
        strcpy(s2,"UPDATE login SET logat = 1, fd =  ");
        bzero(msg2, 100);
        sprintf(msg2, "%d", fd);
        strcat(s2, msg2);
        strcat(s2," where nume='");
        strcat(s2, msg);
        strcat(s2,"' and parola='");
        strcat(s2, msgrasp);
        strcat(s2,"';");
        sql=strdup(s2);



        /* Execute SQL statement */
        rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);

        if( rc != SQLITE_OK ) {
          printf("SQL error: %s\n", zErrMsg);
          sqlite3_free(zErrMsg);
          return -1;
        }

      /*pregatim mesajul de raspuns */
      bzero(msgrasp,100);
      strcpy(msgrasp,"Validat");


      }
      else {
        bzero(msgrasp, 100);
        strcpy(msgrasp,"Nevalidat");
      }

      printf("[server]Trimitem mesajul inapoi...%s\n",msgrasp);
      fflush(stdout);


      /* returnam mesajul clientului */
      if (write (fd, msgrasp, 100) <= 0)
      {
          perror ("[server]Eroare la write() catre client.\n");
          return -1;
      }
      else
          printf ("[server]Mesajul a fost transmis cu succes.\n");
      
      

  }

  sqlite3_close(db);
  return 0;  
}



int comanda(int fd, int v[100]){  /*serverul citeste cereri de acces la fisiere de la client*/

  char msg[100], msg2[100];        //mesajul primit de la client
  char msgrasp[100]=" ";        //mesaj de raspuns pentru client
  char buffer[10000]; //continut fisier
 
  sqlite3 *db;
  char *zErrMsg = 0;
  int rc;
  char *sql;
  char s2[100], s[100];
  int fd02;
  FILE *file;


  /* Open database */
  rc = sqlite3_open("Server.db", &db);

  if( rc ) {
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    return -1;
  }




  /* se asteapta mesajul*/
  bzero (msg, 100);
  printf ("[server]Asteptam cererea de acces la un fisier...\n");
  fflush (stdout);

  /* citirea mesajului */
  if (read (fd, msg, 100) <= 0){
          perror ("[server]Eroare la read() de la client.\n");
          return -1;
      }

  printf ("[server]Mesajul a fost receptionat...%s\n", msg);

  
  bzero(msgrasp,100);
  /*comanda data de client are o forma valida*/
  if(strncmp(msg, "open ", 5) == 0){

     /* Create SQL statement */
     strcpy(s,msg + 5);
     strcpy(s2,"SELECT * from fisiere where id='");
     strcat(s2,s);
     strcat(s2,"';");
     sql=strdup(s2);


      x = -3;

     /* Execute SQL statement */
     rc = sqlite3_exec(db, sql, callback2, 0, &zErrMsg);
     
     if( rc != SQLITE_OK ) {
        printf("SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
     }


     if(x == -3) {strcpy(msgrasp, "Fisierul nu exista. Pentru creare folositi comanda: create");

                  printf("[server]Trimitem mesajul inapoi...%s\n",msgrasp);

                  /* returnam mesajul clientului */
                  if (write (fd, msgrasp, 100) <= 0)
                  {
                      perror ("[server]Eroare la write() catre client.\n");
                      return -1;
                  }
                  else
                      printf ("[server]Mesajul a fost transmis cu succes.\n");}

     else if(x == -2) {strcpy(msgrasp, "Acces refuzat. Fisierul este accesat deja de 2 clienti.");

                      printf("[server]Trimitem mesajul inapoi...%s\n",msgrasp);

                      /* returnam mesajul clientului */
                      if (write (fd, msgrasp, 100) <= 0)
                      {
                          perror ("[server]Eroare la write() catre client.\n");
                          return -1;
                      }
                      else
                          printf ("[server]Mesajul a fost transmis cu succes.\n");}


          /* clientul primeste acces la fisierul specificat*/
          else {strcpy(msgrasp, "Acces permis ");
                strcat(msgrasp, msg+5);

                printf("[server]Trimitem mesajul inapoi...%s\n",msgrasp);

                /* returnam mesajul clientului */
                if (write (fd, msgrasp, 100) <= 0)
                {
                  perror ("[server]Eroare la write() catre client.\n");
                  return -1;
                }
                else printf ("[server]Mesajul a fost transmis cu succes.\n");




                strcpy(msgrasp, "./");
                strcat(msgrasp, msg + 5);                                        

                fd02 = open(msgrasp, O_RDONLY);         
                bzero (buffer, 10000);
                fflush (stdout);
                read(fd02, buffer, 10000);
                buffer[strlen(buffer)]='\0';

                if(strlen(buffer) == 0) {
                  strcpy(buffer, "--fisierGol--");
                  v[fd] = 0;
                }
                else v[fd] = strlen(buffer);

                printf("buffer[%s]buffer\n", buffer);
                close(fd02);

                
                if (write (fd, buffer, sizeof(buffer)) <= 0)
                {
                perror ("[server]Eroare la write() spre client.\n");

                return -1;
                }


                


                if( x == -1){

                  /* Create SQL statement */
                  strcpy(s2,"UPDATE fisiere SET fd1 = ");
                  bzero(msg2, 100);
                  sprintf(msg2, "%d", fd);
                  strcat(s2, msg2);
                  strcat(s2," where id = '");
                  strcat(s2, msg+5);
                  strcat(s2,"' ;");
                  sql=strdup(s2);


                  /* Execute SQL statement */
                  rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);

                  if( rc != SQLITE_OK ) {
                    printf("SQL error: %s\n", zErrMsg);
                    sqlite3_free(zErrMsg);
                  }

                }
                else{




                  /* Create SQL statement */
                  strcpy(s2,"UPDATE fisiere SET fd1 = ");
                  bzero(msg2, 100);
                  sprintf(msg2, "%d", fd);
                  strcat(s2, msg2);
                  strcat(s2," where fd2 = ");
                  bzero(msg2, 100);
                  sprintf(msg2, "%d", x);
                  strcat(s2, msg2);
                  strcat(s2," ;");
                  sql=strdup(s2);



                  /* Execute SQL statement */
                  rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);

                  if( rc != SQLITE_OK ) {
                    printf("SQL error: %s\n", zErrMsg);
                    sqlite3_free(zErrMsg);
                    return -1;
                  }
                  


                  /* Create SQL statement */
                  strcpy(s2,"UPDATE fisiere SET fd2 = ");
                  bzero(msg2, 100);
                  sprintf(msg2, "%d", fd);
                  strcat(s2, msg2);
                  strcat(s2," where fd1 = ");
                  bzero(msg2, 100);
                  sprintf(msg2, "%d", x);
                  strcat(s2, msg2);
                  strcat(s2," ;");
                  sql=strdup(s2);



                  /* Execute SQL statement */
                  rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);

                  if( rc != SQLITE_OK ) {
                    printf("SQL error: %s\n", zErrMsg);
                    sqlite3_free(zErrMsg);
                    return -1;
                  }



                    

                  }    
                      



               }  /*s-a acordat accesul la un fisier care avea 0 clienti sau 1 client*/
  }/*open fisier*/
  /*create fisier*/
  else if(strncmp(msg, "create ", 7) == 0){

         /* Create SQL statement */
         strcpy(s,msg + 7);
         strcpy(s2,"SELECT * from fisiere where id='");
         strcat(s2,s);
         strcat(s2,"';");
         sql=strdup(s2);


         x = -3;

         /* Execute SQL statement */
         rc = sqlite3_exec(db, sql, callback2, 0, &zErrMsg);
         
         if( rc != SQLITE_OK ) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
         }

         strcpy(msgrasp, "./");
         strcat(msgrasp, msg + 7);


         /*fisierul nu exista, deci poate fi creat*/
         if(x == -3){

            file = fopen(msgrasp, "w");
            fclose(file);

           /* Create SQL statement */
           strcpy(s,msg + 7);
           strcpy(s2,"INSERT INTO fisiere VALUES('");
           strcat(s2,s);
           strcat(s2,"', -1, -1);");
           sql=strdup(s2);


           /* Execute SQL statement */
           rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
           
           if( rc != SQLITE_OK ) {
              printf("SQL error: %s\n", zErrMsg);
              sqlite3_free(zErrMsg);
           }


            strcpy(msgrasp, "Fisierul ");
            strcat(msgrasp, msg+7);
            strcat(msgrasp, " a fost creat.");

            printf("[server]Trimitem mesajul inapoi...%s\n",msgrasp);

            /* returnam mesajul clientului */
            if (write (fd, msgrasp, 100) <= 0)
            {
              perror ("[server]Eroare la write() catre client.\n");
              return -1;
            }
            else printf ("[server]Mesajul a fost transmis cu succes.\n");
         }
         else{
            strcpy(msgrasp, "Fisierul ");
            strcat(msgrasp, msg+7);
            strcat(msgrasp, " exista deja. Pentru deschiderea fisierului folositi comanda: open");

            printf("[server]Trimitem mesajul inapoi...%s\n",msgrasp);

            /* returnam mesajul clientului */
            if (write (fd, msgrasp, 100) <= 0)
            {
              perror ("[server]Eroare la write() catre client.\n");
              return -1;
            }
            else printf ("[server]Mesajul a fost transmis cu succes.\n");
         }
       }
       /*download fisier*/
       else if(strncmp(msg, "download ", 9) == 0){

         /* Create SQL statement */
         strcpy(s,msg + 9);
         strcpy(s2,"SELECT * from fisiere where id='");
         strcat(s2,s);
         strcat(s2,"';");
         sql=strdup(s2);


         x = -3;

         /* Execute SQL statement */
         rc = sqlite3_exec(db, sql, callback2, 0, &zErrMsg);
         
         if( rc != SQLITE_OK ) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
         }




         /*fisierul exista, deci poate fi descarcat*/
         if(x != -3){

            strcpy(msgrasp, "Fisierul a fost descarcat.");

            printf("[server]Trimitem mesajul inapoi...%s\n",msgrasp);

            /* returnam mesajul clientului */
            if (write (fd, msgrasp, 100) <= 0)
            {
              perror ("[server]Eroare la write() catre client.\n");
              return -1;
            }
            else printf ("[server]Mesajul a fost transmis cu succes.\n");


            strcpy(msgrasp, "./");
            strcat(msgrasp, msg + 9);                                        

            fd02 = open(msgrasp, O_RDONLY);         
            bzero (buffer, 10000);
            fflush (stdout);
            read(fd02, buffer, 10000);
            buffer[strlen(buffer)]='\0';
            printf("%s", buffer);

            
            if (write (fd, buffer, sizeof(buffer)) <= 0)
            {
            perror ("[server]Eroare la write() spre client.\n");

            return -1;
            }
            close(fd02);


         }
         else{
            strcpy(msgrasp, "Fisierul nu exista. Pentru crearea fisierului folositi comanda: create");

            printf("[server]Trimitem mesajul inapoi...%s\n",msgrasp);

            /* returnam mesajul clientului */
            if (write (fd, msgrasp, 100) <= 0)
            {
              perror ("[server]Eroare la write() catre client.\n");
              return -1;
            }
            else printf ("[server]Mesajul a fost transmis cu succes.\n");
         }

            }/*download fisier*/
            /*comanda invalida pentru fisier*/
            else{
              strcpy(msgrasp, "Comanda invalida");

              printf("[server]Trimitem mesajul inapoi...%s\n",msgrasp);

              /* returnam mesajul clientului */
              if (write (fd, msgrasp, 100) <= 0)
              {
              perror ("[server]Eroare la write() catre client.\n");
              return -1;
              }
              else
              printf ("[server]Mesajul a fost transmis cu succes.\n");
            }


  sqlite3_close(db);
  return 0;  
}



int fisier(int fd, int v[100]){


  char msg[100], msg2[100];        //mesajul primit de la client
  char msgrasp[100]=" ";        //mesaj de raspuns pentru client
  char buffer[10000]; //continut fisier
 
  sqlite3 *db;
  char *zErrMsg = 0;
  int rc;
  char *sql;
  char s2[100], s[100];

  int fd02;
  FILE *file;
  

  int c;
  int j,n;



  /* Open database */
  rc = sqlite3_open("Server.db", &db);

  if( rc ) {
    printf("Can't open database: %s\n", sqlite3_errmsg(db));
    return -1;
  }



    bzero(msg, 100);
    fflush (stdout);

    if ( read(fd, msg, 100) <= 0) return -1;

    fflush(stdout);



    if ( read(fd, &c, sizeof(int)) <= 0) return -1;

    fflush(stdout);




    if( c == 27 ){

      /* Create SQL statement */
      strcpy(s2,"UPDATE fisiere SET fd1 = -1 where fd1 = ");
      bzero(msg2, 100);
      sprintf(msg2, "%d", fd);
      strcat(s2, msg2);
      strcat(s2," ;");
      sql=strdup(s2);


       /* Execute SQL statement */
       rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
       
       if( rc != SQLITE_OK ) {
          printf("SQL error: %s\n", zErrMsg);
          sqlite3_free(zErrMsg);
       }

      /* Create SQL statement */
      strcpy(s2,"UPDATE fisiere SET fd2 = -1 where fd2 = ");
      bzero(msg2, 100);
      sprintf(msg2, "%d", fd);
      strcat(s2, msg2);
      strcat(s2," ;");
      sql=strdup(s2);


       /* Execute SQL statement */
       rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
       
       if( rc != SQLITE_OK ) {
          printf("SQL error: %s\n", zErrMsg);
          sqlite3_free(zErrMsg);
       }

       return 0;
      

    }
    else{

      /* Create SQL statement */
      strcpy(s2,"SELECT fd2 FROM fisiere WHERE fd1 = ");
      bzero(msg2, 100);
      sprintf(msg2, "%d", fd);
      strcat(s2, msg2);
      strcat(s2," ;");
      sql=strdup(s2);


       x = -1;

       /* Execute SQL statement */
       rc = sqlite3_exec(db, sql, callback4, 0, &zErrMsg);
       
       if( rc != SQLITE_OK ) {
          printf("SQL error: %s\n", zErrMsg);
          sqlite3_free(zErrMsg);
       }

      /* Create SQL statement */
      strcpy(s2,"SELECT fd1 FROM fisiere WHERE fd2 = ");
      bzero(msg2, 100);
      sprintf(msg2, "%d", fd);
      strcat(s2, msg2);
      strcat(s2," ;");
      sql=strdup(s2);



       /* Execute SQL statement */
       rc = sqlite3_exec(db, sql, callback4, 0, &zErrMsg);
       
       if( rc != SQLITE_OK ) {
          printf("SQL error: %s\n", zErrMsg);
          sqlite3_free(zErrMsg);
       }


      strcpy(msgrasp, "./");
      strcat(msgrasp, msg);


      fd02 = open(msgrasp, O_RDONLY);         
      bzero (buffer, 10000);
      fflush (stdout);
      read(fd02, buffer, 10000);
      buffer[strlen(buffer)]='\0';
      close(fd02);

      n = strlen(buffer);

      initscr();
      noecho();
      cbreak();


      keypad(stdscr, true);

        if( c >= 32 && c <= 126 ){
          for(j = n-1; j >= v[fd]; j--) buffer[j+1] = buffer[j];
          buffer[v[fd]] = (char)c;
          n++;
          buffer[n] = '\0';
          if(x != -1) if(v[fd] <= v[x]) v[x]++;
          v[fd]++;

          fflush (stdout);
          file = fopen(msgrasp, "w");

          if (fprintf(file, buffer, sizeof(buffer)) <= 0)
          {
          return -1;
          }

          fclose(file);
        }
        else if( c == KEY_BACKSPACE && v[fd] > 0 ){
                for(j = v[fd]; j < n; j++) buffer[j-1] = buffer[j];
                n--;
                buffer[n] = '\0';
                if(x != -1) if(v[fd] <= v[x]) v[x]--;
                v[fd]--;

                fflush (stdout);
                file = fopen(msgrasp, "w");

                if(n != 0) if (fprintf(file, buffer, sizeof(buffer)) <= 0) return -1;

                fclose(file);
              }
              else if( c == KEY_LEFT && v[fd] > 0 ) v[fd]--; 
                   else if( c == KEY_RIGHT && v[fd] < n ) v[fd]++;





      if( c != KEY_UP){
        printf("%s\n", msg);

        printf("%c\n", (char)c);

        printf("buffer: %s\npozitia cursorului: %d\n", buffer, v[fd]);
      }


      endwin();


      fflush(stdout);

      if (write (fd, buffer, sizeof(buffer)) <= 0)
      {
      perror ("[server]Eroare la write() spre client.\n");

      return -1;
      }


      fflush(stdout);


      if (write (fd, &v[fd], sizeof(int)) <= 0)
      {
      perror ("[server]Eroare la write() spre client.\n");

      return -1;
      }



  }


  sqlite3_close(db);  
  return 0;  
}




int lcf(int fd, int v[100]){
  char msg[100];
  int y;

      bzero (msg, 100);
      fflush (stdout);

      /* citirea mesajului */
      if (read (fd, msg, 100) <= 0){
              perror ("[server]Eroare la read() de la client.\n");
              return -1;
      }


  y = -1;

  if(strcmp(msg, "login") == 0) y = login(fd);
    else if(strcmp(msg, "comanda") == 0) y = comanda(fd, v);
      else if(strcmp(msg, "fisier") == 0) y = fisier(fd, v);


  return y;
}


void delog(int fd){

  char msg2[100];
  char s2[100];

  sqlite3 *db;
  char *zErrMsg = 0;
  int rc;
  char *sql;

  /* Open database */
  rc = sqlite3_open("Server.db", &db);

  if( rc ) {
    printf("Can't open database: %s\n", sqlite3_errmsg(db));
  }

  /* Create SQL statement */
  strcpy(s2,"UPDATE login SET logat = 0, fd = -1 where fd = ");
  bzero(msg2, 100);
  sprintf(msg2, "%d", fd);
  strcat(s2, msg2);
  strcat(s2," ;");
  sql=strdup(s2);




  /* Execute SQL statement */
  rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);

  if( rc != SQLITE_OK ) {
    printf("SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }

  sqlite3_close(db);


}



/* functie de convertire a adresei IP a clientului in sir de caractere */
char * conv_addr (struct sockaddr_in address)
{
  static char str[25];
  char port[7];

  /* adresa IP a clientului */
  strcpy (str, inet_ntoa (address.sin_addr));	
  /* portul utilizat de client */
  bzero (port, 7);
  sprintf (port, ":%d", ntohs (address.sin_port));	
  strcat (str, port);
  return (str);
}

/* programul */
int main (){

  struct sockaddr_in server;  /* structurile pentru server si clienti */
  struct sockaddr_in from;
  fd_set readfds;   /* multimea descriptorilor de citire */
  fd_set actfds;    /* multimea descriptorilor activi */
  struct timeval tv;    /* structura de timp pentru select() */
  int sd, client;   /* descriptori de socket */
  int optval=1;       /* optiune folosita pentru setsockopt()*/ 
  int fd;     /* descriptor folosit pentru 
           parcurgerea listelor de descriptori */
  int nfds;     /* numarul maxim de descriptori */
  int len;      /* lungimea structurii sockaddr_in */
  int v[100];


  /* creare socket */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("[server] Eroare la socket().\n");
      return errno;
    }

  /*setam pentru socket optiunea SO_REUSEADDR */ 
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR,&optval,sizeof(optval));

  /* pregatim structurile de date */
  bzero (&server, sizeof (server));

  /* umplem structura folosita de server */
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = htonl (INADDR_ANY);
  server.sin_port = htons (PORT);

  /* atasam socketul */
  if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
      perror ("[server] Eroare la bind().\n");
      return errno;
    }

  /* punem serverul sa asculte daca vin clienti sa se conecteze */
  if (listen (sd, 5) == -1)
    {
      perror ("[server] Eroare la listen().\n");
      return errno;
    }
  
  /* completam multimea de descriptori de citire */
  FD_ZERO (&actfds);		/* initial, multimea este vida */
  FD_SET (sd, &actfds);		/* includem in multime socketul creat */

  tv.tv_sec = 1;		/* se va astepta un timp de 1 sec. */
  tv.tv_usec = 0;
  
  /* valoarea maxima a descriptorilor folositi */
  nfds = sd;

  printf ("[server] Asteptam la portul %d...\n", PORT);
  fflush (stdout);
        
  /* servim in mod concurent clientii... */
  while (1)
    {
      /* ajustam multimea descriptorilor activi (efectiv utilizati) */
      bcopy ((char *) &actfds, (char *) &readfds, sizeof (readfds));

      /* apelul select() */
      if (select (nfds+1, &readfds, NULL, NULL, &tv) < 0)
	{
	  perror ("[server] Eroare la select().\n");
	  return errno;
	}
      /* vedem daca e pregatit socketul pentru a-i accepta pe clienti */
      if (FD_ISSET (sd, &readfds))
	{
	  /* pregatirea structurii client */
	  len = sizeof (from);
	  bzero (&from, sizeof (from));

	  /* a venit un client, acceptam conexiunea */
	  client = accept (sd, (struct sockaddr *) &from, &len);

	  /* eroare la acceptarea conexiunii de la un client */
	  if (client < 0)
	    {
	      perror ("[server] Eroare la accept().\n");
	      continue;
	    }

    if (nfds < client) /* ajusteaza valoarea maximului */
            nfds = client;
            
	  /* includem in lista de descriptori activi si acest socket */
	  FD_SET (client, &actfds);

	  printf("[server] S-a conectat clientul cu descriptorul %d, de la adresa %s.\n",client, conv_addr (from));
	  fflush (stdout);
	}
      /* vedem daca e pregatit vreun socket client pentru a trimite raspunsul */
      for (fd = 0; fd <= nfds; fd++){	/* parcurgem multimea de descriptori */
        /* este un socket de citire pregatit? */
        if (fd != sd && FD_ISSET (fd, &readfds)){
            if ( lcf(fd,v) == -1 ){
              delog(fd);
              printf ("[server] S-a deconectat clientul cu descriptorul %d.\n",fd);
              fflush (stdout);
              close (fd);   /* inchidem conexiunea cu clientul */
              FD_CLR (fd, &actfds);/* scoatem si din multime */
            }
        }
	}			/* for */
    }				/* while */
}				/* main */
