#include "aws_cmd.hpp"


#define AWS_CMD "frm"
#define AWS_CMD_USAGE "frm <filter name>"

int main(int argc, char ** argv)
{
  return aws_cmd(argc, argv, AWS_CMD);
}
