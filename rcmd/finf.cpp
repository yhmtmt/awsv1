#include "aws_cmd.hpp"


#define AWS_CMD "finf"
#define AWS_CMD_USAGE "finf [{<filter name> | n <filter id>}]"

int main(int argc, char ** argv)
{
  return aws_cmd(argc, argv, AWS_CMD);
}
