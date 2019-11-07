#include "aws.hpp"

void c_aws::print_title()
{
  if(name_app)
    cout << name_app;
  else
    cout<< "nanashi";
  
  cout << " Ver." << ver_main << "." << ver_sub;
  cout << " (built " << __DATE__ << " " << __TIME__ << ")" << endl;
  cout << "Copyright (c) " << year_copy << " " << name_coder << " All Rights Reserved" << endl;
  if(contact)
    cout << contact << endl;
}
