// album.c

#define _POSIX_SOURCE
#include <stdlib.h>
#include <stdio.h> 
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <signal.h>
#include "input_prompt.h"

#define INPUT_BUFFER_LEN 100

// functions

// displays image, returns pid of child process
int display(const char*image);

// produces thumbnail with thumbname in directory, returns pid of child process
int gen_thumb(const char *imagepath,const char *thumbname);

// produces medium image with medname, returns pid of child process
int gen_med(const char *imagepath, const char *medname);

// prompts user to give degrees (right) of rotatation, returns degrees of rotation
int rotate_prompt();

// rotate image with number of degrees, returns pid of child process
int rotate(const char *image, const int degrees);

// assuming user will pass valid args, names of a set of raw images
int main(int argc, char *argv[]) {

  // create index.html file
  FILE *fp;
  fp = fopen("index.html", "w");
  if (NULL == fp) {
    perror("error: index.html");
    exit(-1);
  }
  fprintf(fp, "<html><title>Digital Photo Album</title>\n");
  fprintf(fp, "<h1>album<h1>\n");
  fprintf(fp, "Please click on a thumbnail to view a medium-size image\n");

  // generate thumbnail, medimage, pathname, imagename, thumbname, medname of first photo
  int i =1;
  char *pathname1 = argv[i];
  char imagename1[strlen(pathname1)];
  char *last_slash1 = strrchr(pathname1, '/'); //pointer to last slash in pathname1
  if (last_slash1 == NULL){
    strcpy(imagename1, pathname1); // raw photo in current directory
  }
  else {
    strcpy(imagename1, ++last_slash1);
  }
  char thumbname1[strlen(imagename1)+5];
  strcpy(thumbname1,"thumb");
  strcat(thumbname1,imagename1);
  char medname1[strlen(imagename1)+3];
  strcpy(medname1,"med");
  strcat(medname1,imagename1);

  // generate first thumbnail
  //  int genthumb_ppid = (int) getpid();
  //  printf("gen thumb 1 parent %d\n", genthumb_ppid);
  int genthumb_cpid = gen_thumb(pathname1,thumbname1);
  int status1;
  waitpid(genthumb_cpid,&status1,0);

  //generate first medimage
  //  int genmed_ppid = (int) getpid();
  // printf("gen med 1 parent %d\n",genmed_ppid);
  int genmed_cpid = gen_med(pathname1,medname1);
  int status2;
  waitpid(genmed_cpid,&status2,0);

  // loop through arguments (raw photo paths)
  for (int i = 1; i < argc; i++) {
    //    printf("argument %d\n",i);

    // set current image handled is current argument (raw photo path) within loop
    char *pathname = argv[i];
    char imagename[strlen(pathname)];
    char *last_slash = strrchr(pathname, '/'); //pointer to last slash in pathname
    if (last_slash == NULL){
      strcpy(imagename, pathname); // raw photo in current directory
    }
     else {
       strcpy(imagename, ++last_slash);
     }
     char thumbname[strlen(imagename)+5];
     char medname[strlen(imagename)+3];
     strcpy(thumbname,"thumb");
     strcat(thumbname,imagename);
     strcpy(medname,"med");
     strcat(medname, imagename);
    
    // forking process to generate next thumb and med
     //    genthumb_ppid = (int) getpid();
    //    printf("gen parent %d\n", genthumb_ppid);
    int genthumb_pid = fork();    
    if (genthumb_pid < 0){
      perror("error: genthumb fork");
      exit(-1);
    }
    if (0 == genthumb_pid){ // child to next gen thumb
      genthumb_cpid = (int) getpid();
      //   printf("gen thumb child %d\n", genthumb_cpid);
      char *pathname = argv[i+1];
      char imagename[strlen(pathname)];
      char *last_slash = strrchr(pathname, '/'); //pointer to last slash in pathname                                                                 
      if (last_slash == NULL){
        strcpy(imagename, pathname); // raw photo in current directory                                                                               
      }
      else {
        strcpy(imagename, ++last_slash);
      }
      char thumbname[strlen(imagename)+5];
      strcpy(thumbname,"thumb");
      strcat(thumbname,imagename);
      char medname[strlen(imagename)+3];
      strcpy(medname,"med");
      strcat(medname,imagename);

      //fork gen next med
      int genmed_cpid = gen_med(pathname,medname);
      int status;
      waitpid(genmed_cpid,&status,0);

      // create thumbnail
      execlp("convert","convert","-geometry","10\%",pathname,thumbname,NULL);
      perror("error: thumbnail exec");
      exit(-1);
    }

    // display, prompt, and rotate
    if (genthumb_pid > 0){
      
      //fork display
      //      int display_ppid = (int) getpid();
      //      printf("display parent %d\n", display_ppid);
      int display_cpid = display(thumbname);

      // collect ui and kill display
      int deg = rotate_prompt();
      char *caption=calloc(INPUT_BUFFER_LEN, sizeof(char));
      input_string("caption:",caption,INPUT_BUFFER_LEN);
      kill(display_cpid,SIGTERM);

      // check if user wants to rotate must rotate both thumb and med
      if (deg > 0){
	// fork rotation thumb
	int rotate_t_ppid = (int) getpid();
	printf("rotate thumb parent %d\n", rotate_t_ppid);
	int rotate_t_cpid = rotate(thumbname, deg);
	int statusrt;
	waitpid(rotate_t_cpid,&statusrt,0);

	// fork rotation med
	// int rotate_m_ppid = (int) getpid();
	//	printf("rotate med parent %d\n", rotate_m_ppid);
	int rotate_m_cpid = rotate(medname, deg);
	int statusrm;
	waitpid(rotate_m_cpid,&statusrm,0);
      }

      //write into index.html
      fprintf(fp, "<h2>%s</h2>\n",caption);
      fprintf(fp, "<a href=\"%s\"><img src=\"%s\" border=\"1\"></a>\n",medname,thumbname);
    }
    
    // wait for next thumb child (med child is forked within thumb child)
    int status;
    waitpid(genthumb_pid, &status, 0);
  }

    //finish index.html
  fprintf(fp,"</html>");
  fclose(fp);
  waitpid(-1,NULL,0);
  return 0;
}


