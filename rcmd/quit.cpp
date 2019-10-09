#include "aws_cmd.hpp"

#define AWS_CMD "quit"
#define AWS_CMD_USAGE "quit"

int main(int argc, char ** argv)
{
  return aws_cmd(argc, argv, AWS_CMD);
}
