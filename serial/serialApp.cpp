/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "wx_pch.h"
#include "serialApp.h"

//(*AppHeaders
#include "serialMain.h"
#include <wx/image.h>
//*)

#ifndef WIN32
void gtk_init_hack(void) __attribute__((constructor));
void gtk_init_hack(void)  // This will always run before main()
{
  if(setregid(getegid(), -1) == -1)
  {
    fprintf(stderr, "setregid failed\n");
  }
}
#endif

IMPLEMENT_APP(serialApp);

bool serialApp::OnInit()
{
    //(*AppInitialize
    bool wxsOK = true;
    wxInitAllImageHandlers();
    if ( wxsOK )
    {
    	serialFrame* Frame = new serialFrame(0);
    	Frame->Show();
    	SetTopWindow(Frame);
    }
    //*)
    return wxsOK;

}

int serialApp::OnExit()
{
    return 0;
}
