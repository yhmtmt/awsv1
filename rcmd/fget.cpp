#include "aws_cmd.hpp"

#define AWS_CMD "fget"
#define AWS_CMD_USAGE "fget"

int main(int argc, char ** argv)
{
  return aws_cmd(argc, argv, AWS_CMD);
}
