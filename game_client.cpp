#include <iostream>
#include <string.h>
#include <cstdlib>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <unistd.h>

using namespace std;

const int MAX_NUM = 1000;

void msg(char *message, int socket);
void clear(char *arr);

int main(int argc, char const *argv[])
{
  if(argv[1] == NULL || argv[2] == NULL)
  {
    cout << "ERROR: Syntax - ./game_client <IP> <PORT>\n";
    return -1;
  }
  
  if(atoi(argv[2]) <= 1024 || atoi(argv[2]) > 65535)
  {
    cout << "ERROR: Port number must be between 1025 - 65535\n";
    return -1;
  }
  
  const char *ip = argv[1];
  int port = atoi(argv[2]);
  
  int sock = 0;
  struct sockaddr_in serv_addr;
  
  if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    cout << "Socket creation error\n";
    return -1;
  }
  
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);
  
  if(inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0)
  {
    cout << "Invalid address\n";
    return -1;
  }
  
  if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)))
  {
    cout << "Connection Failed\n";
    return -1;
  }
  
  bool done = false;
  char *input = new char[MAX_NUM];
  char servermsg[MAX_NUM] = {0};
  int turn = 0;
  int guesses = 0;
  string test;
  
  cout << "Welcome to Hangman!\n";
  
  cout << "Enter your name: ";
  cin >> input;
  while(strlen(input)> MAX_NUM)
  {
    cout << "Please enter a name with a max " << MAX_NUM << " characters: ";
    cin >> input;
  }
  msg(input, sock);
  
  clear(servermsg);
  read(sock, servermsg, MAX_NUM);
  
  while(!done)
  {
    turn++;
    guesses++;
    cout << "\nTurn " << turn << endl;
    msg("ready", sock);
    clear(servermsg);
    read(sock, servermsg, MAX_NUM);
    cout << "Word: " << servermsg << endl;
    
    cout << "Enter your guess: ";
    cin >> input;
    bool valid = false;
    while(!valid)
    {
      if((input[0] < 65 || input[0] > 90) && (input[0] < 97 || input[0] > 122))
      {
        valid = false;
        cout << "Please enter a letter: ";
        cin >> input;
      }
      else
      {
        valid = true;
        if(input[0] >= 97 && input[0] <= 122)
          input[0] = (input[0] - 32);
      }
    }
    
    msg(&input[0], sock);
    msg("ready", sock);
    clear(servermsg);
    read(sock, servermsg, MAX_NUM);
    cout << servermsg << "!" << endl;
    
    msg("ready", sock);
    clear(servermsg);
    read(sock, servermsg, MAX_NUM);
    
    if((string) servermsg == "won")
    {
      done = true;
      msg("ready", sock);
      clear(servermsg);
      read(sock, servermsg, MAX_NUM);
      cout << "\nCongrats! You guessed the word " << servermsg << "\nin " 
      << guesses << " guesses.";
    }
    else if((string) servermsg == "lost")
    {
      done = true;
      msg("ready", sock);
      clear(servermsg);
      read(sock, servermsg, MAX_NUM);
      cout << "\nGame Over. You cout not guess the word " << servermsg
      << " within " << MAX_NUM << " guesses.";
    }
    else
      done = false;
  }
  
  msg("ready", sock);
  clear(servermsg);
  read(sock, servermsg, MAX_NUM);
  
  cout << " Your Score: " << servermsg;
  
  cout << "\n\nLeaderboard:" << endl;
  
  //first place
  msg("ready", sock);
  clear(servermsg);
  read(sock, servermsg, MAX_NUM);
  cout << "1 -- " << servermsg << " ";
  msg("ready", sock);
  clear(servermsg);
  read(sock, servermsg, MAX_NUM);
  cout << servermsg << endl;
  
  
  //second place
  msg("ready", sock);
  clear(servermsg);
  read(sock, servermsg, MAX_NUM);
  cout << "2 -- " << servermsg << " ";
  msg("ready", sock);
  clear(servermsg);
  read(sock, servermsg, MAX_NUM);
  cout << servermsg << endl;
  
  
  //third place
  msg("ready", sock);
  clear(servermsg);
  read(sock, servermsg, MAX_NUM);
  cout << "3 -- " << servermsg << " ";
  msg("ready", sock);
  clear(servermsg);
  read(sock, servermsg, MAX_NUM);
  cout << servermsg << endl;
  
  return 0;
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