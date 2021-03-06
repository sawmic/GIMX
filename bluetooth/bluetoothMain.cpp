/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#include "wx_pch.h"
#include "bluetoothMain.h"
#include <wx/msgdlg.h>

//(*InternalHeaders(bluetoothFrame)
#include <wx/string.h>
#include <wx/intl.h>
//*)

#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <pwd.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include <wx/aboutdlg.h>
#include <wx/dir.h>
#include "bluetooth.h"

#include "../directories.h"
#include "../shared/updater/updater.h"
#include "../shared/configupdater/configupdater.h"
#include <ConfigurationFile.h>

#include <wx/arrstr.h>

using namespace std;

#define OPT_DIR "/.sixemugui/"

char* homedir;

//(*IdInit(bluetoothFrame)
const long bluetoothFrame::ID_STATICTEXT1 = wxNewId();
const long bluetoothFrame::ID_CHOICE1 = wxNewId();
const long bluetoothFrame::ID_STATICTEXT2 = wxNewId();
const long bluetoothFrame::ID_CHOICE2 = wxNewId();
const long bluetoothFrame::ID_STATICTEXT3 = wxNewId();
const long bluetoothFrame::ID_CHOICE3 = wxNewId();
const long bluetoothFrame::ID_STATICTEXT4 = wxNewId();
const long bluetoothFrame::ID_CHOICE5 = wxNewId();
const long bluetoothFrame::ID_STATICTEXT6 = wxNewId();
const long bluetoothFrame::ID_CHOICE6 = wxNewId();
const long bluetoothFrame::ID_STATICTEXT7 = wxNewId();
const long bluetoothFrame::ID_CHOICE7 = wxNewId();
const long bluetoothFrame::ID_STATICTEXT8 = wxNewId();
const long bluetoothFrame::ID_BUTTON2 = wxNewId();
const long bluetoothFrame::ID_CHECKBOX1 = wxNewId();
const long bluetoothFrame::ID_CHECKBOX2 = wxNewId();
const long bluetoothFrame::ID_CHECKBOX3 = wxNewId();
const long bluetoothFrame::ID_CHECKBOX6 = wxNewId();
const long bluetoothFrame::ID_CHECKBOX7 = wxNewId();
const long bluetoothFrame::ID_CHOICE4 = wxNewId();
const long bluetoothFrame::ID_BUTTON4 = wxNewId();
const long bluetoothFrame::ID_BUTTON3 = wxNewId();
const long bluetoothFrame::ID_PANEL1 = wxNewId();
const long bluetoothFrame::ID_MENUITEM3 = wxNewId();
const long bluetoothFrame::ID_MENUITEM4 = wxNewId();
const long bluetoothFrame::ID_MENUITEM8 = wxNewId();
const long bluetoothFrame::ID_MENUITEM1 = wxNewId();
const long bluetoothFrame::ID_MENUITEM2 = wxNewId();
const long bluetoothFrame::ID_MENUITEM9 = wxNewId();
const long bluetoothFrame::idMenuQuit = wxNewId();
const long bluetoothFrame::ID_MENUITEM7 = wxNewId();
const long bluetoothFrame::ID_MENUITEM5 = wxNewId();
const long bluetoothFrame::ID_MENUITEM6 = wxNewId();
const long bluetoothFrame::idMenuAbout = wxNewId();
const long bluetoothFrame::ID_STATUSBAR1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(bluetoothFrame,wxFrame)
    //(*EventTable(bluetoothFrame)
    //*)
END_EVENT_TABLE()

static int readCommandResults(wxString command, int nb_params, wxString params[], int nb_repeat, wxString results[])
{
    int ret = 0;
    wxArrayString output, errors;
    unsigned int j = 0;
    int pos;

    if(!wxExecute(command, output, errors, wxEXEC_SYNC))
    {
      for(int i=0; i<nb_params*nb_repeat && ret != -1; ++i)
      {
        for(; j<output.GetCount(); ++j)
        {
          pos = output[j].Find(params[i%nb_params]);
          if(pos != wxNOT_FOUND)
          {
            results[i] = output[j].Mid(pos+params[i%nb_params].Length());
            break;
          }
        }
        if(j == output.GetCount() && nb_repeat == 1)
        {
          ret = -1;
        }
      }
    }
    else
    {
      ret = -1;
    }

    return ret;
}

void bluetoothFrame::readSixaxis()
{
    wxString params[2] = {wxT("Current Bluetooth master: "), wxT("Current Bluetooth Device Address: ")};
    wxString results[14];
    unsigned int j;
    wxString command = wxT("sixaddr");

    int res = readCommandResults(command, 2, params, 7, results);

    if(res != -1)
    {
        for(int i=0; i<13; i+=2)
        {
            if(results[i].IsEmpty())
            {
                break;
            }
            for(j=0; j<Choice1->GetCount(); ++j)
            {
              if(Choice1->GetString(j) == results[i+1].MakeUpper())
              {
                break;
              }
            }
            if(j == Choice1->GetCount())
            {
              Choice2->Append(results[i].MakeUpper());
              Choice1->Append(results[i+1].MakeUpper());
            }
        }

        if(Choice1->GetSelection() < 0)
        {
            Choice1->SetSelection(0);
        }
        if(Choice2->GetSelection() < 0)
        {
            Choice2->SetSelection(Choice1->GetSelection());
        }
    }
}

