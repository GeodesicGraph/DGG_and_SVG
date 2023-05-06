#include "main.h"
#include "Communicate.h"

IMPLEMENT_APP(MyApp)

bool MyApp::OnInit()
{

    Communicate *communicate = new Communicate(wxT("DGG Demo"));
    communicate->Show(true);

    return true;
}