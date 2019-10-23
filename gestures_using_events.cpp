#include <stdio.h>      //io
#include <pthread.h>    //threads
// #include <semaphore.h>  //semaphore
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
 
#define TOLERANCE 10000000
 
#define NUM_THREADS 2
#define INC_NUM 100
// przyciski
#define BUTTON_FILE "/dev/input/event3"
 
#define T_BUTTON 1
#define V_DOWN 1
#define V_UP 0
#define C_LEFT 272
#define C_RIGHT 273
#define C_MIDDLE 274
 
#define T_SCROLL 2
#define V_SCRL_UP 1
#define V_SCRL_DOWN -1
#define C_SCROLL 8
 
// pozycja
#define POSITION_FILE "/dev/input/event6"
 
#define T_MOVEMENT 3
#define HORIZONTAL 0
#define VERTICAL 1
// value jest pozycją kursora w danej osi
 
int driverOn = 1;
 
#define TAB_LEFT 0
#define TAB_RIGHT 1
#define TAB_MIDDLE 2
void *getButton(void *x_void_ptr) {
  int fdButtons;
  struct input_event ieButtons;
  int *x_ptr = (int *)x_void_ptr;
  if((fdButtons = open(BUTTON_FILE, O_RDONLY)) == -1) {
        perror("opening device");
        exit(EXIT_FAILURE);
    }
  // read button info
  while(driverOn) {
    read(fdButtons, &ieButtons, sizeof(struct input_event));
    switch (ieButtons.code) {
      case C_LEFT:
        *(x_ptr + TAB_LEFT) = ieButtons.value;
        break;
      case C_RIGHT:
        *(x_ptr + TAB_RIGHT) = ieButtons.value;
        break;
      case C_MIDDLE:
        *(x_ptr + TAB_MIDDLE) = ieButtons.value;
        break;
      }
      // printf("%i  %i  %i\n", *(x_ptr + TAB_LEFT), *(x_ptr + TAB_RIGHT), *(x_ptr + TAB_MIDDLE));
  }
//funkcja musi cos zwracac - moze byc NULL
return NULL;
}
 
#define TAB_X 0
#define TAB_Y 1
void *getPosition(void *x_void_ptr) {
  int fdPosition;
  struct input_event iePosition;
  int *x_ptr = (int *)x_void_ptr;
  if((fdPosition = open(POSITION_FILE, O_RDONLY)) == -1) {
        perror("opening device");
        exit(EXIT_FAILURE);
    }
  // read button info
  while(driverOn) {
    read(fdPosition, &iePosition, sizeof(struct input_event));
    if(iePosition.type == T_MOVEMENT) {
      switch (iePosition.code) {
        case HORIZONTAL:
          *(x_ptr + TAB_X) = iePosition.value;
          break;
        case VERTICAL:
          *(x_ptr + TAB_Y) = iePosition.value;
          break;
        }
    }
      // printf("x = %i\ty = %i\n", *(x_ptr + TAB_X), *(x_ptr + TAB_Y));
  }
  //funkcja musi cos zwracac - moze byc NULL
  return NULL;
}
 
int main(int argc, char *argv[])
{
  int buttons[3] = { 0 };
  int position[2] = { 0 };
  // deklaracja watkow
  pthread_t threads[NUM_THREADS];
 
  // tworzenie nowych watkow
  // printf("Creating threads\n");
  if(pthread_create(&threads[0], NULL, getButton, &buttons)) {
    fprintf(stderr, "Error creating thread 1\n");
    return -1;
  }
  if(pthread_create(&threads[1], NULL, getPosition, &position)) {
    fprintf(stderr, "Error creating thread 2\n");
    return -1;
  }
 
  // gesty
  int gest = 0;
  int beginPos[2];
 
  while(driverOn) {
    if( buttons[TAB_RIGHT] && !gest ) {
      // zaczynamy gest
      beginPos[TAB_X] = position[TAB_X];
      beginPos[TAB_Y] = position[TAB_Y];
      // printf("x = %i\ty = %i\n", beginPos[TAB_X], beginPos[TAB_Y]);
      gest = 1;
    } else if( buttons[TAB_RIGHT] && gest ) {
      // gest trwa
      // nic nie rób
    } else if(gest) {
      // gest się skończył
      gest = 0;
      // oblicz wektor przesunięcia
      int v[2];
      v[TAB_X] = position[TAB_X] - beginPos[TAB_X];
      v[TAB_Y] = beginPos[TAB_Y] - position[TAB_Y];
      // printf("x = %i\ty = %i\n", v[TAB_X], v[TAB_Y]);
      // oblicz kierunek przesunięcia i odpal odpowiedni skrypt
 
      // dzielimy ekran na 4 części prostymi y = x i y = -x
      // (0,0) w punkcie beginPoss
      // tolerancja dla  zwykłych kliknięć
      if(v[TAB_X] * v[TAB_X] + v[TAB_Y] * v[TAB_Y] > TOLERANCE) {
        if(abs(v[TAB_X]) > abs(v[TAB_Y])) {
          if(v[TAB_X] > 0) {
            // prawy trójkąt
            printf("Prawo\n");
            system("xdotool click 1");
            system("xdotool key ctrl+v");
          } else {
            // lewy trójkąt
            printf("Lewo\n");
            system("xdotool click 1");
            system("xdotool key ctrl+c");
          }
        } else {
          if(v[TAB_Y] > 0) {
            // górny trójkąt
            printf("Góra\n");
            system("xdotool key Print");
          } else {
            // dolny trójkąt
            printf("Dół\n");
            driverOn = 0;
          }
        }
      } else {
        int procent= (v[TAB_X]*v[TAB_X]+v[TAB_Y]*v[TAB_Y]) * 100 / TOLERANCE;
        printf("%i procent\n", procent);
      }
    }
  }
 
  // poczekaj, az watki skoncza prace
  int t;
  for(t=0; t<NUM_THREADS; t++){
    if(pthread_join(threads[t], NULL)){
      fprintf(stderr, "Error joining thread %i\n", t);
      return -1;
    }
  }
 
  return 0;
}