void bluetoothFrame::readDongles()
{
    wxString params1[3] = {wxT("BD Address: "), wxT("Name: "), wxT("Manufacturer: ")};
    wxString results1[3];
    wxString params2[1] = {wxT("Chip version: ")};
    wxString results2[1];
    int res;
    wxString previous = Choice3->GetStringSelection();

    Choice3->Clear();
    Choice5->Clear();
    Choice6->Clear();
    Choice7->Clear();

    for(int i=0; i<256; ++i)
    {
        wxString command = wxT("hciconfig -a hci") + wxString::Format(wxT("%i"), i);

        res = readCommandResults(command, 3, params1, 1, results1);

        if(res != -1)
        {
            Choice3->Append(results1[0].Left(17));

            Choice5->Append(results1[1]);

            Choice6->Append(results1[2]);

            command = wxT("hcirevision hci") + wxString::Format(wxT("%i"), i);
            res = readCommandResults(command, 1, params2, 1, results2);

            if(res != -1)
            {
                Choice7->Append(results2[0]);
            }
            else
            {
                Choice7->Append(wxT(""));
            }
        }
        else
        {
            break;
        }
    }

    int sel = Choice3->FindString(previous);
    if(sel < 0)
    {
      sel = 0;
    }
    Choice3->SetSelection(sel);
    Choice5->SetSelection(sel);
    Choice6->SetSelection(sel);
    Choice7->SetSelection(sel);
}

int bluetoothFrame::setDongleAddress()
{
    int j = 0;
    unsigned int k;
    wxString params1[1] = {wxT("Address changed - ")};
    wxString results1[1];
    wxString params2[1] = {wxT("Device address: ")};
    wxString results2[1];
    int res;

    if(Choice1->GetStringSelection().IsEmpty())
    {
        wxMessageBox( _("Please select a Sixaxis Address!"), _("Error"), wxICON_ERROR);
        return -1;
    }
    else if(Choice3->GetStringSelection().IsEmpty())
    {
        wxMessageBox( _("Please select a Bluetooth Dongle!"), _("Error"), wxICON_ERROR);
        return -1;
    }

    for(k=0; k<Choice3->GetCount(); ++k)
    {
        if(Choice3->GetString(k) == Choice1->GetStringSelection())
        {
            wxMessageBox( _("Address already used!"), _("Error"), wxICON_ERROR);
            return -1;
        }
    }

    wxString command = wxT("bdaddr -r -i hci") + wxString::Format(wxT("%i"), Choice3->GetSelection()) + wxT(" ") + Choice1->GetStringSelection();
    res = readCommandResults(command, 1, params1, 1, results1);

    if(res != -1)
    {
        wxMessageBox( results1[0], _("Success"), wxICON_INFORMATION);
    }

    //wait up to 5s for the device to come back
    command = wxT("bdaddr -i hci") + wxString::Format(wxT("%i"), Choice3->GetSelection());
    while(readCommandResults(command, 1, params2, 1, results2) == -1 && j<50)
    {
        usleep(100000);
        j++;
    }

    if(results2[0] !=  Choice1->GetStringSelection())
    {
        wxMessageBox( _("Read address after set: ko!"), _("Error"), wxICON_ERROR);
    }
    else
    {
        Choice3->SetString(Choice3->GetSelection(), Choice1->GetStringSelection());
        wxMessageBox( _("Read address after set: seems ok!"), _("Success"), wxICON_INFORMATION);
    }

    return 0;
}

static void read_filenames(wxChoice* choice)
{
  string filename = "";
  string line = "";
  wxString previous = choice->GetStringSelection();

  /* Read the last config used so as to auto-select it. */
#ifndef WIN32
  filename.append(homedir);
  filename.append(OPT_DIR);
#endif
  filename.append("default");
  ifstream infile (filename.c_str());
  if ( infile.is_open() )
  {
    if( infile.good() )
    {
      getline (infile,line);
    }
    infile.close();
  }

  choice->Clear();

  /* Read all config file names. */
  string ds;
#ifndef WIN32
  ds.append(homedir);
  ds.append(APP_DIR);
#endif
  ds.append(CONFIG_DIR);

  wxDir dir(wxString(ds.c_str(), wxConvUTF8));

  if(!dir.IsOpened())
  {
    cout << "Warning: can't open " << ds << endl;
    return;
  }

  wxString file;
  wxString filespec = wxT("*.xml");

  for (bool cont = dir.GetFirst(&file, filespec, wxDIR_FILES); cont;  cont = dir.GetNext(&file))
  {
    if(!line.empty() && wxString(line.c_str(), wxConvUTF8) == file)
    {
      previous = file;
    }
    choice->Append(file);
  }

  if(previous != wxEmptyString)
  {
    choice->SetSelection(choice->FindString(previous));
  }
  if(choice->GetSelection() < 0)
  {
    choice->SetSelection(0);
  }
}

