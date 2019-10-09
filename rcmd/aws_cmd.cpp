#include "aws_cmd.hpp"


int aws_cmd(int argc, char ** argv, const char * cmd){
  ifstream cf(".aws");
  char buf[CMD_LEN];
  sockaddr_in addr;
  SOCKET sock;
  int res;
  
  res = 0;
  if(!cf.is_open()){
    cerr << "Configuration file \".aws\" does not found." << endl; 
    return 1;
  }

  // find rcmd section in the .aws file.
  vector<char *> tok;
  bool brcmd = false;
  while(!cf.eof()){
    cf.getline(buf, CMD_LEN);
    split_cmd_tok(buf, tok);
    if(strcmp(tok[0], "rcmd") == 0){
      addr.sin_port = htons((unsigned short)atoi(tok[2]));
      addr.sin_family = AF_INET;
      addr.sin_addr.s_addr = inet_addr(tok[1]);
      brcmd = true;
    }
  }
  
  if(brcmd == false){
    cerr << "rcmd section was not found in .aws file." << endl;
    return 1;
  }

  char * ptr = buf;
  strcpy(ptr, cmd);
  ptr += strlen(cmd);
  *ptr = ' '; ptr++;
  for(int iarg = 1; iarg < argc; iarg++){
    for(int i = 0; (*ptr = argv[iarg][i]) != '\0' ; i++, ptr++){
      if(*ptr == '\r' || *ptr == '\n'){
	break;
      }
    }
    *ptr = ' ';
    ptr++;
  }
  *ptr = '\0';

  sock = socket(AF_INET, SOCK_STREAM, 0);
  
  int ret = connect(sock, (sockaddr*)&addr, sizeof(addr));
  if(ret == -1){
    cerr << "fail to connect." << endl;
    return 1;
  }
   
  ret = send(sock, buf, CMD_LEN, 0);
  if(ret == -1){
    cerr << "fail to send data." << endl;
    return 1;
  }

  memset(buf, 0, CMD_LEN);
  recv(sock, buf, CMD_LEN, 0);
  if(buf[0]){ // returned message should include non-zero value at the first byte if succeeded. 
    if(buf[1])
      cout << buf+1 << endl;
  }else{
    cerr << "Error in \"";
    for(int iarg = 0; iarg < argc; iarg++){
      cerr << argv[iarg] << " ";
    }
    cerr << "\"" << endl;
    if(buf[1])
      cerr << "Message: " << buf+1 << endl;
    res = 1;
  }
  
  // finish the command session ("eoc" command is sent) 
  buf[0] = 'e'; buf[1] = 'o'; buf[2] = 'c'; buf[3] = '\0';
  ret = send(sock, buf, CMD_LEN, 0);
  if(ret == -1){
    cerr << "Failed to send end of command message." << endl;
    return 1;
  }
  close(sock);
  
  return res;
}

c_aws_cmd::c_aws_cmd():sock(-1)
{
  cf.open(".aws");
  if(!cf.is_open()){
    cerr << "Configuration file \".aws\" does not found." << endl; 
    exit(1);
  }
  
  // find rcmd section in the .aws file.
  vector<char *> tok;
  bool brcmd = false;
  while(!cf.eof()){
    cf.getline(buf, CMD_LEN);
    split_cmd_tok(buf, tok);
    if(strcmp(tok[0], "rcmd") == 0){
      addr.sin_port = htons((unsigned short)atoi(tok[2]));
      addr.sin_family = AF_INET;
      addr.sin_addr.s_addr = inet_addr(tok[1]);
      brcmd = true;
    }
  }
  
  if(brcmd == false){
    cerr << "rcmd section was not found in .aws file." << endl;
    exit(1);
  }

  sock = socket(AF_INET, SOCK_STREAM, 0);
  
  int ret = connect(sock, (sockaddr*)&addr, sizeof(addr));
  if(ret == -1){
    cerr << "fail to connect." << endl;
    exit(1);
  }
}

c_aws_cmd::~c_aws_cmd()
{
  close_session();
}
