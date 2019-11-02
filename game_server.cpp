#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <string>
#include <cstdlib>
#include <pthread.h>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <thread>
#include <chrono>

using namespace std;

const int MIN_PORT = 1024;
const int MAX_PORT = 65535;
const int MAX_THREADS = 10;
const char* WORD_FILE = "/home/fac/lillethd/cpsc3500/projects/p4/words.txt";
const int NUM_LINES = 57127 + 1;
const int MAX_NUM = 1000;

struct args
{
  int socket;
};

struct leaderboard_entry
{
  char name[MAX_NUM];
  float score;
};

pthread_mutex_t lock;
leaderboard_entry leaderboard[3];

void *Game (void *threadArg);
void msg(char *str, int socket);
void clear(char *arr);
void client_ready(int socket);
void update_leaderboard(float score, char *name);
void pause_thread();

int main(int argc, char const *argv[])
{  
  if(argv[1] == NULL)
  {
    cout << "ERROR: Syntax - ./game_server <PORT>\n";
    return -1;
  }
  
  if(atoi(argv[1]) <= MIN_PORT || atoi(argv[1]) > MAX_PORT)
  {
    cout << "ERROR: Port number must be between " << MIN_PORT + 1 << " - " << MAX_PORT << endl;
    return -1;
  }
  
  int port = atoi(argv[1]);
  
  struct sockaddr_in address;
  int opt = 1;
  int new_socket;
  int addrlen = sizeof(address);
  
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if(server_fd == 0)
  {
    cout << "Can't create a socket!\n";
    return -1;
  }
  
  if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
  {
    cout << "Can't attach socket to port!\n";
    return -1;
  }
  
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);
  
  if(bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
  {
    cout << "Failed to bind socket to port!\n";
    return -1;
  }
  
  if(listen(server_fd, 3) < 0)
  {
    cout << "Can't listen!\n";
    return -1;
  }
  
  int thread_id = 0;
  pthread_t tArray[MAX_THREADS];
  args tArg[MAX_THREADS];
  
  for(int i = 0; i < 3; i++)
  {
    leaderboard[i].score = 0.0;
    for(int k = 0; k < MAX_NUM; k++)
      leaderboard[i].name[k] = '\0';
  }
  
  
  while(true)
  {
    new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    tArg[thread_id].socket = new_socket;
    if(new_socket < 0) 
    { 
      cout << "Can't accept!\n";
      return -1;
    }

	pthread_t id;
    int error = -1;
    error = pthread_create(&id, NULL, Game, (void *) &tArg[thread_id]);
    if (error != 0)
    {
      cout << "FATAL ERROR creating thread " << thread_id << ": " << strerror(error) << endl;
      exit(1);
    }
	thread(pause_thread).detach();
  }
  
  return 0;
}

