#include <iostream>
#include <sstream>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <sys/time.h> 
#include <arpa/inet.h>    //close
#include <errno.h>

#include <pthread.h>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#define SERVER_PORT  8082

#define TRUE             1
#define FALSE            0

#define JPEG_COMPRESSION 50 //100 = no-compression

// Re-Size the original image
#define IMG_WIDTH		 320
#define IMG_HEIGHT		 200
#define SHOW_LOCAL		 1		// Set to 1 is want to see local
#define CAMERA_SOURCE	 1 

using namespace cv;
using namespace std;

pthread_mutex_t lock;
vector<uchar> buff;


void* handleImage(void*);
void* handleSocketWrite(void*);

void *handleImage(void *kinect)
{
	vector<int> param = vector<int>(2);
	param[0]=IMWRITE_JPEG_QUALITY;
	param[1] = JPEG_COMPRESSION;
		
	VideoCapture capture(CAMERA_SOURCE);		

	Mat raw;
	Mat smaller = Mat(IMG_HEIGHT,IMG_WIDTH,CV_8UC3);
	
	capture >> raw;
	
	if( SHOW_LOCAL ){
		namedWindow("raw",1);
	}
	
	while(1){
		
		capture >> raw;
		
		resize(raw, smaller, smaller.size());
		
		pthread_mutex_lock(&lock); 
		imencode(".jpg",smaller,buff,param);
		
		if( SHOW_LOCAL ){
			imshow("raw",smaller);
		}
		
		pthread_mutex_unlock(&lock);

		if( waitKey(5) == 27 ){
			break;
		}
		//sleep(0.05);
	}
	
	
	
	return NULL;
}

void* handleSocketWrite(void* fd)
{	
	//int sock = (int)fd;
	int sock = *((int*)(&fd));	
	
	string header = "HTTP/1.0 200 OK\r\nConnection: close\r\nServer: MPJG-Streamer/0.2\r\nCache-Control: no-store, no-cache, must-revalidate, pre-check=0, post-check=0, max-age=0\r\nPragma: no-cache\r\nExpires: Mon, 3 Jan 2000 12:34:56 GMT\r\nContent-Type: multipart/x-mixed-replace;boundary=informs\r\n\r\n--informs\r\n";
	
	if( send(sock, header.c_str(), header.length(),0) < 0){
		cout << "Can't write header\n";
		return 0;
	}else{
		cout << "Wrote Header!\n";					
	}			   
				
	while(1)
	{
		
		pthread_mutex_lock(&lock); 
		
		ostringstream mm;
		mm <<  "Content-Type: image/jpeg\r\nContent-Length:" << buff.size() << "\r\n\r\n";
		string strMM = mm.str();

		if( send( sock, strMM.c_str(),strMM.length(),0) < 0 ) {
			cout << "Can't write Content Info, size: "<<strMM.length() <<endl;			
			break;  
		}	
				
		if( send( sock, reinterpret_cast<unsigned char*>(&buff[0]), buff.size(),0) < 0){
			cout << "Cant write data\n";
			pthread_mutex_unlock(&lock); 
			break;
		}
		pthread_mutex_unlock(&lock); 

		string bound = "\r\n--informs\r\n";
		if( send( sock, bound.c_str() , bound.length(),0) < 0){				
			cout << "Can't write boundary\n";			
			break;
		}
		
		//cout << "send image\n";
		sleep(0.2);
	}

	pthread_mutex_unlock(&lock);
	
	return 0;
}