int rotate_prompt() {
  char* buffer = calloc(INPUT_BUFFER_LEN, sizeof(char));
  int degrees = 0;
  while (1) {
    input_string(
      "Rotate image (degrees right)? 0 (no rotation), 90 (clockwise),180 (upside down), 270 (counterclockwise)",
       buffer, INPUT_BUFFER_LEN);
    if (0 == strcasecmp(buffer, "0")) {
      break;
    } else if (0 == strcasecmp(buffer, "90")) {
      degrees = 90;
      break;
    } else if (0 == strcasecmp(buffer, "180")) {
      degrees = 180;
      break;
    } else if (0 == strcasecmp(buffer, "270")) {
      degrees = 270;
      break;
    } else {
      printf("%s\n", "Invalid input.");
    }
  }

  free(buffer);
  return degrees;
}

int rotate(const char *imagepath, const int degrees) {
  int rc = fork();
  if (0 == rc) {
    printf("rotate child %d\n", (int) getpid());
    char deg_str[3]; // takes string
    sprintf(deg_str, "%d", degrees);
    execlp("convert", "convert", "-rotate", deg_str, imagepath, imagepath, NULL);
    perror("error: rotate");
  }
  return rc;
}

int gen_thumb(const char *imagepath,const char *thumbname){
  int rc = fork();
  if (0==rc){ // child
    //    printf("gen thumb child %d\n", (int) getpid());
    execlp("convert","convert","-geometry","10\%",imagepath,thumbname,NULL);
    perror("error: gen thumb");
    exit(-1);
  }
  return rc;
}

int gen_med(const char *imagepath, const char *medname){
  int rc = fork();
  if (0 == rc){ // child
    //    printf("gen med child %d\n", (int) getpid());
    execlp("convert","convert","-geometry","25\%",imagepath,medname,NULL);
    perror("error: gen med");
    exit(-1);
  }
  return rc;
}

int display(const char *image){
  int rc = fork();
  if (0 == rc){
    //    printf("display child %d\n", (int) getpid());
    execlp("display","display",image,NULL);
    perror("error: display image");
    exit(-1);
  }
  return rc;
}