void *Game (void *threadArg)
{
  struct args *arguments = (struct args *) threadArg;
  int socket = arguments->socket;
  
  char clientmsg[MAX_NUM] = {0};
  char *name = new char[MAX_NUM];
  srand(time(NULL));
  int random = rand() % NUM_LINES;
  string in;
  char *word = new char[MAX_NUM];
  char *sent_word = new char[MAX_NUM];
  bool correct = false;
  bool won = false;
  bool done = false;
  
  ifstream file;
  file.open(WORD_FILE);
  
  for(int i = 0; i < random; i++)
    file >> in;
  
  file.close();
  
  bool guessed[in.length()];
  
  string tmp;
  
  for(int i = 0; i < in.length(); i++)
    tmp += '-';
  
  strcpy(word, in.c_str());
  strcpy(sent_word, tmp.c_str());
  
  cout << "Word: " << word << endl;
  
  for(int i = 0; i < in.length(); i++)
    guessed[i] = false;
  
  clear(clientmsg);
  if(read(socket, clientmsg, MAX_NUM) == 0)
  {
    cout << "\nClient disconnected.\n\n";
    delete name;
    delete word;
    delete sent_word;
    return NULL;
  }
  
  for(int i = 0; i < MAX_NUM; i++)
    name[i] = clientmsg[i];
  int guesses = 0;
  
  msg("ready", socket);
  
  while(!done)
  {
    guesses++;
    
    //continue if client ready
    client_ready(socket);
    
    msg(sent_word, socket);
    clear(clientmsg);
    if(read(socket, clientmsg, MAX_NUM) == 0)
    {
      cout << "\nClient disconnected.\n\n";
      delete name;
      delete word;
      delete sent_word;
      return NULL;
    }
    char guess = clientmsg[0];
    
    for(int i = 0; i < in.length(); i++)
    {
      if(guess == word[i])
      {
        guessed[i] = true;
        correct = true;
      }
    }
    
    client_ready(socket);
    
    if(correct)
    {
      msg((char *)"Correct", socket);
      for(int i = 0; i < in.length(); i++)
      {
        if(guessed[i] == true)
        {
          sent_word[i] = word[i];
        }
      }
    }
    else
      msg((char *)"Incorrect", socket);
    
    correct = false;
    
    for(int i = 0; i < in.length(); i++)
    {
      if(guessed[i] == true)
      {
        done = true;
        won = true;
      }
      else
      {
        done = false;
        won = false;
        break;
      }
    }
    
    if(guesses == MAX_NUM)
    {
      done = true;
      won = false;
    }
    
    client_ready(socket);
    
    if(won == true)
      msg((char *)"won", socket);
    else if(won == false && done == true)
      msg((char *)"lost", socket);
    else
      msg((char *)"!done", socket);
  }
  
  client_ready(socket);
  msg(word, socket);
  
  float score = (float)guesses / (float)in.length();
  
  pthread_mutex_lock(&lock);
  update_leaderboard(score, name);
  
  
  string str_score;
  char *c_score = new char[MAX_NUM];
  
  str_score = to_string(score);
  /*for(int i = 0; i < str_score.length(); i++)
    c_score[i] = str_score[i];*/
  strcpy(c_score, str_score.c_str());
  client_ready(socket);
  msg(c_score, socket);
  
  //first place
  client_ready(socket);
  if(leaderboard[0].name[0] == '\0')
    msg(" ", socket);
  else
  {
    str_score = to_string(leaderboard[0].score);
    strcpy(c_score, str_score.c_str());
    msg(c_score, socket);
  }
  
  client_ready(socket);
  if(leaderboard[0].name[0] == '\0')
    msg(" ", socket);
  else
    msg(leaderboard[0].name, socket);
  
  
  //second place
  client_ready(socket);
  if(leaderboard[1].name[0] == '\0')
    msg(" ", socket);
  else
  {
    str_score = to_string(leaderboard[1].score);
    strcpy(c_score, str_score.c_str());
    msg(c_score, socket);
  }
  
  client_ready(socket);
  if(leaderboard[1].name[0] == '\0')
    msg(" ", socket);
  else
    msg(leaderboard[1].name, socket);
  
  
  //third place
  client_ready(socket);
  if(leaderboard[2].name[0] == '\0')
    msg(" ", socket);
  else
  {
    str_score = to_string(leaderboard[2].score);
    strcpy(c_score, str_score.c_str());
    msg(c_score, socket);
  }
  
  client_ready(socket);
  if(leaderboard[2].name[0] == '\0')
    msg(" ", socket);
  else
    msg(leaderboard[2].name, socket);

  pthread_mutex_unlock(&lock);
  
  close(socket);
  
  delete name;
  delete word;
  delete sent_word;
  
  cout << "\nClient Disconnected\n\n";
}

void msg(char *message, int socket)
{
  send(socket, message, strlen(message), 0);
}

void clear(char *arr)
{
  for(int i = 0; i < MAX_NUM; i++)
    arr[i] = '\0';
}

void client_ready(int socket)
{
  char clientmsg[MAX_NUM] = {0};
  clear(clientmsg);
  if(read(socket, clientmsg, MAX_NUM) == 0)
  {
    cout << "\nClient disconnected.\n\n";
    return;
  }
}

void update_leaderboard(float score, char *name)
{
  for(int i = 0; i < 3; i++)
  {
    if(leaderboard[i].name[0] == '\0')
    {
      for(int k = 0; k < strlen(name); k++)
        leaderboard[i].name[k] = name[k];
      leaderboard[i].score = score;
      break;
    }
    else if(leaderboard[i].score > score)
    {
      if(i == 2)
      {
        for(int k = 0; k < MAX_NUM; k++)
          leaderboard[2].name[k] = '\0';
        for(int k = 0; k < strlen(name); k++)
          leaderboard[2].name[k] = name[k];
        leaderboard[2].score = score;
      }
      else if(i == 1) 
      {
        for(int k = 0; k < MAX_NUM; k++)
          leaderboard[2].name[k] = '\0';
        for(int k = 0; k < strlen(leaderboard[1].name); k++)
          leaderboard[2].name[k] = leaderboard[1].name[k];
        leaderboard[2].score = leaderboard[1].score;
        
        for(int k = 0; k < MAX_NUM; k++)
          leaderboard[1].name[k] = '\0';
        for(int k = 0; k < strlen(name); k++)
          leaderboard[1].name[k] = name[k];
        leaderboard[1].score = score;
      }
      else
      {
        for(int k = 0; k < MAX_NUM; k++)
          leaderboard[2].name[k] = '\0';
        for(int k = 0; k < strlen(leaderboard[1].name); k++)
          leaderboard[2].name[k] = leaderboard[1].name[k];
        leaderboard[2].score = leaderboard[1].score;
        
        for(int k = 0; k < MAX_NUM; k++)
          leaderboard[1].name[k] = '\0';
        for(int k = 0; k < strlen(leaderboard[0].name); k++)
          leaderboard[1].name[k] = leaderboard[i].name[k];
        leaderboard[1].score = leaderboard[i].score;
        
        for(int k = 0; k < MAX_NUM; k++)
          leaderboard[0].name[k] = '\0';
        for(int k = 0; k < strlen(name); k++)
          leaderboard[0].name[k] = name[k];
        leaderboard[0].score = score;
      }
      
      break;
    }
    else;
  }
}

void pause_thread()
{
	this_thread::sleep_for(chrono::seconds(1));
}