int main(int argc, char *argv[])
{
	pthread_t sockThreadWrite;
	pthread_t imageThread;

   int    i, len, rc, on = 1;
   int    listen_sd, max_sd, new_sd;
   int    desc_ready, end_server = FALSE;
   int    close_conn;
   char   buffer[80];
   struct sockaddr_in   addr;
   struct timeval       timeout;
   struct timeval       timeoutQuick;
   fd_set        master_set, working_set;

   
   pthread_create(&imageThread, NULL, &handleImage, NULL);
   
   
   /*************************************************************/
   /* Create an AF_INET stream socket to receive incoming       */
   /* connections on                                            */
   /*************************************************************/
   listen_sd = socket(AF_INET, SOCK_STREAM, 0);
   if (listen_sd < 0)
   {
      perror("socket() failed");
      exit(-1);
   }

   /*************************************************************/
   /* Allow socket descriptor to be reuseable                   */
   /*************************************************************/
   rc = setsockopt(listen_sd, SOL_SOCKET,  SO_REUSEADDR,
                   (char *)&on, sizeof(on));
   if (rc < 0)
   {
      perror("setsockopt() failed");
      close(listen_sd);
      exit(-1);
   }

   /*************************************************************/
   /* Set socket to be non-blocking.  All of the sockets for    */
   /* the incoming connections will also be non-blocking since  */
   /* they will inherit that state from the listening socket.   */
   /*************************************************************/
   rc = ioctl(listen_sd, FIONBIO, (char *)&on);
   if (rc < 0)
   {
      perror("ioctl() failed");
      close(listen_sd);
      exit(-1);
   }

   /*************************************************************/
   /* Bind the socket                                           */
   /*************************************************************/
   memset(&addr, 0, sizeof(addr));
   addr.sin_family      = AF_INET;
   addr.sin_addr.s_addr = htonl(INADDR_ANY);
   addr.sin_port        = htons(SERVER_PORT);
   rc = bind(listen_sd,
             (struct sockaddr *)&addr, sizeof(addr));
   if (rc < 0)
   {
      perror("bind() failed");
      close(listen_sd);
      exit(-1);
   }

   /*************************************************************/
   /* Set the listen back log                                   */
   /*************************************************************/
   rc = listen(listen_sd, 32);
   if (rc < 0)
   {
      perror("listen() failed");
      close(listen_sd);
      exit(-1);
   }

   /*************************************************************/
   /* Initialize the master fd_set                              */
   /*************************************************************/
   FD_ZERO(&master_set);
   max_sd = listen_sd;
   FD_SET(listen_sd, &master_set);

   /*************************************************************/
   /* Initialize the timeval struct to 3 minutes.  If no        */
   /* activity after 3 minutes this program will end.           */
   /*************************************************************/
   timeout.tv_sec  = 3 * 60;   
   timeout.tv_usec = 0;

   timeoutQuick.tv_sec  = 1;   
   timeoutQuick.tv_usec = 0;
   
   /*************************************************************/
   /* Loop waiting for incoming connects or for incoming data   */
   /* on any of the connected sockets.                          */
   /*************************************************************/
   do
   {
      /**********************************************************/
      /* Copy the master fd_set over to the working fd_set.     */
      /**********************************************************/
      memcpy(&working_set, &master_set, sizeof(master_set));

      /**********************************************************/
      /* Call select() and wait 5 minutes for it to complete.   */
      /**********************************************************/
      printf("Waiting on select()...\n");
      rc = select(max_sd + 1, &working_set, NULL, NULL, &timeout);

      /**********************************************************/
      /* Check to see if the select call failed.                */
      /**********************************************************/
      if (rc < 0)
      {
         perror("  select() failed");
         break;
      }

      /**********************************************************/
      /* Check to see if the 5 minute time out expired.         */
      /**********************************************************/
      if (rc == 0)
      {
         printf("  select() timed out.  End program.\n");
         break;
      }

      /**********************************************************/
      /* One or more descriptors are readable.  Need to         */
      /* determine which ones they are.                         */
      /**********************************************************/
      desc_ready = rc;
      for (i=0; i <= max_sd  &&  desc_ready > 0; ++i)
      {
         /*******************************************************/
         /* Check to see if this descriptor is ready            */
         /*******************************************************/
         if (FD_ISSET(i, &working_set))
         {
            /****************************************************/
            /* A descriptor was found that was readable - one   */
            /* less has to be looked for.  This is being done   */
            /* so that we can stop looking at the working set   */
            /* once we have found all of the descriptors that   */
            /* were ready.                                      */
            /****************************************************/
            desc_ready -= 1;

            /****************************************************/
            /* Check to see if this is the listening socket     */
            /****************************************************/
            if (i == listen_sd)
            {
               printf("  Listening socket is readable\n");
               /*************************************************/
               /* Accept all incoming connections that are      */
               /* queued up on the listening socket before we   */
               /* loop back and call select again.              */
               /*************************************************/
               do
               {
                  /**********************************************/
                  /* Accept each incoming connection.  If       */
                  /* accept fails with EWOULDBLOCK, then we     */
                  /* have accepted all of them.  Any other      */
                  /* failure on accept will cause us to end the */
                  /* server.                                    */
                  /**********************************************/
                  new_sd = accept(listen_sd, NULL, NULL);
                  if (new_sd < 0)
                  {
                     if (errno != EWOULDBLOCK)
                     {
                        perror("  accept() failed");
                        end_server = TRUE;
                     }
                     break;
                  }

                  /**********************************************/
                  /* Add the new incoming connection to the     */
                  /* master read set                            */
                  /**********************************************/
                  printf("  New incoming connection - %d\n", new_sd);
                  FD_SET(new_sd, &master_set);
                  if (new_sd > max_sd)
                     max_sd = new_sd;

                  /**********************************************/
                  /* Loop back up and accept another incoming   */
                  /* connection                                 */
                  /**********************************************/
               } while (new_sd != -1);
            }

            /****************************************************/
            /* This is not the listening socket, therefore an   */
            /* existing connection must be readable             */
            /****************************************************/
            else
            {
               printf("  Descriptor %d is readable\n", i);
               close_conn = FALSE;
               /*************************************************/
               /* Receive all incoming data on this socket      */
               /* before we loop back and call select again.    */
               /*************************************************/
               do
               {
                  /**********************************************/
                  /* Receive data on this connection until the  */
                  /* recv fails with EWOULDBLOCK.  If any other */
                  /* failure occurs, we will close the          */
                  /* connection.                                */
                  /**********************************************/
				  
				  rc = select(max_sd + 1, &working_set, NULL, NULL, &timeoutQuick);
				  
				  if( rc ==  0){
					break;
				  }
				  
                  rc = recv(i, buffer, sizeof(buffer), 0);
                  if (rc < 0)
                  {
                     if (errno != EWOULDBLOCK)
                     {
                        perror("  recv() failed");
                        close_conn = TRUE;						
                     }
                     break;
                  }

                  /**********************************************/
                  /* Check to see if the connection has been    */
                  /* closed by the client                       */
                  /**********************************************/
                  if (rc == 0)
                  {
                     printf("  Connection closed\n");
                     close_conn = TRUE;
                     break;
                  }

                  /**********************************************/
                  /* Data was recevied                          */
                  /**********************************************/
                  len = rc;
                  printf("  %d bytes received\n", len);				

               } while (TRUE);

               /*************************************************/
               /* If the close_conn flag was turned on, we need */
               /* to clean up this active connection.  This     */
               /* clean up process includes removing the        */
               /* descriptor from the master set and            */
               /* determining the new maximum descriptor value  */
               /* based on the bits that are still turned on in */
               /* the master set.                               */
               /*************************************************/
             
			   if (close_conn)
               {
                  close(i);
                  FD_CLR(i, &master_set);
                  if (i == max_sd)
                  {
                     while (FD_ISSET(max_sd, &master_set) == FALSE)
                        max_sd -= 1;
                  }
               }else{					
					pthread_create(&sockThreadWrite, NULL, &handleSocketWrite, (void*)i);
			   }
            } /* End of existing connection is readable */
         } /* End of if (FD_ISSET(i, &working_set)) */
      } /* End of loop through selectable descriptors */

   } while (end_server == FALSE);

   /*************************************************************/
   /* Cleanup all of the sockets that are open                  */
   /*************************************************************/
   for (i=0; i <= max_sd; ++i)
   {
      if (FD_ISSET(i, &master_set))
         close(i);
   }
   
   return 0;
}
