#include "aws_cmd.hpp"


#define AWS_CMD "fpar"
#define AWS_CMD_USAGE "fpar <filter name> <parameter id>"

int main(int argc, char ** argv)
{
  return aws_cmd(argc, argv, AWS_CMD);
}