static int read_sixaxis_config(wxChoice* cdevice, wxChoice* cmaster)
{
    string filename;
    string line;
    string device;
    string master;
    int ret = -1;

    filename.append(homedir);
    filename.append(OPT_DIR);
    filename.append("config");

    if(!::wxFileExists(wxString(filename.c_str(), wxConvUTF8)))
    {
      return 0;
    }

    ifstream myfile(filename.c_str());
    if(myfile.is_open())
    {
        while ( myfile.good() )
        {
            getline (myfile,line);
            if(!line.empty())
            {
                stringstream parser(line);
                parser >> device;
                parser >> master;
                cdevice->Append(wxString(device.c_str(), wxConvUTF8));
                cmaster->Append(wxString(master.c_str(), wxConvUTF8));
                ret = 0;
            }
        }
        myfile.close();
    }
    else
    {
        wxMessageBox( _("Cannot open file: ") + wxString(filename.c_str(), wxConvUTF8), _("Error"), wxICON_ERROR);
    }
    return ret;
}

static void readStartUpdates(wxMenuItem* menuItem)
{
  string filename = "";
  string line = "";

#ifndef WIN32
  filename.append(homedir);
  filename.append(OPT_DIR);
#endif
  filename.append("startUpdates");
  ifstream infile (filename.c_str());
  if ( infile.is_open() )
  {
      if( infile.good() )
      {
          getline (infile,line);
          if(line == "yes")
          {
              menuItem->Check(true);
          }
      }
      infile.close();
  }
}

void bluetoothFrame::refresh()
{
    readSixaxis();
    readDongles();
    read_filenames(ChoiceConfig);
    if(Choice1->GetCount() == 0)
    {
        wxMessageBox( _("No Sixaxis Detected!\nSixaxis usb wire plugged?"), _("Error"), wxICON_ERROR);
    }
    if(Choice3->GetCount() == 0)
    {
        wxMessageBox( _("No Bluetooth Dongle Detected!"), _("Error"), wxICON_ERROR);
    }
    Layout();
    Refresh();
}

bluetoothFrame::bluetoothFrame(wxWindow* parent,wxWindowID id)
{
    locale = new wxLocale(wxLANGUAGE_DEFAULT);
#ifdef WIN32
    locale->AddCatalogLookupPathPrefix(wxT("share/locale"));
#endif
    locale->AddCatalog(wxT("gimx"));

    setlocale( LC_NUMERIC, "C" ); /* Make sure we use '.' to write doubles. */

    //(*Initialize(bluetoothFrame)
    wxMenuItem* MenuItem2;
    wxStaticBoxSizer* StaticBoxSizer2;
    wxMenuItem* MenuItem1;
    wxFlexGridSizer* FlexGridSizer8;
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer2;
    wxMenu* Menu1;
    wxStaticBoxSizer* StaticBoxSizer5;
    wxFlexGridSizer* FlexGridSizer11;
    wxFlexGridSizer* FlexGridSizer7;
    wxFlexGridSizer* FlexGridSizer9;
    wxFlexGridSizer* FlexGridSizer14;
    wxFlexGridSizer* FlexGridSizer6;
    wxFlexGridSizer* FlexGridSizer3;
    wxStaticBoxSizer* StaticBoxSizer8;
    wxStaticBoxSizer* StaticBoxSizer4;
    wxStaticBoxSizer* StaticBoxSizer6;
    wxFlexGridSizer* FlexGridSizer10;
    wxFlexGridSizer* FlexGridSizer13;
    wxMenuBar* MenuBar1;
    wxMenuItem* MenuItem7;
    wxMenu* Menu2;
    wxFlexGridSizer* FlexGridSizer5;
    wxStaticBoxSizer* StaticBoxSizer1;

    Create(parent, wxID_ANY, _("Gimx-bluetooth"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, _T("wxID_ANY"));
    Panel1 = new wxPanel(this, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
    FlexGridSizer1 = new wxFlexGridSizer(2, 1, 0, 0);
    FlexGridSizer7 = new wxFlexGridSizer(1, 2, 0, 0);
    StaticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, Panel1, _("Sixaxis"));
    FlexGridSizer2 = new wxFlexGridSizer(2, 2, 0, 0);
    StaticText1 = new wxStaticText(Panel1, ID_STATICTEXT1, _("Address"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Choice1 = new wxChoice(Panel1, ID_CHOICE1, wxDefaultPosition, wxSize(175,-1), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
    FlexGridSizer2->Add(Choice1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText2 = new wxStaticText(Panel1, ID_STATICTEXT2, _("PS3 address"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Choice2 = new wxChoice(Panel1, ID_CHOICE2, wxDefaultPosition, wxSize(175,-1), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
    FlexGridSizer2->Add(Choice2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer7->Add(StaticBoxSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer2 = new wxStaticBoxSizer(wxHORIZONTAL, Panel1, _("Dongle"));
    FlexGridSizer3 = new wxFlexGridSizer(5, 2, 0, 0);
    StaticText3 = new wxStaticText(Panel1, ID_STATICTEXT3, _("Address"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer3->Add(StaticText3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Choice3 = new wxChoice(Panel1, ID_CHOICE3, wxDefaultPosition, wxSize(175,-1), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE3"));
    FlexGridSizer3->Add(Choice3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText4 = new wxStaticText(Panel1, ID_STATICTEXT4, _("Name"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    FlexGridSizer3->Add(StaticText4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Choice5 = new wxChoice(Panel1, ID_CHOICE5, wxDefaultPosition, wxSize(175,-1), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE5"));
    FlexGridSizer3->Add(Choice5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText6 = new wxStaticText(Panel1, ID_STATICTEXT6, _("Manufacturer"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
    FlexGridSizer3->Add(StaticText6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Choice6 = new wxChoice(Panel1, ID_CHOICE6, wxDefaultPosition, wxSize(175,-1), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE6"));
    FlexGridSizer3->Add(Choice6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText7 = new wxStaticText(Panel1, ID_STATICTEXT7, _("Chip version"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
    FlexGridSizer3->Add(StaticText7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Choice7 = new wxChoice(Panel1, ID_CHOICE7, wxDefaultPosition, wxSize(175,-1), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE7"));
    FlexGridSizer3->Add(Choice7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticText8 = new wxStaticText(Panel1, ID_STATICTEXT8, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
    FlexGridSizer3->Add(StaticText8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button2 = new wxButton(Panel1, ID_BUTTON2, _("Set Dongle Address"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    FlexGridSizer3->Add(Button2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer2->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer7->Add(StaticBoxSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer6 = new wxFlexGridSizer(1, 2, 0, 0);
    StaticBoxSizer4 = new wxStaticBoxSizer(wxHORIZONTAL, Panel1, wxEmptyString);
    FlexGridSizer5 = new wxFlexGridSizer(3, 1, 0, 0);
    FlexGridSizer8 = new wxFlexGridSizer(2, 2, 0, 0);
    StaticBoxSizer5 = new wxStaticBoxSizer(wxHORIZONTAL, Panel1, _("Mouse"));
    FlexGridSizer10 = new wxFlexGridSizer(1, 1, 0, 0);
    CheckBox1 = new wxCheckBox(Panel1, ID_CHECKBOX1, _("grab"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
    CheckBox1->SetValue(true);
    FlexGridSizer10->Add(CheckBox1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer5->Add(FlexGridSizer10, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer8->Add(StaticBoxSizer5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer6 = new wxStaticBoxSizer(wxHORIZONTAL, Panel1, _("Output"));
    FlexGridSizer11 = new wxFlexGridSizer(0, 3, 0, 0);
    CheckBox2 = new wxCheckBox(Panel1, ID_CHECKBOX2, _("gui"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
    CheckBox2->SetValue(false);
    FlexGridSizer11->Add(CheckBox2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox3 = new wxCheckBox(Panel1, ID_CHECKBOX3, _("terminal"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX3"));
    CheckBox3->SetValue(false);
    FlexGridSizer11->Add(CheckBox3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer6->Add(FlexGridSizer11, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer8->Add(StaticBoxSizer6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer5->Add(FlexGridSizer8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer13 = new wxFlexGridSizer(1, 2, 0, 0);
    CheckBox6 = new wxCheckBox(Panel1, ID_CHECKBOX6, _("Force updates"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX6"));
    CheckBox6->SetValue(true);
    FlexGridSizer13->Add(CheckBox6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox7 = new wxCheckBox(Panel1, ID_CHECKBOX7, _("Subpositions"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX7"));
    CheckBox7->SetValue(true);
    FlexGridSizer13->Add(CheckBox7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer5->Add(FlexGridSizer13, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer9 = new wxFlexGridSizer(1, 2, 0, 0);
    StaticBoxSizer8 = new wxStaticBoxSizer(wxHORIZONTAL, Panel1, _("Config"));
    ChoiceConfig = new wxChoice(Panel1, ID_CHOICE4, wxDefaultPosition, wxDefaultSize, 0, 0, wxCB_SORT, wxDefaultValidator, _T("ID_CHOICE4"));
    StaticBoxSizer8->Add(ChoiceConfig, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer9->Add(StaticBoxSizer8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer14 = new wxFlexGridSizer(2, 1, 0, 0);
    Button4 = new wxButton(Panel1, ID_BUTTON4, _("Check"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
    FlexGridSizer14->Add(Button4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button3 = new wxButton(Panel1, ID_BUTTON3, _("Start"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
    FlexGridSizer14->Add(Button3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer9->Add(FlexGridSizer14, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer5->Add(FlexGridSizer9, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StaticBoxSizer4->Add(FlexGridSizer5, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer6->Add(StaticBoxSizer4, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Panel1->SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(Panel1);
    FlexGridSizer1->SetSizeHints(Panel1);
    MenuBar1 = new wxMenuBar();
    Menu1 = new wxMenu();
    MenuItem5 = new wxMenuItem(Menu1, ID_MENUITEM3, _("Edit config"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItem5);
    MenuItem6 = new wxMenuItem(Menu1, ID_MENUITEM4, _("Edit fps config"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItem6);
    MenuAutoBindControls = new wxMenuItem(Menu1, ID_MENUITEM8, _("Auto-bind and convert"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuAutoBindControls);
    MenuItem3 = new wxMenuItem(Menu1, ID_MENUITEM1, _("Refresh\tF5"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItem3);
    MenuItem4 = new wxMenuItem(Menu1, ID_MENUITEM2, _("Save"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItem4);
    MenuItem7 = new wxMenuItem(Menu1, ID_MENUITEM9, _("Open config directory"), wxEmptyString, wxITEM_NORMAL);
    Menu1->Append(MenuItem7);
    MenuItem1 = new wxMenuItem(Menu1, idMenuQuit, _("Quit\tAlt-F4"), _("Quit the application"), wxITEM_NORMAL);
    Menu1->Append(MenuItem1);
    MenuBar1->Append(Menu1, _("&File"));
    Menu2 = new wxMenu();
    MenuGetConfigs = new wxMenuItem(Menu2, ID_MENUITEM7, _("Get configs"), wxEmptyString, wxITEM_NORMAL);
    Menu2->Append(MenuGetConfigs);
    MenuUpdate = new wxMenuItem(Menu2, ID_MENUITEM5, _("Update"), wxEmptyString, wxITEM_NORMAL);
    Menu2->Append(MenuUpdate);
    MenuStartUpdates = new wxMenuItem(Menu2, ID_MENUITEM6, _("Check updates at startup"), wxEmptyString, wxITEM_CHECK);
    Menu2->Append(MenuStartUpdates);
    MenuItem2 = new wxMenuItem(Menu2, idMenuAbout, _("About\tF1"), _("Show info about this application"), wxITEM_NORMAL);
    Menu2->Append(MenuItem2);
    MenuBar1->Append(Menu2, _("Help"));
    SetMenuBar(MenuBar1);
    StatusBar1 = new wxStatusBar(this, ID_STATUSBAR1, 0, _T("ID_STATUSBAR1"));
    int __wxStatusBarWidths_1[2] = { -1, 20 };
    int __wxStatusBarStyles_1[2] = { wxSB_NORMAL, wxSB_NORMAL };
    StatusBar1->SetFieldsCount(2,__wxStatusBarWidths_1);
    StatusBar1->SetStatusStyles(2,__wxStatusBarStyles_1);
    SetStatusBar(StatusBar1);
    SingleInstanceChecker1.Create(_T("gimx-bluetooth_") + wxGetUserId() + _T("_Guard"));

    Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&bluetoothFrame::OnSelectSixaxisBdaddr);
    Connect(ID_CHOICE2,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&bluetoothFrame::OnSelectPS3Bdaddr);
    Connect(ID_CHOICE3,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&bluetoothFrame::OnSelectBtDongle);
    Connect(ID_CHOICE5,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&bluetoothFrame::OnChoice5Select);
    Connect(ID_CHOICE6,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&bluetoothFrame::OnChoice6Select);
    Connect(ID_CHOICE7,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&bluetoothFrame::OnChoice7Select);
    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&bluetoothFrame::OnButton2Click);
    Connect(ID_CHECKBOX2,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&bluetoothFrame::OnCheckBox2Click);
    Connect(ID_CHECKBOX3,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&bluetoothFrame::OnCheckBox3Click);
    Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&bluetoothFrame::OnButton4Click);
    Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&bluetoothFrame::OnButton3Click);
    Connect(ID_MENUITEM3,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&bluetoothFrame::OnMenuEditConfig);
    Connect(ID_MENUITEM4,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&bluetoothFrame::OnMenuEditFpsConfig);
    Connect(ID_MENUITEM8,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&bluetoothFrame::OnMenuAutoBindControls);
    Connect(ID_MENUITEM1,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&bluetoothFrame::OnSelectRefresh);
    Connect(ID_MENUITEM2,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&bluetoothFrame::OnSave);
    Connect(ID_MENUITEM9,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&bluetoothFrame::OnMenuOpenConfigDirectory);
    Connect(idMenuQuit,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&bluetoothFrame::OnQuit);
    Connect(ID_MENUITEM7,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&bluetoothFrame::OnMenuGetConfigs);
    Connect(ID_MENUITEM5,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&bluetoothFrame::OnMenuUpdate);
    Connect(ID_MENUITEM6,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&bluetoothFrame::OnMenuStartUpdates);
    Connect(idMenuAbout,wxEVT_COMMAND_MENU_SELECTED,(wxObjectEventFunction)&bluetoothFrame::OnAbout);
    //*)

    if(SingleInstanceChecker1.IsAnotherRunning())
    {
        wxMessageBox( _("gimx-bluetooth is already running!"), _("Error"), wxICON_ERROR);
        exit(-1);
    }

    if(!getuid())
    {
    	int answer = wxMessageBox(_("It's not recommended to run as root user. Continue?"), _("Confirm"), wxYES_NO);
      if (answer == wxNO)
      {
        exit(0);
      }
    }

#ifndef WIN32
    homedir = getpwuid(getuid())->pw_dir;

	  /* Init user's config directory. */
    if(system("mkdir -p ~/.sixemugui"))
    {
        wxMessageBox( _("Can't init ~/.sixemugui directory!"), _("Error"), wxICON_ERROR);
    }
    if(system("mkdir -p ~/.emuclient/config"))
    {
        wxMessageBox( _("Can't init ~/.emuclient/config!"), _("Error"), wxICON_ERROR);
    }
    if(system("mkdir -p ~/.emuclient/macros"))
    {
        wxMessageBox( _("Can't init ~/.emuclient/macros!"), _("Error"), wxICON_ERROR);
    }
#endif

    read_sixaxis_config(Choice1, Choice2);

    started = false;

    wxCommandEvent event;
    readStartUpdates(MenuStartUpdates);
    if(MenuStartUpdates->IsChecked())
    {
      OnMenuUpdate(event);
    }

    started = true;

    refresh();

    if(ChoiceConfig->IsEmpty())
    {
      int answer = wxMessageBox(_("No config found! Download configs?"), _("Confirm"), wxYES_NO);
      if (answer == wxYES)
      {
        wxCommandEvent event;
        OnMenuGetConfigs(event);
      }
    }

    Panel1->Fit();
    Fit();
}

bluetoothFrame::~bluetoothFrame()
{
    //(*Destroy(bluetoothFrame)
    //*)
}

void bluetoothFrame::OnQuit(wxCommandEvent& event)
{
    Close();
}

void bluetoothFrame::OnAbout(wxCommandEvent& event)
{
  wxAboutDialogInfo info;
  info.SetName(wxTheApp->GetAppName());
  info.SetVersion(wxT(INFO_VERSION));
  wxString text = wxString(wxT(INFO_DESCR)) + wxString(wxT("\n")) + wxString(wxT(INFO_YEAR)) + wxString(wxT(" ")) + wxString(wxT(INFO_DEV)) + wxString(wxT(" ")) + wxString(wxT(INFO_LICENCE));
  info.SetDescription(text);
  info.SetWebSite(wxT(INFO_WEB));

  wxAboutBox(info);
}

void bluetoothFrame::OnSelectSixaxisBdaddr(wxCommandEvent& event)
{
    Choice2->SetSelection(Choice1->GetSelection());
}

void bluetoothFrame::OnSelectPS3Bdaddr(wxCommandEvent& event)
{
    Choice1->SetSelection(Choice2->GetSelection());
}

void bluetoothFrame::OnSelectBtDongle(wxCommandEvent& event)
{
    Choice5->SetSelection(Choice3->GetSelection());
    Choice6->SetSelection(Choice3->GetSelection());
    Choice7->SetSelection(Choice3->GetSelection());
}

void bluetoothFrame::OnSelectRefresh(wxCommandEvent& event)
{
    refresh();
}

void bluetoothFrame::OnButton2Click(wxCommandEvent& event)
{
    int answer = wxMessageBox(_("Did you saved your dongle address?"), _("Confirm"), wxYES_NO | wxCANCEL);

    if (answer == wxYES)
    {
        setDongleAddress();
    }
    else if (answer == wxNO)
    {
        wxMessageBox(_("Please save it!"), _("Info"));
    }
}

void bluetoothFrame::OnChoice5Select(wxCommandEvent& event)
{
    Choice3->SetSelection(Choice5->GetSelection());
    Choice6->SetSelection(Choice5->GetSelection());
    Choice7->SetSelection(Choice5->GetSelection());
}

void bluetoothFrame::OnChoice6Select(wxCommandEvent& event)
{
    Choice3->SetSelection(Choice6->GetSelection());
    Choice5->SetSelection(Choice6->GetSelection());
    Choice7->SetSelection(Choice6->GetSelection());
}

void bluetoothFrame::OnChoice7Select(wxCommandEvent& event)
{
    Choice3->SetSelection(Choice7->GetSelection());
    Choice5->SetSelection(Choice7->GetSelection());
    Choice6->SetSelection(Choice7->GetSelection());
}

class MyProcess : public wxProcess
{
public:
    MyProcess(bluetoothFrame *parent, const wxString& cmd)
        : wxProcess(parent), m_cmd(cmd)
    {
        m_parent = parent;
    }

    void OnTerminate(int pid, int status);

protected:
    bluetoothFrame *m_parent;
    wxString m_cmd;
};

void MyProcess::OnTerminate(int pid, int status)
{
    m_parent->OnProcessTerminated(this, status);
}

void bluetoothFrame::OnButton3Click(wxCommandEvent& event)
{
    wxString command;
    string filename = "";

    if(ChoiceConfig->GetStringSelection().IsEmpty())
    {
      wxMessageBox( _("No config selected!"), _("Error"), wxICON_ERROR);
      return;
    }

    if(Choice3->GetStringSelection().IsEmpty())
    {
        wxMessageBox( _("No Bluetooth Dongle Selected!"), _("Error"), wxICON_ERROR);
        return;
    }

    command.Append(wxT("xterm -e "));

    command.Append(wxT("gimx"));
    if(!CheckBox1->IsChecked())
    {
        command.Append(wxT(" --nograb"));
    }
    if(CheckBox6->IsChecked())
    {
        command.Append(wxT(" --force-updates"));
    }
    if(CheckBox7->IsChecked())
    {
        command.Append(wxT(" --subpos"));
    }
    command.Append(wxT(" --config \""));
    command.Append(ChoiceConfig->GetStringSelection());
    command.Append(wxT("\""));
    if(CheckBox2->IsChecked())
    {
        command.Append(wxT(" --curses"));
    }
    else if(CheckBox3->IsChecked())
    {
        command.Append(wxT(" --status"));
    }
    /*
     * TODO MLA: fix the hci index
     */
    command.Append(wxT(" --hci "));
    command.Append(wxString::Format(wxT("%i"),Choice3->GetSelection()));
    command.Append(wxT(" --bdaddr "));
    command.Append(Choice2->GetStringSelection());
    cout << command.mb_str(wxConvUTF8) << endl;

    filename.append(homedir);
    filename.append(OPT_DIR);
    filename.append("default");
    ofstream outfile (filename.c_str(), ios_base::trunc);
    if(outfile.is_open())
    {
        outfile << ChoiceConfig->GetStringSelection().mb_str(wxConvUTF8) << endl;
        outfile.close();
    }

    StatusBar1->SetStatusText(_("Press Shift+Esc to exit."));

    Button3->Enable(false);

    MyProcess *process = new MyProcess(this, command);

    if(!wxExecute(command, wxEXEC_ASYNC | wxEXEC_NOHIDE, process))
    {
      wxMessageBox( _("can't start emuclient!"), _("Error"), wxICON_ERROR);
    }
}

void bluetoothFrame::OnProcessTerminated(wxProcess *process, int status)
{
    Button3->Enable(true);
    StatusBar1->SetStatusText(wxEmptyString);

    if(status)
    {
      wxMessageBox( _("emuclient error"), _("Error"), wxICON_ERROR);
    }
}

void bluetoothFrame::OnSave(wxCommandEvent& event)
{
    string filename;
    string device;
    string master;
    unsigned int i;

    filename.append(homedir);
    filename.append(OPT_DIR);
    filename.append("config");

    ofstream outfile (filename.c_str(), ios_base::trunc);

    if(outfile.is_open())
    {
        for(i=0; i<Choice1->GetCount(); ++i)
        {
            device = string(Choice1->GetString(i).mb_str(wxConvUTF8));
            master = string(Choice2->GetString(i).mb_str(wxConvUTF8));
            outfile << device << " " << master << endl;
        }
        outfile.close();
    }
    else
    {
        wxMessageBox( _("Cannot open file: ") + wxString(filename.c_str(), wxConvUTF8), _("Error"), wxICON_ERROR);
    }
}

void bluetoothFrame::OnCheckBox2Click(wxCommandEvent& event)
{
    CheckBox3->SetValue(false);
}

void bluetoothFrame::OnCheckBox3Click(wxCommandEvent& event)
{
    CheckBox2->SetValue(false);
}

void bluetoothFrame::OnButton4Click(wxCommandEvent& event)
{
  if(ChoiceConfig->GetStringSelection().IsEmpty())
  {
    wxMessageBox( _("No config selected!"), _("Error"), wxICON_ERROR);
    return;
  }

  string file;
#ifndef WIN32
  file.append(homedir);
  file.append(APP_DIR);
#endif
  file.append(CONFIG_DIR);
  file.append(ChoiceConfig->GetStringSelection().mb_str(wxConvUTF8));

  ConfigurationFile configFile;
  event_catcher evcatch;
  configFile.SetEvCatch(&evcatch);
  int ret = configFile.ReadConfigFile(file);

  if(ret < 0)
  {
    wxMessageBox(wxString(configFile.GetError().c_str(), wxConvUTF8), _("Error"), wxICON_ERROR);
  }
  else if(ret > 0)
  {
    wxMessageBox(wxString(configFile.GetInfo().c_str(), wxConvUTF8), _("Info"), wxICON_INFORMATION);
  }
  else
  {
    wxMessageBox( _("This config seems OK!\n"), _("Info"), wxICON_INFORMATION);
  }
}

void bluetoothFrame::OnMenuEditConfig(wxCommandEvent& event)
{
  if(ChoiceConfig->GetStringSelection().IsEmpty())
  {
    wxMessageBox( _("No config selected!"), _("Error"), wxICON_ERROR);
    return;
  }

  wxString command = wxT("gimx-config -f \"");
  command.Append(ChoiceConfig->GetStringSelection());
  command.Append(wxT("\""));

  if (!wxExecute(command, wxEXEC_ASYNC))
  {
    wxMessageBox(_("Error editing the config file!"), _("Error"), wxICON_ERROR);
  }
}

void bluetoothFrame::OnMenuEditFpsConfig(wxCommandEvent& event)
{
  if(ChoiceConfig->GetStringSelection().IsEmpty())
  {
    wxMessageBox( _("No config selected!"), _("Error"), wxICON_ERROR);
    return;
  }

  wxString command = wxT("gimx-fpsconfig -f \"");
  command.Append(ChoiceConfig->GetStringSelection());
  command.Append(wxT("\""));

  if (!wxExecute(command, wxEXEC_ASYNC))
  {
    wxMessageBox(_("Error editing the config file!"), _("Error"), wxICON_ERROR);
  }
}

void bluetoothFrame::OnMenuUpdate(wxCommandEvent& event)
{
  int ret;

  updater u(VERSION_URL, VERSION_FILE, INFO_VERSION, DOWNLOAD_URL, DOWNLOAD_FILE);

  ret = u.checkversion();

  if (ret > 0)
  {
    int answer = wxMessageBox(_("Update available.\nStart installation?"), _("Confirm"), wxYES_NO);
    if (answer == wxNO)
    {
     return;
    }
    if (u.update() < 0)
    {
      wxMessageBox(_("Can't retrieve update file!"), _("Error"), wxICON_ERROR);
    }
    else
    {
      exit(0);
    }
  }
  else if (ret < 0)
  {
    wxMessageBox(_("Can't check version!"), _("Error"), wxICON_ERROR);
  }
  else if(started)
  {
    wxMessageBox(_("GIMX is up-to-date!"), _("Info"), wxICON_INFORMATION);
  }
}

void bluetoothFrame::OnMenuStartUpdates(wxCommandEvent& event)
{
  string filename;
#ifndef WIN32
  filename.append(homedir);
  filename.append(OPT_DIR);
#endif
  filename.append("startUpdates");
  ofstream outfile (filename.c_str(), ios_base::trunc);
  if(outfile.is_open())
  {
    if(MenuStartUpdates->IsChecked())
    {
      outfile << "yes" << endl;
    }
    else
    {
      outfile << "no" << endl;
    }
    outfile.close();
  }
}

void bluetoothFrame::OnMenuGetConfigs(wxCommandEvent& event)
{
  string dir;
#ifndef WIN32
  dir.append(homedir);
  dir.append(APP_DIR);
#endif
  dir.append(CONFIG_DIR);
  configupdater u(CONFIGS_URL, CONFIGS_FILE, dir);

  list<string>* cl = u.getconfiglist();
  list<string> cl_sel;

  if(cl && !cl->empty())
  {
    wxArrayString choices;

    for(list<string>::iterator it = cl->begin(); it != cl->end(); ++it)
    {
      choices.Add(wxString(it->c_str(), wxConvUTF8));
    }

    wxMultiChoiceDialog dialog(this, _("Select the files to download."), _("Config download"), choices);

    if (dialog.ShowModal() == wxID_OK)
    {
      wxArrayInt selections = dialog.GetSelections();
      wxArrayString configs;

      for ( size_t n = 0; n < selections.GetCount(); n++ )
      {
        string sel = string(choices[selections[n]].mb_str(wxConvUTF8));
        wxString wxfile = wxString(dir.c_str(), wxConvUTF8) + choices[selections[n]];
        if (::wxFileExists(wxfile))
        {
          int answer = wxMessageBox(_("Overwrite local file: ") + choices[selections[n]] + _("?"), _("Confirm"), wxYES_NO);
          if (answer == wxNO)
          {
            continue;
          }
        }
        cl_sel.push_back(sel);
        configs.Add(choices[selections[n]]);
      }

      if(u.getconfigs(&cl_sel) < 0)
      {
        wxMessageBox(_("Can't retrieve configs!"), _("Error"), wxICON_ERROR);
        return;
      }
      if(!cl_sel.empty())
      {
        wxMessageBox(_("Download is complete!"), _("Info"), wxICON_INFORMATION);
        if(!ChoiceConfig->IsEmpty())
        {
          int answer = wxMessageBox(_("Auto-bind and convert?"), _("Confirm"), wxYES_NO);
          if (answer == wxYES)
          {
            autoBindControls(configs);
          }
        }
        read_filenames(ChoiceConfig);
        ChoiceConfig->SetSelection(ChoiceConfig->FindString(wxString(cl_sel.front().c_str(), wxConvUTF8)));
      }
    }
  }
  else
  {
    wxMessageBox(_("Can't retrieve config list!"), _("Error"), wxICON_ERROR);
    return;
  }
}

void bluetoothFrame::autoBindControls(wxArrayString configs)
{
  string dir;
#ifndef WIN32
  dir.append(homedir);
  dir.append(APP_DIR);
#endif
  dir.append(CONFIG_DIR);

  wxString mod_config;

  wxArrayString ref_configs;
  for(unsigned int i=0; i<ChoiceConfig->GetCount(); i++)
  {
    ref_configs.Add(ChoiceConfig->GetString(i));
  }

  wxSingleChoiceDialog dialog(this, _("Select the reference config."), _("Auto-bind and convert"), ref_configs);

  if (dialog.ShowModal() == wxID_OK)
  {
    for(unsigned int j=0; j<configs.GetCount(); ++j)
    {
      ConfigurationFile configFile;
      mod_config = configs[j];

      int ret = configFile.ReadConfigFile(dir + string(mod_config.mb_str(wxConvUTF8)));

      if(ret < 0)
      {
        wxMessageBox(_("Can't read config: ") + mod_config + wxString(configFile.GetError().c_str(), wxConvUTF8), _("Error"), wxICON_ERROR);
        return;
      }

      if(configFile.AutoBind(dir + string(dialog.GetStringSelection().mb_str(wxConvUTF8))) < 0)
      {
        wxMessageBox(_("Can't auto-bind controls for config: ") + mod_config, _("Error"), wxICON_ERROR);
      }
      else
      {
        configFile.ConvertSensitivity(dir + string(dialog.GetStringSelection().mb_str(wxConvUTF8)));
        if(configFile.WriteConfigFile() < 0)
        {
          wxMessageBox(_("Can't write config: ") + mod_config, _("Error"), wxICON_ERROR);
        }
        else
        {
          wxMessageBox(_("Done!"), _("Info"), wxICON_INFORMATION);
        }
      }
    }
  }
}

void bluetoothFrame::OnMenuAutoBindControls(wxCommandEvent& event)
{
  if(ChoiceConfig->GetStringSelection().IsEmpty())
  {
    wxMessageBox( _("No config selected!"), _("Error"), wxICON_ERROR);
    return;
  }

  wxArrayString configs;
  configs.Add(ChoiceConfig->GetStringSelection());

  autoBindControls(configs);
}


void bluetoothFrame::OnMenuOpenConfigDirectory(wxCommandEvent& event)
{
  wxString userConfigDir(homedir, wxConvUTF8);
  userConfigDir.Append(wxT(APP_DIR));
  userConfigDir.Append(wxT(CONFIG_DIR));
  wxExecute(wxT("xdg-open ") + userConfigDir, wxEXEC_ASYNC, NULL);
}
